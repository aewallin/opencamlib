// Internal parallel execution shared by native OpenMP and WebAssembly pthreads.
#pragma once

#include <algorithm>
#include <cstddef>

#ifdef OCL_WASM_THREAD_POOL_SIZE
#include <atomic>
#include <exception>
#include <system_error>
#include <thread>
#include <vector>
#elif defined(_OPENMP)
#include <omp.h>
#endif

namespace ocl { namespace detail {

inline unsigned int default_threads() {
#ifdef OCL_WASM_THREAD_POOL_SIZE
    return std::min<unsigned int>(OCL_WASM_THREAD_POOL_SIZE + 1,
                                 std::max(1u, std::thread::hardware_concurrency()));
#elif defined(_OPENMP)
    return omp_get_num_procs();
#else
    return 1;
#endif
}

inline unsigned int thread_count(unsigned int requested, std::size_t count) {
#ifdef OCL_WASM_THREAD_POOL_SIZE
    requested = std::min<unsigned int>(requested, OCL_WASM_THREAD_POOL_SIZE + 1);
#elif !defined(_OPENMP)
    requested = 1;
#endif
    return static_cast<unsigned int>(std::min<std::size_t>(std::max(1u, requested), count));
}

// Each invocation owns its output; only the call counts are reduced here.
template<class Function>
unsigned int parallel_sum(std::size_t count, unsigned int requested, const Function& function) {
    const unsigned int threads = thread_count(requested, count);
    unsigned int total = 0;
    if (threads <= 1) {
        for (std::size_t i = 0; i < count; ++i) total += function(i);
        return total;
    }
#ifdef OCL_WASM_THREAD_POOL_SIZE
    std::atomic<std::size_t> next{0};
    std::atomic<bool> cancelled{false};
    std::vector<unsigned int> sums(threads, 0);
    std::vector<std::exception_ptr> errors(threads);
    std::vector<std::thread> workers;
    workers.reserve(threads - 1);
    const std::size_t chunk = count >= threads * 32u ? 8 : 1;
    const auto work = [&](unsigned int slot) {
        try {
            while (!cancelled.load(std::memory_order_relaxed)) {
                const auto begin = next.fetch_add(chunk, std::memory_order_relaxed);
                if (begin >= count) break;
                const auto end = std::min(count, begin + chunk);
                for (auto i = begin; i < end; ++i) sums[slot] += function(i);
            }
        } catch (...) {
            errors[slot] = std::current_exception();
            cancelled.store(true, std::memory_order_relaxed);
        }
    };
    try {
        for (unsigned int slot = 1; slot < threads; ++slot) {
            try { workers.emplace_back(work, slot); }
            // A busy/exhausted browser worker pool must not cause a synchronous
            // wait for a new worker to load. The calling thread finishes the work.
            catch (const std::system_error&) { break; }
        }
    } catch (...) {
        // Allocation while constructing a later thread may also fail. Never
        // destroy already started, joinable threads during stack unwinding.
        cancelled.store(true, std::memory_order_relaxed);
        for (auto& worker : workers) worker.join();
        throw;
    }
    work(0);
    for (auto& worker : workers) worker.join();
    for (const auto& error : errors) if (error) std::rethrow_exception(error);
    for (auto sum : sums) total += sum;
#else
    const auto size = static_cast<std::ptrdiff_t>(count);
    #pragma omp parallel for schedule(dynamic) num_threads(threads) reduction(+:total)
    for (std::ptrdiff_t i = 0; i < size; ++i) total += function(i);
#endif
    return total;
}

}} // namespace ocl::detail
