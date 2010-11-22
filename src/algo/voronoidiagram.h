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

#include <boost/graph/adjacency_list.hpp>
#include <boost/python.hpp> 
#include <boost/foreach.hpp> 

#include "point.h"
#include "numeric.h"

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

namespace ocl
{

/// vertex type: 
enum VoronoiVertexType {OUT, IN, UNDECIDED, NEW };
//enum VoronoiEdgeType {LINE, ARC}; // for future line/arc generators...


// p209 of Handbook of Computational geometry suggests DCEL data structure
// doubly connected edge list
//
// each edge e stores:
// - two endpoints of e
// - edges cw or ccw next to e about its endpoints
// - faces the the left and to the right

// allows traversing edges incident on a vertex
// allows traversin edges bounding a face

// FWD declarations
struct VoronoiVertexProps;
struct VoronoiEdgeProps;
struct VoronoiFace;

typedef boost::adjacency_list<     boost::listS,    // out-edges stored in a std::list
                                   boost::vecS, //vecS,     // vertex set stored in a std::vector
                                   boost::bidirectionalS,  // bidirectional graph.
                                   // vertex properties:
                                   VoronoiVertexProps, 
                                   // edge properties:
                                   VoronoiEdgeProps
                                   > VoronoiGraph; 
                    // graph_traits< G >::edge_parallel_category
                    // tags: disallow_parallel_edge_tag
typedef boost::graph_traits< VoronoiGraph >::vertex_descriptor  VoronoiVertex;
typedef boost::graph_traits< VoronoiGraph >::vertex_iterator    VoronoiVertexItr;
typedef boost::graph_traits< VoronoiGraph >::edge_descriptor    VoronoiEdge;
typedef boost::graph_traits< VoronoiGraph >::edge_iterator      VoronoiEdgeItr;
typedef boost::graph_traits< VoronoiGraph >::out_edge_iterator  VoronoiOutEdgeItr;
typedef boost::graph_traits< VoronoiGraph >::adjacency_iterator VoronoiAdjacencyItr;
typedef boost::graph_traits< VoronoiGraph >::vertices_size_type VoronoiVertexSize;

typedef std::vector<VoronoiVertex> VertexVector;

struct VoronoiVertexProps {
    Point position;
    VoronoiVertexType type;
    // std::list<VoronoiEdge> dual_face_edges; // duality vd_vertex <-> dd_face  and dd_vertex<->vd_face 
    Point pk;
    double J2;
    double J3;
    double J4;
    void set_position() {
        double w = J4;
        double x = - J2/w+pk.x;
        double y = J3/w+pk.y;
        position =  Point(x,y);
    }
    double detH(const Point& pl) {
        return J2*(pl.x-pk.x) - J3*(pl.y-pk.y) + 0.5*J4*(square(pl.x-pk.x) + square(pl.y-pk.y));
    }
    void set_J(Point& pi, Point& pj, Point& pk) { // i-j-k should come in CCW order
        // point pk should have the largest angle
        // set the values of J2 J3 J4 and pk
        Point pi_,pj_,pk_;
        if ( pi.isRight(pj,pk) ) {
            pi_ = pj;
            pj_ = pi;
            pk_ = pk;
        } else {
            pi_ = pi;
            pj_ = pj;
            pk_ = pk;
        }
        this->pk = pk;
        J2 = detH_J2( pi_, pj_, pk_);
        J3 = detH_J3( pi_, pj_, pk_);
        J4 = detH_J4( pi_, pj_, pk_);
    }
    double detH_J2(Point& pi, Point& pj, Point& pk) {
        // J2(ijk)
        //   yi-yk   [(xi-xk)^2+(yi-yk)^2]/2
        //   yj-yk   [(xj-xk)^2+(yj-yk)^2]/2
        return (pi.y-pk.y)*(square(pj.x-pk.x)+square(pj.y-pk.y))/2 - (pj.y-pk.y)*(square(pi.x-pk.x)+square(pi.y-pk.y))/2;
    }
    double detH_J3(Point& pi, Point& pj, Point& pk) {
        // J3(ijk)
        //   xi-xk   ((xi-xk)^2+(yi-yk)^2)/2
        //   xj-xk   ((xj-xk)^2+(yj-yk)^2)/2
        return (pi.x-pk.x)*(square(pj.x-pk.x)+square(pj.y-pk.y))/2 - (pj.x-pk.x)*(square(pi.x-pk.x)+square(pi.y-pk.y))/2;
    }
    double detH_J4(Point& pi, Point& pj, Point& pk) {
        // J4(ijk) =
        //    xi-xk  yi-yk 
        //    xj-xk  yj-yk 
        return (pi.x-pk.x)*(pj.y-pk.y) - (pj.x-pk.x)*(pi.y-pk.y);
    }

};

typedef std::vector<unsigned int> FaceVector;
typedef unsigned int FaceIdx;


struct VoronoiEdgeProps {
    VoronoiEdge next; // next edge CCW on face
    VoronoiEdge twin; // twin edge
    FaceIdx face; // the face this edge belongs to
};


enum VoronoiFaceType {INCIDENT, NONINCIDENT};

struct VoronoiFace {
    VoronoiFace( VoronoiEdge e , Point gen, VoronoiFaceType t) {
        edge = e;
        generator = gen;
        type = t;
    }
    VoronoiEdge edge;
    Point generator;
    VoronoiFaceType type;
};

struct FaceList {
    FaceList() {
        faces.clear();
    }
    unsigned int add_face(VoronoiEdge e, Point gen, VoronoiFaceType t) {
        VoronoiFace f(e, gen, t);
        faces.push_back(f);
        return faces.size()-1;
    }
    unsigned int add_face(Point gen, VoronoiFaceType t) { // for when we don't know the associated edge
        return add_face( VoronoiEdge(), gen, t);
    }
    VoronoiFace& operator[](const unsigned int m) {
        return faces[m];
    }
    unsigned int size() const {
        return faces.size();
    }
    std::vector<VoronoiFace> faces;
};


typedef std::vector< std::vector< VoronoiEdge > > VoronoiPlanarEmbedding;

/// \brief Voronoi diagram.
///
/// the dual of a voronoi diagram is the delaunay diagram
// voronoi      delaunay
//  face        vertex
//  vertex      face 
//  edge        edge

class VoronoiDiagram {
    public:
        VoronoiDiagram();
        virtual ~VoronoiDiagram();
        void addVertexSite(Point p);
        boost::python::list getGenerators() ;
        
        boost::python::list getVoronoiVertices() const;
        boost::python::list getFarVoronoiVertices() const;
        
        boost::python::list getVoronoiEdges() const;
        boost::python::list getEdgesGenerators() ;
        
        std::string str() const;
        double getFarRadius() const {return far_radius;}
        void setFarRadius(double r) {far_radius = r;}
    private:

        VoronoiVertex add_vertex( Point position, VoronoiVertexType t );
        VoronoiEdge add_edge(VoronoiVertex v1, VoronoiVertex v2);
        
        unsigned int find_closest_face(const Point& p );

        VertexVector find_seed_vertex(FaceIdx face_idx, const Point& p);
        void augment_vertex_set(VertexVector& v0, Point& p); 
        void add_new_voronoi_vertices(VertexVector& v0, Point& p);
        
        bool isDegreeThree();
        bool current_and_next_on_same_face(VoronoiEdge current_edge); 
        
        VoronoiEdge find_previous_edge(VoronoiEdge e);
        
        VertexVector get_face_vertices(unsigned int face_idx);

        FaceVector get_adjacent_faces( VoronoiVertex q );
        FaceVector get_incident_faces();    
        FaceIdx split_faces(Point& p);
        void split_face(FaceIdx nf, FaceIdx f);
        void remove_vertex_set(VertexVector& v0 , FaceIdx newface);
        
        void insert_vertex_in_edge(VoronoiVertex v, VoronoiEdge e); 
        //VertexVector traverse_v0_tree(VertexVector& v);
        
        void reset_labels();
        
        void init();
        /// the Voronoi diagram
        VoronoiGraph vd;
        /// the Delaunay diagram
        //VoronoiGraph dd;
        /// the voronoi diagram is constructed for sites within a circle with radius far_radius
        double far_radius;
        FaceList  faces;
        VoronoiVertex v01;
        VoronoiVertex v02;
        VoronoiVertex v03;
};

} // end namespace
#endif
// end voronoidiagram.h
