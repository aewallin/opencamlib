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

#include "batchdropcutter_py.hpp" 
#include "pathdropcutter_py.hpp"  
#include "adaptivepathdropcutter_py.hpp"  


/*
 *  Python wrapping of octree and related classes
 */

using namespace ocl;

namespace bp = boost::python;

void export_dropcutter() {

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


}

