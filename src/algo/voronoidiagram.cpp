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
    init();
}

VoronoiDiagram::~VoronoiDiagram() {
    //
}

VoronoiVertex VoronoiDiagram::add_vertex( Point pos, VoronoiVertexType t) {
    VoronoiVertex v = boost::add_vertex(vd);
    vd[v].position = pos;
    vd[v].type = t;
    return v;
}

void VoronoiDiagram::init() {
    VoronoiVertex center = add_vertex( Point(0,0), UNDECIDED );
    VoronoiVertex p1     = add_vertex( Point(0, 3*far_radius), FAR );
    VoronoiVertex p2     = add_vertex( Point( cos(-PI/6)*3*far_radius, sin(-PI/6)*3*far_radius), FAR );
    VoronoiVertex p3     = add_vertex( Point( cos(-5*PI/6)*3*far_radius, sin(-5*PI/6)*3*far_radius), FAR );
    boost::add_edge( center, p1 , vd );
    boost::add_edge( center, p2 , vd );
    boost::add_edge( center, p3 , vd );
    
    // the "arc" edges
    boost::add_edge( p1, p2 , vd );
    boost::add_edge( p2, p3 , vd );
    boost::add_edge( p3, p1 , vd );
    
    // add corresponding sites to dt
    VoronoiVertex v1 = boost::add_vertex(dd);
    VoronoiVertex v2 = boost::add_vertex(dd);
    VoronoiVertex v3 = boost::add_vertex(dd);
    dd[v1].position = Point( cos(PI/6)*3*far_radius, sin(PI/6)*3*far_radius) ;
    dd[v2].position = Point(cos(5*PI/6)*3*far_radius, sin(5*PI/6)*3*far_radius) ;
    dd[v3].position = Point( 0,-3*far_radius) ;
    boost::add_edge( v1 , v2 , dd );
    boost::add_edge( v2 , v3 , dd );
    boost::add_edge( v3 , v1 , dd );
}

void VoronoiDiagram::addVertexSite(Point p) {
    VoronoiVertex v = boost::add_vertex(dd);
    dd[v].position = p ;
}

boost::python::list VoronoiDiagram::getDelaunayVertices() const {
    boost::python::list plist;
    VoronoiVertexItr it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( dd );
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        plist.append( dd[*itr].position );
    }
    return plist;
}

boost::python::list VoronoiDiagram::getVoronoiVertices() const {
    boost::python::list plist;
    VoronoiVertexItr it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( vd );
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        if ( vd[*itr].type != FAR )
            plist.append( vd[*itr].position );
    }
    return plist;
}

boost::python::list VoronoiDiagram::getFarVertices() const {
    boost::python::list plist;
    VoronoiVertexItr it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( vd );
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        if ( vd[*itr].type == FAR )
            plist.append( vd[*itr].position );
    }
    return plist;
}

boost::python::list VoronoiDiagram::getDelaunayEdges() const {
    return getEdges(dd);
}

boost::python::list VoronoiDiagram::getVoronoiEdges() const {
    return getEdges(vd);
}

boost::python::list VoronoiDiagram::getEdges(VoronoiGraph g) const {
    boost::python::list edge_list;
    VoronoiEdgeItr itr, it_end;
    boost::tie( itr, it_end ) = boost::edges( g );
    for (  ; itr != it_end ; ++itr ) { // loop through each edge
            boost::python::list point_list; // the endpoints of each edge
            VoronoiVertex v1 = boost::source( *itr, g  );
            VoronoiVertex v2 = boost::target( *itr, g  );
            point_list.append( g[v1].position );
            point_list.append( g[v2].position );
            edge_list.append(point_list);
    }
    return edge_list;
}




std::string VoronoiDiagram::str() const {
    std::ostringstream o;
    o << "VoronoiDiagram (nVerts="<< boost::num_vertices( vd ) << " , nEdges="<< boost::num_edges( vd ) <<"\n";
    
    return o.str();
}

} // end namespace
// end file weave.cpp
