#include "parallel.hpp"
#include "check.hpp"
#include <vector>

#ifdef OCL_WASM_THREAD_POOL_SIZE
#include <chrono>
#include <stdexcept>
#include <string>
#endif

int main() {
    for (unsigned int threads : {0u, 1u, 2u, 4u}) {
        for (std::size_t count : {0u, 1u, 2u, 31u, 10000u}) {
            std::vector<unsigned int> visits(count, 0);
            const auto sum = ocl::detail::parallel_sum(count, threads, [&](std::size_t i) {
                ++visits[i];
                return static_cast<unsigned int>(i);
            });
            CHECK(sum == count * (count - 1) / 2);
            for (auto visits_to_item : visits) CHECK(visits_to_item == 1);
        }
    }
#ifdef OCL_WASM_THREAD_POOL_SIZE
    CHECK(ocl::detail::thread_count(1000, 10000) == OCL_WASM_THREAD_POOL_SIZE + 1);
    // A rendezvous proves that distinct WASM workers actually run concurrently.
    // A bounded wait turns accidental serial execution into a test failure.
    const auto caller = std::this_thread::get_id();
    std::atomic<unsigned int> entered{0};
    std::atomic<bool> off_caller{false};
    ocl::detail::parallel_sum(2, 2, [&](std::size_t) {
        if (std::this_thread::get_id() != caller) off_caller.store(true);
        entered.fetch_add(1);
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (entered.load() < 2 && std::chrono::steady_clock::now() < deadline)
            std::this_thread::yield();
        CHECK(entered.load() == 2);
        return 0u;
    });
    CHECK(off_caller.load());
    // Propagate a worker exception to the caller, join every worker, then reuse
    // the pool successfully. Never let an exception escape a std::thread.
    bool caught = false;
    try {
        ocl::detail::parallel_sum(1000, 2, [&](std::size_t) -> unsigned int {
            if (std::this_thread::get_id() != caller) throw std::runtime_error("worker failure");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            return 0;
        });
    } catch (const std::runtime_error& error) {
        caught = std::string(error.what()) == "worker failure";
    }
    CHECK(caught);
    CHECK(ocl::detail::parallel_sum(100, 4, [](std::size_t) { return 1u; }) == 100);
#endif
}
