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


#include "millingcutter.h"

#include "batchdropcutter_py.h" // new-style wrapper
#include "batchpushcutter.h" 
#include "pathdropcutter.h"
#include "fiber_py.h"           // new-style wrapper
#include "weave.h"
#include "waterline.h"
#include "clfilter.h"

/*
 *  Python wrapping of octree and related classes
 */

using namespace ocl;

namespace bp = boost::python;

void export_algo() {
    bp::class_<BatchDropCutter>("BatchDropCutter_base")
    ;
    bp::class_<BatchDropCutter_py, bp::bases<BatchDropCutter> >("BatchDropCutter")
        .def("dropCutter1", &BatchDropCutter_py::dropCutter1)
        .def("dropCutter2", &BatchDropCutter_py::dropCutter2)
        .def("dropCutter3", &BatchDropCutter_py::dropCutter3)
        .def("dropCutter4", &BatchDropCutter_py::dropCutter4)
        .def("dropCutter5", &BatchDropCutter_py::dropCutter5)
        .def("getCLPoints", &BatchDropCutter_py::getCLPoints)
        .def("setSTL", &BatchDropCutter_py::setSTL)
        .def("setCutter", &BatchDropCutter_py::setCutter)
        .def("setThreads", &BatchDropCutter_py::setThreads)
        .def("appendPoint", &BatchDropCutter_py::appendPoint)
        .def("getTrianglesUnderCutter", &BatchDropCutter_py::getTrianglesUnderCutter)
        .def_readonly("dcCalls", &BatchDropCutter_py::dcCalls)
        .def_readwrite("nthreads", &BatchDropCutter_py::nthreads)
        .def_readwrite("bucketSize", &BatchDropCutter_py::bucketSize)
    ;
    bp::class_<BatchPushCutter>("BatchPushCutter")
        .def("pushCutter1", &BatchPushCutter::pushCutter1)
        .def("pushCutter2", &BatchPushCutter::pushCutter2)
        .def("pushCutter3", &BatchPushCutter::pushCutter3)
        .def("setSTL", &BatchPushCutter::setSTL)
        .def("setCutter", &BatchPushCutter::setCutter)
        .def("setThreads", &BatchPushCutter::setThreads)
        .def("appendFiber", &BatchPushCutter::appendFiber)
        .def("getOverlapTriangles", &BatchPushCutter::getOverlapTriangles)
        .def("getCLPoints", &BatchPushCutter::getCLPoints)
        .def("getFibers", &BatchPushCutter::getFibers)
        .def_readonly("nCalls", &BatchPushCutter::nCalls)
        .def_readwrite("nthreads", &BatchPushCutter::nthreads)
        .def_readwrite("bucketSize", &BatchPushCutter::bucketSize)
    ;
    bp::class_<Interval>("Interval")
        .def(bp::init<double, double>())
        .def_readonly("upper", &Interval::upper )
        .def_readonly("lower", &Interval::lower )
        .def_readonly("lower_cc", &Interval::lower_cc )
        .def_readonly("upper_cc", &Interval::upper_cc )
        .def("updateUpper", &Interval::updateUpper )
        .def("updateLower", &Interval::updateLower )
        .def("empty", &Interval::empty )
        .def("__str__", &Interval::str )
    ;
    bp::class_<Fiber>("Fiber_base")
    ;
    bp::class_<Fiber_py, bp::bases<Fiber> >("Fiber")
        .def(bp::init<Point,Point>())
        .def_readonly("p1", &Fiber_py::p1)
        .def_readonly("p2", &Fiber_py::p2)
        .def_readonly("dir", &Fiber_py::dir)
        .def("addInterval", &Fiber_py::addInterval)
        .def("point", &Fiber_py::point)
        .def("printInts", &Fiber_py::printInts)
        .def("getInts", &Fiber_py::getInts)
    ;
    bp::class_<Waterline>("Waterline")
        .def("setCutter", &Waterline::setCutter)
        .def("setSTL", &Waterline::setSTL)
        .def("setZ", &Waterline::setZ)
        .def("setTolerance", &Waterline::setTolerance)
        .def("run", &Waterline::run)
        .def("getLoops", &Waterline::py_getLoops)
    ;
    bp::class_<Weave>("Weave")
        .def("addFiber", &Weave::addFiber)
        .def("build", &Weave::build)
        .def("printGraph", &Weave::printGraph)
        .def("face_traverse", &Weave::face_traverse)
        .def("split_components", &Weave::split_components)
        .def("get_components", &Weave::get_components)
        .def("getCLPoints", &Weave::getCLPoints)
        .def("getIPoints", &Weave::getIPoints)
        .def("getEdges", &Weave::getEdges)
        .def("getLoops", &Weave::py_getLoops)
        .def("__str__", &Weave::str)
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

