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
#include "check.hpp"

#include <cmath>
#include <vector>
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
static void batches(ocl::STLSurf& surface, ocl::MillingCutter& cutter) {
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
    for (int axis : {0,1}) {
        std::vector<ocl::Fiber> fibers;
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
static void adaptive(ocl::STLSurf& surface, ocl::MillingCutter& cutter) {
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
        for (int threads : {4,2,1}) {
            operation.setThreads(threads); operation.run();
            equal(expected,operation.getPoints());
            CHECK(calls == operation.getCalls());
        }
    }
    ocl::AdaptiveWaterline operation;
    operation.setSTL(surface); operation.setCutter(&cutter); operation.setSampling(.3); operation.setZ(1);
    operation.setThreads(1); operation.run();
    const auto x = operation.getXFibers(), y = operation.getYFibers();
    const auto loops = operation.getLoops();
    operation.reset(); operation.setThreads(4); operation.run();
    equal(x,operation.getXFibers()); equal(y,operation.getYFibers());
    const auto parallel = operation.getLoops();
    CHECK(loops.size() == parallel.size());
    for (std::size_t i = 0; i < loops.size(); ++i) {
        CHECK(loops[i].size() == parallel[i].size());
        for (std::size_t j = 0; j < loops[i].size(); ++j) equal(loops[i][j],parallel[i][j]);
    }
}
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

int main() {
    composite_contacts();
#ifdef _OPENMP
    const int original_threads = omp_get_max_threads();
#endif
    ocl::STLSurf surface; model(surface);
    ocl::CylCutter cylinder(.6,5);
    ocl::BallCutter ball(.6,5);
    ocl::BullCutter bull(.6,.1,5);
    ocl::ConeCutter cone(.6,.7,5);
    for (auto* cutter : std::vector<ocl::MillingCutter*>{&cylinder,&ball,&bull,&cone}) {
        batches(surface,*cutter);
        adaptive(surface,*cutter);
    }
    Composite composite_cylinder(cylinder), composite_ball(ball);
    for (auto* cutter : std::vector<ocl::MillingCutter*>{&composite_cylinder,&composite_ball}) {
        batches(surface,*cutter);
        adaptive(surface,*cutter);
    }
#ifdef _OPENMP
    CHECK(omp_get_max_threads() == original_threads);
#endif
}
