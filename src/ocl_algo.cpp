/*  $Id$
 * 
 *  Copyright (c) 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com).
 *  
 *  This file is part of OpenCAMlib 
 *  (see https://github.com/aewallin/opencamlib).
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <boost/python.hpp>


#include "batchpushcutter_py.hpp"  
#include "fiber_py.hpp"           
#include "weave_py.hpp"           
#include "waterline_py.hpp"      
#include "adaptivewaterline_py.hpp"  
#include "lineclfilter_py.hpp"    
#include "numeric.hpp"

#include "zigzag.hpp"

#include "clsurface.hpp"

#include "tsp.hpp" // fixme: contains python

/*
 *  Python wrapping of octree and related classes
 */

using namespace ocl;

namespace bp = boost::python;

void export_algo() {
    bp::def("eps", eps); // machine epsilon, see numeric.cpp
    bp::def("epsF", epsF);
    bp::def("epsD", epsD);
    bp::class_<ZigZag>("ZigZag")
        .def("run", &ZigZag::run)
        .def("setDirection", &ZigZag::setDirection)
        .def("setOrigin", &ZigZag::setOrigin)
        .def("setStepOver", &ZigZag::setStepOver)
        .def("addPoint", &ZigZag::addPoint)
        .def("getOutput", &ZigZag::getOutput)
        .def("__str__", &ZigZag::str)
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
        .def("run2", &Waterline_py::run2)
        .def("reset", &Waterline_py::reset)
        .def("getLoops", &Waterline_py::py_getLoops)
        .def("setThreads", &Waterline_py::setThreads)
        .def("getThreads", &Waterline_py::getThreads)
        .def("getXFibers", &Waterline_py::py_getXFibers)
        .def("getYFibers", &Waterline_py::py_getYFibers)
        
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
        .def("run2", &AdaptiveWaterline_py::run2)
        .def("reset", &AdaptiveWaterline_py::reset)
        //.def("run2", &AdaptiveWaterline_py::run2) // uses Weave::build2()
        .def("getLoops", &AdaptiveWaterline_py::py_getLoops)
        .def("setThreads", &AdaptiveWaterline_py::setThreads)
        .def("getThreads", &AdaptiveWaterline_py::getThreads)
        .def("getXFibers", &AdaptiveWaterline_py::getXFibers)
        .def("getYFibers", &AdaptiveWaterline_py::getYFibers)
    ;
    
    bp::enum_<weave::VertexType>("WeaveVertexType")
        .value("CL", weave::CL)
        .value("CL_DONE",weave::CL_DONE)
        .value("ADJ",weave::ADJ)
        .value("TWOADJ",weave::TWOADJ)
        .value("INT",weave::INT)
        .value("FULLINT",weave::FULLINT)
    ;
    
    
    /*
    bp::class_<weave::Weave>("Weave_base")
    ;
    bp::class_<weave::Weave_py, bp::bases<weave::Weave> >("Weave")
        .def("addFiber", &weave::Weave_py::addFiber)
        .def("build", &weave::Weave_py::build)
        .def("build2", &weave::Weave_py::build2)
        .def("printGraph", &weave::Weave_py::printGraph)
        .def("face_traverse", &weave::Weave_py::face_traverse)
        //.def("split_components", &weave::Weave_py::split_components)
        //.def("get_components", &weave::Weave_py::get_components)
        .def("getCLVertices", &weave::Weave_py::getCLVertices)
        .def("getINTVertices", &weave::Weave_py::getINTVertices)
        .def("getVertices", &weave::Weave_py::getVertices)
        .def("numVertices", &weave::Weave_py::numVertices)
        .def("getEdges", &weave::Weave_py::getEdges)
        .def("getLoops", &weave::Weave_py::py_getLoops)
        .def("__str__", &weave::Weave_py::str)
    ;
    */
    
    bp::class_<LineCLFilter>("LineCLFilter_base")
    ;
    bp::class_<LineCLFilter_py, bp::bases<LineCLFilter> >("LineCLFilter")
        .def("addCLPoint",  &LineCLFilter_py::addCLPoint)
        .def("setTolerance",&LineCLFilter_py::setTolerance)
        .def("run",         &LineCLFilter_py::run)
        .def("getCLPoints", &LineCLFilter_py::getCLPoints)
    ;

    // some strange problem with hedi::face_edges()... let's not compile for now..
    bp::class_< clsurf::CutterLocationSurface >("CutterLocationSurface")  
        .def(bp::init<double>())
        .def("run", &clsurf::CutterLocationSurface::run)
        .def("setMinSampling", &clsurf::CutterLocationSurface::setMinSampling)
        .def("setSampling", &clsurf::CutterLocationSurface::setSampling)
        .def("setSTL", &clsurf::CutterLocationSurface::setSTL)
        .def("setCutter", &clsurf::CutterLocationSurface::setCutter)
        .def("getVertices", &clsurf::CutterLocationSurface::getVertices)
        .def("getEdges", &clsurf::CutterLocationSurface::getEdges)
        .def("__str__", &clsurf::CutterLocationSurface::str)
    ;
/*
    bp::class_< tsp::TSPSolver >("TSPSolver")  
        .def("addPoint", &tsp::TSPSolver::addPoint)
        .def("run", &tsp::TSPSolver::run)
        .def("getOutput", &tsp::TSPSolver::getOutput)
        .def("getLength", &tsp::TSPSolver::getLength)
        .def("reset", &tsp::TSPSolver::reset)
    ;
    */
}

