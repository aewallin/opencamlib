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
#include <stack>
#include <queue>

#include <boost/python.hpp>  // TODO: lots of python wrapping here...
#include <boost/foreach.hpp> 


#include "point.h"
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
        /// create diagram with given far-radius and number of bins
        VoronoiDiagram(double far, unsigned int n_bins);
        virtual ~VoronoiDiagram() {delete fgrid;}
        /// add a vertex generator at given position
        void addVertexSite(Point p);
        /// return the dual graph
        HalfEdgeDiagram* getDelaunayTriangulation();
        void setDelaunayTriangulation() {
            //if (dt != NULL)
            //    delete dt;
            dt = getDelaunayTriangulation();
        }
        
        // for visualizing the closest face
        Point getClosestFaceGenerator( const Point p ) {
            HEFace closest_face = fgrid->grid_find_closest_face( p );
            return hed[closest_face].generator;
        }
        // for visualizing seed-vertex
        Point getSeedVertex( const Point p ) {
            HEFace closest_face = fgrid->grid_find_closest_face( p );
            HEVertex v = find_seed_vertex(closest_face, p);
            return hed[ v ].position;
        }
        // for visualizing the delete-set
        boost::python::list getDeleteSet( Point p ) { // no const here(?)
            boost::python::list out;
            HEFace closest_face = fgrid->grid_find_closest_face( p );
            HEVertex v_seed = find_seed_vertex(closest_face, p);
            hed[v_seed].type = IN;
            VertexVector v0;
            v0.push_back(v_seed); 
            augment_vertex_set_B(v0, p);
            BOOST_FOREACH( HEVertex v, v0) {
                boost::python::list vert;
                vert.append( hed[ v ].position );
                vert.append( hed[ v ].type );
                out.append( vert );
            }
            //HEFace closest_face = fgrid->grid_find_closest_face( p );
            //HEVertex v = find_seed_vertex(closest_face, p);
            reset_labels();
            
            return out;
            
        }
        
        boost::python::list getVertexSet();
        boost::python::list getDelaunayEdges();
        /// return list of generators to python
        boost::python::list getGenerators() ;
        /// return list of vd vertices to python
        boost::python::list getVoronoiVertices() const;
        /// return list of the three special far-vertices to python
        boost::python::list getFarVoronoiVertices() const;
        /// return list of vd-edges to python
        boost::python::list getVoronoiEdges() const;
        /// return edges and generators to python
        boost::python::list getEdgesGenerators();
        /// string repr
        std::string str() const;
        /// return the far radius
        double getFarRadius() const {return far_radius;}
        
    private:
        /// among the vertices of f, find the one with the lowest detH value
        HEVertex find_seed_vertex(HEFace f, const Point& p);
        /// starting with set v, expand it maximally
        void augment_vertex_set_B(VertexVector& v, Point& p);
        /// better implementation from Sugihara-Iri
        void augment_vertex_set_RB(VertexVector& q, Point& p);
        void augment_vertex_set_M(VertexVector& q, Point& p);
        
        bool adjacentInVertexNotInFace( HEVertex w, HEFace f );
        bool adjacentInVertexInFace( HEVertex w, HEFace f );
        bool onOtherIncidentFace( HEVertex v, HEFace f );

        void printFaceVertexTypes(HEFace f);
        void printVertices(VertexVector& q);
        int outVertexCount(HEFace f);
        bool allIncidentFacesOK();
        
        bool not_in_queue(HEVertex w, std::queue<HEVertex> Q);
        
        bool faceVerticesConnected( HEFace f, VoronoiVertexType Vtype );
        bool  noUndecidedInFace( HEFace f );
        VertexVector findRepairVerts(HEFace f, VoronoiVertexType Vtype);
        void markAdjecentFacesIncident(std::stack<HEFace>& S, HEVertex v);
        void markAdjecentFacesIncident(HEVertex v);
        bool allIn(VertexVector& q);
        /// add the new vertices  
        void add_new_voronoi_vertices(VertexVector& v, Point& p);
        /// split faces when adding new generator p
        HEFace split_faces(Point& p);
        /// split the face
        void split_face(HEFace new_f, HEFace f);
        /// remove vertices in the set
        void remove_vertex_set(VertexVector& v0 , HEFace newface);
        /// set all vertices to UNDECIDED and all faces to NONINCIDENT
        void reset_labels();
        /// initialize the diagram with three generators
        void init();
        /// sanity-check for the diagram
        bool isValid();
        /// sanity-check. the diagram should be of degree three (at least with point generators)
        bool isDegreeThree();
        /// sanity-check.
        bool face_count_equals_generator_count();
        

    // DATA
        /// the half-edge diagram of the vd
        HalfEdgeDiagram hed;
        /// delaunay triangulation, i.e. the dual graph of the VD
        HalfEdgeDiagram* dt;
        /// a grid which allows fast nearest-neighbor search
        FaceGrid* fgrid; // for grid-search
        /// the voronoi diagram is constructed for sites within a circle with radius far_radius
        double far_radius;
        /// special initial vertex
        HEVertex v01;
        /// special initial vertex
        HEVertex v02;
        /// special initial vertex
        HEVertex v03;
        
        // initial generators
        Point gen1;
        Point gen2;
        Point gen3;
        /// the number of generators
        int gen_count;
        /// temporary variable for incident faces
        FaceVector incident_faces;
        /// temporary variable for in-vertices, out-vertices that need to be reset
        /// after each generator has been inserted
        VertexVector in_vertices;
};


} // end namespace
#endif
// end voronoidiagram.h
