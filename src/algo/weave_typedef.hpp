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
#ifndef WEAVE_TYPEDEF_H
#define WEAVE_TYPEDEF_H

#include "halfedgediagram.hpp"
#include "interval.hpp"

namespace ocl {

namespace weave {


// we use the traits-class here so that EdgeProps can have Edge as a member
typedef boost::adjacency_list_traits<boost::listS, 
                                     boost::listS, 
                                     boost::bidirectionalS, 
                                     boost::listS >::edge_descriptor Edge;

/// vertex type: CL-point, internal point, adjacent point
enum VertexType {CL, CL_DONE, ADJ, TWOADJ, INT, FULLINT};

/// vertex properties
struct VertexProps {
    VertexProps() {
        init();
    }
    /// construct vertex at position p with type t
    VertexProps( Point p, VertexType t) {
        position=p;
        type=t;
        init();
    }
    /// construct vertex at position p with type t
    VertexProps( Point p, VertexType t, std::vector<Interval>::iterator x, std::vector<Interval>::iterator y )
    : xi( x ), yi( y ) {
        position=p;
        type=t;
        init();
    }
    
    void init() {
        index = count;
        count++;
    }
    VertexType type;
// HE data
    /// the position of the vertex
    Point position;
    /// index of vertex
    int index;
    /// global vertex count
    static int count;
    
    // x interval
    std::vector<Interval>::iterator xi;
    // y interval
    std::vector<Interval>::iterator yi;
    
};

/// edge properties
struct EdgeProps {
    EdgeProps() {}
    /// the next edge, counterclockwise, from this edge
    Edge next;
    /// previous edge, to make Weave::build() faster, since we avoid calling hedi::previous_edge() 
    Edge prev;
    /// the twin edge
    Edge twin;
};

typedef unsigned int Face; 

/// properties of a face in the weave
struct FaceProps {
    /// create face with given edge, generator, and type
    FaceProps( Edge e ) {
        edge = e;
    }
    /// face index
    Face idx;
    /// one edge that bounds this face
    Edge edge;
};

 
  
// the graph type for the weave
typedef ocl::hedi::HEDIGraph<     boost::listS,             // out-edges stored here
                       boost::listS,             // vertex set stored here
                       boost::bidirectionalS,    // undirecgted or bidirectional graph?
                       VertexProps,              // vertex properties
                       EdgeProps,                // edge properties
                       FaceProps,                // face properties
                       boost::no_property,       // graph properties
                       boost::listS             // edge storage
                       > WeaveGraph;

typedef WeaveGraph::Vertex Vertex;
typedef WeaveGraph::VertexItr    VertexItr;

//typedef boost::graph_traits< WeaveGraph >::vertex_descriptor  Vertex;
//
// typedef boost::graph_traits< WeaveGraph >::edge_descriptor    Edge;
//typedef boost::graph_traits< WeaveGraph >::edge_iterator      EdgeItr;
//typedef boost::graph_traits< WeaveGraph >::out_edge_iterator  OutEdgeItr;
//typedef boost::graph_traits< WeaveGraph >::adjacency_iterator AdjacencyItr;

/// intersections between intervals are stored as a VertexPair
/// pair.first is a vertex descriptor of the weave graph
/// pair.second is the coordinate along the fiber of the intersection
typedef std::pair< Vertex, double > VertexPair;

/// compare based on pair.second, the coordinate of the intersection
struct VertexPairCompare {
    /// comparison operator
    bool operator() (const VertexPair& lhs, const VertexPair& rhs) const
    { return lhs.second < rhs.second ;}
};

/// intersections stored in this set (for rapid finding of neighbors etc)
typedef std::set< VertexPair, VertexPairCompare > VertexIntersectionSet;

typedef VertexIntersectionSet::iterator VertexPairIterator;    

} // end weave namespace

} // end ocl namespace
#endif
// end file weave_typedef.hpp
