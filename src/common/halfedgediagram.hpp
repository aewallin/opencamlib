/*  $Id$
 * 
 *  Copyright (c) 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com).
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
#ifndef HALFEDGEDIAGRAM_HPP
#define HALFEDGEDIAGRAM_HPP

#include <vector>
#include <list>

#include <boost/graph/adjacency_list.hpp>
#include <boost/foreach.hpp> 

// bundled BGL properties, see: http://www.boost.org/doc/libs/1_44_0/libs/graph/doc/bundles.html

// dcel notes from http://www.holmes3d.net/graphics/dcel/

// vertex (boost::out_edges)
//  -leaving pointer to HalfEdge that has this vertex as origin
//   if many HalfEdges have this vertex as origin, choose one arbitrarily

// HalfEdge
//  - origin pointer to vertex (boost::source)
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


/// HEDIGraph is a A half-edge diagram class.
/// Templated on Vertex/Edge/Face property classes which allow
/// attaching information to vertices/edges/faces that is 
/// required for a particular algorithm.
/// 
/// Inherits from boost::adjacency_list
/// minor additions allow storing face-properties.
///
/// the hedi namespace contains functions for manipulating HEDIGraphs
///
/// For a general description of the half-edge data structure see e.g.:
///  - http://www.holmes3d.net/graphics/dcel/
///  - http://openmesh.org/index.php?id=228

namespace ocl {

namespace hedi  { 

template <class TOutEdgeList, 
          class TVertexList,
          class TDirected, 
          class TVertexProperties,
          class TEdgeProperties,
          class TFaceProperties,
          class TGraphProperties,
          class TEdgeList 
          >
class HEDIGraph {
    public:
        
        typedef typename boost::adjacency_list< TOutEdgeList,            
                                                TVertexList,            
                                                TDirected,   
                                                TVertexProperties,             
                                                TEdgeProperties,                
                                                TGraphProperties,
                                                TEdgeList
                                                > BGLGraph;
        typedef unsigned int Face; 
        typedef typename boost::graph_traits< BGLGraph >::edge_descriptor Edge;
        typedef typename boost::graph_traits< BGLGraph >::vertex_descriptor Vertex;
        typedef typename boost::graph_traits< BGLGraph >::vertex_iterator    VertexItr;
        typedef typename boost::graph_traits< BGLGraph >::edge_iterator      EdgeItr;
        typedef typename boost::graph_traits< BGLGraph >::out_edge_iterator  OutEdgeItr;
        typedef typename boost::graph_traits< BGLGraph >::adjacency_iterator AdjacencyItr;
                
        typedef std::vector<Vertex> VertexVector;
        typedef std::vector<Face> FaceVector;
        typedef std::vector<Edge> EdgeVector;  

        inline TFaceProperties& operator[](Face f)  { return faces[f];  }
        inline const TFaceProperties& operator[](Face f) const  { return faces[f]; } 
        
        inline TEdgeProperties& operator[](Edge e)  { return g[e];  }
        inline const TEdgeProperties& operator[](Edge e) const  { return g[e];  }
        
        inline TVertexProperties& operator[](Vertex v)  { return g[v];  }
        inline const TVertexProperties& operator[](Vertex v) const  { return g[v];  }
        
//DATA
        std::vector< TFaceProperties > faces;
        BGLGraph g;

Vertex null_vertex() {
    return boost::graph_traits<BGLGraph>::null_vertex();
}

/// add a blank vertex and return its descriptor
Vertex add_vertex() { 
    return boost::add_vertex( g );
}

/*
/// add a vertex with given properties, return vertex descriptor
template < class FVertexProperty>
typename boost::graph_traits< BGLGraph >::vertex_descriptor add_vertex(typedef const FVertexProperty& prop) {
    return boost::add_vertex( prop, g );
}*/

/// add an edge between vertices v1-v2
Edge add_edge(Vertex v1, Vertex v2) {
    Edge e;
    bool b;
    boost::tie( e , b ) = boost::add_edge( v1, v2, g);
    return e;
}

/// add an edge with given properties between vertices v1-v2
//template < class EdgeProperty>
/*
typename boost::graph_traits< BGLGraph >::edge_descriptor add_edge( 
                                                       typename boost::graph_traits< BGLGraph >::vertex_descriptor v1, 
                                                       typename boost::graph_traits< BGLGraph >::vertex_descriptor v2, 
                                                       typename  TEdgeProperties prop
                                                       ) {
    typename boost::graph_traits< BGLGraph >::edge_descriptor e;
    bool b;
    boost::tie( e , b ) = boost::add_edge( v1, v2, prop, g);
    return e;
}*/

/// make e1 the twin of e2 (and vice versa)
void twin_edges( Edge e1, Edge e2 ) {
    g[e1].twin = e2;
    g[e2].twin = e1;
}


/// add a face 
Face add_face() {
    TFaceProperties f_prop;
    faces.push_back( f_prop); 
    Face index = faces.size()-1;
    faces[index].idx = index;
    return index;    
}
        

/// return the target vertex of the given edge
Vertex target(Edge e) const { return boost::target(e, g); }

/// return the source vertex of the given edge
Vertex source(Edge e) const { return boost::source(e, g); }

/// return all vertices in a vector of vertex descriptors
VertexVector vertices() const {
    typedef typename boost::graph_traits< BGLGraph >::vertex_descriptor  HEVertex;
    typedef std::vector<HEVertex> VertexVector;
    typedef typename boost::graph_traits< BGLGraph >::vertex_iterator    HEVertexItr;
    VertexVector vv;
    HEVertexItr it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( g );
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        vv.push_back( *itr );
    }
    return vv;
}

/// return all vertices adjacent to given vertex
VertexVector adjacent_vertices(  Vertex v) {
    VertexVector vv;
    BOOST_FOREACH( Edge edge, out_edges( v ) ) {
        vv.push_back( target( edge ) );
    }
    return vv;
}

/// return all vertices of given face

VertexVector face_vertices(Face face_idx) {
    VertexVector verts;
    Edge startedge = faces[face_idx].edge; // the edge where we start
    Vertex start_target = boost::target( startedge, g); 
    verts.push_back(start_target);
    Edge current = g[startedge].next;
    do {
        Vertex current_target = boost::target( current, g); 
        verts.push_back(current_target);
        current = g[current].next;
    } while ( current != startedge );
    return verts;
}

/// return edges of face f
EdgeVector face_edges( Face f ) {
    //Edge start_edge = g[ f ].edge; // was cast: (Face)f
    Edge start_edge = faces[f].edge;
    Edge current_edge = start_edge;
    EdgeVector out;
    do {
        out.push_back(current_edge);
        current_edge = g[current_edge].next;
    } while( current_edge != start_edge );
    return out;
}


/// return degree of given vertex
unsigned int degree( Vertex v)  { 
    return boost::degree( v, g); 
}

/// return number of vertices in graph
unsigned int num_vertices() const { 
    return static_cast<unsigned int>(boost::num_vertices( g ));
}

/// return out_edges of given vertex
EdgeVector out_edges( Vertex v) { 
    typedef typename boost::graph_traits< BGLGraph >::out_edge_iterator  HEOutEdgeItr;
    EdgeVector ev;
    HEOutEdgeItr it, it_end;
    boost::tie( it, it_end ) = boost::out_edges( v, g );
    for ( ; it != it_end ; ++it ) {
        ev.push_back(*it);
    }
    return ev;
}

/// return all edges
EdgeVector edges() const {
    typedef typename boost::graph_traits< BGLGraph >::edge_iterator      HEEdgeItr; 
    EdgeVector ev;
    HEEdgeItr it, it_end;
    boost::tie( it, it_end ) = boost::edges( g );
    for ( ; it != it_end ; ++it ) {
        ev.push_back(*it);
    }
    return ev;
}

/// return v1-v2 edge descriptor
Edge edge( Vertex v1, Vertex v2 ) {
    typedef typename std::pair<Edge, bool> EdgeBool;
    EdgeBool result = boost::edge(v1, v2, g );
    return result.first;
}

/// return the previous edge. traverses all edges in face until previous found.
Edge previous_edge( Edge e ) {
    Edge previous = g[e].next;
    while ( g[previous].next != e ) {
        previous = g[previous].next;
    }
    return previous;
}


/// return true if v1-v2 edge exists
bool has_edge( Vertex v1, Vertex v2) {
    typedef typename std::pair<Edge, bool> EdgeBool;
    EdgeBool result = boost::edge(v1, v2, g );
    return result.second;
}

/// return adjacent faces to the given vertex
FaceVector adjacent_faces( Vertex q ) {
    typedef typename boost::graph_traits< BGLGraph >::out_edge_iterator  HEOutEdgeItr;
    std::set<unsigned int> face_set;
    HEOutEdgeItr itr, itr_end;
    boost::tie( itr, itr_end) = boost::out_edges(q, g);
    for ( ; itr!=itr_end ; ++itr ) {
        face_set.insert( g[*itr].face );
    }
    FaceVector fv;
    BOOST_FOREACH(unsigned int m, face_set) {
        fv.push_back(m);
    }
    return fv;
}

/// return number of faces in graph
unsigned int num_faces() const { 
    return faces.size(); 
}

/// return number of edges in graph
unsigned int num_edges() const { 
    return static_cast<unsigned int>(boost::num_edges( g ));
}

/// inserts given vertex into edge e, and into the twin edge e_twin
void insert_vertex_in_edge(Vertex v, Edge e ) {
    // the vertex v is in the middle of edge e
    //                    face
    //                    e1   e2
    // previous-> source  -> v -> target -> next
    //            tw_trg  <- v <- tw_src <- tw_previous
    //                    te2  te1
    //                    twin_face
    
    Edge twin = g[e].twin;
    Vertex source = boost::source( e , g );
    Vertex target = boost::target( e , g);
    Vertex twin_source = boost::source( twin , g);
    Vertex twin_target = boost::target( twin , g );
    assert( source == twin_target );    
    assert( target == twin_source );
    
    Face face = g[e].face;
    Face twin_face = g[twin].face;
    Edge previous = previous_edge(e);
    assert( g[previous].face == g[e].face );
    Edge twin_previous = previous_edge(twin);
    assert( g[twin_previous].face == g[twin].face );
    
    Edge e1 = add_edge( source, v );
    Edge e2 = add_edge( v, target );
    
    // preserve the left/right face link
    g[e1].face = face;
    g[e2].face = face;
    // next-pointers
    g[previous].next = e1;
    g[e1].next = e2;
    g[e2].next = g[e].next;
    
    
    Edge te1 = add_edge( twin_source, v  );
    Edge te2 = add_edge( v, twin_target  );
    
    g[te1].face = twin_face;
    g[te2].face = twin_face;
    
    g[twin_previous].next = te1;
    g[te1].next = te2;
    g[te2].next = g[twin].next;
    
    // TWINNING (note indices 'cross', see ASCII art above)
    g[e1].twin = te2;
    g[te2].twin = e1;
    g[e2].twin = te1;
    g[te1].twin = e2;
    
    // update the faces (required here?)
    faces[face].edge = e1;
    faces[twin_face].edge = te1;
    
    // finally, remove the old edge
    boost::remove_edge( e   , g);
    boost::remove_edge( twin, g);
}


/// inserts given vertex into edge e
/*
template <class BGLGraph>
void insert_vertex_in_half_edge(typename boost::graph_traits< BGLGraph >::vertex_descriptor  v, 
                           typename boost::graph_traits< BGLGraph >::edge_descriptor e, 
                           BGLGraph& g) {
    typedef typename boost::graph_traits< BGLGraph >::edge_descriptor    HEEdge;
    typedef typename boost::graph_traits< BGLGraph >::vertex_descriptor  HEVertex;
    // the vertex v is in the middle of edge e
    //                    face
    //                    e1   e2
    // previous-> source  -> v -> target -> next
    
    HEVertex source = boost::source( e , g );
    HEVertex target = boost::target( e , g);
    unsigned int face = g[e].face;
    HEEdge previous = previous_edge(e, g);
    assert( g[previous].face == g[e].face );
    HEEdge e1 = add_edge( source, v , g);
    HEEdge e2 = add_edge( v, target , g);
    // preserve the left/right face link
    g[e1].face = face;
    g[e2].face = face;
    // next-pointers
    g[previous].next = e1;
    g[e1].next = e2;
    g[e2].next = g[e].next;
    // update the faces (required here?)
    g.faces[face].edge = e1;
    // finally, remove the old edge
    boost::remove_edge( e   , g);
    // NOTE: twinning is not done here, since the twin edge is not split...
}

*/
        /// check that all edges belong to the correct face, TODO: template this to make it a useful check
        /*
        bool checkFaces() {
            BOOST_FOREACH(FaceProps f, faces) {
                BOOST_FOREACH( HEEdge e, face_edges(f.idx)) {
                    if ( g[e].face != f.idx )
                        return false;
                }
            }
            return true;
        }*/

/// delete a vertex
void delete_vertex(Vertex v) { 
    clear_vertex(v);
    remove_vertex(v); 
}

/// clear given vertex. this removes all edges connecting to the vertex.
void clear_vertex( Vertex v) { 
    boost::clear_vertex( v, g ); 
}
/// remove given vertex
void remove_vertex( Vertex v) { 
    boost::remove_vertex( v , g );
}

void remove_edge( Vertex v1, Vertex v2) {
    boost::remove_edge( v1, v2  , g);
}

void remove_edge( Edge e) {
    boost::remove_edge( e   , g);
}

}; // end class definition


} // end hedi namespace

} // end ocl namespace
#endif
// end halfedgediagram.hpp
