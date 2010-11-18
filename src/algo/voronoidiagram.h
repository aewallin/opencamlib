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


// bundled BGL properties
// see: http://www.boost.org/doc/libs/1_44_0/libs/graph/doc/bundles.html

namespace ocl
{

/// vertex type: 
enum VoronoiVertexType {OUT, IN, UNDECIDED, INCIDENT, NONINCIDENT};
enum VoronoiEdgeType {LINE, ARC};




// FWD declarations
struct VoronoiVertexProps;
struct VoronoiEdgeProps;

typedef boost::adjacency_list<     boost::listS,    // out-edges stored in a std::list
                                   boost::vecS,     // vertex set stored in a std::vector
                                   boost::undirectedS,  // an un directed  graph.
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

struct VoronoiVertexProps {
    Point position;
    VoronoiVertexType type;
    std::list<VoronoiEdge> dual_face_edges; // duality vd_vertex <-> dd_face  and dd_vertex<->vd_face  
};

struct VoronoiEdgeProps {
    VoronoiEdgeType type;
    VoronoiEdge dual_edge; //  duality: vd_edge <-> dd_edge
    double t;
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
        boost::python::list getDelaunayVertices() const;
        
        boost::python::list getVoronoiVertices() const;

        boost::python::list getVoronoiEdges() const;
        boost::python::list getDelaunayEdges() const;
        boost::python::list getEdges(const VoronoiGraph& g) const;
        std::string str() const;
        double getFarRadius() const {return far_radius;}
        void setFarRadius(double r) {far_radius = r;}
    private:
        void assign_dual_edge(VoronoiEdge vd_e, VoronoiEdge dd_e);
        VoronoiVertex add_vertex( Point position, VoronoiVertexType t, VoronoiGraph& g);
        VoronoiEdge add_edge(VoronoiVertex v1, VoronoiVertex v2, VoronoiGraph& g);
        void assign_dual_face_edge(VoronoiGraph& d, VoronoiVertex v, VoronoiEdge e);
        
        VoronoiVertex find_closest_Delaunay_vertex( Point& p );
        
        double detH(Point& pi, Point& pj, Point& pk, Point& pl);
        double detH_J2(Point& pi, Point& pj, Point& pk);
        double detH_J3(Point& pi, Point& pj, Point& pk);
        double detH_J4(Point& pi, Point& pj, Point& pk);
        
        void init();
        /// the Voronoi diagram
        VoronoiGraph vd;
        /// the Delaunay diagram
        VoronoiGraph dd;
        /// the voronoi diagram is constructed for sites within a circle with radius far_radius
        double far_radius;
};

} // end namespace
#endif
// end voronoidiagram.h
