/*  $Id: voronoidiagram.cpp 664 2011-02-13 17:37:33Z anders.e.e.wallin $
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

#ifndef VODI_G_H
#define VODI_G_H

#include <vector>

#include <boost/graph/adjacency_list.hpp>

#include "point.h"
#include "halfedgediagram.hpp"



namespace ocl {

// this file contains typedefs used by voronoidiagram.h

struct VertexProps; 
struct EdgeProps;
struct FaceProps;


typedef unsigned int HEFace;    

// the type of graph with which we construct the voronoi-diagram
typedef HEDIGraph<     boost::listS,             // out-edges stored in a std::list
                       boost::listS,             // vertex set stored here
                       boost::bidirectionalS,    // bidirectional graph.
                       VertexProps,              // vertex properties
                       EdgeProps,                // edge properties
                       FaceProps,                // face properties
                       boost::no_property,       // graph properties
                       boost::listS              // edge storage
                       > HEGraph;

typedef boost::graph_traits< HEGraph >::vertex_descriptor  HEVertex;
typedef boost::graph_traits< HEGraph >::vertex_iterator    HEVertexItr;
typedef boost::graph_traits< HEGraph >::edge_descriptor    HEEdge;
typedef boost::graph_traits< HEGraph >::edge_iterator      HEEdgeItr;
typedef boost::graph_traits< HEGraph >::out_edge_iterator  HEOutEdgeItr;
typedef boost::graph_traits< HEGraph >::adjacency_iterator HEAdjacencyItr;
typedef boost::graph_traits< HEGraph >::vertices_size_type HEVertexSize;




// typedef std::vector< std::vector< HEEdge > > HEPlanarEmbedding;

/// voronoi-vertices can be of these four different types
/// as we incrementally construct the diagram the type is updated as follows:
/// OUT-vertices will not be deleted
/// IN-vertices will be deleted
/// UNDECIDED-vertices have not been examied yet
/// NEW-vertices are constructed on OUT-IN edges
enum VoronoiVertexType {OUT, IN, UNDECIDED, NEW };

/// properties of a vertex in the voronoi diagram
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
    void set_J(const Point& pi, const Point& pj, const Point& pkin) { 
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
/// each edge stores a pointer to the next HEEdge 
/// and the HEFace to which this HEEdge belongs
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
/// each face stores one edge on the boundary of the face
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


// these containers are used instead of iterators when accessing
// adjacent vertices, edges, faces.
// it may be faster to rewrite the code so it uses iterators, as does the BGL.
typedef std::vector<HEVertex> VertexVector;
typedef std::vector<HEFace> FaceVector;
typedef std::vector<HEEdge> EdgeVector;  


}
#endif
