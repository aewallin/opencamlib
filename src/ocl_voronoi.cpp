/*  $Id: $
 * 
 *  Copyright 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com)
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


#ifndef WIN32
#include "voronoidiagram_py.hpp"  
#endif

/*
 *  Python wrapping of voronoi diagram
 */

using namespace ocl;

namespace bp = boost::python;

void export_voronoi() {

#ifndef WIN32
    bp::class_<VoronoiDiagram >("VoronoiDiagram_base")
    ;
    bp::class_< VoronoiDiagram_py, bp::bases<VoronoiDiagram> >("VoronoiDiagram")
        .def(bp::init<double, unsigned int>())
        .def("addVertexSite",  &VoronoiDiagram_py::addVertexSite)
        .def("getGenerators",  &VoronoiDiagram_py::getGenerators)
        .def("getEdgesGenerators",  &VoronoiDiagram_py::getEdgesGenerators)
        .def("getVoronoiVertices",  &VoronoiDiagram_py::getVoronoiVertices)
        .def("getFarVoronoiVertices",  &VoronoiDiagram_py::getFarVoronoiVertices)
        .def("getFarRadius",  &VoronoiDiagram_py::getFarRadius)
        .def("getVoronoiEdges",  &VoronoiDiagram_py::getVoronoiEdges)
        //.def("setDelaunayTriangulation",  &VoronoiDiagram_py::setDelaunayTriangulation)
        //.def("getDelaunayEdges",  &VoronoiDiagram_py::getDelaunayEdges)
        .def("getClosestFaceGenerator",  &VoronoiDiagram_py::getClosestFaceGenerator)
        .def("getSeedVertex",  &VoronoiDiagram_py::getSeedVertex)
        .def("getDeleteSet",  &VoronoiDiagram_py::getDeleteSet)
        .def("getDeleteEdges",  &VoronoiDiagram_py::getDeleteEdges)
        .def("getModEdges",  &VoronoiDiagram_py::getModEdges)
        .def("__str__", &VoronoiDiagram_py::str)
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
    

#endif
}

