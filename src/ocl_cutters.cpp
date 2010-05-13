/*  Copyright 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com)
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
#include "point.h"
#include "cutter.h"


/*
 *  wrap cutters
 */

using namespace ocl;

namespace bp = boost::python;

void export_cutters() {
// documentation here:
// http://www.boost.org/doc/libs/1_43_0/libs/python/doc/tutorial/doc/html/python/exposing.html#python.inheritance

    // bp::class_<MillingCutterWrap, boost::noncopyable>("MillingCutter", bp::no_init)
    bp::class_<MillingCutterWrap , boost::noncopyable>("MillingCutter")
        .def("vertexDrop", &MillingCutter::vertexDrop, &MillingCutterWrap::default_vertexDrop )
        .def("facetDrop",  &MillingCutter::facetDrop )
        .def("edgeDrop",   &MillingCutter::edgeDrop )
        .def("__str__",    &MillingCutter::str, &MillingCutterWrap::default_str )
        .add_property("radius", &MillingCutter::getRadius )
        .add_property("length", &MillingCutter::getLength, &MillingCutter::setLength  )
        .add_property("diameter", &MillingCutter::getDiameter, &MillingCutter::setDiameter )
    ; 
    /*
    bp::class_<MillingCutter>("MillingCutter")
        .def("vertexDrop", &MillingCutter::vertexDrop )
        .def("facetDrop",  &MillingCutter::facetDrop )
        .def("edgeDrop",   &MillingCutter::edgeDrop )
        .def("__str__",   bp::virtual(&MillingCutter::str) )
        .add_property("radius", &MillingCutter::getRadius )
        .add_property("length", &MillingCutter::getLength, &MillingCutter::setLength  )
        .add_property("diameter", &MillingCutter::getDiameter, &MillingCutter::setDiameter )
    ;*/
    bp::class_<CylCutter, bp::bases<MillingCutter> >("CylCutter")
        .def(bp::init<double>()) 
        .def("vertexDrop", &CylCutter::vertexDrop)
        .def("facetDrop", &CylCutter::facetDrop)
        .def("edgeDrop", &CylCutter::edgeDrop)
        .def("dropCutter", &CylCutter::dropCutter)
        .def("vertexPush", &CylCutter::vertexPush)
        .def("facetPush", &CylCutter::facetPush)
        .def("edgePush", &CylCutter::edgePush)
        //.def("offsetCutter", &CylCutter::offsetCutter, bp::return_value_policy<bp::manage_new_object>() )
        .def("dropCutterSTL", &CylCutter::dropCutterSTL)
    ;
    bp::class_<BallCutter, bp::bases<MillingCutter> >("BallCutter")
        .def(bp::init<double>())
        .def("vertexDrop", &BallCutter::vertexDrop)
        .def("facetDrop", &BallCutter::facetDrop)
        .def("edgeDrop", &BallCutter::edgeDrop)
        .def("dropCutter", &BallCutter::dropCutter)
        .def("dropCutterSTL", &BallCutter::dropCutterSTL)
    ;
    bp::class_<BullCutter, bp::bases<MillingCutter> >("BullCutter")
        .def(bp::init<double, double>())
        .def("vertexDrop", &BullCutter::vertexDrop)
        .def("facetDrop", &BullCutter::facetDrop)
        .def("edgeDrop", &BullCutter::edgeDrop)
    ;
    bp::class_<ConeCutter, bp::bases<MillingCutter> >("ConeCutter")
        .def(bp::init<double, double>())
        .def("vertexDrop", &ConeCutter::vertexDrop)
        .def("facetDrop", &ConeCutter::facetDrop)
        .def("edgeDrop", &ConeCutter::edgeDrop)
    ;
    bp::class_<CylConeCutter, bp::bases<MillingCutter> >("CylConeCutter")
        .def(bp::init<double, double, double>())
        .def("vertexDrop", &CylConeCutter::vertexDrop)
        .def("facetDrop", &CylConeCutter::facetDrop)
        .def("edgeDrop", &CylConeCutter::edgeDrop)
    ;
    bp::class_<BallConeCutter, bp::bases<MillingCutter> >("BallConeCutter")
        .def(bp::init<double, double, double>())
        .def("vertexDrop", &BallConeCutter::vertexDrop)
        .def("facetDrop", &BallConeCutter::facetDrop)
        .def("edgeDrop", &BallConeCutter::edgeDrop)
    ;
    bp::class_<BullConeCutter, bp::bases<MillingCutter> >("BullConeCutter")
        .def(bp::init<double, double, double, double>())
        .def("vertexDrop", &BullConeCutter::vertexDrop)
        .def("facetDrop", &BullConeCutter::facetDrop)
        .def("edgeDrop", &BullConeCutter::edgeDrop)
    ;
    bp::class_<ConeConeCutter, bp::bases<MillingCutter> >("ConeConeCutter")
        .def(bp::init<double, double, double, double>())
        .def("vertexDrop", &ConeConeCutter::vertexDrop)
        .def("facetDrop", &ConeConeCutter::facetDrop)
        .def("edgeDrop", &ConeConeCutter::edgeDrop)
    ;
}

