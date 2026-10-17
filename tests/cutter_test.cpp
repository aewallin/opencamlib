#include "adaptivepathdropcutter.hpp"
#include "adaptivewaterline.hpp"
#include "batchdropcutter.hpp"
#include "batchpushcutter.hpp"
#include "ballcutter.hpp"
#include "bullcutter.hpp"
#include "conecutter.hpp"
#include "compositecutter.hpp"
#include "cylcutter.hpp"
#include "stlsurf.hpp"
#include "parallel.hpp"
#include "check.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#if defined(OCL_WASM_THREAD_POOL_SIZE) || defined(_OPENMP)
#include <atomic>
#include <chrono>
#include <thread>
#endif
#ifdef _OPENMP
#include <omp.h>
#endif

static void equal(const ocl::Point& a, const ocl::Point& b) {
    CHECK(a.x == b.x && a.y == b.y && a.z == b.z);
}
static void equal(const ocl::CCPoint& a, const ocl::CCPoint& b) {
    equal(static_cast<const ocl::Point&>(a), static_cast<const ocl::Point&>(b));
    CHECK(a.type == b.type);
}
static void equal(const std::vector<ocl::CLPoint>& a, const std::vector<ocl::CLPoint>& b) {
    CHECK(a.size() == b.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        equal(static_cast<const ocl::Point&>(a[i]), static_cast<const ocl::Point&>(b[i]));
        equal(*a[i].cc, *b[i].cc);
    }
}
static void equal(const std::vector<ocl::Fiber>& a, const std::vector<ocl::Fiber>& b) {
    CHECK(a.size() == b.size());
    for (std::size_t n = 0; n < a.size(); ++n) {
        equal(a[n].p1,b[n].p1); equal(a[n].p2,b[n].p2);
        CHECK(a[n].ints.size() == b[n].ints.size());
        for (std::size_t i = 0; i < a[n].ints.size(); ++i) {
            const auto& x = a[n].ints[i]; const auto& y = b[n].ints[i];
            CHECK(x.lower == y.lower && x.upper == y.upper);
            equal(x.lower_cc,y.lower_cc); equal(x.upper_cc,y.upper_cc);
        }
    }
}
static ocl::Point vertex(int x, int y) {
    return ocl::Point(x*.3, y*.3, 1 + .7*std::cos(x*.3)*std::cos(y*.3));
}
static void model(ocl::STLSurf& surface) {
    for (int x = -10; x < 10; ++x) {
        for (int y = -10; y < 10; ++y) {
            ocl::Triangle a(vertex(x,y),vertex(x+1,y),vertex(x+1,y+1));
            ocl::Triangle b(vertex(x,y),vertex(x+1,y+1),vertex(x,y+1));
            surface.addTriangle(a); surface.addTriangle(b);
        }
    }
}

static void batch_drop(ocl::STLSurf& surface, ocl::MillingCutter& cutter) {
    std::vector<ocl::CLPoint> expected;
    int calls = 0;
    for (int threads : {1,4}) {
        ocl::BatchDropCutter operation;
        operation.setSTL(surface); operation.setCutter(&cutter); operation.setThreads(threads);
        for (int x = -12; x <= 12; ++x)
            for (int y = -10; y <= 10; ++y) {
                ocl::CLPoint point(x*.31,y*.33,-1); operation.appendPoint(point);
            }
        operation.run();
        if (threads == 1) { expected = operation.getCLPoints(); calls = operation.getCalls(); }
        else { equal(expected,operation.getCLPoints()); CHECK(calls == operation.getCalls()); }
    }
    CHECK(calls > 0);
}

static void batch_push(ocl::STLSurf& surface, ocl::MillingCutter& cutter) {
    for (int axis : {0,1}) {
        std::vector<ocl::CLPoint> expected;
        std::vector<ocl::Fiber> fibers;
        int calls = 0;
        for (int threads : {1,4}) {
            ocl::BatchPushCutter operation;
            if (axis == 0) operation.setXDirection(); else operation.setYDirection();
            operation.setSTL(surface); operation.setCutter(&cutter); operation.setThreads(threads);
            for (int i = -15; i <= 15; ++i) {
                ocl::Fiber fiber(axis == 0 ? ocl::Point(-4,i*.25,1) : ocl::Point(i*.25,-4,1),
                                axis == 0 ? ocl::Point(4,i*.25,1) : ocl::Point(i*.25,4,1));
                operation.appendFiber(fiber);
            }
            operation.run();
            if (threads == 1) {
                fibers = *operation.getFibers(); calls = operation.getCalls();
                expected = operation.getCLPoints();
            } else {
                equal(fibers,*operation.getFibers()); CHECK(calls == operation.getCalls());
                equal(expected,operation.getCLPoints());
            }
        }
        CHECK(calls > 0);
    }
}

static void adaptive_path(ocl::STLSurf& surface, ocl::MillingCutter& cutter) {
    // Serial builds still check repeatability and a requested parallel count,
    // but need not repeat the same serial calculation for every thread count.
    const std::vector<int> thread_counts = ocl::detail::thread_count(4, 4) > 1
        ? std::vector<int>{4,2,1} : std::vector<int>{4};
    for (int spans : {0,1,8}) {
        ocl::Path path;
        for (int i = 0; i < spans; ++i) {
            if (i % 3 == 1)
                path.append(ocl::Arc(ocl::Point(-2,0,-1), ocl::Point(2,0,-1), ocl::Point(0,0,-1), true));
            else
                path.append(ocl::Line(ocl::Point(-4,-2+i*.5,-1),ocl::Point(4,-2+i*.5,-1)));
        }
        ocl::AdaptivePathDropCutter operation;
        operation.setSTL(surface); operation.setCutter(&cutter); operation.setPath(&path);
        operation.setSampling(.3);
        operation.setThreads(1); operation.run();
        const auto expected = operation.getPoints();
        const int calls = operation.getCalls();
        CHECK(expected.empty() == (spans == 0));
        for (int threads : thread_counts) {
            operation.setThreads(threads); operation.run();
            equal(expected,operation.getPoints());
            CHECK(calls == operation.getCalls());
        }
    }
}

static void adaptive_waterline(ocl::STLSurf& surface, ocl::MillingCutter& cutter) {
    ocl::AdaptiveWaterline operation;
    operation.setSTL(surface); operation.setCutter(&cutter);
    const std::vector<int> thread_counts = ocl::detail::thread_count(4, 4) > 1
        ? std::vector<int>{4,2} : std::vector<int>{4};
    struct Settings {double z, sampling, minimum;};
    // Dense refinement, minimum-spacing cutoffs, then a coarse empty contour.
    // Reuse the operation across heights and thread counts to catch stale data.
    for (const auto& settings : {Settings{1,.3,.03}, Settings{1.3,.7,.7}, Settings{3,20,.1}}) {
        operation.setSampling(settings.sampling); operation.setMinSampling(settings.minimum);
        operation.setZ(settings.z);
        for (bool smart_weave : {false,true}) {
            const auto run = [&] {if (smart_weave) operation.run2(); else operation.run();};
            operation.setThreads(1); run(); // Original depth-first reference.
            const auto x = operation.getXFibers(), y = operation.getYFibers();
            const auto loops = operation.getLoops();
            const int calls = operation.getCalls();
            if (settings.z == 3) CHECK(loops.empty());
            else CHECK(calls > 0);
            for (int threads : thread_counts) {
                operation.setThreads(threads); run();
                equal(x,operation.getXFibers()); equal(y,operation.getYFibers());
                CHECK(calls == operation.getCalls());
                const auto parallel = operation.getLoops();
                CHECK(loops.size() == parallel.size());
                for (std::size_t i = 0; i < loops.size(); ++i) {
                    CHECK(loops[i].size() == parallel[i].size());
                    for (std::size_t j = 0; j < loops[i].size(); ++j) equal(loops[i][j],parallel[i][j]);
                }
            }
        }
    }
    ocl::STLSurf empty;
    operation.setSTL(empty); operation.setThreads(4); operation.run();
    CHECK(operation.getCalls() == 0);
    CHECK(operation.getXFibers().empty() && operation.getYFibers().empty() && operation.getLoops().empty());
}

#if defined(OCL_WASM_THREAD_POOL_SIZE) || defined(_OPENMP)
// Rendezvous inside real contact calculations: the four quarter-position
// fibers must run concurrently, not just the original pair of axis tasks.
struct ConcurrentBall : ocl::BallCutter {
    const unsigned int expected = ocl::detail::thread_count(4, 4);
    mutable std::atomic<unsigned int> entered{0};
    ConcurrentBall() : ocl::BallCutter(.6, 5) {}
    bool vertexPush(const ocl::Fiber& fiber, ocl::Interval& interval, const ocl::Triangle& triangle) const override {
        const double coordinate = fiber.p1.y == fiber.p2.y ? fiber.p1.y : fiber.p1.x;
        if (std::abs(std::abs(coordinate) - 2.3) < 1e-10) {
            if (entered.fetch_add(1) < expected) {
                const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
                while (entered.load() < expected && std::chrono::steady_clock::now() < deadline)
                    std::this_thread::yield();
                CHECK(entered.load() >= expected);
            }
        }
        return ocl::BallCutter::vertexPush(fiber, interval, triangle);
    }
};

static void concurrent_waterline() {
    ocl::STLSurf surface;
    ocl::Triangle triangle(ocl::Point(-4,-4,1), ocl::Point(4,-4,1), ocl::Point(0,4,1));
    surface.addTriangle(triangle);
    ConcurrentBall cutter;
    ocl::AdaptiveWaterline operation;
    operation.setCutter(&cutter); operation.setSTL(surface);
    operation.setSampling(.3); operation.setZ(1); operation.setThreads(4);
    operation.run();
    CHECK(cutter.entered.load() >= cutter.expected);
}
#endif
// Components are borrowed by CompositeCutter; keep their ownership in the test.
struct Composite : ocl::CompositeCutter {
    explicit Composite(ocl::MillingCutter& component) {
        length = component.getLength();
        addCutter(component,component.getRadius(),length,0);
    }
};

struct TrackedContact : ocl::CCPoint {
    int& destroyed;
    explicit TrackedContact(int& count) : destroyed(count) {}
    ~TrackedContact() override { ++destroyed; }
};

static void composite_contacts() {
    ocl::CylCutter cylinder(2,5);
    Composite composite(cylinder);
    const ocl::Triangle triangle(ocl::Point(-2,-2,1),ocl::Point(2,-2,1),ocl::Point(0,2,1));
    for (bool facet : {true,false}) {
        int destroyed = 0;
        ocl::CLPoint point(0,0,-1);
        delete point.cc.exchange(new TrackedContact(destroyed));
        CHECK(facet ? composite.facetDrop(point,triangle) : composite.edgeDrop(point,triangle));
        CHECK(destroyed == 1);
        CHECK(point.z == 1);
        CHECK(point.cc.load()->type == (facet ? ocl::FACET : ocl::EDGE));
    }
    // Existing fiber intervals are output accumulated from other triangles;
    // they must not affect the next triangle's contact calculation.
    ocl::Fiber empty(ocl::Point(-5,0,0),ocl::Point(5,0,0));
    ocl::Fiber populated = empty;
    for (int n = 0; n < 10; ++n) {
        ocl::Interval interval(n*.05,n*.05+.02);
        populated.addInterval(interval);
    }
    ocl::Interval a, b;
    CHECK(composite.pushCutter(empty,a,triangle));
    CHECK(composite.pushCutter(populated,b,triangle));
    CHECK(a.lower == b.lower && a.upper == b.upper);
    equal(a.lower_cc,b.lower_cc); equal(a.upper_cc,b.upper_cc);
    CHECK(populated.ints.size() == 10);
}

int main(int argc, char** argv) {
#ifdef _OPENMP
    const int original_threads = omp_get_max_threads();
#endif
    const bool all = argc == 1;
    const std::string selected_test = argc > 1 ? argv[1] : "";
    const std::string selected_cutter = argc == 3 ? argv[2] : "";
    bool matched = false;
    if (all || (argc == 2 && selected_test == "composite_contacts")) {
        composite_contacts();
        matched = true;
    }
#if defined(OCL_WASM_THREAD_POOL_SIZE) || defined(_OPENMP)
    if (all || (argc == 2 && selected_test == "concurrent_waterline")) {
        concurrent_waterline();
        matched = true;
    }
#endif
    if (all || argc == 3) {
        ocl::STLSurf surface; model(surface);
        ocl::CylCutter cylinder(.6,5);
        ocl::BallCutter ball(.6,5);
        ocl::BullCutter bull(.6,.1,5);
        ocl::ConeCutter cone(.6,.7,5);
        Composite composite_cylinder(cylinder), composite_ball(ball);
        struct Cutter {
            const char* name;
            ocl::MillingCutter* value;
        };
        const Cutter cutters[] = {
            {"cylinder", &cylinder}, {"ball", &ball}, {"bull", &bull}, {"cone", &cone},
            {"composite_cylinder", &composite_cylinder}, {"composite_ball", &composite_ball}
        };
        struct Test {
            const char* name;
            void (*run)(ocl::STLSurf&, ocl::MillingCutter&);
        };
        const Test tests[] = {
            {"batch_drop", batch_drop}, {"batch_push", batch_push},
            {"adaptive_path", adaptive_path}, {"adaptive_waterline", adaptive_waterline}
        };
        for (const auto& cutter : cutters) {
            if (!all && selected_cutter != cutter.name) continue;
            for (const auto& test : tests) {
                if (!all && selected_test != test.name) continue;
                test.run(surface, *cutter.value);
                matched = true;
            }
        }
    }
    if (!matched) {
        std::cerr << "Usage: " << argv[0]
                  << " [composite_contacts|"
#if defined(OCL_WASM_THREAD_POOL_SIZE) || defined(_OPENMP)
                  << "concurrent_waterline|"
#endif
                  << "<algorithm> <cutter>]\n"
                  << "Algorithms: batch_drop, batch_push, adaptive_path, adaptive_waterline\n"
                  << "Cutters: cylinder, ball, bull, cone, composite_cylinder, composite_ball\n"
                  << "With no arguments, run all cases.\n";
        return EXIT_FAILURE;
    }
#ifdef _OPENMP
    CHECK(omp_get_max_threads() == original_threads);
#endif
    return EXIT_SUCCESS;
}
