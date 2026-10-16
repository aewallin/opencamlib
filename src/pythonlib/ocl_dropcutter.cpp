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

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "adaptivepathdropcutter.hpp"
#include "batchdropcutter.hpp"
#include "pathdropcutter.hpp"

/*
 *  Python wrapping of octree and related classes
 */

namespace py = pybind11;
using namespace ocl;

void export_dropcutter(py::module_& m)
{
    py::class_<BatchDropCutter>(m, "BatchDropCutter")
        .def(py::init<>())
        .def("run", &BatchDropCutter::run, py::call_guard<py::gil_scoped_release>())
        .def("getCLPoints", &BatchDropCutter::getCLPoints)
        .def("setSTL", &BatchDropCutter::setSTL, py::keep_alive<1, 2>())
        .def("setCutter", &BatchDropCutter::setCutter, py::keep_alive<1, 2>())
        .def("setThreads", &BatchDropCutter::setThreads)
        .def("getThreads", &BatchDropCutter::getThreads)
        .def("appendPoint", &BatchDropCutter::appendPoint)
        .def("getTrianglesUnderCutter", &BatchDropCutter::getTrianglesUnderCutter)
        .def("getCalls", &BatchDropCutter::getCalls)
        .def("getBucketSize", &BatchDropCutter::getBucketSize)
        .def("setBucketSize", &BatchDropCutter::setBucketSize);

    py::class_<PathDropCutter>(m, "PathDropCutter")
        .def(py::init<>())
        .def("run", &PathDropCutter::run, py::call_guard<py::gil_scoped_release>())
        .def("getCLPoints", &PathDropCutter::getCLPoints)
        .def("setCutter", &PathDropCutter::setCutter, py::keep_alive<1, 2>())
        .def("setSTL", &PathDropCutter::setSTL, py::keep_alive<1, 2>())
        .def("setSampling", &PathDropCutter::setSampling)
        .def("setPath", &PathDropCutter::setPath, py::keep_alive<1, 2>())
        .def("getZ", &PathDropCutter::getZ)
        .def("setZ", &PathDropCutter::setZ)
        .def_property("minimumZ", &PathDropCutter::getZ, &PathDropCutter::setZ);

    py::class_<AdaptivePathDropCutter>(m, "AdaptivePathDropCutter")
        .def(py::init<>())
        .def("run", &AdaptivePathDropCutter::run, py::call_guard<py::gil_scoped_release>())
        .def("getCLPoints", &AdaptivePathDropCutter::getCLPoints)
        .def("setCutter", &AdaptivePathDropCutter::setCutter, py::keep_alive<1, 2>())
        .def("setSTL", &AdaptivePathDropCutter::setSTL, py::keep_alive<1, 2>())
        .def("setSampling", &AdaptivePathDropCutter::setSampling)
        .def("setMinSampling", &AdaptivePathDropCutter::setMinSampling)
        .def("setCosLimit", &AdaptivePathDropCutter::setCosLimit)
        .def("getSampling", &AdaptivePathDropCutter::getSampling)
        .def("setPath", &AdaptivePathDropCutter::setPath, py::keep_alive<1, 2>())
        .def("getZ", &AdaptivePathDropCutter::getZ)
        .def("setZ", &AdaptivePathDropCutter::setZ)
        .def_property("minimumZ", &AdaptivePathDropCutter::getZ, &AdaptivePathDropCutter::setZ);
}
