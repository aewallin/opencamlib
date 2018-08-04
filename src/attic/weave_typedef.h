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

#include <boost/graph/adjacency_list.hpp>

#include "point.h"

namespace ocl
{

/// vertex type: CL-point, internal point, adjacent point
enum WeaveVertexType {CL, CL_DONE, ADJ, TWOADJ, INT };

/// properties of a vertex in the weave
struct WeaveVertexProps {
    /// the position of the vertex
    Point position;
    /// the type of the vertex
    WeaveVertexType type;
    /// index?
    int index; // USED??
    /// the weave-component to which this vertex belongs. 
    std::size_t component; // USED??
};

/// properties of an edge in the weave
/*
struct WeaveEdgeProps {
    /// index? used?
    int index;
};*/

typedef boost::adjacency_list<     boost::listS,    // out-edges stored in a std::list
                                   boost::vecS,     // vertex set stored in a std::vector
                                   boost::undirectedS,  // an un directed  graph.
                                   WeaveVertexProps,   // vertex properties:
                                   // edge properties:
                                   boost::property< boost::edge_color_t, bool,
                                   boost::property< boost::edge_index_t, int > >
                                   > WeaveGraph; 
                    // graph_traits< G >::edge_parallel_category
                    // tags: disallow_parallel_edge_tag
typedef boost::graph_traits< WeaveGraph >::vertex_descriptor WeaveVertex;
typedef boost::graph_traits< WeaveGraph >::vertex_iterator VertexIterator;
typedef boost::graph_traits< WeaveGraph >::edge_descriptor WeaveEdge;
typedef boost::graph_traits< WeaveGraph >::edge_iterator EdgeIterator;
typedef boost::graph_traits< WeaveGraph >::out_edge_iterator OutEdgeIterator;
typedef boost::graph_traits< WeaveGraph >::adjacency_iterator AdjacencyIterator;
typedef boost::graph_traits< WeaveGraph >::vertices_size_type VertexSize;

/// intersections between intervals are stored as a VertexPair
/// pair.first is a vertex descriptor of the weave graph
/// pair.second is the coordinate along the fiber of the intersection
typedef std::pair< WeaveVertex, double > VertexPair;

/// compare based on pair.second, the coordinate of the intersection
struct VertexPairCompare {
    /// comparison operator
    bool operator() (const VertexPair& lhs, const VertexPair& rhs) const
    { return lhs.second < rhs.second ;}
};

/// intersections stored in this set (for rapid finding of neighbors etc)
typedef std::set< VertexPair, VertexPairCompare > VertexIntersectionSet;

typedef VertexIntersectionSet::iterator VertexPairIterator;                               

/// type for the planar embedding of the weave graph
/// the planar_face_traverse boost graph library algorithm requires a planar embedding to work
typedef std::vector< std::vector< WeaveEdge > > WeavePlanarEmbedding;

} // end namespace
#endif
// end weave_typedef.h
