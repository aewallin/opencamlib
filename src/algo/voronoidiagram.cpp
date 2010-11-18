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

VoronoiVertex VoronoiDiagram::add_vertex( Point pos, VoronoiVertexType t, VoronoiGraph& d) {
    VoronoiVertex v = boost::add_vertex(d);
    d[v].position = pos;
    d[v].type = t;
    return v;
}

VoronoiEdge VoronoiDiagram::add_edge(VoronoiVertex v1, VoronoiVertex v2, VoronoiGraph& d) {
    VoronoiEdge e;
    bool b;
    boost::tie( e , b ) = boost::add_edge( v1, v2, d);
    return e;
}

void VoronoiDiagram::init() {
    VoronoiVertex center = add_vertex( Point(0,0), UNDECIDED, vd );
    VoronoiVertex v1     = add_vertex( Point(0, 3*far_radius), OUT, vd );
    VoronoiVertex v2     = add_vertex( Point( cos(-PI/6)*3*far_radius, sin(-PI/6)*3*far_radius), OUT, vd );
    VoronoiVertex v3     = add_vertex( Point( cos(-5*PI/6)*3*far_radius, sin(-5*PI/6)*3*far_radius), OUT, vd );
    // the "peace" edges
    VoronoiEdge e1 = add_edge( center, v1 , vd );         
    VoronoiEdge e2 = add_edge( center, v2 , vd );
    VoronoiEdge e3 = add_edge( center, v3 , vd );
    // the "arc" edges
    VoronoiEdge arc_12 = add_edge( v1, v2 , vd );
    VoronoiEdge arc_23 = add_edge( v2, v3 , vd );
    VoronoiEdge arc_31 = add_edge( v3, v1 , vd );
    
    // add corresponding sites to dt
    VoronoiVertex d1 = add_vertex(Point(cos(PI/6)*3*far_radius, sin(PI/6)*3*far_radius),NONINCIDENT,dd);
    VoronoiVertex d2 = add_vertex(Point(cos(5*PI/6)*3*far_radius, sin(5*PI/6)*3*far_radius),NONINCIDENT,dd);
    VoronoiVertex d3 = add_vertex(Point( 0,-3*far_radius),NONINCIDENT,dd);
    
    VoronoiEdge dd12 = add_edge( d1 , d2 , dd );
    VoronoiEdge dd23 = add_edge( d2 , d3 , dd );
    VoronoiEdge dd31 = add_edge( d3 , d1 , dd );
    assign_dual_edge( e1, dd12 );
    assign_dual_edge( e2, dd31 );
    assign_dual_edge( e3, dd23 );
    
    // dual of Delaunay vertices are Voronoi-faces
    assign_dual_face_edge( dd, d1, e1 ); 
    assign_dual_face_edge( dd, d1, e2 ); 
    assign_dual_face_edge( dd, d1, arc_12 ); 
    assign_dual_face_edge( dd, d2, e1 ); 
    assign_dual_face_edge( dd, d2, e3 ); 
    assign_dual_face_edge( dd, d2, arc_31 );
    assign_dual_face_edge( dd, d3, e2 ); 
    assign_dual_face_edge( dd, d3, e3 ); 
    assign_dual_face_edge( dd, d3, arc_23 );
}

void VoronoiDiagram::assign_dual_face_edge(VoronoiGraph& d, VoronoiVertex v, VoronoiEdge e) {
    d[v].dual_face_edges.push_back(e);
}


void VoronoiDiagram::assign_dual_edge(VoronoiEdge vde, VoronoiEdge dde) {
    vd[vde].dual_edge = dde;
    dd[dde].dual_edge = vde;
}

void VoronoiDiagram::addVertexSite(Point p) {

    
    // 1) find the dd-vertex closest to p
    // 2) find the vd-face corresponding to p
    // 3) amont the vd-vertices in the found face, find the one with the smallest H
    // 4) expand the tree of to-be-deleted vertices as far as possible:
    //   4.1 tree property
    //   4.2 H<0 property
    
    // 1)
    VoronoiVertex dd_closest = find_closest_Delaunay_vertex( p );
    std::cout << " closest dd-vertex is "<<dd_closest<<" at " << dd[dd_closest].position << "\n";
    VoronoiVertex dd_new = add_vertex(p,NONINCIDENT,dd);
    std::cout << " added new vertex to dd: " << dd_new << "\n";
    
    // 2) evaluate H on the corresponding voronoi-face vertices
    std::cout << "voronoi face corresponding to dd-closest= "<<dd_closest<<" has edges:\n";
    //std::cout << " dd[dd_closest].dual_face_edges.size() = " << dd[dd_closest].dual_face_edges.size() << "\n";
    BOOST_FOREACH( VoronoiEdge e, dd[dd_closest].dual_face_edges ) {
        std::cout << "voronoi edge: " << e << "\n";
    }

}



VoronoiVertex VoronoiDiagram::find_closest_Delaunay_vertex( Point& p ) {
    VoronoiVertexItr it_end, itr;
    VoronoiVertex closest_vertex;
    double closest_distance = 3*far_radius;
    double d;
    boost::tie( itr, it_end ) = boost::vertices( dd );
    for (  ; itr != it_end ; ++itr ) {
        d = (dd[*itr].position - p).norm();
        if (d<closest_distance ) {
            closest_distance=d;
            closest_vertex=*itr;
        }
    }
    std::cout << " find_closest_Delaunay_vertex \n";
    std::cout << "   vertex_id= " << closest_vertex << " at distance " << closest_distance << "\n";
    assert( closest_distance < 3*far_radius ) ;
    return closest_vertex;
}

double VoronoiDiagram::detH(Point& pi, Point& pj, Point& pk, Point& pl) {
    // when a new vornoi-point qijk is generated, compute and store J2 J3 J4 and the use the
    // following for different pl:
    // J2(ijk)(xl-xk) - J3(ijk)(yl-yk) + (1/2)J4(ijk) [ (xl-xk)^2 + (yl-yk)^2 ]
    // choose pk so that among three angles pi-pj-pk the angle at pk is closest to pi/2
    // because abs( J4(ijk)/2 ) is the area of this triangle
    // compute H for points that are close to new generator pl
    

    double J2 = detH_J2(pi,pj,pk);
    double J3 = detH_J3(pi,pj,pk);
    double J4 = detH_J4(pi,pj,pk);
    
    return J2+J3+J4;
}

double VoronoiDiagram::detH_J2(Point& pi, Point& pj, Point& pk) {
    // J2(ijk)
    //   yi-yk   [(xi-xk)^2+(yi-yk)^2]/2
    //   yj-yk   [(xj-xk)^2+(yj-yk)^2]/2
    return (pi.y-pk.y)*(square(pj.x-pk.x)+square(pj.y-pk.y))/2 - (pj.y-pk.y)*(square(pi.x-pk.x)+square(pi.y-pk.y))/2;
}

double VoronoiDiagram::detH_J3(Point& pi, Point& pj, Point& pk) {
    // J3(ijk)
    //   xi-xk   ((xi-xk)^2+(yi-yk)^2)/2
    //   xj-xk   ((xj-xk)^2+(yj-yk)^2)/2
    return (pi.x-pk.x)*(square(pj.x-pk.x)+square(pj.y-pk.y))/2 - (pj.x-pk.x)*(square(pi.x-pk.x)+square(pi.y-pk.y))/2;
}

double VoronoiDiagram::detH_J4(Point& pi, Point& pj, Point& pk) {
    // J4(ijk) =
    //    xi-xk  yi-yk 
    //    xj-xk  yj-yk 
    return (pi.x-pk.x)*(pj.y-pk.y) - (pj.x-pk.x)*(pi.y-pk.y);
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

boost::python::list VoronoiDiagram::getEdges(const VoronoiGraph& g) const {
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
