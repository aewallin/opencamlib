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
#include "ocl.h"
#include "revision.h"
#include <boost/python/docstring_options.hpp>

/// return the revision string
std::string revision() {
    return OCL_REV_STRING;
}

std::string ocl_docstring() {
    return "OpenCAMLib docstring";
}

/*
 *  Python wrapping
 */

using namespace ocl;

namespace bp = boost::python;

void export_cutters();
void export_geometry();
void export_octree();

BOOST_PYTHON_MODULE(ocl) {
    bp::docstring_options doc_options(false, false);
    //doc_options.disable_all();
    //doc_options.disable_py_signatures();
    bp::def("revision", revision);
    
    bp::def("__doc__", ocl_docstring);
    
    bp::def("eps", eps);
    
    export_geometry(); // see ocl_geometry.cpp
    
    export_cutters(); // see ocl_cutters.cpp
    
    export_octree(); // see ocl_octree.cpp
    
    bp::class_<STLReader>("STLReader")
        .def(bp::init<const std::wstring&, STLSurf&>())
    ;

    bp::class_<BatchDropCutter>("BatchDropCutter")
        .def("dropCutter1", &BatchDropCutter::dropCutter1)
        .def("dropCutter2", &BatchDropCutter::dropCutter2)
        .def("dropCutter3", &BatchDropCutter::dropCutter3)
        .def("dropCutter4", &BatchDropCutter::dropCutter4)
        .def("getCLPoints", &BatchDropCutter::getCLPoints)
        //.def("getCCPoints", &BatchDropCutter::getCCPoints)
        .def("setSTL", &BatchDropCutter::setSTL)
        .def("setCutter", &BatchDropCutter::setCutter)
        .def("appendPoint", &BatchDropCutter::appendPoint)
        .def("getTrianglesUnderCutter", &BatchDropCutter::getTrianglesUnderCutter)
        .def_readonly("dcCalls", &BatchDropCutter::dcCalls)
        .def_readwrite("nthreads", &BatchDropCutter::nthreads)
    ;

    bp::class_<Fiber>("Fiber")
        .def(bp::init<Point,Point>())
        .def_readonly("p1", &Fiber::p1)
        .def_readonly("p2", &Fiber::p2)
        .def_readonly("dir", &Fiber::dir)
        .def("addInterval", &Fiber::addInterval)
        //.def("calcDir", &Fiber::calcDir)
        //.def("addInt", &Fiber::addInt)
        //.def("condense", &Fiber::condense)
        .def("point", &Fiber::point)
        .def("printInts", &Fiber::printInts)
        .def("getInts", &Fiber::getInts)
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
    bp::class_<PathDropCutter>("PathDropCutter")
        .def(bp::init<STLSurf*>())
        .def(bp::init<PathDropCutter>())
        .def("getCLPoints", &PathDropCutter::getCLPoints)
        .def("run",static_cast< void (PathDropCutter::*)(void)>(&PathDropCutter::run))
        .def("setCutter", &PathDropCutter::setCutter)
        .def("setPath", &PathDropCutter::setPath)
        .def_readwrite("minimumZ", &PathDropCutter::minimumZ)
    ;
    bp::class_<LineCLFilter>("LineCLFilter")
        .def("addCLPoint",  &LineCLFilter::addCLPoint)
        .def("setTolerance",&LineCLFilter::setTolerance)
        .def("run",         &LineCLFilter::run)
        .def("getCLPoints", &LineCLFilter::getCLPoints)
    ;
}



