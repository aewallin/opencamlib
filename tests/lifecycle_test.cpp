#include "check.hpp"
#include "pathdropcutter.hpp"
#include "adaptivepathdropcutter.hpp"
#include "adaptivewaterline.hpp"
#include "cylcutter.hpp"
#include "stlsurf.hpp"
#include <limits>
#include <type_traits>

template<class Exception, class Function>
static void throws(Function f) {
    try { f(); } catch (const Exception&) { return; }
    CHECK(false);
}

struct CountedSpan : ocl::LineSpan {
    static int alive;
    CountedSpan() : LineSpan(ocl::Line({0,0,0}, {1,0,0})) { ++alive; }
    CountedSpan(const CountedSpan& other) : LineSpan(other) { ++alive; }
    ~CountedSpan() { --alive; }
    std::unique_ptr<ocl::Span> clone() const override {
        return std::make_unique<CountedSpan>(*this);
    }
};
int CountedSpan::alive = 0;

static void paths() {
    {
        CountedSpan span;
        ocl::Path path;
        path.append(span);
        path.append(ocl::Arc({1,0,0}, {-1,0,0}, {0,0,0}, true));
        ocl::Path copy(path);
        CHECK(CountedSpan::alive == 3);
        CHECK(copy.span_list.size() == 2);
        CHECK(copy.span_list.front() != path.span_list.front());
        CHECK(copy.span_list.back()->type() == ocl::ArcSpanType);
        CHECK(copy.span_list.back()->getPoint(.5) == path.span_list.back()->getPoint(.5));
        const ocl::Path& same = copy;
        copy = same;
        CHECK(CountedSpan::alive == 3);
        ocl::Path moved(std::move(copy));
        CHECK(copy.span_list.empty());
        CHECK(moved.span_list.size() == 2);
        copy = std::move(moved);
        CHECK(moved.span_list.empty());
        path = ocl::Path();
        CHECK(CountedSpan::alive == 2);
        CHECK(copy.span_list.front()->getPoint(1) == ocl::Point(1,0,0));
    }
    CHECK(CountedSpan::alive == 0);
    static_assert(!std::is_copy_constructible<ocl::PathDropCutter>::value,
                  "Operations must not shallow-copy owned resources");
}

template<class DropCutter>
static void dropcutter() {
    for (double scale : {0.001, 1.0, 1000.0}) {
        ocl::STLSurf surface;
        surface.addTriangle(ocl::Triangle({-10*scale,-10*scale,scale},
                            {10*scale,-10*scale,scale}, {0,10*scale,scale}));
        ocl::CylCutter cutter(.5*scale, 5*scale);
        ocl::Path path;
        path.append(ocl::Line({-scale,0,-10*scale}, {scale,0,-10*scale}));
        path.append(ocl::Arc({scale,0,-10*scale}, {-scale,0,-10*scale},
                            {0,0,-10*scale}, true));
        DropCutter operation;
        throws<std::logic_error>([&] { operation.run(); });
        throws<std::invalid_argument>([&] { operation.setPath(nullptr); });
        throws<std::invalid_argument>([&] { operation.setCutter(nullptr); });
        for (double invalid : {0.0, -1.0, std::numeric_limits<double>::infinity(),
                               std::numeric_limits<double>::quiet_NaN()})
            throws<std::invalid_argument>([&] { operation.setSampling(invalid); });
        operation.setSTL(surface); operation.setCutter(&cutter);
        throws<std::logic_error>([&] { operation.run(); });
        operation.setPath(&path); operation.setSampling(.2*scale);
        for (double floor : {2*scale, -2*scale}) {
            operation.setZ(floor);
            std::vector<ocl::CLPoint> reference;
            for (int threads : {1,4,1}) {
                operation.setThreads(threads); operation.run();
                const auto result = operation.getCLPoints();
                CHECK(result.size() > 5);
                for (const auto& point : result)
                    CHECK(std::abs(point.z - std::max(floor, scale)) < 1e-9*scale);
                if (reference.empty()) reference = result;
                else {
                    CHECK(result.size() == reference.size());
                    for (std::size_t i = 0; i < result.size(); ++i)
                        CHECK(result[i] == reference[i]);
                }
            }
        }
        ocl::STLSurf empty;
        operation.setSTL(empty); operation.setZ(-3*scale); operation.run();
        for (const auto& point : operation.getCLPoints()) CHECK(point.z == -3*scale);
        ocl::Path empty_path;
        operation.setPath(&empty_path); operation.run();
        CHECK(operation.getCLPoints().empty());
    }
}

template<class Waterline>
static void waterline() {
    ocl::STLSurf surface;
    const ocl::Point rim[] = {{-2,-2,0}, {2,-2,0}, {2,2,0}, {-2,2,0}};
    for (int i = 0; i < 4; ++i)
        surface.addTriangle(ocl::Triangle(rim[i], rim[(i+1)%4], {0,0,3}));
    ocl::CylCutter cutter(.5, 5);
    Waterline operation;
    throws<std::logic_error>([&] { operation.run(); });
    operation.setSTL(surface); operation.setCutter(&cutter); operation.setZ(1);
    operation.setSampling(.25); operation.setThreads(1); operation.run();
    const auto loops = operation.getLoops();
    CHECK(!loops.empty());
    operation.run();
    CHECK(operation.getLoops() == loops);
    operation.reset();
    CHECK(operation.getLoops().empty());
    operation.run();
    CHECK(operation.getLoops() == loops);
    ocl::STLSurf empty;
    operation.setSTL(empty); operation.run();
    CHECK(operation.getLoops().empty());
}

int main() {
    paths();
    dropcutter<ocl::PathDropCutter>();
    dropcutter<ocl::AdaptivePathDropCutter>();
    waterline<ocl::Waterline>();
    waterline<ocl::AdaptiveWaterline>();
    ocl::AdaptivePathDropCutter operation;
    throws<std::invalid_argument>([&] { operation.setMinSampling(0); });
    throws<std::invalid_argument>([&] { operation.setCosLimit(2); });
    throws<std::invalid_argument>([&] { operation.setZ(std::numeric_limits<double>::infinity()); });
}
