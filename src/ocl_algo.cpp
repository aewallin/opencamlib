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

#include "batchdropcutter.h"
#include "batchpushcutter.h"
#include "pathdropcutter.h"
#include "fiber.h"
#include "weave.h"
#include "waterline.h"
#include "clfilter.h"

/*
 *  Python wrapping of octree and related classes
 */

using namespace ocl;

namespace bp = boost::python;

void export_algo() {
    bp::class_<BatchDropCutter>("BatchDropCutter")
        .def("dropCutter1", &BatchDropCutter::dropCutter1)
        .def("dropCutter2", &BatchDropCutter::dropCutter2)
        .def("dropCutter3", &BatchDropCutter::dropCutter3)
        .def("dropCutter4", &BatchDropCutter::dropCutter4)
        .def("dropCutter5", &BatchDropCutter::dropCutter5)
        .def("getCLPoints", &BatchDropCutter::getCLPoints)
        .def("setSTL", &BatchDropCutter::setSTL)
        .def("setCutter", &BatchDropCutter::setCutter)
        .def("setThreads", &BatchDropCutter::setThreads)
        .def("appendPoint", &BatchDropCutter::appendPoint)
        .def("getTrianglesUnderCutter", &BatchDropCutter::getTrianglesUnderCutter)
        .def_readonly("dcCalls", &BatchDropCutter::dcCalls)
        .def_readwrite("nthreads", &BatchDropCutter::nthreads)
        .def_readwrite("bucketSize", &BatchDropCutter::bucketSize)
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
    bp::class_<Fiber>("Fiber")
        .def(bp::init<Point,Point>())
        .def_readonly("p1", &Fiber::p1)
        .def_readonly("p2", &Fiber::p2)
        .def_readonly("dir", &Fiber::dir)
        .def("addInterval", &Fiber::addInterval)
        .def("point", &Fiber::point)
        .def("printInts", &Fiber::printInts)
        .def("getInts", &Fiber::getInts)
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

