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
#ifndef VODI_H
#define VODI_H

#include <vector>
#include <list>

#include <boost/python.hpp> 
#include <boost/foreach.hpp> 

#include "point.h"
#include "numeric.h"
#include "halfedgediagram.h"
#include "facegrid.h"

namespace ocl
{

/// \brief Voronoi diagram.
///
/// the dual of a voronoi diagram is the delaunay diagram(triangulation).
///  voronoi-faces are dual to delaunay-vertices.
///  vornoi-vertices are dual to delaunay-faces 
///  voronoi-edges are dual to delaunay-edges
class VoronoiDiagram {
    public:
        VoronoiDiagram() {}
        VoronoiDiagram(double far, unsigned int n_bins);
        
        virtual ~VoronoiDiagram() {}
        
        void addVertexSite(Point p);
        
        boost::python::list getGenerators() ;
        boost::python::list getVoronoiVertices() const;
        boost::python::list getFarVoronoiVertices() const;
        boost::python::list getVoronoiEdges() const;
        boost::python::list getEdgesGenerators() ;
        std::string str() const;
        
        double getFarRadius() const {return far_radius;}
        
    private:
        /// among the vertices belonging to f, find the one with the lowest detH value
        VertexVector find_seed_vertex(HEFace f, const Point& p);
        void augment_vertex_set(VertexVector& v, Point& p); 
        void add_new_voronoi_vertices(VertexVector& v, Point& p);
        HEFace split_faces(Point& p);
        void split_face(HEFace new_f, HEFace f);
        void remove_vertex_set(VertexVector& v0 , HEFace newface);
        /// set all vertices to UNDECIDED and all faces to NONINCIDENT
        void reset_labels();
        /// initialize the diagram with three generators
        void init();
        
        bool isValid();
        bool isDegreeThree();
        bool face_count_equals_generator_count();
        

    // DATA
        HalfEdgeDiagram hed;
        /// the voronoi diagram is constructed for sites within a circle with radius far_radius
        double far_radius;
        HEVertex v01;
        HEVertex v02;
        HEVertex v03;
        int gen_count;
        FaceVector incident_faces;
        VertexVector in_vertices;
    // for grid-search
        FaceGrid fgrid;
};


} // end namespace
#endif
// end voronoidiagram.h
