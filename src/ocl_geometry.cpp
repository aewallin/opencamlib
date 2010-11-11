/*  $Id$
 * 
 *  Copyright 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com)
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

#include <boost/python.hpp>

#include "point.h"              // contains no python-specific code
#include "ccpoint.h"            // no python
#include "clpoint.h"            // no python
#include "triangle_py.h"        // new-style python wrapper-class
#include "stlsurf_py.h"         // new-style wrapper
#include "ellipse.h"           // no python
#include "ellipseposition.h"
#include "bbox.h"               // no python
#include "path_py.h"            // new-style wrapper
#include "stlreader.h"          // no python

/*
 *  Python wrapping
 */

using namespace ocl;

namespace bp = boost::python;


void export_geometry() {
    bp::class_<Point>("Point") 
        .def(bp::init<double, double, double>())
        .def(bp::init<double, double>())
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
        .def("__str__", &Point::str)
        .def_readwrite("x", &Point::x)
        .def_readwrite("y", &Point::y)
        .def_readwrite("z", &Point::z)
    ;
    bp::class_<CLPoint>("CLPoint")  // FIXME: should inherit from Point
        .def(bp::init<CLPoint>())
        .def(bp::init<double, double, double>())
        .def(bp::init<double, double, double, CCPoint&>())
        .def("__str__", &CLPoint::str)
        .def_readwrite("x", &CLPoint::x)
        .def_readwrite("y", &CLPoint::y)
        .def_readwrite("z", &CLPoint::z)
        .def("cc", &CLPoint::getCC)
        .def("getCC", &CLPoint::getCC)
    ;   
    bp::class_<CCPoint>("CCPoint")  // FIXME: CCPoint should inherit from Point
        .def(bp::init<CCPoint>())
        .def(bp::init<double, double, double>())
        .def("__str__", &CCPoint::str)
        .def_readwrite("type", &CCPoint::type)
        .def_readwrite("x", &CCPoint::x)
        .def_readwrite("y", &CCPoint::y)
        .def_readwrite("z", &CCPoint::z)
    ;
    bp::enum_<CCType>("CCType")
        .value("NONE", NONE)
        .value("VERTEX",VERTEX)
        .value("VERTEX_CYL",VERTEX_CYL)
        .value("EDGE",EDGE)
        .value("EDGE_SHAFT",EDGE_SHAFT)
        .value("EDGE_HORIZ",EDGE_HORIZ)
        .value("EDGE_CYL",EDGE_CYL)
        .value("EDGE_BALL",EDGE_BALL)
        .value("EDGE_HORIZ_CYL",EDGE_HORIZ_CYL)
        .value("EDGE_HORIZ_TOR",EDGE_HORIZ_TOR)
        .value("EDGE_POS",EDGE_POS)
        .value("EDGE_NEG",EDGE_NEG)
        .value("FACET", FACET)
        .value("FACET_TIP", FACET_TIP)
        .value("FACET_CYL", FACET_CYL)
        .value("ERROR", ERROR)
    ;
    bp::class_<Triangle>("Triangle_base") // needed by Triangle_py as a base-class
    ;
    bp::class_<Triangle_py, bp::bases<Triangle> >("Triangle")
        .def(bp::init<Point,Point,Point>())
        .def("getPoints", &Triangle_py::getPoints)
        .def("__str__", &Triangle_py::str) 
        .def_readonly("p", &Triangle_py::p)
    ;
    bp::class_<STLSurf>("STLSurf_base") // needed by STLSurf_py below
    ;
    bp::class_<STLSurf_py, bp::bases<STLSurf> >("STLSurf")
        .def("addTriangle", &STLSurf_py::addTriangle)
        .def("__str__", &STLSurf_py::str)
        .def("size", &STLSurf_py::size)
        .def("getBounds", &STLSurf_py::getBounds)
        .def("getTriangles", &STLSurf_py::getTriangles)
        .def_readonly("tris", &STLSurf_py::tris)
        .def_readonly("bb", &STLSurf_py::bb)
    ;
    bp::class_<STLReader>("STLReader")
        .def(bp::init<const std::wstring&, STLSurf&>())
    ;
    bp::class_<Bbox>("Bbox")
        .def("isInside", &Bbox::isInside )
        .def_readonly("maxpt", &Bbox::maxpt)
        .def_readonly("minpt", &Bbox::minpt)
    ;
    // Epos and the Ellipse are used for the toroidal tool edge-tests
    bp::class_<EllipsePosition>("EllipsePosition")
        //.def("setD", &Epos::setD)
        .def_readwrite("s", &EllipsePosition::s)
        .def_readwrite("t", &EllipsePosition::t)
        //.def_readwrite("d", &Epos::diangle)
        .def("__str__", &EllipsePosition::str)
    ;
    bp::class_<Ellipse>("Ellipse")
        .def(bp::init<Point&, double, double, double>())
        .def("ePoint", &Ellipse::ePoint)
        .def("oePoint", &Ellipse::oePoint)
        .def("normal", &Ellipse::normal)
        //.def("tangent", &Ellipse::tangent)
        //.def("error", &Ellipse::error)
        //.def_readonly("epos1", &Ellipse::epos1)
        //.def_readonly("epos2", &Ellipse::epos2)
        //.def_readonly("center", &Ellipse::center)
    ;
    bp::class_<Line>("Line")
        .def(bp::init<Point,Point>())
        .def(bp::init<Line>())
        .def_readwrite("p1", &Line::p1)
        .def_readwrite("p2", &Line::p2)
    ;
    bp::class_<Arc>("Arc")
        .def(bp::init<Point,Point,Point,bool>())
        .def(bp::init<Arc>())
        .def_readwrite("p1", &Arc::p1)
        .def_readwrite("p2", &Arc::p2)
        .def_readwrite("c",  &Arc::c)
        .def_readwrite("dir",  &Arc::dir)
    ;
    bp::enum_<SpanType>("SpanType")
        .value("LineSpanType", LineSpanType)
        .value("ArcSpanType", ArcSpanType)
        .export_values()
    ;
    bp::class_<Path>("Path_base")
    ;
    bp::class_<Path_py, bp::bases<Path> >("Path")
        .def(bp::init<>())
        .def(bp::init<Path>())
        .def("getSpans", &Path_py::getSpans)
        .def("getTypeSpanPairs", &Path_py::getTypeSpanPairs)
        .def("append",static_cast< void (Path_py::*)(const Line &l)>(&Path_py::append))
        .def("append",static_cast< void (Path_py::*)(const Arc &a)>(&Path_py::append))
    ;
}

