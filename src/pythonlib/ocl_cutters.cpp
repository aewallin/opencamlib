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

#include <boost/python.hpp>

#include "millingcutter.hpp"
#include "millingcutter_py.hpp"
#include "cylcutter.hpp"
#include "ballcutter.hpp"
#include "bullcutter.hpp"
#include "conecutter.hpp"
#include "compositecutter.hpp"

/*
 *  wrap cutters
 */

using namespace ocl;

namespace bp = boost::python;

void export_cutters() {
// documentation here:
// http://www.boost.org/doc/libs/1_43_0/libs/python/doc/tutorial/doc/html/python/exposing.html#python.inheritance

    bp::class_<MillingCutter_py , boost::noncopyable>("MillingCutter")
        .def("vertexDrop", &MillingCutter::vertexDrop, &MillingCutter_py::default_vertexDrop )
        .def("facetDrop",  &MillingCutter::facetDrop,  &MillingCutter_py::default_facetDrop )
        .def("edgeDrop",   &MillingCutter::edgeDrop,   &MillingCutter_py::default_edgeDrop )
        .def("dropCutter", &MillingCutter::dropCutter)
        .def("pushCutter", &MillingCutter::pushCutter)
        .def("offsetCutter", &MillingCutter::offsetCutter,  bp::return_value_policy<bp::manage_new_object>() )
        .def("__str__",    &MillingCutter::str, &MillingCutter_py::default_str )
        .def("getRadius", &MillingCutter::getRadius )
        .def("getLength", &MillingCutter::getLength )
        .def("getDiameter", &MillingCutter::getDiameter )
    ; 
    bp::class_<CylCutter, bp::bases<MillingCutter> >("CylCutter")
        .def(bp::init<double, double>()) 
        .def("dropCutterSTL", &CylCutter::dropCutterSTL)
    ;
    bp::class_<BallCutter, bp::bases<MillingCutter> >("BallCutter")
        .def(bp::init<double, double>())
        .def("dropCutterSTL", &BallCutter::dropCutterSTL)
    ;
    bp::class_<BullCutter, bp::bases<MillingCutter> >("BullCutter")
        .def(bp::init<double, double, double>())
    ;
    bp::class_<ConeCutter, bp::bases<MillingCutter> >("ConeCutter")
        .def(bp::init<double, double, double>())
    ;
    
    bp::class_<CompCylCutter, bp::bases<MillingCutter> >("CompCylCutter")
        .def(bp::init<double, double>())
    ;
    bp::class_<CompBallCutter, bp::bases<MillingCutter> >("CompBallCutter")
        .def(bp::init<double, double>())
    ;
    
    bp::class_<CylConeCutter, bp::bases<MillingCutter> >("CylConeCutter")
        .def(bp::init<double, double, double>())
    ;
    bp::class_<BallConeCutter, bp::bases<MillingCutter> >("BallConeCutter")
        .def(bp::init<double, double, double>())
    ;
    bp::class_<BullConeCutter, bp::bases<MillingCutter> >("BullConeCutter")
        .def(bp::init<double, double, double, double>())
    ;
    bp::class_<ConeConeCutter, bp::bases<MillingCutter> >("ConeConeCutter")
        .def(bp::init<double, double, double, double>())
    ;
}

