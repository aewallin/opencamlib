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

#include "adaptivewaterline.hpp"
#include "batchpushcutter.hpp"
#include "clsurface.hpp"
#include "lineclfilter.hpp"
#include "numeric.hpp"
#include "simple_weave.hpp"
#include "smart_weave.hpp"
#include "tsp.hpp"
#include "waterline.hpp"
#include "weave.hpp"
#include "zigzag.hpp"

namespace py = pybind11;
using namespace ocl;

void export_algo(py::module& m)
{
    m.def("eps", eps, "machine epsilon, see numeric.cpp");
    m.def("epsF", epsF);
    m.def("epsD", epsD);

    py::class_<ZigZag>(m, "ZigZag")
        .def(py::init<>())
        .def("run", &ZigZag::run, py::call_guard<py::gil_scoped_release>())
        .def("setDirection", &ZigZag::setDirection)
        .def("setOrigin", &ZigZag::setOrigin)
        .def("setStepOver", &ZigZag::setStepOver)
        .def("addPoint", &ZigZag::addPoint)
        .def("getOutput", &ZigZag::getOutput)
        .def("__str__", &ZigZag::str);

    py::class_<BatchPushCutter>(m, "BatchPushCutter")
        .def(py::init<>())
        .def("run", &BatchPushCutter::run, py::call_guard<py::gil_scoped_release>())
        .def("setSTL", &BatchPushCutter::setSTL, py::keep_alive<1, 2>())
        .def("setCutter", &BatchPushCutter::setCutter, py::keep_alive<1, 2>())
        .def("setThreads", &BatchPushCutter::setThreads)
        .def("appendFiber", &BatchPushCutter::appendFiber)
        .def("getOverlapTriangles", &BatchPushCutter::getOverlapTriangles)
        .def("getCLPoints", &BatchPushCutter::getCLPoints)
        .def("getFibers", &BatchPushCutter::getFibers)
        .def("getCalls", &BatchPushCutter::getCalls)
        .def("getThreads", &BatchPushCutter::getThreads)
        .def("setBucketSize", &BatchPushCutter::setBucketSize)
        .def("getBucketSize", &BatchPushCutter::getBucketSize)
        .def("setXDirection", &BatchPushCutter::setXDirection)
        .def("setYDirection", &BatchPushCutter::setYDirection);

    py::class_<Waterline>(m, "Waterline")
        .def(py::init<>())
        .def("setCutter", &Waterline::setCutter, py::keep_alive<1, 2>())
        .def("setSTL", &Waterline::setSTL, py::keep_alive<1, 2>())
        .def("setZ", &Waterline::setZ)
        .def("setSampling", &Waterline::setSampling)
        .def("run", &Waterline::run, py::call_guard<py::gil_scoped_release>())
        .def("run2", &Waterline::run2, py::call_guard<py::gil_scoped_release>())
        .def("reset", &Waterline::reset)
        .def("getLoops", &Waterline::getLoops)
        .def("setThreads", &Waterline::setThreads)
        .def("getThreads", &Waterline::getThreads)
        .def("getXFibers", &Waterline::getXFibers)
        .def("getYFibers", &Waterline::getYFibers);

    py::class_<AdaptiveWaterline>(m, "AdaptiveWaterline")
        .def(py::init<>())
        .def("setCutter", &AdaptiveWaterline::setCutter, py::keep_alive<1, 2>())
        .def("setSTL", &AdaptiveWaterline::setSTL, py::keep_alive<1, 2>())
        .def("setZ", &AdaptiveWaterline::setZ)
        .def("setSampling", &AdaptiveWaterline::setSampling)
        .def("setMinSampling", &AdaptiveWaterline::setMinSampling)
        .def("run", &AdaptiveWaterline::run, py::call_guard<py::gil_scoped_release>())
        .def("run2", &AdaptiveWaterline::run2, py::call_guard<py::gil_scoped_release>())
        .def("reset", &AdaptiveWaterline::reset)
        .def("getLoops", &AdaptiveWaterline::getLoops)
        .def("setThreads", &AdaptiveWaterline::setThreads)
        .def("getThreads", &AdaptiveWaterline::getThreads)
        .def("getXFibers", &AdaptiveWaterline::getXFibers)
        .def("getYFibers", &AdaptiveWaterline::getYFibers);

    py::enum_<weave::VertexType>(m, "WeaveVertexType")
        .value("CL", weave::CL)
        .value("CL_DONE", weave::CL_DONE)
        .value("ADJ", weave::ADJ)
        .value("TWOADJ", weave::TWOADJ)
        .value("INT", weave::INT)
        .value("FULLINT", weave::FULLINT);

    py::class_<weave::Weave>(m, "Weave")
        .def("addFiber", &weave::Weave::addFiber)
        .def("build", &weave::Weave::build, py::call_guard<py::gil_scoped_release>())
        .def("printGraph", &weave::Weave::printGraph)
        .def("face_traverse", &weave::Weave::face_traverse, py::call_guard<py::gil_scoped_release>())
        .def("getVertices", &weave::Weave::getVertices)
        .def("getVerticesByType", &weave::Weave::getVerticesByType)
        .def("getCLVertices", [](const weave::Weave& w) { return w.getVerticesByType(weave::CL); })
        .def("getINTVertices", [](const weave::Weave& w) { return w.getVerticesByType(weave::INT); })
        .def("numVertices", &weave::Weave::numVertices)
        .def("getEdges", &weave::Weave::getEdges)
        .def("getLoops", &weave::Weave::getLoops)
        .def("__str__", &weave::Weave::str);

    py::class_<weave::SimpleWeave, weave::Weave>(m, "SimpleWeave").def(py::init<>());

    py::class_<weave::SmartWeave, weave::Weave>(m, "SmartWeave").def(py::init<>());

    py::class_<LineCLFilter>(m, "LineCLFilter")
        .def(py::init<>())
        .def("addCLPoint", &LineCLFilter::addCLPoint)
        .def("setTolerance", &LineCLFilter::setTolerance)
        .def("run", &LineCLFilter::run, py::call_guard<py::gil_scoped_release>())
        .def("getCLPoints", [](const LineCLFilter& f) { return f.clpoints; });

    py::class_<clsurf::CutterLocationSurface>(m, "CutterLocationSurface")
        .def(py::init<double>())
        .def("run", &clsurf::CutterLocationSurface::run, py::call_guard<py::gil_scoped_release>())
        .def("setMinSampling", &clsurf::CutterLocationSurface::setMinSampling)
        .def("setSampling", &clsurf::CutterLocationSurface::setSampling)
        .def("setSTL", &clsurf::CutterLocationSurface::setSTL, py::keep_alive<1, 2>())
        .def("setCutter", &clsurf::CutterLocationSurface::setCutter, py::keep_alive<1, 2>())
        .def("getVertices", &clsurf::CutterLocationSurface::getVertices)
        .def("getEdges", &clsurf::CutterLocationSurface::getEdges)
        .def("__str__", &clsurf::CutterLocationSurface::str);

    py::class_<tsp::TSPSolver>(m, "TSPSolver")
        .def(py::init<>())
        .def("addPoint", &tsp::TSPSolver::addPoint)
        .def("run", &tsp::TSPSolver::run, py::call_guard<py::gil_scoped_release>())
        .def("getOutput", &tsp::TSPSolver::getOutput)
        .def("getLength", &tsp::TSPSolver::getLength)
        .def("reset", &tsp::TSPSolver::reset);
}
