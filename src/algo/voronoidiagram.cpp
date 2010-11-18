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

#include <set>
#include <stack>

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
    
    // dual of Voronoi vertices are Delaunay-faces
    assign_dual_face_edge( vd, center, dd12 );
    assign_dual_face_edge( vd, center, dd23 );
    assign_dual_face_edge( vd, center, dd31 );
    
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
    // B1.1
    VoronoiVertex dd_closest = find_closest_Delaunay_vertex( p );
    std::cout << " closest dd-vertex is "<<dd_closest<<" at " << dd[dd_closest].position << "\n";
    VoronoiVertex dd_new = add_vertex(p,NONINCIDENT,dd);
    std::cout << " added new vertex to dd: " << dd_new << "\n";
    
    // 2) evaluate H on the corresponding voronoi-face vertices
    std::cout << "voronoi face corresponding to dd-closest= "<<dd_closest<<" has edges:\n";
    
    // B1.2
    VoronoiVertex vd_seed = find_seed_vd_vertex(dd_closest, dd_new);
    std::cout << " seed vertex is " << vd_seed << " at " << vd[vd_seed].position << "\n";
    
    // add seed to set V0
    VertexVector v0;
    v0.push_back(vd_seed);
    vd[vd_seed].type = IN;
    augment_vertex_set(v0);
    
    // generate new vertices on all edges connecting to V0
    
    // generate new edges that form a loop around the region
    
    // remove vertices V0 and edges
}

void VoronoiDiagram::augment_vertex_set(VertexVector& q) {
    // RB2   voronoi-point q(a,b,c) is the seed
    // add cycles Ca,Cb,Cc to the stack
    VertexVector dd_cycles = get_generator_vertices( dd, vd, q[0] );
    assert(dd_cycles.size()==3);
    // B1.3
    std::stack<VoronoiVertex> S;
    BOOST_FOREACH(VoronoiVertex cycle, dd_cycles) {
        dd[cycle].type = INCIDENT;
        S.push(cycle);
    }
    while ( !S.empty() ) { 
        // BOOST_FOREACH(VoronoiVertex cycle, dd_cycles) { //loop through each cycle
        VoronoiVertex c_alfa = S.top();
        S.pop();
        dd[c_alfa].type = INCIDENT;
        VertexVector cycle_verts = get_generator_vertices(vd, dd, c_alfa); // get vertices of this cycle
        // B2.1  mark "out"  v in cycle_alfa if 
        //  (T6) v is adjacent to an IN vertex in V-Vin
        //  (T7) v is on an "incident" cycle other than cycle and is not adjacent to a vertex in Vin
        
        // B2.2 if subgraph (Vout,Eout) is disconnected, find minimal set V* of undecided vertices such that Vout U V* is connected
        // and set v=OUT for all V*
        
        // B2.3 if there is no "OUT" vertex in V(cycle) find vertkex v "undecided" with maximal H and put v=OUT
        
        // B2.4 while UNDECIDED vertices remain:
        //      B2.4.1 find an undecided vertex Qabc with maximal abs(H)
        //      B2.4.2 if H >= 0, put v=OUT.   if Vout disconnected, find V* of "undecided" vertices to make it connected
        //             and mark all V* OUT
        //      B2.4.3 if H < 0 set v=IN for a,b,c
        //         if c(j)=nonincident,  put c(j)=incident and add c(j) to the stack
        //         if Vin disconnected,
        //            (i) find subset C* to make it connected
        //            (ii) mark all of V* IN
        //            (iii) for nonincident cycles cj indcident on vertices in V*
        //                 put cj="incident" and add to stack.
        //
            
    }
}

VoronoiVertex VoronoiDiagram::find_seed_vd_vertex(VoronoiVertex dd_closest, VoronoiVertex dd_new) {
    // evaluate H on all vertices along dd_closest:s dual facet edges and return
    // the one with the lowest H
    VertexVector q_verts = get_generator_vertices(vd, dd, dd_closest);
    double minimumH = 1;
    VoronoiVertex minimalVertex;
    BOOST_FOREACH( VoronoiVertex q, q_verts) {
        if ( vd[q].type != OUT ) {
            // for voronoi vertex q, find the regions i j k that define it
            VertexVector generators = get_generator_vertices(dd,vd,q);
            std::cout << q << " at " << vd[q].position << " has "<< generators.size() << "generators \n";
            assert( generators.size() == 3 );
            double h = detH( dd[generators[0]].position, dd[generators[1]].position, dd[generators[2]].position, dd[dd_new].position);
            std::cout << " h= " << h << "\n";
            Point qnew = newVoronoiVertex(dd[generators[0]].position, dd[generators[1]].position, dd[generators[2]].position, dd[dd_new].position);
            std::cout << " qnew= " << qnew << "\n";
            if (h<minimumH) {
                minimumH = h;
                minimalVertex = q;
            }
        }
        
    }
    return minimalVertex;
}


VertexVector VoronoiDiagram::get_generator_vertices(VoronoiGraph& dual, VoronoiGraph& diagram, VoronoiVertex v) {
    std::set<VoronoiVertex> verts;
    BOOST_FOREACH( VoronoiEdge e, diagram[v].dual_face_edges ) {
        VoronoiVertex v1 = boost::source( e, dual  );
        VoronoiVertex v2 = boost::target( e, dual  );
        verts.insert(v1);
        verts.insert(v2); 
    }
    VertexVector vv;
    BOOST_FOREACH( VoronoiVertex v, verts) {
        vv.push_back(v);
    }
    return vv;
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
    double h = J2*(pl.x-pk.x) - J3*(pl.y-pk.y) + 0.5*J4*(square(pl.x-pk.x) + square(pl.y-pk.y)) ;
    if ( pj.isRight(pi,pk) )
        return h;
    else
        return -h;
}

Point VoronoiDiagram::newVoronoiVertex(Point& pi, Point& pj, Point& pk, Point& pl) {
    double w = detH_J4(pi,pj,pk);
    double x = - detH_J2(pi,pj,pk)/w+pk.x;
    double y = detH_J3(pi,pj,pk)/w+pk.y;
    return Point(x,y);
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
