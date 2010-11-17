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

#include <boost/graph/adjacency_list.hpp>

#include "point.h"

/*
namespace boost {
    enum vertex_position_t {vertex_position=20};
    enum vertex_type_t {vertex_type=21};
    //enum vertex_component_t {vertex_component=22};
    BOOST_INSTALL_PROPERTY(vertex, position);
    BOOST_INSTALL_PROPERTY(vertex, type);
    //BOOST_INSTALL_PROPERTY(vertex, component);
}*/

// see: http://www.boost.org/doc/libs/1_44_0/libs/graph/doc/bundles.html

namespace ocl
{

/// vertex type: 
enum VoronoiVertexType {OUT, IN, UNDECIDED};

struct VoronoiVertexProps {
    Point position;
    VoronoiVertexType type;
};

struct VoronoiEdgeProps {
    double t;
};

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

                               

typedef std::vector< std::vector< VoronoiEdge > > VoronoiPlanarEmbedding;

class VoronoiDiagram {
    public:
        VoronoiDiagram();
        virtual ~VoronoiDiagram();
        void add_vertex( Point& position, VoronoiVertexType t);
        std::string str() const;
    private:
        VoronoiGraph g;
};

} // end namespace
#endif
// end voronoidiagram.h
