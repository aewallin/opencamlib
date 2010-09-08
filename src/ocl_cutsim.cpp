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

#include "volume.h"         // no boost-python
#include "volume_wrap.h"    // wrapper
#include "octnode_py.h"     // new-style wrapper
#include "octree_py.h"      // new-style wrapper

/*
 *  Python wrapping of octree and related classes
 */

using namespace ocl;

namespace bp = boost::python;

void export_cutsim() {
    bp::class_<Octree>("Octree_base")
    ;
    bp::class_<Octree_py, bp::bases<Octree> >("Octree")
        .def(bp::init<double, unsigned int, Point& >())
        .def("max_depth", &Octree_py::get_max_depth)
        .def("root_scale", &Octree_py::get_root_scale)
        .def("leaf_scale", &Octree_py::leaf_scale)
        .def("get_leaf_nodes",       &Octree_py::py_get_leaf_nodes)
        .def("mc_triangles",       &Octree_py::py_mc_triangles)
        .def("side_triangles",       &Octree_py::py_s_triangles)
        .def("diff_negative", &Octree_py::diff_negative_root)
        .def("init",       &Octree_py::init)
        .def("mc",       &Octree_py::mc)
        .def("__str__",            &Octree_py::str)
    ;
    bp::class_<Octnode>("Octnode_base")
    ;
    bp::class_<Octnode_py, bp::bases<Octnode> >("Octnode")
        .def_readonly("depth", &Octnode_py::depth)
        .def_readonly("scale", &Octnode_py::scale)
        .def("center", &Octnode_py::py_get_center)
        .def("subdivide",        &Octnode_py::subdivide)
        .def("__str__",        &Octnode_py::str)
        .def("vertices",       &Octnode_py::py_get_vertices)
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
        .def("setPos", &CylCutterVolume::setPos )
        .def_readwrite("radius", &CylCutterVolume::radius)
        .def_readwrite("pos", &CylCutterVolume::pos)
        .def_readwrite("length", &CylCutterVolume::length)
    ;
    bp::class_<BallCutterVolume, bp::bases<OCTVolume> >("BallCutterVolume")
        .def("setPos", &BallCutterVolume::setPos )
        .def_readwrite("radius", &BallCutterVolume::radius)
        .def_readwrite("pos", &BallCutterVolume::pos)
        .def_readwrite("length", &BallCutterVolume::length)
    ;
    bp::class_<BullCutterVolume, bp::bases<OCTVolume> >("BullCutterVolume")
        .def("setPos", &BullCutterVolume::setPos )
        .def_readwrite("radius", &BullCutterVolume::radius)
        .def_readwrite("r1", &BullCutterVolume::r1)
        .def_readwrite("r2", &BullCutterVolume::r2)
        .def_readwrite("pos", &BullCutterVolume::pos)
        .def_readwrite("length", &BullCutterVolume::length)
    ;
    bp::class_<PlaneVolume, bp::bases<OCTVolume> >("PlaneVolume")
        .def(bp::init< bool, unsigned int, double>())
    ;
}

