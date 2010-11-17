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
#include "numeric.h"
//#include "pft_visitor.h"

namespace ocl
{

VoronoiDiagram::VoronoiDiagram() {
    far_radius = 100;
    vertexSites.clear();
    init();
}

VoronoiDiagram::~VoronoiDiagram() {
    //
}

VoronoiVertex VoronoiDiagram::add_vertex( Point pos, VoronoiVertexType t) {
    VoronoiVertex v = boost::add_vertex(g);
    g[v].position = pos;
    g[v].type = t;
    return v;
}

void VoronoiDiagram::init() {
    VoronoiVertex center = add_vertex( Point(0,0), UNDECIDED );
    VoronoiVertex p1 = add_vertex( Point(0,far_radius), FAR );
    VoronoiVertex p2 = add_vertex( Point(cos(PI/6)*far_radius,-sin(PI/6)*far_radius), FAR );
    VoronoiVertex p3 = add_vertex( Point(-cos(PI/6)*far_radius,-sin(PI/6)*far_radius), FAR );
    boost::add_edge( center, p1 , g );
    boost::add_edge( center, p2 , g );
    boost::add_edge( center, p3 , g );
    
    // the "arc" edges
    boost::add_edge( p1, p2 , g );
    boost::add_edge( p2, p3 , g );
    boost::add_edge( p3, p1 , g );
    
    // corresponding sites
    double vertex_far_r = far_radius/3;
    addVertexSite( Point(cos(PI/6)*vertex_far_r,sin(PI/6)*vertex_far_r) );
    addVertexSite( Point(-cos(PI/6)*vertex_far_r,sin(PI/6)*vertex_far_r) );
    addVertexSite( Point(0,-vertex_far_r) );
}

void VoronoiDiagram::addVertexSite(Point p) {
    vertexSites.push_back(p);
}

boost::python::list VoronoiDiagram::getVertexSites() const {
    boost::python::list plist;
    BOOST_FOREACH( Point p, vertexSites ) {
        plist.append(p);
    }
    return plist;
}

boost::python::list VoronoiDiagram::getVertices() const {
    boost::python::list plist;
    VoronoiVertexItr it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( g );
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        if (g[*itr].type != FAR )
            plist.append( g[*itr].position );
    }
    return plist;
}

boost::python::list VoronoiDiagram::getFarVertices() const {
    boost::python::list plist;
    VoronoiVertexItr it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( g );
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        if ( g[*itr].type == FAR )
            plist.append( g[*itr].position );
    }
    return plist;
}

boost::python::list VoronoiDiagram::getEdges() const {

    boost::python::list edge_list;
    VoronoiEdgeItr itr, it_end;
    boost::tie( itr, it_end ) = boost::edges( g );
    for (  ; itr != it_end ; ++itr ) { // loop through each edge
        
            boost::python::list point_list; // the endpoints of each edge
            VoronoiVertex v1 = boost::source( *itr, g  );
            VoronoiVertex v2 = boost::target( *itr, g  );
            point_list.append(g[v1].position);
            point_list.append(g[v2].position);
            edge_list.append(point_list);
        
    }
    return edge_list;

}


std::string VoronoiDiagram::str() const {
    std::ostringstream o;
    o << "VoronoiDiagram (nVerts="<< boost::num_vertices( g ) << " , nEdges="<< boost::num_edges( g ) <<"\n";
    
    return o.str();
}

} // end namespace
// end file weave.cpp
