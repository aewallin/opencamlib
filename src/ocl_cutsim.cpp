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

//#include "point.h"
//#include "oellipse.h"
#include "millingcutter.h"
#include "volume.h"
#include "octree.h"
#include "ocode.h"
#include "octree2.h"

/*
 *  Python wrapping of octree and related classes
 */

using namespace ocl;

namespace bp = boost::python;

void export_cutsim() {
    bp::class_<Octree>("Octree")
        .def(bp::init<double, unsigned int, Point& >())
        .def_readonly("max_depth", &Octree::max_depth)
        .def_readonly("root_scale", &Octree::root_scale)
        .def_readonly("root_center", &Octree::root_center)
        .def("get_leaf_nodes",       &Octree::py_get_leaf_nodes)
        //.def("get_surface_nodes",       &Octree::py_get_surface_nodes)
        .def("mc_triangles",       &Octree::py_mc_triangles)
        .def("diff_positive", &Octree::diff_positive_root)
        .def("diff_negative", &Octree::diff_negative_root)
        .def("diff_negative2", &Octree::diff_negative2_root)
        .def("init",       &Octree::init)
        .def("mc",       &Octree::mc)
        .def("__str__",            &Octree::str)
    ;
    bp::class_<Octnode>("Octnode")
        .def_readonly("depth", &Octnode::depth)
        .def_readonly("scale", &Octnode::scale)
        .def("center", &Octnode::py_get_center)
        .def("subdivide",        &Octnode::subdivide)
        .def("__str__",        &Octnode::str)
        .def("vertices",       &Octnode::py_get_vertices)
    ;
    bp::class_<Ocode>("Ocode")
        .def(bp::init<>())
        .def(bp::init<Ocode>())
        .def("point", &Ocode::point)
        .def("corner", &Ocode::corner)
        .def("degree", &Ocode::degree)
        .def("get_scale", &Ocode::get_scale)
        .def("set_scale", &Ocode::set_scale)
        .def("get_depth", &Ocode::get_depth)
        .def("set_depth", &Ocode::set_depth)
        .def("containedIn", &Ocode::containedIn)
        .def("str", &Ocode::str)
    ;
 
   bp::class_<LinOCT>("LinOCT")
        .def(bp::init<>())
        .def("append", &LinOCT::append)
        .def("size", &LinOCT::size)
        .def("get_nodes", &LinOCT::get_nodes)
        .def("get_triangles", &LinOCT::get_triangles)
        .def("build", &LinOCT::build)
        .def("init", &LinOCT::init)
        .def("sum", &LinOCT::sum)
        .def("diff", &LinOCT::diff)
        .def("operation", &LinOCT::operation)
        .def("sort", &LinOCT::sort)
        .def("condense", &LinOCT::condense)
        .def("str", &LinOCT::str)
    ;
    bp::class_<OCTVolumeWrap, boost::noncopyable>("OCTVolume", bp::no_init)
        .def("isInside", bp::pure_virtual(&OCTVolume::isInside) )
    ;
    bp::class_<SphereOCTVolume, bp::bases<OCTVolume> >("SphereOCTVolume")
        .def("isInside", &SphereOCTVolume::isInside )
        .def("isInsideBB", &SphereOCTVolume::isInsideBB )
        .def("calcBB", &SphereOCTVolume::calcBB )
        .def_readwrite("center", &SphereOCTVolume::center)
        .def_readwrite("radius", &SphereOCTVolume::radius)
        .def_readwrite("bb", &SphereOCTVolume::bb )
    ;
    bp::class_<CubeOCTVolume, bp::bases<OCTVolume> >("CubeOCTVolume")
        .def("isInside", &CubeOCTVolume::isInside )
        .def("calcBB", &CubeOCTVolume::calcBB )
        .def_readwrite("center", &CubeOCTVolume::center)
        .def_readwrite("side", &CubeOCTVolume::side)
        .def_readwrite("bb", &CubeOCTVolume::bb )
    ;
    bp::class_<CylinderOCTVolume, bp::bases<OCTVolume> >("CylinderOCTVolume")
        .def("isInside", &CylinderOCTVolume::isInside )
        .def("calcBB", &CylinderOCTVolume::calcBB )
        .def_readwrite("p1", &CylinderOCTVolume::p1)
        .def_readwrite("p2", &CylinderOCTVolume::p2)
        .def_readwrite("radius", &CylinderOCTVolume::radius)
        .def_readwrite("bb", &CylinderOCTVolume::bb )
    ;
    bp::class_<CylMoveOCTVolume, bp::bases<OCTVolume> >("CylMoveOCTVolume")
        .def(bp::init<CylCutter, Point, Point>())
        .def("isInside", &CylMoveOCTVolume::isInside )
        .def_readwrite("p1", &CylMoveOCTVolume::p1)
        .def_readwrite("p2", &CylMoveOCTVolume::p2)
        .def_readwrite("bb", &CylMoveOCTVolume::bb)
        .def_readwrite("box", &CylMoveOCTVolume::box)
    ;
    bp::class_<BoxOCTVolume, bp::bases<OCTVolume> >("BoxOCTVolume")
        .def("isInside", &BoxOCTVolume::isInside )
        .def("calcBB", &BoxOCTVolume::calcBB )
        .def_readwrite("corner", &BoxOCTVolume::corner)
        .def_readwrite("v1", &BoxOCTVolume::v1)
        .def_readwrite("v2", &BoxOCTVolume::v2)
        .def_readwrite("v3", &BoxOCTVolume::v3)
    ;
    bp::class_<CylCutterVolume, bp::bases<OCTVolume> >("CylCutterVolume")
        .def("isInside", &CylCutterVolume::isInside )
        .def("calcBB", &CylCutterVolume::calcBB )
        .def_readwrite("radius", &CylCutterVolume::radius)
        .def_readwrite("pos", &CylCutterVolume::pos)
        .def_readwrite("length", &CylCutterVolume::length)
    ;

}

