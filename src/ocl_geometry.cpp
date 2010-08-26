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

#include "point.h"
#include "ccpoint.h"
#include "clpoint.h"
#include "triangle.h"
#include "stlsurf.h"
#include "oellipse.h"
#include "millingcutter.h"
#include "bbox.h"
#include "fiber.h"
#include "path.h"
#include "stlreader.h"

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
        .value("EDGE_CYL",EDGE_CYL)
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
        .def("__str__", &Triangle::str) 
        .def_readonly("p", &Triangle::p)
    ;
    bp::class_<STLSurf>("STLSurf")
        .def("addTriangle", &STLSurf::addTriangle)
        .def("__str__", &STLSurf::str)
        .def("size", &STLSurf::size)
        .def("getBounds", &STLSurf::getBounds)
        .def("getTriangles", &STLSurf::getTriangles)
        .def_readonly("tris", &STLSurf::tris)
        .def_readonly("bb", &STLSurf::bb)
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
    bp::class_<Epos>("Epos")
        .def("setD", &Epos::setD)
        .def_readwrite("s", &Epos::s)
        .def_readwrite("t", &Epos::t)
        .def_readwrite("d", &Epos::diangle)
        .def("__str__", &Epos::str)
    ;
    bp::class_<Ellipse>("Ellipse")
        .def(bp::init<Point&, double, double, double>())
        .def("ePoint", &Ellipse::ePoint)
        .def("oePoint", &Ellipse::oePoint)
        .def("normal", &Ellipse::normal)
        .def("tangent", &Ellipse::tangent)
        //.def("error", &Ellipse::error)
        .def_readonly("epos1", &Ellipse::epos1)
        .def_readonly("epos2", &Ellipse::epos2)
        .def_readonly("center", &Ellipse::center)
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
    bp::class_<Path>("Path")
        .def(bp::init<>())
        .def(bp::init<Path>())
        .def("getSpans", &Path::getSpans)
        .def("getTypeSpanPairs", &Path::getTypeSpanPairs)
        .def("append",static_cast< void (Path::*)(const Line &l)>(&Path::append))
        .def("append",static_cast< void (Path::*)(const Arc &a)>(&Path::append))
    ;
}

