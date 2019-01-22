#include <emscripten.h>
#include <emscripten/bind.h>

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
#include "waterline.hpp"
#include "adaptivepathdropcutter.hpp"
#include "adaptivewaterline.hpp"
// #include "zigzag.hpp"
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

EMSCRIPTEN_BINDINGS(opencamlib)
{
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
        .function("addTriangle", &STLSurf::addTriangle)
        .function("size", &STLSurf::size);

    class_<Bbox>("Bbox")
        .function("isInside", &Bbox::isInside)
        .property("maxpt", &Bbox::maxpt)
        .property("minpt", &Bbox::minpt);
    
    // Epos and the Ellipse are used for the toroidal tool edge-tests
    class_<EllipsePosition>("EllipsePosition")
        .property("s", &EllipsePosition::s)
        .property("t", &EllipsePosition::t)
        .function("setDiangle", &EllipsePosition::setDiangle)
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
        .function("setCutter", &Operation::setCutter, allow_raw_pointers())
        .function("getCLPoints", &Operation::getCLPoints)
        .function("setSTL", &Operation::setSTL, allow_raw_pointers())
        .function("setSampling", &Operation::setSampling);

    class_<BatchDropCutter, emscripten::base<Operation>>("BatchDropCutter")
        .constructor()
        .function("run", &BatchDropCutter::run);
        // .function("setSTL", &BatchDropCutter::setSTL)
        // .function("setCutter", &BatchDropCutter::setCutter)
        // .function("setThreads", &BatchDropCutter::setThreads)
        // .function("getThreads", &BatchDropCutter::getThreads)
        // .function("appendPoint", &BatchDropCutter::appendPoint)
        // .function("getTrianglesUnderCutter", &BatchDropCutter::getTrianglesUnderCutter)
        // .function("getCalls", &BatchDropCutter::getCalls)
        // .function("getBucketSize", &BatchDropCutter::getBucketSize)
        // .function("setBucketSize", &BatchDropCutter::setBucketSize);

    class_<PathDropCutter, emscripten::base<Operation>>("PathDropCutter")
        .constructor()
        .function("run", &PathDropCutter::run)
        // .function("setCutter", &PathDropCutter::setCutter)
        // .function("setSTL", &PathDropCutter::setSTL)
        // .function("setSampling", &PathDropCutter::setSampling)
        .function("setPath", &PathDropCutter::setPath, allow_raw_pointers())
        .function("getZ", &PathDropCutter::getZ)
        .function("setZ", &PathDropCutter::setZ)
        .function("getPoints", &PathDropCutter::getPoints);

    class_<AdaptivePathDropCutter, emscripten::base<Operation>>("AdaptivePathDropCutter")
        .constructor()
        .function("run", &AdaptivePathDropCutter::run)
        // .function("setCutter", &AdaptivePathDropCutter::setCutter)
        // .function("setSTL", &AdaptivePathDropCutter::setSTL)
        // .function("setSampling", &AdaptivePathDropCutter::setSampling)
        .function("setMinSampling", &AdaptivePathDropCutter::setMinSampling)
        .function("setCosLimit", &AdaptivePathDropCutter::setCosLimit)
        // .function("getSampling", &AdaptivePathDropCutter::getSampling)
        .function("setPath", &AdaptivePathDropCutter::setPath, allow_raw_pointers())
        .function("getZ", &AdaptivePathDropCutter::getZ)
        .function("setZ", &AdaptivePathDropCutter::setZ)
        .function("getPoints", &AdaptivePathDropCutter::getPoints);

    // class_<ZigZag>("ZigZag")
    //     .function("run", &ZigZag::run)
    //     .function("setDirection", &ZigZag::setDirection)
    //     .function("setOrigin", &ZigZag::setOrigin)
    //     .function("setStepOver", &ZigZag::setStepOver)
    //     .function("addPoint", &ZigZag::addPoint)
    //     .function("getOutput", &ZigZag::getOutput)
    //     .function("__str__", &ZigZag::str);

    class_<BatchPushCutter>("BatchPushCutter")
        .constructor();
    // class_<BatchPushCutter, bases<BatchPushCutter>>("BatchPushCutter")
    //     .function("run", &BatchPushCutter::run)
    //     .function("setSTL", &BatchPushCutter::setSTL)
    //     .function("setCutter", &BatchPushCutter::setCutter)
    //     .function("setThreads", &BatchPushCutter::setThreads)
    //     .function("appendFiber", &BatchPushCutter::appendFiber)
    //     .function("getOverlapTriangles", &BatchPushCutter::getOverlapTriangles)
    //     .function("getCLPoints", &BatchPushCutter::getCLPoints)
    //     .function("getFibers", &BatchPushCutter::getFibers)
    //     .function("getCalls", &BatchPushCutter::getCalls)
    //     .function("setThreads", &BatchPushCutter::setThreads)
    //     .function("getThreads", &BatchPushCutter::getThreads)
    //     .function("setBucketSize", &BatchPushCutter::setBucketSize)
    //     .function("getBucketSize", &BatchPushCutter::getBucketSize)
    //     .function("setXDirection", &BatchPushCutter::setXDirection)
    //     .function("setYDirection", &BatchPushCutter::setYDirection);

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
        .constructor();

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
        .function("setZ", &Waterline::setZ)
        .function("run", &Waterline::run)
        .function("getLoops", &Waterline::getLoops);

    // class_<Waterline, bases<Waterline>>("Waterline")
    //     .function("setCutter", &Waterline::setCutter)
    //     .function("setSTL", &Waterline::setSTL)
    //     .function("setZ", &Waterline::setZ)
    //     .function("setSampling", &Waterline::setSampling)
    //     .function("run", &Waterline::run)
    //     .function("run2", &Waterline::run2)
    //     .function("reset", &Waterline::reset)
    //     .function("getLoops", &Waterline::py_getLoops)
    //     .function("setThreads", &Waterline::setThreads)
    //     .function("getThreads", &Waterline::getThreads)
    //     .function("getXFibers", &Waterline::py_getXFibers)
    //     .function("getYFibers", &Waterline::py_getYFibers);

    class_<AdaptiveWaterline, emscripten::base<Waterline>>("AdaptiveWaterline")
        .constructor()
        // .function("setZ", &AdaptiveWaterline::setZ)
        .function("setMinSampling", &AdaptiveWaterline::setMinSampling);
        // .function("run", &AdaptiveWaterline::run);
    // class_<AdaptiveWaterline, bases<AdaptiveWaterline>>("AdaptiveWaterline")
    //     .function("setCutter", &AdaptiveWaterline::setCutter)
    //     .function("setSTL", &AdaptiveWaterline::setSTL)
    //     .function("setZ", &AdaptiveWaterline::setZ)
    //     .function("setSampling", &AdaptiveWaterline::setSampling)
    //     .function("setMinSampling", &AdaptiveWaterline::setMinSampling)
    //     .function("run", &AdaptiveWaterline::run)
    //     .function("run2", &AdaptiveWaterline::run2)
    //     .function("reset", &AdaptiveWaterline::reset)
    //     //.function("run2", &AdaptiveWaterline::run2) // uses Weave::build2()
    //     .function("getLoops", &AdaptiveWaterline::py_getLoops)
    //     .function("setThreads", &AdaptiveWaterline::setThreads)
    //     .function("getThreads", &AdaptiveWaterline::getThreads)
    //     .function("getXFibers", &AdaptiveWaterline::getXFibers)
    //     .function("getYFibers", &AdaptiveWaterline::getYFibers);

    enum_<weave::VertexType>("WeaveVertexType")
        .value("CL", weave::CL)
        .value("CL_DONE", weave::CL_DONE)
        .value("ADJ", weave::ADJ)
        .value("TWOADJ", weave::TWOADJ)
        .value("INT", weave::INT)
        .value("FULLINT", weave::FULLINT);

    class_<LineCLFilter>("LineCLFilter")
        .constructor();
    // class_<LineCLFilter, bases<LineCLFilter>>("LineCLFilter")
    //     .function("addCLPoint", &LineCLFilter::addCLPoint)
    //     .function("setTolerance", &LineCLFilter::setTolerance)
    //     .function("run", &LineCLFilter::run)
    //     .function("getCLPoints", &LineCLFilter::getCLPoints);

    // some strange problem with hedi::face_edges()... let's not compile for now..
    // class_<clsurf::CutterLocationSurface>("CutterLocationSurface")
    //     .constructor<double>()
    //     .function("run", &clsurf::CutterLocationSurface::run)
    //     .function("setMinSampling", &clsurf::CutterLocationSurface::setMinSampling)
    //     .function("setSampling", &clsurf::CutterLocationSurface::setSampling)
    //     .function("setSTL", &clsurf::CutterLocationSurface::setSTL)
    //     .function("setCutter", &clsurf::CutterLocationSurface::setCutter)
    //     .function("getVertices", &clsurf::CutterLocationSurface::getVertices)
    //     .function("getEdges", &clsurf::CutterLocationSurface::getEdges)
    //     .function("__str__", &clsurf::CutterLocationSurface::str);
    /*
    class_< tsp::TSPSolver >("TSPSolver")  
        .function("addPoint", &tsp::TSPSolver::addPoint)
        .function("run", &tsp::TSPSolver::run)
        .function("getOutput", &tsp::TSPSolver::getOutput)
        .function("getLength", &tsp::TSPSolver::getLength)
        .function("reset", &tsp::TSPSolver::reset)
    ;
    */

    /////////////
    // CUTTERS //
    /////////////
    class_<MillingCutter>("MillingCutter");
        // .function("vertexDrop", &MillingCutter::vertexDrop)
        // .function("facetDrop", &MillingCutter::facetDrop)
        // .function("edgeDrop", &MillingCutter::edgeDrop)
        // .function("dropCutter", &MillingCutter::dropCutter)
        // .function("pushCutter", &MillingCutter::pushCutter)
        // .function("offsetCutter", &MillingCutter::offsetCutter)
        // .function("__str__", &MillingCutter::str)
        // .function("getRadius", &MillingCutter::getRadius)
        // .function("getLength", &MillingCutter::getLength)
        // .function("getDiameter", &MillingCutter::getDiameter);

    class_<CylCutter, emscripten::base<MillingCutter>>("CylCutter")
        .constructor<double, double>();
        // .function("dropCutterSTL", &CylCutter::dropCutterSTL);

    class_<BallCutter, emscripten::base<MillingCutter>>("BallCutter")
        .constructor<double, double>();
        // .function("dropCutterSTL", &BallCutter::dropCutterSTL);

    class_<BullCutter, emscripten::base<MillingCutter>>("BullCutter")
        .constructor<double, double, double>();
    class_<ConeCutter, emscripten::base<MillingCutter>>("ConeCutter")
        .constructor<double, double, double>();

    // class_<CompCylCutter, emscripten::base<MillingCutter>>("CompCylCutter")
    //     .constructor<double, double>();
    // class_<CompBallCutter, emscripten::base<MillingCutter>>("CompBallCutter")
    //     .constructor<double, double>();

    // class_<CylConeCutter, emscripten::base<MillingCutter>>("CylConeCutter")
    //     .constructor<double, double, double>();
    // class_<BallConeCutter, emscripten::base<MillingCutter>>("BallConeCutter")
    //     .constructor<double, double, double>();
    // class_<BullConeCutter, emscripten::base<MillingCutter>>("BullConeCutter")
    //     .constructor<double, double, double, double>();
    // class_<ConeConeCutter, emscripten::base<MillingCutter>>("ConeConeCutter")
    //     .constructor<double, double, double, double>();
}