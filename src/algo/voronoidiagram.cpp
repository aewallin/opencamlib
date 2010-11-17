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

#include <boost/foreach.hpp>
#include <boost/graph/adjacency_list.hpp> 
#include <boost/graph/connected_components.hpp>
#include <boost/graph/planar_face_traversal.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>

#include "voronoidiagram.h"
//#include "pft_visitor.h"

namespace ocl
{

VoronoiDiagram::VoronoiDiagram() {
    //fibers.clear();
    //xfibers.clear();
    //yfibers.clear();
    //loops.clear();
}

VoronoiDiagram::~VoronoiDiagram() {
    //
}

void VoronoiDiagram::add_vertex( Point& pos, VoronoiVertexType t) {
    VoronoiVertex v = boost::add_vertex(g);
    g[v].position = pos;
    g[v].type = t;
    //boost::put( boost::vertex_position , g , v , position );
    //boost::put( boost::vertex_type , g , v , t );
    //i.intersections.insert( VertexPair( v, ipos) );
}

/*
void Weave::printGraph() const {
    std::cout << " number of vertices: " << boost::num_vertices( g ) << "\n";
    std::cout << " number of edges: " << boost::num_edges( g ) << "\n";
    VertexIterator it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( g );
    int n=0, n_cl=0, n_internal=0;
    
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        if ( boost::get( boost::vertex_type, g, *itr ) == CL )
            ++n_cl;
        else
            ++n_internal;
        ++n;
    }
    std::cout << " counted " << n << " vertices\n";
    std::cout << "          CL-nodes: " << n_cl << "\n";
    std::cout << "    internal-nodes: " << n_internal << "\n";
}
*/
  

std::string VoronoiDiagram::str() const {
    std::ostringstream o;
    o << "VoronoiDiagram\n";
    
    return o.str();
}

} // end namespace
// end file weave.cpp
