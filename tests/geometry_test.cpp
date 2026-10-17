#include "simple_weave.hpp"
#include "smart_weave.hpp"
#include "waterline.hpp"
#include "adaptivewaterline.hpp"
#include "pathdropcutter.hpp"
#include "adaptivepathdropcutter.hpp"
#include "stlsurf.hpp"
#include "ballcutter.hpp"
#include "bullcutter.hpp"
#include "conecutter.hpp"
#include "cylcutter.hpp"
#include "line.hpp"
#include "path.hpp"
#include "check.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <string>

using Key = std::array<long long, 3>;
using Loop = std::vector<Key>;
using Loops = std::vector<Loop>;

static Key key(const ocl::Point& p) {
    return {{std::llround(p.x * 1e8), std::llround(p.y * 1e8), std::llround(p.z * 1e8)}};
}

// Ignore arbitrary starting vertices and loop order, retaining every segment.
static Loops canonical(const std::vector<std::vector<ocl::Point>>& loops) {
    Loops result;
    for (const auto& points : loops) {
        CHECK(!points.empty());
        Loop loop;
        for (const auto& p : points) loop.push_back(key(p));
        std::rotate(loop.begin(), std::min_element(loop.begin(), loop.end()), loop.end());
        result.push_back(loop);
    }
    std::sort(result.begin(), result.end());
    return result;
}

static void dump(const Loops& loops) {
    std::cout << loops.size() << '\n';
    for (const auto& loop : loops) {
        std::cout << loop.size() << '\n';
        for (const auto& p : loop) std::cout << p[0] << ' ' << p[1] << ' ' << p[2] << '\n';
    }
}

static Loops reversed(Loops loops) {
    // SimpleWeave and SmartWeave have historically used opposite winding.
    for (auto& loop : loops) std::reverse(loop.begin() + 1, loop.end());
    std::sort(loops.begin(), loops.end());
    return loops;
}

template<class Weave>
static Loops weave(bool hole) {
    Weave w;
    for (int axis = 0; axis < 2; ++axis) {
        for (int i = 1; i <= 7; ++i) {
            ocl::Fiber f(axis == 0 ? ocl::Point(0, i, 0) : ocl::Point(i, 0, 0),
                         axis == 0 ? ocl::Point(8, i, 0) : ocl::Point(i, 8, 0));
            if (hole && i >= 3 && i <= 5) {
                ocl::Interval low(0, 2.5 / 8), high(5.5 / 8, 1);
                f.addInterval(low); f.addInterval(high);
            } else {
                ocl::Interval full(0, 1);
                f.addInterval(full);
            }
            w.addFiber(f);
        }
    }
    // Isolated intervals exercise removal of vertices during construction.
    ocl::Fiber isolated(ocl::Point(12, 9, 0), ocl::Point(14, 9, 0));
    ocl::Interval full(0, 1);
    isolated.addInterval(full);
    w.addFiber(isolated);
    w.build();
    w.face_traverse();
    auto result = canonical(w.getLoops());
    CHECK(result.size() == (hole ? 2u : 1u));
    CHECK(result[0].size() == 28);
    if (hole) CHECK(result[1].size() == 12);
    // Every point is an interval endpoint on the expected outer/inner boundary.
    for (std::size_t i = 0; i < result.size(); ++i) {
        long long lo = i == 0 ? 0 : 250000000;
        long long hi = i == 0 ? 800000000 : 550000000;
        for (const auto& p : result[i]) {
            CHECK(p[2] == 0);
            CHECK(p[0] >= lo && p[0] <= hi && p[1] >= lo && p[1] <= hi);
            CHECK(p[0] == lo || p[0] == hi || p[1] == lo || p[1] == hi);
        }
    }
    return result;
}

static void add_model(ocl::STLSurf& surface, double offset) {
    std::array<ocl::Point, 4> rim{{{offset - 2, 0, 0}, {offset, -1.5, 0},
                                 {offset + 2, 0, 0}, {offset, 1.5, 0}}};
    ocl::Point top(offset, 0, 3), bottom(offset, 0, -1);
    for (std::size_t i = 0; i < rim.size(); ++i) {
        ocl::Triangle upper(rim[i], rim[(i + 1) % 4], top);
        ocl::Triangle lower(rim[(i + 1) % 4], rim[i], bottom);
        surface.addTriangle(upper);
        surface.addTriangle(lower);
    }
}

template<class Waterline>
static Loops waterline(ocl::STLSurf& surface, ocl::MillingCutter& cutter, double z,
                      bool smart) {
    Waterline w;
    w.setSTL(surface); w.setCutter(&cutter); w.setSampling(0.23); w.setZ(z);
    w.setThreads(1);
    if (smart) w.run2(); else w.run();
    auto result = canonical(w.getLoops());
    CHECK(result.size() == 2);
    for (const auto& loop : result) {
        CHECK(loop.size() >= 8);
        for (const auto& p : loop) CHECK(p[2] == std::llround(z * 1e8));
    }
    w.reset();
    if (smart) w.run2(); else w.run();
    CHECK(canonical(w.getLoops()) == result);
    return result;
}

template<class DropCutter>
static Loop dropcutter(ocl::STLSurf& surface, ocl::MillingCutter& cutter) {
    ocl::Path path;
    ocl::Line line(ocl::Point(-3, 0.2, 0), ocl::Point(9, 0.2, 0));
    path.append(line);
    DropCutter operation;
    operation.setSTL(surface); operation.setCutter(&cutter); operation.setPath(&path);
    operation.setSampling(0.27); operation.setZ(-0.5);
    operation.setThreads(1);
    operation.run();
    Loop points;
    for (const auto& p : operation.getPoints()) {
        CHECK(std::isfinite(p.z) && p.z >= -0.5);
        points.push_back(key(p));
    }
    CHECK(points.size() >= 40);
    return points;
}

static void triangle_slices() {
    const std::array<ocl::Point, 3> vertices{{{0,0,0}, {4,0,2}, {0,4,4}}};
    std::array<int, 3> order{{0,1,2}};
    do {
        ocl::Triangle triangle(vertices[order[0]], vertices[order[1]], vertices[order[2]]);
        for (int z : {1,2,3}) {
            ocl::Point a, b;
            CHECK(triangle.zslice_verts(a,b,z));
            const ocl::Point first = z == 1 ? ocl::Point(2,0,1) :
                                     z == 2 ? ocl::Point(4,0,2) : ocl::Point(2,2,3);
            const ocl::Point second(0,z,z);
            CHECK((a == first && b == second) || (a == second && b == first));
        }
        for (int z : {-1,0,4,5}) {
            ocl::Point a(9,8,7), b(6,5,4);
            CHECK(!triangle.zslice_verts(a,b,z));
            CHECK(a == ocl::Point(9,8,7) && b == ocl::Point(6,5,4));
        }
    } while (std::next_permutation(order.begin(),order.end()));
}

static ocl::Interval interval(double lower, double upper, double id) {
    ocl::Interval value(lower,upper);
    value.lower_cc = ocl::CCPoint(id,0,0,ocl::VERTEX);
    value.upper_cc = ocl::CCPoint(id,1,0,ocl::EDGE);
    return value;
}

static void interval_merges() {
    ocl::Fiber fiber;
    auto a = interval(.8,.9,1), b = interval(.1,.2,2), c = interval(.4,.5,3);
    fiber.addInterval(a); fiber.addInterval(b); fiber.addInterval(c);
    // Preserve non-overlapping order and append the union after the survivors.
    auto overlap = interval(.2,.4,4);
    fiber.addInterval(overlap);
    CHECK(fiber.ints.size() == 2);
    CHECK(fiber.ints[0].lower == .8 && fiber.ints[0].upper == .9);
    CHECK(fiber.ints[1].lower == .1 && fiber.ints[1].upper == .5);
    CHECK(fiber.ints[1].lower_cc.x == 2 && fiber.ints[1].upper_cc.x == 3);
    // Equal endpoints retain the earlier contact, including its contact type.
    auto tied = interval(.1,.5,5);
    fiber.addInterval(tied);
    CHECK(fiber.ints.size() == 2);
    CHECK(fiber.ints[1].lower_cc.x == 2 && fiber.ints[1].lower_cc.type == ocl::VERTEX);
    CHECK(fiber.ints[1].upper_cc.x == 3 && fiber.ints[1].upper_cc.type == ocl::EDGE);
    auto contained = interval(.2,.3,6);
    fiber.ints[1].in_weave = true;
    fiber.addInterval(contained);
    CHECK(fiber.ints[1].in_weave);
    // A containing interval replaces all endpoints and resets weave metadata.
    auto enclosing = interval(-1,2,7);
    fiber.addInterval(enclosing);
    CHECK(fiber.ints.size() == 1);
    CHECK(fiber.ints[0].lower == -1 && fiber.ints[0].upper == 2);
    CHECK(fiber.ints[0].lower_cc.x == 7 && fiber.ints[0].upper_cc.x == 7);
    CHECK(!fiber.ints[0].in_weave);
    ocl::Interval empty;
    fiber.addInterval(empty);
    CHECK(fiber.ints.size() == 1);
}

int main(int argc, char**) {
    triangle_slices();
    interval_merges();
    bool print = argc > 1; // Full coordinates for comparison with another build.
    for (bool hole : {false, true}) {
        auto simple = weave<ocl::weave::SimpleWeave>(hole);
        auto smart = weave<ocl::weave::SmartWeave>(hole);
        CHECK(simple == reversed(smart));
        if (print) { dump(simple); dump(smart); }
    }
    ocl::STLSurf surface;
    add_model(surface, 0); add_model(surface, 6);
    ocl::CylCutter cylinder(0.6, 5);
    ocl::BallCutter ball(0.6, 5);
    ocl::BullCutter bull(0.6, 0.1, 5);
    ocl::ConeCutter cone(0.6, 0.7, 5);
    for (ocl::MillingCutter* cutter : std::vector<ocl::MillingCutter*>{&cylinder, &ball, &bull, &cone}) {
        for (double z : {0.4, 1.3}) {
            auto simple = waterline<ocl::Waterline>(surface, *cutter, z, false);
            auto smart = waterline<ocl::Waterline>(surface, *cutter, z, true);
            CHECK(simple == reversed(smart));
            auto adaptive = waterline<ocl::AdaptiveWaterline>(surface, *cutter, z, false);
            auto adaptive_smart = waterline<ocl::AdaptiveWaterline>(surface, *cutter, z, true);
            CHECK(adaptive == reversed(adaptive_smart));
            if (print) { dump(simple); dump(smart); dump(adaptive); dump(adaptive_smart); }
        }
        auto path = dropcutter<ocl::PathDropCutter>(surface, *cutter);
        auto adaptive_path = dropcutter<ocl::AdaptivePathDropCutter>(surface, *cutter);
        if (print) { dump({path}); dump({adaptive_path}); }
    }
}
