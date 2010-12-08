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



enum VoronoiVertexType {OUT, IN, UNDECIDED, NEW };
struct VertexProps {
    VertexProps() {}
    VertexProps( Point p, VoronoiVertexType t) {
        position=p;
        type=t;
    }
    /// based on previously calculated J2, J3, and J4, set the position of the vertex
    void set_position();
    /// based on precalculated J2, J3, J4, calculate the H determinant for Point pl
    double detH(const Point& pl);
    void set_J(Point& pi, Point& pj, Point& pk);
    double detH_J2(Point& pi, Point& pj, Point& pk);
    double detH_J3(Point& pi, Point& pj, Point& pk);
    double detH_J4(Point& pi, Point& pj, Point& pk);
// DATA
    Point position;
    VoronoiVertexType type;
    Point pk;
    double J2;
    double J3;
    double J4;
};
struct EdgeProps {
    EdgeProps() {}
    EdgeProps(HEEdge n, HEEdge t, HEFace f){
        next = n;
        twin = t;
        face = f;
    }
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
    bool operator<(const FaceProps& f) const {return (this->idx<f.idx);}

    HEFace idx;
    HEEdge edge;
    Point generator;
    VoronoiFaceType type;
};
 
//template <class VertexProps, class EdgeProps, class FaceProps, class HEVertex, class >
class HalfEdgeDiagram : public HEGraph {
    public:
        HalfEdgeDiagram(); 
        virtual ~HalfEdgeDiagram() {}
    
    // add vertex,edge,face to diagram
        HEVertex add_vertex() { return boost::add_vertex( *this ); }
        HEVertex add_vertex(VertexProps prop) {  return boost::add_vertex( prop, *this ); }
        HEEdge   add_edge(HEVertex v1, HEVertex v2);
        HEEdge   add_edge(HEVertex v1, HEVertex v2, EdgeProps prop);
        HEFace   add_face(FaceProps prop);
        
        void delete_vertex(HEVertex v) { clear_vertex(v);remove_vertex(v); }
        
    // access vertices    
        HEVertex target(HEEdge e) const { return boost::target( e, *this); }
        HEVertex source(HEEdge e) const { return boost::source( e, *this); }
        VertexVector vertices() const;
        VertexVector adjacent_vertices(HEVertex v) const;
        VertexVector face_vertices(HEFace f) const;
        unsigned int degree(HEVertex v) const { return boost::degree( v, *this); }
        unsigned int num_vertices() const { return boost::num_vertices( *this ); }
        
    // access edges
        EdgeVector out_edges( HEVertex v ) const;
        EdgeVector edges() const;
        HEEdge previous_edge(HEEdge e);
        unsigned int num_edges() const { return boost::num_edges( *this ); }
        
    // access faces
        FaceVector adjacent_faces( HEVertex q );
        unsigned int num_faces() const { return faces.size(); }
        
    // Directly access vertex,edge,face properties
        using HEGraph::operator[]; 
        FaceProps& operator[](HEFace f)  { return faces[f]; }
        const FaceProps& operator[](HEFace f) const  { return faces[f]; }
        
    // general half-edge methods:
        void insert_vertex_in_edge(HEVertex v, HEEdge e);

    private:
        void clear_vertex(HEVertex v) { boost::clear_vertex( v, *this ); }
        void remove_vertex(HEVertex v) { boost::remove_vertex( v , *this ); }
        std::vector<FaceProps> faces;
};

} // end namespace
#endif
// end halfedgediagram.h
