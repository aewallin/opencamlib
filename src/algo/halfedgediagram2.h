/*  $Id: $
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
#ifndef HEDI2_H
#define HEDI2_H

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

// fwd decl:    
struct VertexProps; 
struct EdgeProps;
//struct FaceProps;
//struct HEEdge;

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




// typedef std::vector< std::vector< HEEdge > > HEPlanarEmbedding;

enum VoronoiVertexType {OUT, IN, UNDECIDED, NEW };

/// vertex properties of a vertex in the voronoi diagram
struct VertexProps {
    VertexProps() {
        init();
    }
    /// construct vertex at position p with type t
    VertexProps( Point p, VoronoiVertexType t) {
        position=p;
        type=t;
        init();
    }
    void init() {
        index = count;
        count++;
        in_queue = false;
    }
    void reset() {
        in_queue = false;
        type = UNDECIDED;
    }
    /// based on previously calculated J2, J3, and J4, set the position of the vertex
    void set_position() {
        double w = J4;
        double x = - J2/w+pk.x;
        double y = J3/w+pk.y;
        position =  Point(x,y);
    }
    /// based on precalculated J2, J3, J4, calculate the H determinant for Point pl
    double detH(const Point& pl) {
        H = J2*(pl.x-pk.x) - J3*(pl.y-pk.y) + 0.5*J4*(square(pl.x-pk.x) + square(pl.y-pk.y));
        return H;
    }
    bool operator<(const VertexProps& other) const {
        return ( abs(this->H) < abs(other.H) );
    }
    /// set the J values
    void set_J(Point& pi, Point& pj, Point& pkin) { 
        // 1) i-j-k should come in CCW order
        Point pi_,pj_,pk_;
        if ( pi.isRight(pj,pkin) ) {
            pi_ = pj;
            pj_ = pi;
            pk_ = pkin;
        } else {
            pi_ = pi;
            pj_ = pj;
            pk_ = pkin;
        }
        assert( !pi_.isRight(pj_,pk_) );
        // 2) point pk should have the largest angle 
        // largest angle is opposite longest side.
        Point pi__,pj__,pk__;
        pi__ = pi_;                          
        pj__ = pj_;                          
        pk__ = pk_;
        double longest_side = (pi_ - pj_).xyNorm();
        if (  (pj_ - pk_).xyNorm() > longest_side ) {
            longest_side = (pj_ - pk_).xyNorm(); //j-k is longest, so i should be new k
            pk__ = pi_;                         // old  i-j-k 
            pi__ = pj_;                         // new  k-i-j
            pj__ = pk_;
        }
        if ( (pi_ - pk_).xyNorm() > longest_side ) { // i-k is longest, so j should be new k                    
            pk__ = pj_;                          // old  i-j-k
            pj__ = pi_;                          // new  j-k-i
            pi__ = pk_;
        }
        
        assert( !pi__.isRight(pj__,pk__) );
        assert( (pi__ - pj__).xyNorm() >=  (pj__ - pk__).xyNorm() );
        assert( (pi__ - pj__).xyNorm() >=  (pk__ - pi__).xyNorm() );
        
        this->pk = pk__;
        J2 = detH_J2( pi__, pj__, pk__);
        J3 = detH_J3( pi__, pj__, pk__);
        J4 = detH_J4( pi__, pj__, pk__);
        //assert( J4 > 0.0 );
    }
    /// calculate J2
    double detH_J2(Point& pi, Point& pj, Point& pk) {
        return (pi.y-pk.y)*(square(pj.x-pk.x)+square(pj.y-pk.y))/2 - (pj.y-pk.y)*(square(pi.x-pk.x)+square(pi.y-pk.y))/2;
    }
    /// calculate J3
    double detH_J3(Point& pi, Point& pj, Point& pk) {
        return (pi.x-pk.x)*(square(pj.x-pk.x)+square(pj.y-pk.y))/2 - (pj.x-pk.x)*(square(pi.x-pk.x)+square(pi.y-pk.y))/2;
    }
    /// calculate J4
    double detH_J4(Point& pi, Point& pj, Point& pk) {
        return (pi.x-pk.x)*(pj.y-pk.y) - (pj.x-pk.x)*(pi.y-pk.y);
    }
// VD DATA
    /// vertex type
    VoronoiVertexType type;
    /// the reference point for J-calculations
    Point pk;
    /// J2 determinant
    double J2;
    /// J3 determinant
    double J3;
    /// J4 determinant
    double J4;
    double H;
    bool in_queue;
// HE data
    /// the position of the vertex
    Point position;
    /// index of vertex
    int index;
    /// global vertex count
    static int count;
};

/// properties of an edge in the voronoi diagram
// HEEdge, HEFace
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
// HEFace, HEEdge
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




//template <class VertexProps, class EdgeProps, class FaceProps>
/// half-edge diagram class.
class HalfEdgeDiagram  {
    public:

        typedef std::vector<HEVertex> VertexVector;
        typedef std::vector<HEFace> FaceVector;
        typedef std::vector<HEEdge> EdgeVector;  
    
        HalfEdgeDiagram() {}
        virtual ~HalfEdgeDiagram() {}
    
    // add vertex,edge,face to diagram
        /// add a blank vertex and return its descriptor
        HEVertex add_vertex() { 
            return boost::add_vertex( g );
        }
        /// add a vertex with given properties, return vertex descriptor
        HEVertex add_vertex(VertexProps prop) {
            return boost::add_vertex( prop, g );
        }
        /// add an edge between vertices v1-v2
        HEEdge add_edge(HEVertex v1, HEVertex v2) {
            HEEdge e;
            bool b;
            boost::tie( e , b ) = boost::add_edge( v1, v2, g);
            return e;
        }
        /// add an edge with given properties
        HEEdge add_edge(HEVertex v1, HEVertex v2, EdgeProps prop) {
            HEEdge e;
            bool b;
            boost::tie( e , b ) = boost::add_edge( v1, v2, prop, g);
            return e;
        }
        /// add a face with given properties
        HEFace add_face(FaceProps f_prop) {
            faces.push_back(f_prop);
            HEFace index = faces.size()-1;
            faces[index].idx = index;
            return index;    
        }
        /// delete a vertex
        void delete_vertex(HEVertex v) { 
            clear_vertex(v);
            remove_vertex(v); 
        }
        
    // access vertices    
        /// return the target vertex of the given edge
        HEVertex target(HEEdge e) const { 
            return boost::target( e, g);
        }
        /// return the source vertex of the given edge
        HEVertex source(HEEdge e) const { 
            return boost::source( e, g); 
        }
        /// return all vertices
        VertexVector vertices() const {
            VertexVector vv;
            HEVertexItr it_begin, it_end, itr;
            boost::tie( it_begin, it_end ) = boost::vertices( g );
            for ( itr=it_begin ; itr != it_end ; ++itr ) {
                vv.push_back( *itr );
            }
            return vv;
        }
        /// return all vertices adjecent to given vertex
        VertexVector adjacent_vertices(HEVertex v) const {
            VertexVector vv;
            BOOST_FOREACH( HEEdge edge, out_edges( v ) ) {
                vv.push_back( target( edge ) );
            }
            return vv;
        }
        /// return all vertices of given face
        VertexVector face_vertices(HEFace face_idx) const {
            VertexVector verts;
            HEEdge startedge = faces[face_idx].edge; // the edge where we start
            HEVertex start_target = boost::target( startedge, g); 
            verts.push_back(start_target);
            HEEdge current = g[startedge].next;
            do {
                HEVertex current_target = boost::target( current, g); 
                assert( current_target != start_target );
                verts.push_back(current_target);
                current = g[current].next;
            } while ( current != startedge );
            return verts;
        }
        /// return degree of given vertex
        unsigned int degree(HEVertex v) const { 
            return boost::degree( v, g); 
        }
        /// return number of vertices in graph
        unsigned int num_vertices() const { 
            return boost::num_vertices( g ); 
        }
        
    // access edges
        /// return out_edges of given vertex
        EdgeVector out_edges( HEVertex v ) const {
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
            EdgeVector ev;
            HEEdgeItr it, it_end;
            boost::tie( it, it_end ) = boost::edges( g );
            for ( ; it != it_end ; ++it ) {
                ev.push_back(*it);
            }
            return ev;
        }
        /// return edges of face f
        EdgeVector face_edges(HEFace f) {
            HEEdge start_edge = faces[f].edge;
            HEEdge current_edge = start_edge;
            EdgeVector out;
            do {
                out.push_back(current_edge);
                current_edge = g[current_edge].next;
            } while( current_edge != start_edge );
            return out;
        }
        /// return the previous edge. traverses all edges in face until previous found.
        HEEdge previous_edge(HEEdge e) {
            HEEdge previous = g[e].next;
            while ( g[previous].next != e ) {
                previous = g[previous].next;
            }
            return previous;
        }
        /// return number of edges in graph
        unsigned int num_edges() const { 
            return boost::num_edges( g ); 
        }
        /// return true if v1-v2 edge exists
        bool edge( HEVertex v1, HEVertex v2 ) {
            typedef std::pair<HEEdge, bool> EdgeBool;
            EdgeBool result = boost::edge(v1, v2, g );
            return result.second;
        }
        
    // access faces
        /// return adjacent faces to the given vertex
        FaceVector adjacent_faces( HEVertex q ) {
            std::set<HEFace> face_set;
            HEOutEdgeItr itr, itr_end;
            boost::tie( itr, itr_end) = boost::out_edges(q, g);
            for ( ; itr!=itr_end ; ++itr ) {
                face_set.insert( g[*itr].face );
            }
            //assert( face_set.size() == 3); // degree of q is three, so has three faces
            FaceVector fv;
            BOOST_FOREACH(HEFace m, face_set) {
                fv.push_back(m);
            }
            return fv;
        }
        /// return number of faces in graph
        unsigned int num_faces() const { return faces.size(); }
        
    // Directly access vertex,edge,face properties
        /// access to the base-class operator[]
        //using HEGraph::operator[]; 
        /// operator[] for accessing face properties
        VertexProps& operator[](HEVertex v)  { return g[v]; }
        const VertexProps& operator[](HEVertex v)  const { return g[v]; }
        EdgeProps& operator[](HEEdge e)  { return g[e]; }
        const EdgeProps& operator[](HEEdge e)  const { return g[e]; }
        
        FaceProps& operator[](HEFace f)  { return faces[f]; }
        /// const operator[] for accessing face properties
        const FaceProps& operator[](HEFace f) const  { return faces[f]; }
        
    // general half-edge methods:
        /// inserts given vertex and its twin into edge e
        void insert_vertex_in_edge(HEVertex v, HEEdge e) {
            // the vertex v is in the middle of edge e
            //                    face
            //                    e1   e2
            // previous-> source  -> v -> target -> next
            //            tw_trg  <- v <- tw_src <- tw_previous
            //                    te2  te1
            //                    twin_face
            
            HEEdge twin = g[e].twin;
            HEVertex source = boost::source( e , g );
            HEVertex target = boost::target( e , g);
            HEVertex twin_source = boost::source( twin , g);
            HEVertex twin_target = boost::target( twin , g );
            assert( source == twin_target );    
            assert( target == twin_source );
            
            HEFace face = g[e].face;
            HEFace twin_face = g[twin].face;
            HEEdge previous = previous_edge(e);
            assert( g[previous].face == g[e].face );
            HEEdge twin_previous = previous_edge(twin);
            assert( g[twin_previous].face == g[twin].face );
            
            HEEdge e1 = add_edge( source, v  );
            HEEdge e2 = add_edge( v, target  );
            
            // preserve the left/right face link
            g[e1].face = face;
            g[e2].face = face;
            // next-pointers
            g[previous].next = e1;
            g[e1].next = e2;
            g[e2].next = g[e].next;
            
            
            HEEdge te1 = add_edge( twin_source, v  );
            HEEdge te2 = add_edge( v, twin_target  );
            
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

        
        /// check that all edges belong to the correct face
        bool checkFaces() {
            BOOST_FOREACH(FaceProps f, faces) {
                BOOST_FOREACH( HEEdge e, face_edges(f.idx)) {
                    if ( g[e].face != f.idx )
                        return false;
                }
            }
            return true;
        }
        
    private:
        /// clear given vertex. this removes all edges connecting to the vertex.
        void clear_vertex(HEVertex v) { 
            boost::clear_vertex( v, g ); 
        }
        /// remove given vertex
        void remove_vertex(HEVertex v) { 
            boost::remove_vertex( v , g );
        }
    // DATA
        /// a vector of face properties
        std::vector<FaceProps> faces;
        /// the boost BGL graph
        HEGraph g;
};

} // end namespace
#endif
// end halfedgediagram2.h
