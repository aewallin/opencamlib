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

VoronoiEdge VoronoiDiagram::add_edge(VoronoiVertex v1, VoronoiVertex v2) {
    VoronoiEdge e;
    bool b;
    boost::tie( e , b ) = boost::add_edge( v1, v2, vd);
    return e;
}

void VoronoiDiagram::init() {
    double far_multiplier = 6;
    // add vertices
    VoronoiVertex v0     = add_vertex( Point(0,0), UNDECIDED);
    VoronoiVertex v1     = add_vertex( Point(0, far_multiplier*far_radius), OUT );
    VoronoiVertex v3     = add_vertex( Point( cos(-PI/6)*far_multiplier*far_radius, sin(-PI/6)*far_multiplier*far_radius), OUT );
    VoronoiVertex v2     = add_vertex( Point( cos(-5*PI/6)*far_multiplier*far_radius, sin(-5*PI/6)*far_multiplier*far_radius), OUT );
    // add the "peace" edges and the "arc" edges
    VoronoiEdge e1 = add_edge( v0, v1 );         
    VoronoiEdge e2 = add_edge( v0, v2 );
    VoronoiEdge e3 = add_edge( v0, v3 );
    VoronoiEdge e4 = add_edge( v1, v2 );
    VoronoiEdge e5 = add_edge( v2, v3 );
    VoronoiEdge e6 = add_edge( v3, v1 );
    
    // the locations of the initial generators:
    Point gen2 = Point(cos(PI/6)*3*far_radius, sin(PI/6)*3*far_radius);
    Point gen3 = Point(cos(5*PI/6)*3*far_radius, sin(5*PI/6)*3*far_radius);
    Point gen1 = Point( 0,-3*far_radius);
    
    // add face objects
    FaceIdx out =  faces.add_face(e4, Point(0,0,1e9), NONINCIDENT ); // the outer face at infinity
    FaceIdx  f1 =  faces.add_face(e4, gen3, NONINCIDENT);
    FaceIdx  f2 =  faces.add_face(e5, gen1, NONINCIDENT);
    FaceIdx  f3 =  faces.add_face(e6, gen2, NONINCIDENT);
    
    // set props for center vd-point
    vd[v0].set_J( gen1, gen2, gen3 ); // this sets J2,J3,J4 and pk, so that detH(pl) can be called later
    
    // set left/right face for each edge
    vd[e1].left_face  = f1;
    vd[e1].right_face = f3;
    vd[e2].left_face  = f2;
    vd[e2].right_face = f1;
    vd[e3].left_face  = f3;
    vd[e3].right_face = f2;
    vd[e4].left_face  = f1;
    vd[e4].right_face = out;
    vd[e5].left_face  = f2;
    vd[e5].right_face = out;
    vd[e6].left_face  = f3;
    vd[e6].right_face = out;
    
    // set CCW next edge for each edge
    vd[e1].left_next = e4;
    vd[e1].right_next = e3;
    
    vd[e2].left_next = e5;
    vd[e2].right_next = e1;
    
    vd[e3].left_next = e6;
    vd[e3].right_next = e2;
    
    vd[e4].left_next = e2;
    vd[e4].right_next = e5;
    
    vd[e5].left_next = e3;
    vd[e5].right_next = e6;
    
    vd[e6].left_next = e1;
    vd[e6].right_next = e4;
}

/*
void VoronoiDiagram::assign_dual_face_edge(VoronoiGraph& d, VoronoiVertex v, VoronoiEdge e) {
    d[v].dual_face_edges.push_back(e);
}


void VoronoiDiagram::assign_dual_edge(VoronoiEdge vde, VoronoiEdge dde) {
    vd[vde].dual_edge = dde;
    dd[dde].dual_edge = vde;
}*/

void VoronoiDiagram::addVertexSite(Point p) {

    // 1) find the dd-vertex closest to p
    // 2) find the vd-face corresponding to p
    // 3) amont the vd-vertices in the found face, find the one with the smallest H
    // 4) expand the tree of to-be-deleted vertices as far as possible:
    //   4.1 tree property
    //   4.2 H<0 property
    
    // 1)
    // B1.1
    FaceIdx closest_face = find_closest_face( p );
    std::cout << " closest face is "<< closest_face << " at " << faces[closest_face].generator << "\n";
    // FIXME TODO faces.add_face( edge?, p);
    //std::cout << " added new vertex to dd: " << dd_new << "\n";

    // B1.2 find seed vertex by evaluating H on the vertices of the found face
    VoronoiVertex vd_seed = find_seed_vertex(closest_face, p);
    
    std::cout << " seed vertex is " << vd_seed << " at " << vd[vd_seed].position << "\n";
    
    // add seed to set V0
    VertexVector v0;
    v0.push_back(vd_seed);
    vd[vd_seed].type = IN;
    // find the set V0
    augment_vertex_set(v0);
    
    // generate new vertices on all edges in V0 connecting to OUT-vertices
    BOOST_FOREACH( VoronoiVertex v, v0 ) {
        assert( vd[v].type == IN );
        VoronoiOutEdgeItr edgeItr, edgeItrEnd;
        boost::tie( edgeItr, edgeItrEnd ) = boost::out_edges(v, vd);
        //std::vector<Point> q_positions;
        std::vector<VoronoiEdge> q_edges;
        for ( ; edgeItr != edgeItrEnd ; ++edgeItr ) {
            VoronoiVertex adj_vertex = boost::target( *edgeItr, vd );
            std::cout << " adj_vertex= " << adj_vertex << "\n";
            std::cout << " adj_vertex= " << adj_vertex << " type=" << vd[adj_vertex].type << " \n";
            if ( vd[adj_vertex].type == OUT ) {
                // *edgeItr connects IN <-> OUT, so we should generate new voronoi-vertex on it
                // the generators for this new voronoi-point will be the new dd-vertex we are adding dd_new
                // and the dd-vertices corresponding to the left and right face of the vd-edge *edgeItr
                //FaceIdx left_face = vd[*edgeItr].left;
                //FaceIdx right_face = vd[*edgeItr].right;
                //Point generator1 = faces[left_face].generator;
                //Point generator2 = faces[right_face].generator; 
                //Point q_pos = newVoronoiVertex( generator1 , generator2, p); // FIXME: calls detH, should be optimized
                q_edges.push_back(*edgeItr);
                //q_positions.push_back(q_pos);
                // adding vertices here invalidates the iterator edgeItr, so we can't do it here...
            }
        }
        
        // create new vertices at q_positions[] on edges q_edges[]
        for( unsigned int m=0; m<q_edges.size(); ++m)  {
            VoronoiVertex q = boost::add_vertex( vd ) ;
            vd[q].type = UNDECIDED;
            FaceIdx left_face = vd[q_edges[m]].left_face;
            FaceIdx right_face = vd[q_edges[m]].right_face;
            Point generator1 = faces[left_face].generator;
            Point generator2 = faces[right_face].generator;
                
            vd[q].set_J( generator1, generator2, p);
            vd[q].set_position();
            std::cout << " added vertex "<< q << " on edge " << q_edges[m] <<" at  " << vd[q].position << " to vd. \n";
            // position vertex here instead ?
                        
            insert_vertex_in_edge( q, q_edges[m] );
            
        }
    }
    std::cout << " vd vertex generation done \n";
    
    // generate new edges that form a loop around the region
    
    // remove vertices V0 and edges
}

void VoronoiDiagram::insert_vertex_in_edge(VoronoiVertex v, VoronoiEdge e) {
    // the vertex v is in the middle of edge e
    // source <-> v <-> target
    //         e1    e2
    VoronoiVertex target = boost::target( e , vd );
    VoronoiVertex source = boost::source( e , vd );
    FaceIdx left_face = vd[e].left_face;
    FaceIdx right_face = vd[e].right_face;
    VoronoiEdge e1 = add_edge( source, v );
    VoronoiEdge e2 = add_edge( v, target );
    // preserve the left/right face link
    vd[e1].left_face = left_face;
    vd[e1].right_face = right_face;
    vd[e2].left_face = left_face;
    vd[e2].right_face = right_face;
    
    set_next_property( left_face, e, e1, e2);

    // finally, remove the old edge
    boost::remove_edge( target, source, vd);
}

VoronoiEdge VoronoiDiagram::get_next_face_edge(FaceIdx f, VoronoiEdge e) {
    if ( vd[e].left_face == f )
        return vd[e].left_next;
    else
        return vd[e].right_next;
}

void VoronoiDiagram::set_next_property(FaceIdx face_idx, VoronoiEdge e_old, VoronoiEdge e1, VoronoiEdge e2) {
    // traverse around the face, setting the correct edge.next property for e1 and e2
    // source <->e_old<->target
    // source <->e1<->e2<->target
    // e1 and e2 replace e_old
    VoronoiEdge current = faces[face_idx].edge; // the edge where we start
    VoronoiEdge end = vd[current].left_next;
    //VoronoiVertex current = boost::target(e, vd); // vertex where we start
    if ( vd[e_old].left_face == face_idx ) {
        vd[e1].left_next = e2;
        vd[e2].left_next = vd[e_old].left_next;
        vd[e1].right_next = vd[e_old].right_next;
        vd[e2].right_next = e1;
    } else {
    }

    
}

FaceVector VoronoiDiagram::get_adjacent_faces( VoronoiVertex q ) {
    // given the vertex q, find the three adjacent faces
    std::set<FaceIdx> face_set;
    VoronoiOutEdgeItr itr, itr_end;
    boost::tie( itr, itr_end) = boost::out_edges(q, vd);
    for ( ; itr!=itr_end ; ++itr ) {
        face_set.insert( vd[*itr].left_face );
        face_set.insert( vd[*itr].right_face );
    }
    assert( face_set.size() == 3); // degree of q is three, so has three faces
    FaceVector fv;
    BOOST_FOREACH(FaceIdx m, face_set) {
        fv.push_back(m);
    }
    return fv;
}

void VoronoiDiagram::augment_vertex_set(VertexVector& q) {
    // RB2   voronoi-point q( a, b, c ) is the seed
    // add faces Ca,Cb,Cc to the stack
    FaceVector adjacent_faces = get_adjacent_faces( q[0] );
    
    // dd_cycles = get_generator_vertices( dd, vd, q[0] );
    assert( adjacent_faces.size()==3 );
    
    // B1.3  we push all the adjacent faces onto the stack, and label them INCIDENT
    std::stack<FaceIdx> S;
    BOOST_FOREACH(FaceIdx f, adjacent_faces) {
        faces[f].type = INCIDENT;
        S.push(f);
    }
    while ( !S.empty() ) { 
        FaceIdx c_alfa = S.top();
        S.pop();
        std::cout << " augmenting from face = " << c_alfa << "\n";
        //VertexVector cycle_verts = get_generator_vertices(vd, dd, c_alfa); // get vertices of this cycle
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

VoronoiVertex VoronoiDiagram::find_seed_vertex(FaceIdx face_idx, const Point& p) {
    // evaluate H on all vertices along facet edges and return
    // vertex with the lowest H
    
    // find vertices that bound face_idx 
    VertexVector q_verts = get_face_vertices(face_idx);                 assert( q_verts.size() >= 3 );
    double minimumH = 1; // safe, because we expect the min H to be negative...
    VoronoiVertex minimalVertex;
    double h;
    BOOST_FOREACH( VoronoiVertex q, q_verts) {
        if ( vd[q].type != OUT ) {
            h = vd[q].detH( p ); 
            if (h<minimumH) { // find minimum H value among q_verts
                minimumH = h;
                minimalVertex = q;
            }
        }
    }
    assert( minimumH < 0 );
    return minimalVertex;
}

VertexVector VoronoiDiagram::get_face_vertices(unsigned int face_idx) {
    // traverse face and return all vertices found
    VoronoiEdge e = faces[face_idx].edge; // the edge where we start
    VoronoiEdge startedge = e;
    //VoronoiVertex end = boost::source(e, vd); // the vertex where we end
    VoronoiVertex current = boost::target(e, vd); // vertex where we start
    
    VertexVector verts;
    bool done = false;
    while ( !done ) {
        verts.push_back(current);
        current = get_next_cw_vertex(current, e);
        e = get_next_face_edge(face_idx, e);
        if ( e == startedge )
            done = true;
    }
    //verts.push_back(end);
    return verts;
}


VoronoiVertex VoronoiDiagram::get_next_cw_vertex(VoronoiVertex current, VoronoiEdge e) {
    VoronoiVertex s = boost::source( vd[e].left_next, vd);
    VoronoiVertex t = boost::target( vd[e].left_next, vd);
    if ( current != s )
        return s;
    else
        return t;
}

/*
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
    assert( vv.size() == 3 );
    return vv;
}*/

unsigned int VoronoiDiagram::find_closest_face(const Point& p ) {
    //VoronoiVertexItr it_end, itr;
    FaceIdx closest_face;
    double closest_distance = 3*far_radius;
    double d;
    //boost::tie( itr, it_end ) = boost::vertices( dd );
    //BOOST_FOREACH(VoronoiFace f, faces.faces) {
    for (FaceIdx  m=0;m<faces.size();++m) {
    //for (  ; itr != it_end ; ++itr ) {
        d = ( faces[m].generator - p).norm();
        if (d<closest_distance ) {
            closest_distance=d;
            closest_face=m;
        }
    }
    std::cout << " find_closest_Delaunay_vertex \n";
    std::cout << "   face id= " << closest_face << " at distance " << closest_distance << "\n";
    assert( closest_distance < 3*far_radius ) ;
    return closest_face;
}

/*
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
}*/

/*
Point VoronoiDiagram::newVoronoiVertex(Point& pi, Point& pj, Point& pk) {
    double w = detH_J4(pi,pj,pk);
    double x = - detH_J2(pi,pj,pk)/w+pk.x;
    double y = detH_J3(pi,pj,pk)/w+pk.y;
    return Point(x,y);
}*/

/*
double VoronoiDiagram::detH_J2(Point& pi, Point& pj, Point& pk) {
    // J2(ijk)
    //   yi-yk   [(xi-xk)^2+(yi-yk)^2]/2
    //   yj-yk   [(xj-xk)^2+(yj-yk)^2]/2
    return (pi.y-pk.y)*(square(pj.x-pk.x)+square(pj.y-pk.y))/2 - (pj.y-pk.y)*(square(pi.x-pk.x)+square(pi.y-pk.y))/2;
}*/

/*
double VoronoiDiagram::detH_J3(Point& pi, Point& pj, Point& pk) {
    // J3(ijk)
    //   xi-xk   ((xi-xk)^2+(yi-yk)^2)/2
    //   xj-xk   ((xj-xk)^2+(yj-yk)^2)/2
    return (pi.x-pk.x)*(square(pj.x-pk.x)+square(pj.y-pk.y))/2 - (pj.x-pk.x)*(square(pi.x-pk.x)+square(pi.y-pk.y))/2;
}*/

/*
double VoronoiDiagram::detH_J4(Point& pi, Point& pj, Point& pk) {
    // J4(ijk) =
    //    xi-xk  yi-yk 
    //    xj-xk  yj-yk 
    return (pi.x-pk.x)*(pj.y-pk.y) - (pj.x-pk.x)*(pi.y-pk.y);
}*/


boost::python::list VoronoiDiagram::getGenerators()  {
    boost::python::list plist;
    //VoronoiVertexItr it_begin, it_end, itr;
    //boost::tie( it_begin, it_end ) = boost::vertices( dd );
    for ( FaceIdx m=0;m<faces.size();++m ) {
        plist.append( faces[m].generator );
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
