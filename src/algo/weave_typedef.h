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
#ifndef WEAVE_TYPEDEF_H
#define WEAVE_TYPEDEF_H

#include <boost/graph/adjacency_list.hpp>

#include "point.h"

namespace ocl
{

/// vertex type: CL-point, internal point, adjacent point
enum WeaveVertexType {CL, CL_DONE, ADJ, TWOADJ, INT };

struct WeaveVertexProps {
    Point position;
    WeaveVertexType type;
    int index;
    std::size_t component;
};

struct WeaveEdgeProps {
    int index;
};

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

typedef std::pair< WeaveVertex, double > VertexPair;

/// compare based on pair.second
struct VertexPairCompare {
    /// comparison operator
    bool operator() (const VertexPair& lhs, const VertexPair& rhs) const
    { return lhs.second < rhs.second ;}
};

typedef std::set< VertexPair, VertexPairCompare >::iterator VertexPairIterator;                               

typedef std::vector< std::vector< WeaveEdge > > WeavePlanarEmbedding;

} // end namespace
#endif
// end weave_typedef.h
