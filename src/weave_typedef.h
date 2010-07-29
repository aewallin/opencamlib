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
//#include "ccpoint.h"
//#include "numeric.h"
#include "fiber.h"

namespace boost {
    enum vertex_position_t {vertex_position=20};
    enum vertex_type_t {vertex_type=21};
    BOOST_INSTALL_PROPERTY(vertex, position);
    BOOST_INSTALL_PROPERTY(vertex, type);
}

namespace ocl
{




/// vertex type: CL-point, internal point, adjacent point
enum VertexType {CL, INT, ADJ, CL_DONE};
             
typedef boost::adjacency_list<     boost::listS,    // out-edges stored in a std::list
                                   boost::vecS,     // vertex set stored in a std::vector
                                   boost::undirectedS,  // an un directed  graph.
                                   // vertex properties:
                                   boost::property< boost::vertex_position_t , Point, // 3D position in space
                                        boost::property< boost::vertex_color_t, boost::default_color_type  ,
                                        boost::property< boost::vertex_type_t, VertexType  ,
                                        boost::property< boost::vertex_distance_t, std::size_t, 
                                        boost::property<boost::vertex_degree_t, int,
                                        boost::property<boost::vertex_in_degree_t, int,
                                        boost::property<boost::vertex_out_degree_t, int > > > > > > >, 
                                   // edge properties:
                                   boost::property< boost::edge_weight_t, double >
                                   > WeaveGraph; 
                    // graph_traits< G >::edge_parallel_category
                    // tags: disallow_parallel_edge_tag
typedef boost::graph_traits< WeaveGraph >::vertex_descriptor VertexDescriptor;
typedef boost::graph_traits< WeaveGraph >::vertex_iterator VertexIterator;
typedef boost::graph_traits< WeaveGraph >::edge_iterator EdgeIterator;
typedef boost::graph_traits< WeaveGraph >::adjacency_iterator AdjacencyIterator;
typedef boost::graph_traits< WeaveGraph >::vertices_size_type VertexSize;


typedef std::pair< VertexDescriptor, double > VertexPair;
struct VertexPairCompare {
  bool operator() (const VertexPair& lhs, const VertexPair& rhs) const
  { return lhs.second < rhs.second ;}
};
typedef std::set< VertexPair, VertexPairCompare >::iterator VertexPairIterator;
//typedef ::iterator VertexPairIterator;

/*
typedef boost::adjacency_list<     boost::listS,    // out-edges stored in a std::list
                                   boost::vecS,     // vertex set stored in a std::vector
                                   boost::undirectedS,  // an un directed  graph.
                                   // vertex properties:
                                   boost::property< boost::vertex_name_t , Interval,
                                        boost::property< boost::vertex_color_t, bool > >,
                                   // edge properties:
                                   boost::property< boost::edge_weight_t, Point >
                                   > InvWeaveGraph;
typedef boost::graph_traits< InvWeaveGraph >::vertex_descriptor InvVertexDescriptor;
typedef boost::graph_traits< InvWeaveGraph >::edge_descriptor InvEdgeDescriptor;
typedef boost::graph_traits< InvWeaveGraph >::vertex_iterator InvVertexIterator;
typedef boost::graph_traits< InvWeaveGraph >::edge_iterator InvEdgeIterator;
  */                                 

}
#endif
