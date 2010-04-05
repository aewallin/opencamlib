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
        .value("FACET", FACET)
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
    bp::class_<OCTNode>("OCTNode")
        .def_readonly("scale", &OCTNode::scale)
        .def("get_max_scale", &OCTNode::get_max_scale)
        .def("nodePoint", &OCTNode::nodePoint)
        .def("str", &OCTNode::str)
    ;
    bp::class_<ParallelFinish>("ParallelFinish")
        .def("initCLPoints", &ParallelFinish::initCLpoints)
        .def("dropCutterSTL1", &ParallelFinish::dropCutterSTL1)
        .def("dropCutterSTL2", &ParallelFinish::dropCutterSTL2)
        .def("getCLPoints", &ParallelFinish::getCLPoints)
        .def("getCCPoints", &ParallelFinish::getCCPoints)
        .def("initSTLSurf", &ParallelFinish::initSTLSurf)
        .def("getTrianglesUnderCutter", &ParallelFinish::getTrianglesUnderCutter)
        .def_readonly("dcCalls", &ParallelFinish::dcCalls)
    ;
    /*
    bp::class_<KDNode>("KDNode", bp::no_init) 
        .def("stlSurf2KDTree", &KDNode::stlSurf2KDTree)
        .def("str", &KDNode::str)
    ;
    */
   /* bp::class_<Spread>("Spread", bp::no_init)
        .def(bp::init<int, double, double>())
    ;*/
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

