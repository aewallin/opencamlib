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

#include "batchdropcutter_py.h" 
#include "batchpushcutter_py.h"  
#include "pathdropcutter_py.h"  
#include "adaptivepathdropcutter_py.h"  
#include "fiber_py.h"           
#include "weave_py.h"           
#include "waterline_py.h"      
#include "adaptivewaterline_py.h"  
#include "lineclfilter_py.h"    
#include "numeric.h"
#include "voronoidiagram.h"  // FIX
#include "zigzag.h"

/*
 *  Python wrapping of octree and related classes
 */

using namespace ocl;

namespace bp = boost::python;

void export_algo() {
    bp::def("eps", eps); // machine epsilon, see numeric.cpp
    bp::def("epsF", epsF);
    bp::def("epsD", epsD);
    bp::def("revision", revision); // returns OCL revision string to python
    bp::class_<ZigZag>("ZigZag")
        .def("run", &ZigZag::run)
        .def("setDirection", &ZigZag::setDirection)
        .def("setOrigin", &ZigZag::setOrigin)
        .def("setStepOver", &ZigZag::setStepOver)
        .def("addPoint", &ZigZag::addPoint)
        .def("getOutput", &ZigZag::getOutput)
        .def("__str__", &ZigZag::str)
    ;
    bp::class_<BatchDropCutter>("BatchDropCutter_base")
    ;
    bp::class_<BatchDropCutter_py, bp::bases<BatchDropCutter> >("BatchDropCutter")
        .def("run", &BatchDropCutter_py::run)
        .def("getCLPoints", &BatchDropCutter_py::getCLPoints_py)
        .def("setSTL", &BatchDropCutter_py::setSTL)
        .def("setCutter", &BatchDropCutter_py::setCutter)
        .def("setThreads", &BatchDropCutter_py::setThreads)
        .def("getThreads", &BatchDropCutter_py::getThreads)
        .def("appendPoint", &BatchDropCutter_py::appendPoint)
        .def("getTrianglesUnderCutter", &BatchDropCutter_py::getTrianglesUnderCutter)
        .def("getCalls", &BatchDropCutter_py::getCalls)
        .def("getBucketSize", &BatchDropCutter_py::getBucketSize)
        .def("setBucketSize", &BatchDropCutter_py::setBucketSize)
    ;
    bp::class_<BatchPushCutter>("BatchPushCutter_base")
    ;
    bp::class_<BatchPushCutter_py, bp::bases<BatchPushCutter> >("BatchPushCutter")
        .def("run", &BatchPushCutter_py::run)
        .def("setSTL", &BatchPushCutter_py::setSTL)
        .def("setCutter", &BatchPushCutter_py::setCutter)
        .def("setThreads", &BatchPushCutter_py::setThreads)
        .def("appendFiber", &BatchPushCutter_py::appendFiber)
        .def("getOverlapTriangles", &BatchPushCutter_py::getOverlapTriangles)
        .def("getCLPoints", &BatchPushCutter_py::getCLPoints)
        .def("getFibers", &BatchPushCutter_py::getFibers_py)
        .def("getCalls", &BatchPushCutter_py::getCalls)
        .def("setThreads", &BatchPushCutter_py::setThreads)
        .def("getThreads", &BatchPushCutter_py::getThreads)
        .def("setBucketSize", &BatchPushCutter_py::setBucketSize)
        .def("getBucketSize", &BatchPushCutter_py::getBucketSize)
        .def("setXDirection", &BatchPushCutter_py::setXDirection)
        .def("setYDirection", &BatchPushCutter_py::setYDirection)
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
    bp::class_<Waterline>("Waterline_base")
    ;
    bp::class_<Waterline_py, bp::bases<Waterline> >("Waterline")
        .def("setCutter", &Waterline_py::setCutter)
        .def("setSTL", &Waterline_py::setSTL)
        .def("setZ", &Waterline_py::setZ)
        .def("setSampling", &Waterline_py::setSampling)
        .def("run", &Waterline_py::run)
        .def("getLoops", &Waterline_py::py_getLoops)
        .def("setThreads", &Waterline_py::setThreads)
        .def("getThreads", &Waterline_py::getThreads)
        .def("getXFibers", &Waterline_py::getXFibers)
        .def("getYFibers", &Waterline_py::getYFibers)
        
    ;
    bp::class_<AdaptiveWaterline>("AdaptiveWaterline_base")
    ;
    bp::class_<AdaptiveWaterline_py, bp::bases<AdaptiveWaterline> >("AdaptiveWaterline")
        .def("setCutter", &AdaptiveWaterline_py::setCutter)
        .def("setSTL", &AdaptiveWaterline_py::setSTL)
        .def("setZ", &AdaptiveWaterline_py::setZ)
        .def("setSampling", &AdaptiveWaterline_py::setSampling)
        .def("setMinSampling", &AdaptiveWaterline_py::setMinSampling)
        .def("run", &AdaptiveWaterline_py::run)
        .def("getLoops", &AdaptiveWaterline_py::py_getLoops)
        .def("setThreads", &AdaptiveWaterline_py::setThreads)
        .def("getThreads", &AdaptiveWaterline_py::getThreads)
        .def("getXFibers", &AdaptiveWaterline_py::getXFibers)
        .def("getYFibers", &AdaptiveWaterline_py::getYFibers)
    ;
    bp::class_<Weave>("Weave_base")
    ;
    bp::class_<Weave_py, bp::bases<Weave> >("Weave")
        .def("addFiber", &Weave_py::addFiber)
        .def("build", &Weave_py::build)
        .def("printGraph", &Weave_py::printGraph)
        .def("face_traverse", &Weave_py::face_traverse)
        .def("split_components", &Weave_py::split_components)
        .def("get_components", &Weave_py::get_components)
        .def("getCLPoints", &Weave_py::getCLPoints)
        .def("getIPoints", &Weave_py::getIPoints)
        .def("getEdges", &Weave_py::getEdges)
        .def("getLoops", &Weave_py::py_getLoops)
        .def("__str__", &Weave_py::str)
    ;
    bp::class_<PathDropCutter>("PathDropCutter_base")
    ;
    bp::class_<PathDropCutter_py , bp::bases<PathDropCutter> >("PathDropCutter")
        .def("run", &PathDropCutter_py::run)
        .def("getCLPoints", &PathDropCutter_py::getCLPoints_py)
        .def("setCutter", &PathDropCutter_py::setCutter)
        .def("setSTL", &PathDropCutter_py::setSTL)
        .def("setSampling", &PathDropCutter_py::setSampling)
        .def("setPath", &PathDropCutter_py::setPath)
        .def("getZ", &PathDropCutter_py::getZ)
        .def("setZ", &PathDropCutter_py::setZ)
    ;
    bp::class_<AdaptivePathDropCutter>("AdaptivePathDropCutter_base")
    ;
    bp::class_<AdaptivePathDropCutter_py , bp::bases<AdaptivePathDropCutter> >("AdaptivePathDropCutter")
        .def("run", &AdaptivePathDropCutter_py::run)
        .def("getCLPoints", &AdaptivePathDropCutter_py::getCLPoints_py)
        .def("setCutter", &AdaptivePathDropCutter_py::setCutter)
        .def("setSTL", &AdaptivePathDropCutter_py::setSTL)
        .def("setSampling", &AdaptivePathDropCutter_py::setSampling)
        .def("setMinSampling", &AdaptivePathDropCutter_py::setMinSampling)
        .def("setCosLimit", &AdaptivePathDropCutter_py::setCosLimit)
        .def("getSampling", &AdaptivePathDropCutter_py::getSampling)
        .def("setPath", &AdaptivePathDropCutter_py::setPath)
        .def("getZ", &AdaptivePathDropCutter_py::getZ)
        .def("setZ", &AdaptivePathDropCutter_py::setZ)
    ;
    bp::class_<LineCLFilter>("LineCLFilter_base")
    ;
    bp::class_<LineCLFilter_py, bp::bases<LineCLFilter> >("LineCLFilter")
        .def("addCLPoint",  &LineCLFilter_py::addCLPoint)
        .def("setTolerance",&LineCLFilter_py::setTolerance)
        .def("run",         &LineCLFilter_py::run)
        .def("getCLPoints", &LineCLFilter_py::getCLPoints)
    ;
    bp::class_<VoronoiDiagram >("VoronoiDiagram")
        .def(bp::init<double, unsigned int>())
        //.def(bp::init<double>())
        .def("addVertexSite",  &VoronoiDiagram::addVertexSite)
        .def("getGenerators",  &VoronoiDiagram::getGenerators)
        .def("getEdgesGenerators",  &VoronoiDiagram::getEdgesGenerators)
        .def("getVoronoiVertices",  &VoronoiDiagram::getVoronoiVertices)
        .def("getFarVoronoiVertices",  &VoronoiDiagram::getFarVoronoiVertices)
        .def("getFarRadius",  &VoronoiDiagram::getFarRadius)
        .def("getVoronoiEdges",  &VoronoiDiagram::getVoronoiEdges)
        .def("setDelaunayTriangulation",  &VoronoiDiagram::setDelaunayTriangulation)
        .def("getDelaunayEdges",  &VoronoiDiagram::getDelaunayEdges)
        .def("getClosestFaceGenerator",  &VoronoiDiagram::getClosestFaceGenerator)
        .def("getSeedVertex",  &VoronoiDiagram::getSeedVertex)
        .def("__str__", &VoronoiDiagram::str)
    ;
    bp::enum_<VoronoiVertexType>("VoronoiVertexType")
        .value("OUT", OUT)   
        .value("IN", IN)
        .value("UNDECIDED", UNDECIDED)
        .value("NEW", NEW)
    ;
    bp::enum_<VoronoiFaceType>("VoronoiFaceType")
        .value("INCIDENT", INCIDENT)
        .value("NONINCIDENT", NONINCIDENT)
    ;
}

