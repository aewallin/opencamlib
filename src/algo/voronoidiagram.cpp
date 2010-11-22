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
    v01     = add_vertex( Point(0, far_multiplier*far_radius), OUT );
    v02     = add_vertex( Point( cos(-5*PI/6)*far_multiplier*far_radius, sin(-5*PI/6)*far_multiplier*far_radius), OUT );
    v03     = add_vertex( Point( cos(-PI/6)*far_multiplier*far_radius, sin(-PI/6)*far_multiplier*far_radius), OUT );

    // the locations of the initial generators:
    Point gen2 = Point(cos(PI/6)*3*far_radius, sin(PI/6)*3*far_radius);
    Point gen3 = Point(cos(5*PI/6)*3*far_radius, sin(5*PI/6)*3*far_radius);
    Point gen1 = Point( 0,-3*far_radius);
    // set props for center vd-point
    vd[v0].set_J( gen1, gen2, gen3 ); // this sets J2,J3,J4 and pk, so that detH(pl) can be called later
    
    // Point outer = Point(0,0,1e9); // the outer face at infinity
    
    // add face 1: v0-v1-v2
    VoronoiEdge e1 = add_edge( v0, v01 );   
    VoronoiEdge e2 = add_edge( v01, v02 );
    VoronoiEdge e3 = add_edge( v02, v0 ); // face, twin, next
    FaceIdx f1 =  faces.add_face(e2, gen3, NONINCIDENT);
    vd[e1].face = f1;
    vd[e2].face = f1;
    vd[e3].face = f1;
    vd[e1].next = e2;
    vd[e2].next = e3;
    vd[e3].next = e1;
    
    // add face 2: v0-v2-v3
    VoronoiEdge e4 = add_edge( v0, v02 );   
    VoronoiEdge e5 = add_edge( v02, v03 );
    VoronoiEdge e6 = add_edge( v03, v0 ); // face, twin, next
    FaceIdx f2 =  faces.add_face(e5, gen1, NONINCIDENT);
    vd[e4].face = f2;
    vd[e5].face = f2;
    vd[e6].face = f2;
    vd[e4].next = e5;
    vd[e5].next = e6;
    vd[e6].next = e4;
    
    // add face 3: v0-v3-v1 
    VoronoiEdge e7 = add_edge( v0, v03 );   
    VoronoiEdge e8 = add_edge( v03, v01 );
    VoronoiEdge e9 = add_edge( v01, v0 ); // face, twin, next
    FaceIdx f3 =  faces.add_face(e8, gen2, NONINCIDENT);
    vd[e7].face = f3;
    vd[e8].face = f3;
    vd[e9].face = f3;
    vd[e7].next = e8;
    vd[e8].next = e9;
    vd[e9].next = e7;
    
    // twin edges
    vd[e1].twin = e9;
    vd[e9].twin = e1;
    
    vd[e2].twin = VoronoiEdge();
    vd[e5].twin = VoronoiEdge();
    vd[e8].twin = VoronoiEdge();
    
    vd[e3].twin = e4;
    vd[e4].twin = e3;
    
    vd[e6].twin = e7;
    vd[e7].twin = e6;
}



void VoronoiDiagram::addVertexSite(Point p) {
    // 1) find the dd-vertex closest to p
    // 2) find the vd-face corresponding to p
    // 3) amont the vd-vertices in the found face, find the one with the smallest H
    // 4) expand the tree of to-be-deleted vertices as far as possible:
    //   4.1 tree property
    //   4.2 H<0 property
    
    // 1)
    // B1.1  find the face corresponding to the closest generator
    FaceIdx closest_face = find_closest_face( p );
    // B1.2 find seed vertex by evaluating H on the vertices of the found face
    //VoronoiVertex vd_seed = find_seed_vertex(closest_face, p);
    VertexVector v0 = find_seed_vertex(closest_face, p);
    // expand from seed t0 find the set V0
    augment_vertex_set(v0, p);
    // add new vertices on all edges that connect v0 IN edges to OUT edges
    add_new_voronoi_vertices(v0, p);
    
    // generate new edges that form a loop around the region to be deleted
    FaceIdx newface = split_faces(p);
    // fix the next-pointers in newface, then remove set v0
    remove_vertex_set(v0, newface);
    // reset IN/OUT/UNDECIDED for verts, and INCIDENT/NONINCIDENT for faces
    reset_labels();
}

FaceIdx VoronoiDiagram::split_faces(Point& p) {
    FaceVector incident_faces = get_incident_faces();
    // create new face and pass to split_face() 
    FaceIdx newface =  faces.add_face( p, NONINCIDENT);
    BOOST_FOREACH( FaceIdx f, incident_faces ) {
        split_face(newface, f);
    }
    return newface;
}

void VoronoiDiagram::reset_labels() {
    VoronoiVertexItr it, it_end;
    boost::tie( it, it_end ) = boost::vertices( vd );
    for ( ; it !=it_end ; ++it ) {
        if ( vd[*it].type != UNDECIDED ) {
            vd[*it].type = UNDECIDED;
        }
    }
    // the outer vertices are special:
    vd[v01].type = OUT;
    vd[v02].type = OUT;
    vd[v03].type = OUT;
    
    for ( FaceIdx m=0; m<faces.size() ; ++m ) {
        faces[m].type = NONINCIDENT;
    }

}

void VoronoiDiagram::remove_vertex_set(VertexVector& v0 , FaceIdx newface) {
    std::cout << " remove_vertex_set () for new face "<< newface << "\n";
    
    VoronoiEdge current_edge = faces[newface].edge;
    VoronoiVertex current_target; 
    VoronoiEdge start_edge = current_edge;
    bool done = false;
    
    // this repairs the next-pointers for newface that are broken.
    while (!done) {
        current_target = boost::target( current_edge, vd );
        //std::cout << " finding next for " << current_edge << "\n";
        // find the correct next edge for current_edge
        VoronoiVertex current_source = boost::source( current_edge, vd );
        VoronoiOutEdgeItr it, it_end;
        boost::tie( it, it_end ) = boost::out_edges( current_target, vd);
        for ( ; it != it_end ; ++it ) {
            VoronoiVertex out_target = boost::target( *it, vd);
            if ( vd[out_target].type == NEW ) {
                if ( out_target != current_source ) { 
                    // this is the edge we want to take
                    vd[current_edge].next = *it;
                    assert( vd[current_edge].face ==  vd[ vd[current_edge].next ].face );
                    std::cout << " setting next for " << current_edge << " to " << vd[current_edge].next <<"\n";
                }
            }
        }
        if ( vd[current_edge].next == start_edge )
            done = true;
        // jump to the next edge
        current_edge = vd[current_edge].next;
        //char c;
        //std::cin >> c;
    }
    
    // it should now be safe to delete v0
    // remove vertices V0 and edges
    BOOST_FOREACH( VoronoiVertex v, v0 ) {
        boost::clear_vertex( v, vd);
        vd[v].type = OUT;
    }
    //BOOST_FOREACH( VoronoiVertex v, v0 ) {
    //    boost::remove_vertex( v, vd);
    //}
    
}

// split the face f
void VoronoiDiagram::split_face(FaceIdx newface, FaceIdx f) {
    std::cout << "split_face() on face = " << f << "\n";
    VoronoiVertex new_source; // this is found as OUT-NEW-IN
    VoronoiVertex new_target; // this is found as IN-NEW-OUT
    // the new vertex on face f connects new_source -> new_target
    
    VoronoiEdge current_edge = faces[f].edge;
    VoronoiEdge start_edge = current_edge;
    std::cout << " starting at edge " << current_edge << " on face " <<  vd[current_edge].face <<" next= "<< vd[current_edge].next << "\n";
    assert( f == vd[current_edge].face );
    
    VoronoiVertexType currentType = OUT;
    VoronoiVertexType nextType  = NEW;
    VoronoiEdge new_previous;
    VoronoiEdge new_next;
    VoronoiEdge twin_next;
    VoronoiEdge twin_previous;
    
    bool found = false;
    int iters = 0;
    std::cout << " OUT=" << OUT<<"\n";
    std::cout << " IN=" << IN<<"\n";
    std::cout << " UNDECIDED=" << UNDECIDED<<"\n";
    std::cout << " NEW=" << NEW<<"\n";
    std::cout << "Looking for types " << currentType << " and " << nextType <<"\n";
    // v.type= "<< vd[current_vertex].type << "\n";
    while (!found) {
        VoronoiVertex current_vertex = boost::target( current_edge, vd );
        VoronoiEdge next_edge = vd[current_edge].next;
        VoronoiVertex next_vertex = boost::target( next_edge , vd );
        if ( vd[current_vertex].type == currentType ) {
            if (vd[next_vertex].type == nextType ) {
                new_source = next_vertex;
                new_previous = next_edge;
                twin_next = vd[next_edge].next;
                found = true;
            }
        }
        iters++;
        assert( current_and_next_on_same_face( current_edge ) );
        
        current_edge = vd[current_edge].next;
        std::cout << current_edge<<" v.type= "<< vd[current_vertex].type << "\n";
        //if ( vd[start_edge].next == current_edge ) {
        //    std::cout << start_edge << " == " << current_edge << " ERROR \n";
        //    std::cout << " loop!!";
        //    assert(0);
        //}
        //if ( iters > 10 ) {
        //    std::cout << " new_source find WARNING: iters=" << iters << "\n";
        //    std::cout << " current_edge = " << current_edge << " next=" << vd[current_edge].next << "\n";
        //}
        if ( iters > 18 )
            assert(0);
        
        
        
    }
    //std::cout << " found NEW source vertex " << new_source << "\n";
    found = false;
    currentType = IN;
    nextType = NEW;
    current_edge = faces[f].edge;
    iters = 0;
    while (!found) {
        // std::cout << "current_edge = " << current_edge << "\n";
        VoronoiVertex current_vertex = boost::target( current_edge, vd );
        VoronoiEdge next_edge = vd[current_edge].next;
        VoronoiVertex next_vertex = boost::target( next_edge , vd );
        if ( vd[current_vertex].type == currentType ) {
            if (vd[next_vertex].type == nextType ) {
                new_target = next_vertex;
                new_next = vd[next_edge].next;
                twin_previous = next_edge;
                found = true;
            }
        }
        current_edge = vd[current_edge].next;
        iters++;
        if ( iters > 10 ) {
            std::cout << " new_target find WARNING: iters=" << iters << "\n";
            std::cout << " current_edge = " << current_edge << " next=" << vd[current_edge].next << "\n";
            
        }
        if ( iters > 15 )
            assert(0);
    }
    //std::cout << " found NEW target vertex " << new_target << "\n";
    
    // now connect new_previous -> new_source -> new_target -> new_next
    VoronoiEdge e_new = add_edge( new_source, new_target ); // face,next,twin
    vd[new_previous].next = e_new;
    vd[e_new].next = new_next;
    vd[e_new].face = f;
    
    
    // the twin edge that bounds the new face
    VoronoiEdge e_twin = add_edge( new_target, new_source );
    vd[e_twin].twin = e_new;
    vd[e_new].twin = e_twin;
    vd[e_twin].face = newface;
    faces[newface].edge = e_twin;
    vd[twin_previous].next = e_twin;
    vd[e_twin].next = twin_next;
    
    // make sure faces are ok
    faces[f].edge = e_new;
    
    std::cout << "  created new edge "<< e_new << " next = "<< vd[e_new].next << " for face "<< f << "\n";
    std::cout << "         twin edge "<< e_twin <<" next = "<< vd[e_twin].next << " for face "<< newface << "\n";
    //assert( isDegreeThree() );
}

FaceVector VoronoiDiagram::get_incident_faces() {
    FaceVector output;
    for (FaceIdx m=0;m<faces.size(); ++m) {
        if ( faces[m].type == INCIDENT )
            output.push_back(m);
    }
    return output;
}

void VoronoiDiagram::add_new_voronoi_vertices(VertexVector& v0, Point& p) {
    // generate new vertices on all edges in V0 connecting to OUT-vertices
    std::cout << "add_new_voronoi_vertices() \n";
    assert( !v0.empty() );
    std::vector<VoronoiEdge> q_edges; // new vertices generated on these edges
    BOOST_FOREACH( VoronoiVertex v, v0 ) {
        assert( vd[v].type == IN ); // all verts in v0 are IN
        VoronoiOutEdgeItr edgeItr, edgeItrEnd;
        boost::tie( edgeItr, edgeItrEnd ) = boost::out_edges(v, vd);
        // among the out-edges of v, find the one that is OUT
        for ( ; edgeItr != edgeItrEnd ; ++edgeItr ) {
            VoronoiVertex adj_vertex = boost::target( *edgeItr, vd );
            if ( vd[adj_vertex].type == OUT ) 
                q_edges.push_back(*edgeItr);// adding vertices here invalidates the iterator edgeItr, so we can't do it here...
        }
    }
    assert( !q_edges.empty() );
    
    for( unsigned int m=0; m<q_edges.size(); ++m)  {  // create new vertices on edges q_edges[]
        VoronoiVertex q = boost::add_vertex( vd ) ;
        vd[q].type = NEW;
        FaceIdx face = vd[q_edges[m]].face;
        VoronoiEdge twin = vd[q_edges[m]].twin;
        FaceIdx twin_face = vd[twin].face;
        Point generator1 = faces[face].generator;
        Point generator2 = faces[twin_face].generator;
        vd[q].set_J( generator1, generator2, p); // ordering of generators shoud be CCW !?
        vd[q].set_position();
        std::cout << " added vertex "<< q << " on edge " << q_edges[m] <<" at  " << vd[q].position << " to vd. \n";
        
        if ( faces[face].type != INCIDENT ) {
            std::cout << "ERROR new vertex " << q << " on edge " << q_edges[m] << " on face " << face << "\n";
            std::cout << "ERROR left_face " << face << " not INCIDENT\n";
        }
        if ( faces[twin_face].type != INCIDENT ) {
            std::cout << "ERROR new vertex " << q << " on edge " << q_edges[m] << " on face " << twin_face << "\n";
            std::cout << "ERROR twin_face " << twin_face << " not INCIDENT\n";
        }
        
        assert( faces[face].type == INCIDENT);
        assert( faces[twin_face].type == INCIDENT);
        
        
        insert_vertex_in_edge( q, q_edges[m] );
        

    }
        
    
    std::cout << "vd vertex generation done \n";
}



void VoronoiDiagram::insert_vertex_in_edge(VoronoiVertex v, VoronoiEdge e) {
    std::cout << " inserting vertex " << v << " in edge " << e << "\n";
    
    // the vertex v is in the middle of edge e
    //                      face
    //                    e1   e2
    // previous-> source  -> v -> target
    //            tw_targ <- v <- tw_sour <- tw_previous
    //                    te2  te1
    //                     twin_face
    
    VoronoiEdge twin = vd[e].twin;
    VoronoiVertex source = boost::source( e , vd );
    VoronoiVertex target = boost::target( e , vd );
    VoronoiVertex twin_source = boost::source( twin , vd );
    VoronoiVertex twin_target = boost::target( twin , vd );
    assert( source == twin_target );
    assert( target == twin_source );
    
    FaceIdx face = vd[e].face;
    FaceIdx twin_face = vd[twin].face;
    VoronoiEdge previous = find_previous_edge(e);
    VoronoiEdge twin_previous = find_previous_edge(twin);
    
    VoronoiEdge e1 = add_edge( source, v );
    VoronoiEdge e2 = add_edge( v, target );
    
    // preserve the left/right face link
    vd[e1].face = face;
    vd[e1].next = e2;
    vd[e2].face = face;
    vd[e2].next = vd[e].next;
    vd[previous].next = e1;
    
    VoronoiEdge te1 = add_edge( twin_source, v );
    VoronoiEdge te2 = add_edge( v, twin_target );
    vd[te1].face = twin_face;
    vd[te1].next = te2;
    vd[te2].face = twin_face;
    vd[te2].next = vd[twin].next;
    vd[twin_previous].next = te1;
    // TWINNING (note indices 'cross', see ASCII art above)
    vd[e1].twin = te2;
    vd[te2].twin = e1;
    vd[e2].twin = te1;
    vd[te1].twin = e2;
    std::cout << "  " << e << " split into " << e1 << " and " << e2 << "\n";
    std::cout << "  " << twin << " split into " << te1 << " and " << te2 << "\n";
    
    // update the faces
    faces[face].edge = e1;
    faces[twin_face].edge = te1;
    
    // finally, remove the old edge
    boost::remove_edge( e, vd);
    boost::remove_edge( twin, vd);
}

VoronoiEdge VoronoiDiagram::find_previous_edge(VoronoiEdge e) {
    VoronoiEdge previous = vd[e].next;
    while ( vd[previous].next != e ) {
        previous = vd[previous].next;
    }
    return previous;
}

FaceVector VoronoiDiagram::get_adjacent_faces( VoronoiVertex q ) {
    // given the vertex q, find the three adjacent faces
    std::set<FaceIdx> face_set;
    VoronoiOutEdgeItr itr, itr_end;
    boost::tie( itr, itr_end) = boost::out_edges(q, vd);
    for ( ; itr!=itr_end ; ++itr ) {
        face_set.insert( vd[*itr].face );
    }
    assert( face_set.size() == 3); // degree of q is three, so has three faces
    FaceVector fv;
    BOOST_FOREACH(FaceIdx m, face_set) {
        fv.push_back(m);
    }
    return fv;
}

void VoronoiDiagram::augment_vertex_set(VertexVector& q, Point& p) {
    // RB2   voronoi-point q[0] = q( a, b, c ) is the seed
    // add faces Ca,Cb,Cc to the stack
    FaceVector adjacent_faces = get_adjacent_faces( q[0] );
    assert( adjacent_faces.size()==3 ); // in a degree three graph every vertex has three adjacent faces
    
    std::stack<FaceIdx> S; // B1.3  we push all the adjacent faces onto the stack, and label them INCIDENT
    std::cout << "   seed vertex " << q[0] << " has adjacent faces: ";
    BOOST_FOREACH(FaceIdx f, adjacent_faces) {
        faces[f].type = INCIDENT;
        std::cout << " " << f << " ";
        //std::cout << " faces[f].type = " << faces[f].type  << "  \n";
        S.push(f);
    }
    std::cout << "\n";
    
    while ( !S.empty() ) { 
        FaceIdx f = S.top();
        S.pop();
        std::cout << "    augmenting from face = " << f << "\n";
        VoronoiEdge current_edge = faces[f].edge;
        VoronoiEdge start_edge = current_edge;
        bool done=false;
        while (!done) {
            assert( faces[ vd[current_edge].face ].type == INCIDENT );
            
            VoronoiVertex v = boost::target( current_edge , vd );
            if ( vd[v].type == UNDECIDED ) {
                if ( vd[v].detH( p ) < 0.0 ) {
                    std::cout << "     found IN vertex " << v << " on edge " << current_edge <<" face " << f <<"\n";
                    vd[v].type = IN;
                    q.push_back(v);
                    // also set the twin face to incident and push on the stack
                    VoronoiEdge twin_edge = vd[current_edge].twin;
                    FaceIdx twin_face = vd[twin_edge].face;
                    if ( faces[twin_face].type != INCIDENT ) {
                        faces[twin_face].type = INCIDENT;
                        std::cout << "     setting twin face " << twin_face << " INCIDENT \n";
                        S.push(twin_face);
                    }
                } else {
                    vd[v].type = OUT;
                }
            }
            current_edge = vd[current_edge].next;
            assert( current_and_next_on_same_face(current_edge) );
            if ( current_edge == start_edge )
                done = true;
        }
        // B2.1  mark "out"  v in cycle_alfa if 
        //  (T6) v is adjacent to an IN vertex in V-Vin
        //  (T7) v is on an "incident" cycle other than this cycle and is not adjacent to a vertex in Vin
        
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

bool VoronoiDiagram::current_and_next_on_same_face(VoronoiEdge current_edge) {
    return ( vd[current_edge].face == vd[ vd[current_edge].next ].face );
}

VertexVector VoronoiDiagram::find_seed_vertex(FaceIdx face_idx, const Point& p) {
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
    VertexVector output;
    vd[minimalVertex].type = IN;
    output.push_back(minimalVertex);
    return output;
}

// traverse face and return all vertices found
VertexVector VoronoiDiagram::get_face_vertices(FaceIdx face_idx) {
    VertexVector verts;
    VoronoiEdge startedge = faces[face_idx].edge; // the edge where we start
    VoronoiVertex start_target = boost::target( startedge, vd); 
    verts.push_back(start_target);
    VoronoiEdge current = vd[startedge].next;
    do {
        VoronoiVertex current_target = boost::target( current, vd); 
        verts.push_back(current_target);
        current = vd[current].next;
    } while ( current != startedge );
    return verts;
}


unsigned int VoronoiDiagram::find_closest_face(const Point& p ) {
    std::cout << "find_closest_face() \n";
    FaceIdx closest_face;
    double closest_distance = 3*far_radius;
    double d;
    for (FaceIdx  m=0;m<faces.size();++m) {
        d = ( faces[m].generator - p).norm();
        if (d<closest_distance ) {
            closest_distance=d;
            closest_face=m;
        }
    }
    
    std::cout << "   face " << closest_face << " is closest, distance to generator = " << closest_distance << "\n";
    assert( closest_distance < 3*far_radius ) ;
    return closest_face;
}


boost::python::list VoronoiDiagram::getGenerators()  {
    boost::python::list plist;
    for ( FaceIdx m=0;m<faces.size();++m ) {
        plist.append( faces[m].generator );
    }
    return plist;
}

bool VoronoiDiagram::isDegreeThree() {
    VoronoiVertexItr it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( vd );
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        if ( boost::degree( *itr, vd ) != 6 ) {
            if ( (*itr != v01) && (*itr!=v02) && (*itr!=v03) )
                return false;
        }
    }
    return true;
}

boost::python::list VoronoiDiagram::getVoronoiVertices() const {
    boost::python::list plist;
    VoronoiVertexItr it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( vd );
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        if ( boost::degree( *itr, vd ) == 6 ) {
            plist.append( vd[*itr].position );
        }
    }
    return plist;
}

boost::python::list VoronoiDiagram::getFarVoronoiVertices() const {
    boost::python::list plist;
    VoronoiVertexItr it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( vd );
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        if ( boost::degree( *itr, vd ) == 4 ) {
            plist.append( vd[*itr].position );
        }
    }
    return plist;
}

// return edge and the generator corresponding to its face
boost::python::list VoronoiDiagram::getEdgesGenerators()  {
    boost::python::list edge_list;
    VoronoiEdgeItr itr, it_end;
    boost::tie( itr, it_end ) = boost::edges( vd );
    for (  ; itr != it_end ; ++itr ) { // loop through each edge
            boost::python::list point_list; // the endpoints of each edge
            VoronoiVertex v1 = boost::source( *itr, vd  );
            VoronoiVertex v2 = boost::target( *itr, vd  );
            Point src = vd[v1].position;
            Point tar = vd[v2].position;
            // shorten the edge, for visualization
            Point src_short = src + (10/((tar-src).norm())) * (tar-src);
            Point tar_short = src + (1-10/((tar-src).norm())) * (tar-src);
            point_list.append( src_short );
            point_list.append( tar_short );
            FaceIdx f = vd[*itr].face;
            Point gen = faces[f].generator;
            Point orig = gen.xyClosestPoint(src, tar);
            Point dir = gen-orig;
            dir.xyNormalize(); 
            point_list.append( dir );
            edge_list.append(point_list);
    }
    return edge_list;
}

boost::python::list VoronoiDiagram::getVoronoiEdges() const {
    boost::python::list edge_list;
    VoronoiEdgeItr itr, it_end;
    boost::tie( itr, it_end ) = boost::edges( vd );
    for (  ; itr != it_end ; ++itr ) { // loop through each edge
            boost::python::list point_list; // the endpoints of each edge
            VoronoiVertex v1 = boost::source( *itr, vd  );
            VoronoiVertex v2 = boost::target( *itr, vd  );
            point_list.append( vd[v1].position );
            point_list.append( vd[v2].position );
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
