/*  $Id$
 *
 *  Copyright (c) 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com).
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

#include "ballcutter.hpp"
#include "bullcutter.hpp"
#include "compositecutter.hpp"
#include "conecutter.hpp"
#include "cylcutter.hpp"
#include "millingcutter.hpp"

namespace py = pybind11;
using namespace ocl;

void export_cutters(py::module& m)
{
    py::class_<MillingCutter>(m, "MillingCutter")
        .def("vertexDrop", &MillingCutter::vertexDrop)
        .def("facetDrop", &MillingCutter::facetDrop)
        .def("edgeDrop", &MillingCutter::edgeDrop)
        .def("dropCutter", &MillingCutter::dropCutter)
        .def("pushCutter", &MillingCutter::pushCutter)
        .def("offsetCutter", &MillingCutter::offsetCutter, py::return_value_policy::take_ownership)
        .def("__str__", &MillingCutter::str)
        .def("getRadius", &MillingCutter::getRadius)
        .def("getLength", &MillingCutter::getLength)
        .def("getDiameter", &MillingCutter::getDiameter);

    py::class_<CylCutter, MillingCutter>(m, "CylCutter")
        .def(py::init<double, double>())
        .def("dropCutterSTL", &CylCutter::dropCutterSTL, py::call_guard<py::gil_scoped_release>());

    py::class_<BallCutter, MillingCutter>(m, "BallCutter")
        .def(py::init<double, double>())
        .def("dropCutterSTL", &BallCutter::dropCutterSTL, py::call_guard<py::gil_scoped_release>());

    py::class_<BullCutter, MillingCutter>(m, "BullCutter")
        .def(py::init<double, double, double>());

    py::class_<ConeCutter, MillingCutter>(m, "ConeCutter")
        .def(py::init<double, double, double>());

    py::class_<CompCylCutter, MillingCutter>(m, "CompCylCutter")
        .def(py::init<double, double>());

    py::class_<CompBallCutter, MillingCutter>(m, "CompBallCutter")
        .def(py::init<double, double>());

    py::class_<CylConeCutter, MillingCutter>(m, "CylConeCutter")
        .def(py::init<double, double, double>());

    py::class_<BallConeCutter, MillingCutter>(m, "BallConeCutter")
        .def(py::init<double, double, double>());

    py::class_<BullConeCutter, MillingCutter>(m, "BullConeCutter")
        .def(py::init<double, double, double, double>());

    py::class_<ConeConeCutter, MillingCutter>(m, "ConeConeCutter")
        .def(py::init<double, double, double, double>());
}
