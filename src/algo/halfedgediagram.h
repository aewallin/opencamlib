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
#ifndef HEDI_H
#define HEDI_H


#include <vector>
#include <list>

#include <boost/graph/adjacency_list.hpp>
#include <boost/python.hpp> 
#include <boost/foreach.hpp> 

#include <boost/multi_array.hpp> // for bucketing in FaceList


#include "point.h"
#include "numeric.h"
namespace ocl
{
    

// bundled BGL properties
// see: http://www.boost.org/doc/libs/1_44_0/libs/graph/doc/bundles.html

// dcel notes from http://www.holmes3d.net/graphics/dcel/

// vertex
//  -leaving pointer to HalfEdge that has this vertex as origin
//   if many HalfEdges have this vertex as origin, choose one arbitrarily

// HalfEdge
//  - origin pointer to vertex
//  - face to the left of halfedge
//  - twin pointer to HalfEdge (on the right of this edge)
//  - next pointer to HalfEdge
//     this edge starts from h->twin->origin and ends at next vertex in h->face
//     traveling ccw around boundary
//     (allows face traverse, follow h->next until we arrive back at h)

// Face
//  - edge pointer to HalfEdge
//    this edge has this Face object as face
//    half-edge can be any one on the boundary of face


// special "infinite face", face on "outside" of boundary
// may or may not store edge pointer
    
struct VertexProps; // FWD declarations
struct EdgeProps;
struct FaceProps;
typedef boost::adjacency_list< boost::listS,            // out-edges stored in a std::list
                           boost::listS,            // vertex set stored here
                           boost::bidirectionalS,   // bidirectional graph.
                           VertexProps,             // vertex properties
                           EdgeProps                // edge properties
                           > HEGraph; 
typedef boost::graph_traits< HEGraph >::vertex_descriptor  HEVertex;
typedef boost::graph_traits< HEGraph >::vertex_iterator    HEVertexItr;
typedef boost::graph_traits< HEGraph >::edge_descriptor    HEEdge;
typedef boost::graph_traits< HEGraph >::edge_iterator      HEEdgeItr;
typedef boost::graph_traits< HEGraph >::out_edge_iterator  HEOutEdgeItr;
typedef boost::graph_traits< HEGraph >::adjacency_iterator HEAdjacencyItr;
typedef boost::graph_traits< HEGraph >::vertices_size_type HEVertexSize;
typedef std::vector<HEVertex> VertexVector;
typedef unsigned int HEFace;   
typedef std::vector<HEFace> FaceVector;
typedef boost::multi_array<FaceVector* , 2> Grid;
typedef Grid::index GridIndex;

enum VoronoiVertexType {OUT, IN, UNDECIDED, NEW };
struct VertexProps {
    Point position;
    VoronoiVertexType type;
    Point pk;
    double J2;
    double J3;
    double J4;
    /// based on previously calculated J2, J3, and J4, set the position of the vertex
    void set_position();
    /// based on precalculated J2, J3, J4, calculate the H determinant for Point pl
    double detH(const Point& pl);
    void set_J(Point& pi, Point& pj, Point& pk);
    double detH_J2(Point& pi, Point& pj, Point& pk);
    double detH_J3(Point& pi, Point& pj, Point& pk);
    double detH_J4(Point& pi, Point& pj, Point& pk);
};
struct EdgeProps {
    HEEdge next; 
    HEEdge twin;
    HEFace face; 
};

enum VoronoiFaceType {INCIDENT, NONINCIDENT};
struct FaceProps {
    FaceProps( HEEdge e , Point gen, VoronoiFaceType t) {
        edge = e;
        generator = gen;
        type = t;
    }
    HEEdge edge;
    Point generator;
    VoronoiFaceType type;
};
 
/// a VoronoiFace list which is updated when we build the voronoi diagram
class FaceList {
    public:
        FaceList();
        FaceList(double far, unsigned int n_bins);
        HEFace add_face(HEEdge e, Point& gen, VoronoiFaceType t);
        FaceProps& operator[](const unsigned int m);
        unsigned int size() const;    
        HEFace find_closest_face(const Point& p);
        HEFace grid_find_closest_face(const Point& p);

    private:     
        GridIndex get_grid_index( double x );
        HEFace find_closest_in_set( std::set<HEFace>& set, const Point&p );
        void insert_faces_from_neighbors( std::set<HEFace>& set, GridIndex row, GridIndex col , GridIndex dist );
        void insert_faces_from_bucket( std::set<HEFace>& set, GridIndex row, GridIndex col );
    // DATA
        std::vector<FaceProps> faces;
        double far_radius;
        double binwidth;
        GridIndex nbins;
        Grid* grid;
    
};


//template <class VertexProps, class EdgeProps>
class HalfEdgeDiagram : public HEGraph {
    public:
        HalfEdgeDiagram() {}
        HalfEdgeDiagram(double far, unsigned int n_bins);
        virtual ~HalfEdgeDiagram() {}
        
        HEVertex add_vertex();
        HEVertex add_vertex( Point pos, VoronoiVertexType t);
        HEVertex target(HEEdge e) const;
        HEVertex source(HEEdge e) const;
        HEFace add_face(HEEdge e, Point gen, VoronoiFaceType t);
        HEFace add_face(Point gen, VoronoiFaceType t);
        HEFace find_closest_face(const Point& p);
        void set_face_type(HEFace f, VoronoiFaceType t);
        HEFace face_size();
        VoronoiFaceType face_type(HEFace f);
        HEEdge add_edge(HEVertex v1, HEVertex v2);
        HEEdge face_edge(HEFace f);
        Point& face_generator(HEFace f);
        void set_face_edge(HEFace f, HEEdge e);
        VertexVector get_face_vertices(HEFace face_idx);
        FaceVector get_adjacent_faces( HEVertex q );
        void insert_vertex_in_edge(HEVertex v, HEEdge e);
        
        bool current_and_next_on_same_face(HEEdge current_edge) {
            return ( (*this)[current_edge].face == (*this)[ (*this)[current_edge].next ].face );
        }

    private:
        HEEdge find_previous_edge(HEEdge e);
        FaceList  faces;
};

} // end namespace
#endif
// end halfedgediagram.h
