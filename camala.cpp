/*  Copyright 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com)
 *  
 *  This file is part of Camala.
 *
 *  Camala is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Camala is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Camala.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "camala.h"

/*
 *  Python wrapping
 */

using namespace boost::python;

BOOST_PYTHON_MODULE(camala) {

    class_<Point>("Point") 
		.def(init<double, double, double>())
		.def(init<Point>())
		.def(other<double>() * self)
		.def(self * other<double>())
		.def(self -= other<Point>())
		.def(self - other<Point>())
		.def(self += other<Point>())
		.def(self + other<Point>())
		.def("norm", &Point::norm)
		.def("normalize", &Point::normalize)
		.def("dot", &Point::dot)
		.def("cross", &Point::cross)
        .def("isRight", &Point::isRight)
        .def("isInside", &Point::isInside)
        .def("xyDistance", &Point::xyDistance)
		.def("str", &Point::str)
		.def_readwrite("x", &Point::x)
		.def_readwrite("y", &Point::y)
		.def_readwrite("z", &Point::z)
		.def_readonly("id", &Point::id)
    ;   
    
    class_<Triangle>("Triangle")
		.def(init<Point,Point,Point>())
		.def("str", &Triangle::str) // FIXME
		.def_readonly("p", &Triangle::p)
		.def_readonly("n", &Triangle::n)
		.def_readonly("id", &Triangle::id)
	;
	
	class_<STLSurf>("STLSurf")
		.def("addTriangle", &STLSurf::addTriangle)
		.def("str", &STLSurf::str)
		.def_readonly("tris", &STLSurf::tris)
		.def_readonly("id", &STLSurf::id)
	;
    class_<CylCutter>("CylCutter")
        .def(init<double>())
        .def("vertexDrop", &CylCutter::vertexDrop)
        .def("facetDrop", &CylCutter::facetDrop)
        .def("edgeDrop", &CylCutter::edgeDrop)
        .def("str", &CylCutter::str)
        .add_property("diameter", &CylCutter::getDiameter, &CylCutter::setDiameter )
    ;
}

