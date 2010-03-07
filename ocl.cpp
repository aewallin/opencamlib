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
        .def_readonly("type", &CCPoint::type)
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
        .def("str", &Triangle::str) // FIXME
        .def_readonly("p", &Triangle::p)
        //.def_readonly("n", &Triangle::n)
        .def_readonly("id", &Triangle::id)
    ;
    
    bp::class_<STLSurf>("STLSurf")
        .def("addTriangle", &STLSurf::addTriangle)
        .def("str", &STLSurf::str)
        .def_readonly("tris", &STLSurf::tris)
        .def_readonly("id", &STLSurf::id)
    ;

    bp::class_<CylCutter>("CylCutter")
        .def(bp::init<double>())
        .def("vertexDrop", &CylCutter::vertexDrop)
        .def("facetDrop", &CylCutter::facetDrop)
        .def("edgeDrop", &CylCutter::edgeDrop)
        .def("dropCutter", &CylCutter::dropCutter)
        .def("dropCutterSTL", &CylCutter::dropCutterSTL)
        .def("str", &CylCutter::str)
        .add_property("diameter", &CylCutter::getDiameter, &CylCutter::setDiameter )
    ;
}

