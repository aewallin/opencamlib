/*  Copyright 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com)
 *  
 *  This file is part of OpenCAMlib.
 *
 *  OpenCAMlib is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  OpenCAMlib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with OpenCAMlib.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "ocl.h"

/*
 *  Python wrapping
 */

using namespace ocl;

namespace bp = boost::python;

BOOST_PYTHON_MODULE(ocl) {
    bp::class_<Point>("Point") 
        .def(bp::init<double, double, double>())
        .def(bp::init<Point>())
        .def(bp::other<double>() * bp::self)
        .def(bp::self * bp::other<double>())
        .def(bp::self -= bp::other<Point>())
        .def(bp::self - bp::other<Point>())
        .def(bp::self += bp::other<Point>())
        .def(bp::self + bp::other<Point>())
        .def("norm", &Point::norm)
        .def("normalize", &Point::normalize)
        .def("dot", &Point::dot)
        .def("cross", &Point::cross)
        .def("isRight", &Point::isRight)
        .def("isInside", &Point::isInside)
        .def("isInsidePoints", &Point::isInside)
        .def("xyDistance", &Point::xyDistance)
        .def("str", &Point::str)
        .def_readwrite("x", &Point::x)
        .def_readwrite("y", &Point::y)
        .def_readwrite("z", &Point::z)
        .def_readonly("id", &Point::id)
    ;   
    bp::class_<CCPoint>("CCPoint") 
        .def(bp::init<CCPoint>())
        .def("str", &CCPoint::str)
        .def_readwrite("type", &CCPoint::type)
        .def_readwrite("x", &CCPoint::x)
        .def_readwrite("y", &CCPoint::y)
        .def_readwrite("z", &CCPoint::z)
    ;
    bp::enum_<CCType>("CCType")
        .value("NONE", NONE)
        .value("VERTEX",VERTEX)
        .value("EDGE",EDGE)
        .value("EDGE_HORIZ_CYL",EDGE_HORIZ_CYL)
        .value("EDGE_HORIZ_TOR",EDGE_HORIZ_TOR)
        .value("EDGE_POS",EDGE_POS)
        .value("EDGE_NEG",EDGE_NEG)
        .value("FACET", FACET)
        .value("FACET_TIP", FACET_TIP)
        .value("FACET_CYL", FACET_CYL)
        .value("ERROR", ERROR)
    ;
    bp::class_<Triangle>("Triangle")
        .def(bp::init<Point,Point,Point>())
        .def("getPoints", &Triangle::getPoints)
        .def("str", &Triangle::str) // FIXME
        .def_readonly("p", &Triangle::p)
        //.def_readonly("n", &Triangle::n)
        .def_readonly("id", &Triangle::id)
    ;
    bp::class_<STLSurf>("STLSurf")
        .def(bp::init<const std::wstring&>())
        .def("addTriangle", &STLSurf::addTriangle)
        .def("str", &STLSurf::str)
        .def("size", &STLSurf::size)
        .def_readonly("tris", &STLSurf::tris)
        .def_readonly("id", &STLSurf::id)
        .def("build_kdtree", &STLSurf::build_kdtree)
        .def("get_kd_triangles", &STLSurf::get_kd_triangles)
        .def("jump_kd_up", &STLSurf::jump_kd_up)
        .def("jump_kd_hi", &STLSurf::jump_kd_hi)
        .def("jump_kd_lo", &STLSurf::jump_kd_lo)
        .def("jump_kd_reset", &STLSurf::jump_kd_reset)
        .def("get_kd_level", &STLSurf::get_kd_level)
        .def("get_kd_cut", &STLSurf::get_kd_cut)
        .def("getTrianglesUnderCutter", &STLSurf::getTrianglesUnderCutter)
    ;
    bp::class_<MillingCutterWrap, boost::noncopyable>("MillingCutter", bp::no_init)
        .def("vertexDrop", bp::pure_virtual(&MillingCutter::vertexDrop) )
        .def("facetDrop", bp::pure_virtual(&MillingCutter::facetDrop) )
        .def("edgeDrop", bp::pure_virtual(&MillingCutter::edgeDrop) )
        .add_property("radius", &MillingCutter::getRadius )
        .add_property("length", &MillingCutter::getLength, &MillingCutter::setLength  )
        .add_property("diameter", &MillingCutter::getDiameter, &MillingCutter::setDiameter )
    ;
    bp::class_<CylCutter, bp::bases<MillingCutter> >("CylCutter")
        .def(bp::init<double>())
        .def("vertexDrop", &CylCutter::vertexDrop)
        .def("facetDrop", &CylCutter::facetDrop)
        .def("edgeDrop", &CylCutter::edgeDrop)
        .def("dropCutter", &CylCutter::dropCutter)
        .def("dropCutterSTL", &CylCutter::dropCutterSTL)
        .def("str", &CylCutter::str)
    ;
    bp::class_<BallCutter, bp::bases<MillingCutter> >("BallCutter")
        .def(bp::init<double>())
        .def("vertexDrop", &BallCutter::vertexDrop)
        .def("facetDrop", &BallCutter::facetDrop)
        .def("edgeDrop", &BallCutter::edgeDrop)
        .def("dropCutter", &BallCutter::dropCutter)
        .def("dropCutterSTL", &BallCutter::dropCutterSTL)
        .def("str", &BallCutter::str)
    ;
    bp::class_<BullCutter, bp::bases<MillingCutter> >("BullCutter")
        .def(bp::init<double, double>())
        .def("vertexDrop", &BullCutter::vertexDrop)
        .def("facetDrop", &BullCutter::facetDrop)
        .def("edgeDrop", &BullCutter::edgeDrop)
        .def("str", &BullCutter::str)
    ;
    bp::class_<ConeCutter, bp::bases<MillingCutter> >("ConeCutter")
        .def(bp::init<double, double>())
        .def("vertexDrop", &ConeCutter::vertexDrop)
        .def("facetDrop", &ConeCutter::facetDrop)
        .def("edgeDrop", &ConeCutter::edgeDrop)
        .def("str", &ConeCutter::str)
    ;
    bp::class_<CylConeCutter, bp::bases<MillingCutter> >("CylConeCutter")
        .def(bp::init<double, double, double>())
        .def("vertexDrop", &CylConeCutter::vertexDrop)
        .def("facetDrop", &CylConeCutter::facetDrop)
        .def("edgeDrop", &CylConeCutter::edgeDrop)
    ;
    bp::class_<BallConeCutter, bp::bases<MillingCutter> >("BallConeCutter")
        .def(bp::init<double, double, double>())
        .def("vertexDrop", &BallConeCutter::vertexDrop)
        .def("facetDrop", &BallConeCutter::facetDrop)
        .def("edgeDrop", &BallConeCutter::edgeDrop)
    ;
    bp::class_<BullConeCutter, bp::bases<MillingCutter> >("BullConeCutter")
        .def(bp::init<double, double, double, double>())
        .def("vertexDrop", &BullConeCutter::vertexDrop)
        .def("facetDrop", &BullConeCutter::facetDrop)
        .def("edgeDrop", &BullConeCutter::edgeDrop)
    ;
    bp::class_<ConeConeCutter, bp::bases<MillingCutter> >("ConeConeCutter")
        .def(bp::init<double, double, double, double>())
        .def("vertexDrop", &ConeConeCutter::vertexDrop)
        .def("facetDrop", &ConeConeCutter::facetDrop)
        .def("edgeDrop", &ConeConeCutter::edgeDrop)
    ;
    bp::class_<Epos>("Epos")
        .def("setS", &Epos::setS)
        .def("setT", &Epos::setT)
        .def("stepTangent", &Epos::stepTangent)
        .def_readwrite("s", &Epos::s)
        .def_readwrite("t", &Epos::t)
        .def("str", &Epos::str)
    ;
    bp::class_<Ellipse>("Ellipse")
        .def(bp::init<Point&, double, double, double>())
        .def("ePoint", &Ellipse::ePoint)
        .def("oePoint", &Ellipse::oePoint)
        .def("normal", &Ellipse::normal)
        .def("tangent", &Ellipse::tangent)
        .def("error", &Ellipse::error)
        .def("solver", &Ellipse::solver)
        .def_readonly("epos1", &Ellipse::epos1)
        .def_readonly("epos2", &Ellipse::epos2)
        .def_readonly("center", &Ellipse::center)
    ;
    bp::class_<BatchDropCutter>("BatchDropCutter")
        .def("dropCutter1", &BatchDropCutter::dropCutter1)
        .def("dropCutter2", &BatchDropCutter::dropCutter2)
        .def("dropCutter3", &BatchDropCutter::dropCutter3)
        .def("dropCutter4", &BatchDropCutter::dropCutter4)
        .def("getCLPoints", &BatchDropCutter::getCLPoints)
        .def("getCCPoints", &BatchDropCutter::getCCPoints)
        .def("setSTL", &BatchDropCutter::setSTL)
        .def("setCutter", &BatchDropCutter::setCutter)
        .def("appendPoint", &BatchDropCutter::appendPoint)
        .def("getTrianglesUnderCutter", &BatchDropCutter::getTrianglesUnderCutter)
        .def_readonly("dcCalls", &BatchDropCutter::dcCalls)
        .def_readwrite("nthreads", &BatchDropCutter::nthreads)
    ;
    bp::class_<Ocode>("Ocode")
        .def(bp::init<>())
        .def(bp::init<Ocode>())
        .def("point", &Ocode::point)
        .def("corner", &Ocode::corner)
        .def("degree", &Ocode::degree)
        .def("get_scale", &Ocode::get_scale)
        .def("set_scale", &Ocode::set_scale)
        .def("get_depth", &Ocode::get_depth)
        .def("set_depth", &Ocode::set_depth)
        .def("containedIn", &Ocode::containedIn)
        .def("str", &Ocode::str)
    ;
    bp::class_<LinOCT>("LinOCT")
        .def(bp::init<>())
        .def("append", &LinOCT::append)
        .def("size", &LinOCT::size)
        //.def("expand_at", &LinOCT::expand_at)
        .def("get_nodes", &LinOCT::get_nodes)
        //.def("delete_at", &LinOCT::delete_at)
        .def("build", &LinOCT::build)
        .def("init", &LinOCT::init)
        .def("sum", &LinOCT::sum)
        .def("diff", &LinOCT::diff)
        .def("operation", &LinOCT::operation)
        .def("sort", &LinOCT::sort)
        .def("condense", &LinOCT::condense)
        .def("str", &LinOCT::str)
    ;
    bp::class_<OCTVolumeWrap, boost::noncopyable>("OCTVolume", bp::no_init)
        .def("isInside", bp::pure_virtual(&OCTVolume::isInside) )
    ;
    bp::class_<SphereOCTVolume, bp::bases<OCTVolume> >("SphereOCTVolume")
        .def("isInside", &SphereOCTVolume::isInside )
        .def("isInsideBB", &SphereOCTVolume::isInsideBB )
        .def("calcBB", &SphereOCTVolume::calcBB )
        .def_readwrite("center", &SphereOCTVolume::center)
        .def_readwrite("radius", &SphereOCTVolume::radius)
        .def_readwrite("bb", &SphereOCTVolume::bb )
    ;
    bp::class_<CubeOCTVolume, bp::bases<OCTVolume> >("CubeOCTVolume")
        .def("isInside", &CubeOCTVolume::isInside )
        .def("calcBB", &CubeOCTVolume::calcBB )
        .def_readwrite("center", &CubeOCTVolume::center)
        .def_readwrite("side", &CubeOCTVolume::side)
        .def_readwrite("bb", &CubeOCTVolume::bb )
    ;
    bp::class_<CylinderOCTVolume, bp::bases<OCTVolume> >("CylinderOCTVolume")
        .def("isInside", &CylinderOCTVolume::isInside )
        .def("calcBB", &CylinderOCTVolume::calcBB )
        .def_readwrite("p1", &CylinderOCTVolume::p1)
        .def_readwrite("p2", &CylinderOCTVolume::p2)
        .def_readwrite("radius", &CylinderOCTVolume::radius)
        .def_readwrite("bb", &CylinderOCTVolume::bb )
    ;
    bp::class_<CylMoveOCTVolume, bp::bases<OCTVolume> >("CylMoveOCTVolume")
        .def(bp::init<CylCutter, Point, Point>())
        .def("isInside", &CylMoveOCTVolume::isInside )
        .def_readwrite("p1", &CylMoveOCTVolume::p1)
        .def_readwrite("p2", &CylMoveOCTVolume::p2)
        .def_readwrite("bb", &CylMoveOCTVolume::bb)
        .def_readwrite("box", &CylMoveOCTVolume::box)
    ;
    bp::class_<BoxOCTVolume, bp::bases<OCTVolume> >("BoxOCTVolume")
        .def("isInside", &BoxOCTVolume::isInside )
        .def_readwrite("corner", &BoxOCTVolume::corner)
        .def_readwrite("v1", &BoxOCTVolume::v1)
        .def_readwrite("v2", &BoxOCTVolume::v2)
        .def_readwrite("v3", &BoxOCTVolume::v3)
    ;
    bp::class_<Bbox>("Bbox")
        .def("isInside", &Bbox::isInside )
        .def_readwrite("maxx", &Bbox::maxx)
        .def_readwrite("minx", &Bbox::minx)
        .def_readwrite("maxy", &Bbox::maxy)
        .def_readwrite("miny", &Bbox::miny)
        .def_readwrite("maxz", &Bbox::maxz)
        .def_readwrite("minz", &Bbox::minz)
    ;
    bp::class_<Line>("Line")
        .def(bp::init<Point,Point>())
        .def(bp::init<Line>())
    ;
    bp::class_<Arc>("Arc")
        .def(bp::init<Point,Point,Point,bool>())
        .def(bp::init<Arc>())
    ;
    bp::class_<Path>("Path")
        .def(bp::init<>())
        .def(bp::init<Path>())
        .def("getSpans", &Path::getSpans)
        .def("append",static_cast< void (Path::*)(const Line &l)>(&Path::append))
        .def("append",static_cast< void (Path::*)(const Arc &a)>(&Path::append))
    ;
    bp::class_<PathDropCutterFinish>("PathDropCutterFinish")
        .def(bp::init<STLSurf*>())
        .def(bp::init<PathDropCutterFinish>())
        .def("getCLPoints", &PathDropCutterFinish::getCLPoints)
        .def("run",static_cast< void (PathDropCutterFinish::*)(void)>(&PathDropCutterFinish::run))
        .def("setCutter", &PathDropCutterFinish::setCutter)
        .def("setPath", &PathDropCutterFinish::setPath)
        .def_readwrite("minimumZ", &PathDropCutterFinish::minimumZ)
    ;
}

