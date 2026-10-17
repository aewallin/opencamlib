#include <emscripten.h>
#include <emscripten/bind.h>
#include "ocl.hpp"

// GEOMETRY
#include "point.hpp"
#include "ccpoint.hpp"
#include "clpoint.hpp"
#include "triangle.hpp"
#include "bbox.hpp"
#include "path.hpp"
#include "line.hpp"
#include "ellipse.hpp"
#include "ellipseposition.hpp"

// STL
#include "stlsurf.hpp"
#include "stlreader.hpp"

// ALGO
#include "operation.hpp"
#include "batchpushcutter.hpp"
#include "waterline.hpp"
#include "adaptivepathdropcutter.hpp"
#include "compositecutter.hpp"
#include "adaptivewaterline.hpp"
#include "zigzag.hpp"
#include "weave.hpp"
#include "lineclfilter.hpp"
// #include "clsurface.hpp"

// CUTTERS
#include "millingcutter.hpp"
#include "cylcutter.hpp"
#include "ballcutter.hpp"
#include "bullcutter.hpp"
#include "conecutter.hpp"

using namespace emscripten;
using namespace ocl;

// Translate core validation failures to ordinary JavaScript errors.
template<class Method> struct CheckedMethod;
template<class Result, class Class, class... Args>
struct CheckedMethod<Result (Class::*)(Args...)> {
    template<Result (Class::*method)(Args...)>
    static Result call(Class& self, Args... args) {
        try { return (self.*method)(args...); }
        catch (const std::exception& error) {
            val::global("Error").new_(std::string(error.what())).throw_();
            throw;
        }
    }
};
template<auto method>
auto checked() { return &CheckedMethod<decltype(method)>::template call<method>; }

EMSCRIPTEN_BINDINGS(opencamlib)
{
    function("maxThreads", &ocl::max_threads);
    //////////////
    // GEOMETRY //
    //////////////
    class_<Point>("Point")
        .constructor()
        .constructor<double, double, double>()
        .constructor<double, double>()
        .constructor<Point>()
        .function("norm", &Point::norm)
        .function("xyNorm", &Point::xyNorm)
        .function("normalize", &Point::normalize)
        .function("dot", &Point::dot)
        .function("cross", &Point::cross)
        .function("xRotate", &Point::xRotate)
        .function("yRotate", &Point::yRotate)
        .function("zRotate", &Point::zRotate)
        .function("isRight", &Point::isRight)
        //.function("isInside", &Point::isInside)
        //.function("isInsidePoints", &Point::isInside)
        .function("xyDistance", &Point::xyDistance)
        .function("__str__", &Point::str)
        .property("x", &Point::x)
        .property("y", &Point::y)
        .property("z", &Point::z);

    class_<CLPoint, emscripten::base<Point>>("CLPoint")
        .constructor<CLPoint>()
        .constructor<double, double, double>()
        .constructor<double, double, double, CCPoint &>()
        .function("__str__", &CLPoint::str)
        .function("cc", &CLPoint::getCC)
        .function("getCC", &CLPoint::getCC);

    class_<CCPoint, emscripten::base<Point>>("CCPoint")
        .constructor<CCPoint>()
        .constructor<double, double, double>()
        .function("__str__", &CCPoint::str);
    // .property("type", &CCPoint::type) // @todo figure out the problem

    class_<Triangle>("Triangle")
        .constructor()
        .constructor<Point, Point, Point>();

    enum_<CCType>("CCType")
        .value("NONE", NONE)
        .value("VERTEX", VERTEX)
        .value("VERTEX_CYL", VERTEX_CYL)
        .value("EDGE", EDGE)
        .value("EDGE_SHAFT", EDGE_SHAFT)
        .value("EDGE_HORIZ", EDGE_HORIZ)
        .value("EDGE_CYL", EDGE_CYL)
        .value("EDGE_BALL", EDGE_BALL)
        .value("EDGE_CONE", EDGE_CONE)
        .value("EDGE_CONE_BASE", EDGE_CONE_BASE)
        .value("EDGE_HORIZ_CYL", EDGE_HORIZ_CYL)
        .value("EDGE_HORIZ_TOR", EDGE_HORIZ_TOR)
        .value("EDGE_POS", EDGE_POS)
        .value("EDGE_NEG", EDGE_NEG)
        .value("FACET", FACET)
        .value("FACET_TIP", FACET_TIP)
        .value("FACET_CYL", FACET_CYL)
        .value("ERROR", ERROR);

    class_<STLReader>("STLReader")
        .constructor<const std::wstring &, STLSurf &>();

    class_<STLSurf>("STLSurf")
        .constructor()
        .function("addTriangle", checked<&STLSurf::addTriangle>())
        .function("size", &STLSurf::size);

    class_<Bbox>("Bbox")
        .function("isInside", &Bbox::isInside)
        .property("maxpt", &Bbox::maxpt)
        .property("minpt", &Bbox::minpt);

    // Epos and the Ellipse are used for the toroidal tool edge-tests
    class_<EllipsePosition>("EllipsePosition")
        .property("s", &EllipsePosition::s)
        .property("t", &EllipsePosition::t)
        .function("setDiangle", checked<&EllipsePosition::setDiangle>())
        .function("__str__", &EllipsePosition::str);

    // class_<Ellipse>("Ellipse")
    //     .constructor<Point &, double, double, double>()
    //     .function("ePoint", &Ellipse::ePoint)
    //     .function("oePoint", &Ellipse::oePoint)
    //     .function("normal", &Ellipse::normal);

    class_<Line>("Line")
        .constructor<Point, Point>()
        .constructor<Line>()
        .property("p1", &Line::p1)
        .property("p2", &Line::p2);

    class_<Arc>("Arc")
        .constructor<Point, Point, Point, bool>()
        .constructor<Arc>()
        .property("p1", &Arc::p1)
        .property("p2", &Arc::p2)
        .property("c", &Arc::c)
        .property("dir", &Arc::dir);

    enum_<SpanType>("SpanType")
        .value("LineSpanType", LineSpanType)
        .value("ArcSpanType", ArcSpanType);

    class_<Path>("Path")
        .constructor()
        .constructor<Path>()
        .function("appendLine", static_cast<void (Path::*)(const Line &l)>(&Path::append))
        .function("appendArc", static_cast<void (Path::*)(const Arc &a)>(&Path::append));

    //////////
    // ALGO //
    //////////
    class_<Operation>("Operation")
        .function("setCutter", checked<&Operation::setCutter>(), allow_raw_pointers())
        .function("getCLPoints", &Operation::getCLPoints)
        .function("setSTL", checked<&Operation::setSTL>(), allow_raw_pointers())
        .function("setSampling", checked<&Operation::setSampling>())
        .function("setThreads", checked<&Operation::setThreads>())
        .function("getThreads", &Operation::getThreads);

    class_<BatchDropCutter, emscripten::base<Operation>>("BatchDropCutter")
        .constructor()
        .function("run", checked<&BatchDropCutter::run>())
        .function("setThreads", checked<&BatchDropCutter::setThreads>())
        .function("getThreads", &BatchDropCutter::getThreads)
        .function("appendPoint", &BatchDropCutter::appendPoint)
        // .function("getTrianglesUnderCutter", &BatchDropCutter::getTrianglesUnderCutter.)
        .function("getCalls", &BatchDropCutter::getCalls)
        .function("getBucketSize", &BatchDropCutter::getBucketSize)
        .function("setBucketSize", checked<&BatchDropCutter::setBucketSize>());

    class_<PathDropCutter, emscripten::base<Operation>>("PathDropCutter")
        .constructor()
        .function("run", checked<&PathDropCutter::run>())
        .function("setPath", checked<&PathDropCutter::setPath>(), allow_raw_pointers())
        .function("getZ", &PathDropCutter::getZ)
        .function("setZ", checked<&PathDropCutter::setZ>())
        .function("getPoints", &PathDropCutter::getPoints);

    class_<AdaptivePathDropCutter, emscripten::base<Operation>>("AdaptivePathDropCutter")
        .constructor()
        .function("run", checked<&AdaptivePathDropCutter::run>())
        .function("setMinSampling", checked<&AdaptivePathDropCutter::setMinSampling>())
        .function("setCosLimit", checked<&AdaptivePathDropCutter::setCosLimit>())
        .function("getSampling", &AdaptivePathDropCutter::getSampling)
        .function("setPath", checked<&AdaptivePathDropCutter::setPath>(), allow_raw_pointers())
        .function("getZ", &AdaptivePathDropCutter::getZ)
        .function("setZ", checked<&AdaptivePathDropCutter::setZ>())
        .function("getPoints", &AdaptivePathDropCutter::getPoints);

    class_<ZigZag>("ZigZag")
        .constructor()
        .function("run", checked<&ZigZag::run>())
        .function("setDirection", checked<&ZigZag::setDirection>(), allow_raw_pointers())
        .function("setOrigin", checked<&ZigZag::setOrigin>(), allow_raw_pointers())
        .function("setStepOver", checked<&ZigZag::setStepOver>())
        .function("addPoint", &ZigZag::addPoint)
        // .function("getOutput", &ZigZag::getOutput)
        .function("__str__", &ZigZag::str);

    class_<BatchPushCutter, emscripten::base<Operation>>("BatchPushCutter")
        .constructor()
        .function("run", checked<&BatchPushCutter::run>())
        .function("appendFiber", &BatchPushCutter::appendFiber)
        // .function("getOverlapTriangles", &BatchPushCutter::getOverlapTriangles)
        // .function("getFibers", &BatchPushCutter::getFibers)
        .function("getCalls", &BatchPushCutter::getCalls)
        .function("setThreads", checked<&BatchPushCutter::setThreads>())
        .function("getThreads", &BatchPushCutter::getThreads)
        .function("setBucketSize", checked<&BatchPushCutter::setBucketSize>())
        .function("getBucketSize", &BatchPushCutter::getBucketSize)
        .function("setXDirection", checked<&BatchPushCutter::setXDirection>())
        .function("setYDirection", checked<&BatchPushCutter::setYDirection>());

    class_<Interval>("Interval")
        .constructor<double, double>()
        .property("upper", &Interval::upper)
        .property("lower", &Interval::lower)
        .property("lower_cc", &Interval::lower_cc)
        .property("upper_cc", &Interval::upper_cc)
        .function("updateUpper", &Interval::updateUpper)
        .function("updateLower", &Interval::updateLower)
        .function("empty", &Interval::empty)
        .function("__str__", &Interval::str);

    class_<Fiber>("Fiber")
        .constructor<Point, Point>();

    // class_<Fiber, bases<Fiber>>("Fiber")
    //     .constructor<Point, Point>()
    //     .property("p1", &Fiber::p1)
    //     .property("p2", &Fiber::p2)
    //     .property("dir", &Fiber::dir)
    //     .function("addInterval", &Fiber::addInterval)
    //     .function("point", &Fiber::point)
    //     .function("printInts", &Fiber::printInts)
    //     .function("getInts", &Fiber::getInts);

    register_vector<Point>("std::vector<Point>");
    register_vector<CLPoint>("std::vector<CLPoint>");
    register_vector<std::vector<Point>>("std::vector<std::vector<Point>>");

    class_<Waterline, emscripten::base<Operation>>("Waterline")
        .constructor()
        .function("setCutter", checked<&Waterline::setCutter>(), allow_raw_pointers())
        .function("setSTL", checked<&Waterline::setSTL>(), allow_raw_pointers())
        .function("setZ", checked<&Waterline::setZ>())
        .function("setSampling", checked<&Waterline::setSampling>())
        .function("run", checked<&Waterline::run>())
        .function("run2", checked<&Waterline::run2>())
        .function("reset", &Waterline::reset)
        .function("getLoops", &Waterline::getLoops)
        .function("setThreads", checked<&Waterline::setThreads>())
        .function("getThreads", &Waterline::getThreads);
    // .function("getXFibers", &Waterline::py_getXFibers)
    // .function("getYFibers", &Waterline::py_getYFibers);

    class_<AdaptiveWaterline, emscripten::base<Waterline>>("AdaptiveWaterline")
        .constructor()
        .function("setZ", checked<&AdaptiveWaterline::setZ>())
        .function("setMinSampling", checked<&AdaptiveWaterline::setMinSampling>())
        .function("run", checked<&AdaptiveWaterline::run>())
        .function("run2", checked<&AdaptiveWaterline::run2>())
        .function("reset", &AdaptiveWaterline::reset)
        // .function("getLoops", &AdaptiveWaterline::py_getLoops)
        .function("setThreads", checked<&AdaptiveWaterline::setThreads>())
        .function("getThreads", &AdaptiveWaterline::getThreads);
    // .function("getXFibers", &AdaptiveWaterline::getXFibers)
    // .function("getYFibers", &AdaptiveWaterline::getYFibers);

    enum_<weave::VertexType>("WeaveVertexType")
        .value("CL", weave::CL)
        .value("CL_DONE", weave::CL_DONE)
        .value("ADJ", weave::ADJ)
        .value("TWOADJ", weave::TWOADJ)
        .value("INT", weave::INT)
        .value("FULLINT", weave::FULLINT);

    class_<LineCLFilter>("LineCLFilter")
        .constructor()
        .function("addCLPoint", &LineCLFilter::addCLPoint)
        .function("setTolerance", checked<&LineCLFilter::setTolerance>())
        .function("run", checked<&LineCLFilter::run>());
    // .function("getCLPoints", &LineCLFilter::getCLPoints);

    // some strange problem with hedi::face_edges()... let's not compile for now..
    // class_<clsurf::CutterLocationSurface>("CutterLocationSurface")
    //     .constructor<double>()
    //     .function("run", checked<&clsurf::CutterLocationSurface::run>())
    //     .function("setMinSampling", checked<&clsurf::CutterLocationSurface::setMinSampling>())
    //     .function("setSampling", checked<&clsurf::CutterLocationSurface::setSampling>())
    //     .function("setSTL", checked<&clsurf::CutterLocationSurface::setSTL>(), allow_raw_pointers())
    //     .function("setCutter", checked<&clsurf::CutterLocationSurface::setCutter>(), allow_raw_pointers())
    //     // .function("getVertices", &clsurf::CutterLocationSurface::getVertices)
    //     // .function("getEdges", &clsurf::CutterLocationSurface::getEdges)
    //     .function("__str__", &clsurf::CutterLocationSurface::str);
    /*
    class_<tsp::TSPSolver >("TSPSolver")
        .function("addPoint", &tsp::TSPSolver::addPoint)
        .function("run", checked<&tsp::TSPSolver::run>())
        .function("getOutput", &tsp::TSPSolver::getOutput)
        .function("getLength", &tsp::TSPSolver::getLength)
        .function("reset", &tsp::TSPSolver::reset)
    ;
    */

    /////////////
    // CUTTERS //
    /////////////
    class_<MillingCutter>("MillingCutter")
        .constructor()
        .function("vertexDrop", &MillingCutter::vertexDrop)
        .function("facetDrop", &MillingCutter::facetDrop)
        .function("edgeDrop", &MillingCutter::edgeDrop)
        .function("dropCutter", &MillingCutter::dropCutter)
        .function("pushCutter", &MillingCutter::pushCutter)
        .function("offsetCutter", &MillingCutter::offsetCutter, allow_raw_pointers())
        .function("__str__", &MillingCutter::str)
        .function("getRadius", &MillingCutter::getRadius)
        .function("getLength", &MillingCutter::getLength)
        .function("getDiameter", &MillingCutter::getDiameter);

    class_<CylCutter, emscripten::base<MillingCutter>>("CylCutter")
        .constructor<double, double>()
        .function("dropCutterSTL", &CylCutter::dropCutterSTL, allow_raw_pointers());

    class_<BallCutter, emscripten::base<MillingCutter>>("BallCutter")
        .constructor<double, double>()
        .function("dropCutterSTL", &BallCutter::dropCutterSTL, allow_raw_pointers());

    class_<BullCutter, emscripten::base<MillingCutter>>("BullCutter")
        .constructor<double, double, double>();
    class_<ConeCutter, emscripten::base<MillingCutter>>("ConeCutter")
        .constructor<double, double, double>();

    class_<CompCylCutter, emscripten::base<MillingCutter>>("CompCylCutter")
        .constructor<double, double>();
    class_<CompBallCutter, emscripten::base<MillingCutter>>("CompBallCutter")
        .constructor<double, double>();

    class_<CylConeCutter, emscripten::base<MillingCutter>>("CylConeCutter")
        .constructor<double, double, double>();
    class_<BallConeCutter, emscripten::base<MillingCutter>>("BallConeCutter")
        .constructor<double, double, double>();
    class_<BullConeCutter, emscripten::base<MillingCutter>>("BullConeCutter")
        .constructor<double, double, double, double>();
    class_<ConeConeCutter, emscripten::base<MillingCutter>>("ConeConeCutter")
        .constructor<double, double, double, double>();
}
