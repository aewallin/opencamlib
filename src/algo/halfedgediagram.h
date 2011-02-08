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
#include <boost/foreach.hpp> 

#include "point.h"
#include "numeric.h"
namespace ocl
{
    
// bundled BGL properties, see: http://www.boost.org/doc/libs/1_44_0/libs/graph/doc/bundles.html

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
    
struct VertexProps; 
struct EdgeProps;
struct FaceProps;
typedef unsigned int HEFace; 

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
typedef std::vector<HEFace> FaceVector;
typedef std::vector<HEEdge> EdgeVector;  

    
typedef std::vector< std::vector< HEEdge > > HEPlanarEmbedding;

enum VoronoiVertexType {OUT, IN, UNDECIDED, NEW };

/// vertex properties of a vertex in the voronoi diagram
struct VertexProps {
    VertexProps() {
        index = count;
        count++;
    }
    /// construct vertex at position p with type t
    VertexProps( Point p, VoronoiVertexType t) {
        position=p;
        type=t;
        index = count;
        count++;
        
    }
    /// based on previously calculated J2, J3, and J4, set the position of the vertex
    void set_position();
    /// based on precalculated J2, J3, J4, calculate the H determinant for Point pl
    double detH(const Point& pl);
    bool operator<(const VertexProps& other) const {
        return ( abs(this->H) < abs(other.H) );
    }
    /// set the J values
    void set_J(Point& pi, Point& pj, Point& pk);
    /// calculate J2
    double detH_J2(Point& pi, Point& pj, Point& pk);
    /// calculate J3
    double detH_J3(Point& pi, Point& pj, Point& pk);
    /// calculate J4
    double detH_J4(Point& pi, Point& pj, Point& pk);
// DATA
    /// the position of the vertex
    Point position;
    /// vertex type
    VoronoiVertexType type;
    /// index of vertex
    int index;
    static int count;
    /// the reference point for J-calculations
    Point pk;
    /// J2 determinant
    double J2;
    /// J3 determinant
    double J3;
    /// J4 determinant
    double J4;
    double H;
};

/// properties of an edge in the voronoi diagram
struct EdgeProps {
    EdgeProps() {}
    /// create edge with given next, twin, and face
    EdgeProps(HEEdge n, HEEdge t, HEFace f){
        next = n;
        twin = t;
        face = f;
    }
    /// the next edge, counterclockwise, from this edge
    HEEdge next; 
    /// the twin edge
    HEEdge twin;
    /// the face to which this edge belongs
    HEFace face; 
};

/// types of faces in the voronoi diagram
enum VoronoiFaceType {INCIDENT, NONINCIDENT};

/// properties of a face in the voronoi diagram
struct FaceProps {
    /// create face with given edge, generator, and type
    FaceProps( HEEdge e , Point gen, VoronoiFaceType t) {
        edge = e;
        generator = gen;
        type = t;
    }
    /// operator for sorting faces
    bool operator<(const FaceProps& f) const {return (this->idx<f.idx);}
    /// face index
    HEFace idx;
    /// one edge that bounds this face
    HEEdge edge;
    /// the generator for this face
    Point generator;
    /// face type
    VoronoiFaceType type;
};
 
//template <class VertexProps, class EdgeProps, class FaceProps, class HEVertex, class >
/// half-edge diagram class.
class HalfEdgeDiagram : public HEGraph {
    public:
        HalfEdgeDiagram(); 
        virtual ~HalfEdgeDiagram() {}
    
    // add vertex,edge,face to diagram
        /// add a blank vertex and return its descriptor
        HEVertex add_vertex() { return boost::add_vertex( *this ); }
        /// add a vertex with given properties, return vertex descriptor
        HEVertex add_vertex(VertexProps prop) {  return boost::add_vertex( prop, *this ); }
        /// add an edge between vertices v1-v2
        HEEdge   add_edge(HEVertex v1, HEVertex v2);
        /// add an edge with given properties
        HEEdge   add_edge(HEVertex v1, HEVertex v2, EdgeProps prop);
        /// add a face with given properties
        HEFace   add_face(FaceProps prop);
        /// delete a vertex
        void delete_vertex(HEVertex v) { 
            clear_vertex(v);
            remove_vertex(v); 
        }
        
    // access vertices    
        /// return the target vertex of the given edge
        HEVertex target(HEEdge e) const { return boost::target( e, *this); }
        /// return the source vertex of the given edge
        HEVertex source(HEEdge e) const { return boost::source( e, *this); }
        /// return all vertices
        VertexVector vertices() const;
        /// return all vertices adjecent to given vertex
        VertexVector adjacent_vertices(HEVertex v) const;
        /// return all vertices of given face
        VertexVector face_vertices(HEFace f) const;
        /// return degree of given vertex
        unsigned int degree(HEVertex v) const { return boost::degree( v, *this); }
        /// return number of vertices in graph
        unsigned int num_vertices() const { return boost::num_vertices( *this ); }
        
    // access edges
        /// return out_edges of given vertex
        EdgeVector out_edges( HEVertex v ) const;
        /// return all edges
        EdgeVector edges() const;
        /// return edges of face f
        EdgeVector face_edges(HEFace f) {
            HEEdge start_edge = (*this)[f].edge;
            HEEdge current_edge = start_edge;
            EdgeVector out;
            do {
                out.push_back(current_edge);
                current_edge = (*this)[current_edge].next;
            } while( current_edge != start_edge );
            return out;
        }
        /// return the previous edge. traverses all edges in face until previous found.
        HEEdge previous_edge(HEEdge e);
        /// return number of edges in graph
        unsigned int num_edges() const { return boost::num_edges( *this ); }
        /// return true if v1-v2 edge exists
        bool edge( HEVertex v1, HEVertex v2 ) {
            typedef std::pair<HEEdge, bool> EdgeBool;
            EdgeBool result = boost::edge(v1, v2, *this );
            return result.second;
        }
        
    // access faces
        /// return adjacent faces to the given vertex
        FaceVector adjacent_faces( HEVertex q );
        /// return number of faces in graph
        unsigned int num_faces() const { return faces.size(); }
        
    // Directly access vertex,edge,face properties
        /// access to the base-class operator[]
        using HEGraph::operator[]; 
        /// operator[] for accessing face properties
        FaceProps& operator[](HEFace f)  { return faces[f]; }
        /// const operator[] for accessing face properties
        const FaceProps& operator[](HEFace f) const  { return faces[f]; }
        
    // general half-edge methods:
        /// inserts given vertex and its twin into edge e
        void insert_vertex_in_edge(HEVertex v, HEEdge e);
        
        bool checkFaces() {
            BOOST_FOREACH(FaceProps f, faces) {
                BOOST_FOREACH( HEEdge e, face_edges(f.idx)) {
                    if ( (*this)[e].face != f.idx )
                        return false;
                }
            }
            return true;
        }
        
    private:
        /// clear given vertex. this removes all edges connecting to the vertex.
        void clear_vertex(HEVertex v) { boost::clear_vertex( v, *this ); }
        /// remove given vertex
        void remove_vertex(HEVertex v) { boost::remove_vertex( v , *this ); }
        /// a vector of face properties
        std::vector<FaceProps> faces;
};

} // end namespace
#endif
// end halfedgediagram.h
