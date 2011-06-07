/*  $Id$
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
#ifndef WEAVE2_TYPEDEF_H
#define WEAVE2_TYPEDEF_H

#include "halfedgediagram.hpp"

namespace ocl
{

namespace weave2
{

struct VertexProps; 
struct EdgeProps;
struct FaceProps;
typedef unsigned int Face;  
  
// the graph type for the weave
typedef HEDIGraph<     boost::listS,             // out-edges stored here
                       boost::listS,             // vertex set stored here
                       boost::bidirectionalS,    // undirecgted or bidirectional graph?
                       VertexProps,              // vertex properties
                       EdgeProps,                // edge properties
                       FaceProps,                // face properties
                       boost::no_property,       // graph properties
                       boost::listS             // edge storage
                       > WeaveGraph;

typedef boost::graph_traits< WeaveGraph >::vertex_descriptor  Vertex;
typedef boost::graph_traits< WeaveGraph >::vertex_iterator    VertexItr;
typedef boost::graph_traits< WeaveGraph >::edge_descriptor    Edge;
typedef boost::graph_traits< WeaveGraph >::edge_iterator      EdgeItr;
typedef boost::graph_traits< WeaveGraph >::out_edge_iterator  OutEdgeItr;
typedef boost::graph_traits< WeaveGraph >::adjacency_iterator AdjacencyItr;

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


/// vertex type: CL-point, internal point, adjacent point
enum VertexType {CL, CL_DONE, ADJ, TWOADJ, INT };



} // end weave2 namespace

} // end ocl namespace
#endif
// end file weave2.h
