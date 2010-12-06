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


#include "voronoidiagram.h"
#include "numeric.h"

namespace ocl
{

VoronoiDiagram::VoronoiDiagram(double far, unsigned int n_bins) {
    hed = HalfEdgeDiagram(far, n_bins);
    //faces = FaceList(far_radius, n_bins);
    far_radius=far;
    gen_count=3;
    init();
    in_vertices.clear();
    incident_faces.clear();
}



VoronoiDiagram::~VoronoiDiagram() {
    //
}
/* VoronoiVertex VoronoiDiagram::add_vertex( Point pos, VoronoiVertexType t) {
    VoronoiVertex v = boost::add_vertex(vd);
    vd[v].position = pos;
    vd[v].type = t;
    return v;
}*/

/* VoronoiEdge VoronoiDiagram::add_edge(VoronoiVertex v1, VoronoiVertex v2) {
    VoronoiEdge e;
    bool b;
    boost::tie( e , b ) = boost::add_edge( v1, v2, vd);
    return e;
}*/

// sanity check
/*
bool VoronoiDiagram::isValid() {
    if (!isDegreeThree() )
        return false;
    if (!face_count_equals_generator_count())
        return false;
    return true;
}*/

/*
bool VoronoiDiagram::isDegreeThree() {
    // the outermost init() vertices have special degree, all others == 6
    VoronoiVertexItr it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( vd );
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        if ( boost::degree( *itr, vd ) != 6 ) {
            if ( (*itr != v01) && (*itr!=v02) && (*itr!=v03) )
                return false;
        }
    }
    return true;
}*/

/*
bool VoronoiDiagram::face_count_equals_generator_count() {
    int vertex_count = boost::num_vertices(vd);
    int face_count = (vertex_count- 4)/2 + 3;
    if (face_count != gen_count) {
        std::cout << " num_vertices = " << vertex_count << "\n";
        std::cout << " gen_count = " << gen_count << "\n";
        std::cout << " face_count = " << face_count << "\n";
        assert(0);
    }
    return ( face_count == gen_count );
}*/

void VoronoiDiagram::init() {
    double far_multiplier = 6;
    // add vertices
    HEVertex v0;
    v0  = hed.add_vertex( Point(0,0), UNDECIDED);
    v01 = hed.add_vertex( Point(0, far_multiplier*far_radius), OUT );
    v02 = hed.add_vertex( Point( cos(-5*PI/6)*far_multiplier*far_radius, sin(-5*PI/6)*far_multiplier*far_radius), OUT );
    v03 = hed.add_vertex( Point( cos(-PI/6)*far_multiplier*far_radius, sin(-PI/6)*far_multiplier*far_radius), OUT );

    // the locations of the initial generators:
    double gen_mutliplier = 3;
    Point gen2 = Point(cos(PI/6)*gen_mutliplier*far_radius, sin(PI/6)*gen_mutliplier*far_radius);
    Point gen3 = Point(cos(5*PI/6)*gen_mutliplier*far_radius, sin(5*PI/6)*gen_mutliplier*far_radius);
    Point gen1 = Point( 0,-gen_mutliplier*far_radius);
    // set props for center vd-point

    hed[v0].set_J( gen1, gen2, gen3 ); // this sets J2,J3,J4 and pk, so that detH(pl) can be called later
        
    // add face 1: v0-v1-v2
    HEEdge e1 = hed.add_edge( v0, v01  );   
    HEEdge e2 = hed.add_edge( v01, v02 );
    HEEdge e3 = hed.add_edge( v02, v0  ); 
    HEFace f1 =  hed.add_face(e2, gen3, NONINCIDENT);
    //faces.add_face(e2, gen3, NONINCIDENT);
    hed[e1].face = f1;
    hed[e2].face = f1;
    hed[e3].face = f1;
    hed[e1].next = e2;
    hed[e2].next = e3;
    hed[e3].next = e1;
    
    // add face 2: v0-v2-v3
    HEEdge e4 = hed.add_edge( v0, v02  );   
    HEEdge e5 = hed.add_edge( v02, v03 );
    HEEdge e6 = hed.add_edge( v03, v0  ); 
    HEFace f2 =  hed.add_face(e5, gen1, NONINCIDENT);
    hed[e4].face = f2;
    hed[e5].face = f2;
    hed[e6].face = f2;
    hed[e4].next = e5;
    hed[e5].next = e6;
    hed[e6].next = e4;
    
    // add face 3: v0-v3-v1 
    HEEdge e7 = hed.add_edge( v0, v03  );   
    HEEdge e8 = hed.add_edge( v03, v01 );
    HEEdge e9 = hed.add_edge( v01, v0  ); 
    HEFace f3 =  hed.add_face(e8, gen2, NONINCIDENT);
    hed[e7].face = f3;
    hed[e8].face = f3;
    hed[e9].face = f3;
    hed[e7].next = e8;
    hed[e8].next = e9;
    hed[e9].next = e7;
    
    // twin edges
    hed[e1].twin = e9;
    hed[e9].twin = e1;
    hed[e2].twin = HEEdge(); // the outermost edges have invalid twins
    hed[e5].twin = HEEdge();
    hed[e8].twin = HEEdge();
    hed[e3].twin = e4;
    hed[e4].twin = e3;
    hed[e6].twin = e7;
    hed[e7].twin = e6;
    
    //assert( isValid() );
}



void VoronoiDiagram::addVertexSite(Point p) {
    gen_count++;
    // 1) find the dd-vertex closest to p
    // 2) find the vd-face corresponding to p
    // 3) amont the vd-vertices in the found face, find the one with the smallest H
    // 4) expand the tree of to-be-deleted vertices as far as possible:
    //   4.1 tree property
    //   4.2 H<0 property
    
    // 1)
    // B1.1  find the face corresponding to the closest generator

    //FaceIdx closest_face = faces.find_closest_face( p );
    
    
    HEFace closest_face = hed.grid_find_closest_face( p );
    //std::cout << " closest_face2 = " << closest_face2 << " \n";
    //assert( closest_face == closest_face2 );
    
    // B1.2 find seed vertex by evaluating H on the vertices of the found face
    //VoronoiVertex vd_seed = find_seed_vertex(closest_face, p);
    VertexVector v0 = find_seed_vertex(closest_face, p);
    // expand from seed t0 find the set V0
    augment_vertex_set(v0, p);
    // add new vertices on all edges that connect v0 IN edges to OUT edges
    add_new_voronoi_vertices(v0, p);
    
    // generate new edges that form a loop around the region to be deleted
    HEFace newface = split_faces(p);
    // fix the next-pointers in newface, then remove set v0
    remove_vertex_set(v0, newface);
    // reset IN/OUT/UNDECIDED for verts, and INCIDENT/NONINCIDENT for faces
    reset_labels();
    
    //assert( isValid() );
}

HEFace VoronoiDiagram::split_faces(Point& p) {
    //FaceVector incident_faces = get_incident_faces();
    // create new face and pass to split_face() 
    HEFace newface =  hed.add_face( p, NONINCIDENT);
    BOOST_FOREACH( HEFace f, incident_faces ) {
        split_face(newface, f);
    }
    return newface;
}

void VoronoiDiagram::reset_labels() {
    BOOST_FOREACH( HEVertex v, in_vertices ) {
        if ( hed[v].type != UNDECIDED ) {
            hed[v].type = UNDECIDED;
        }
    }
    in_vertices.clear();
    
    // the outer vertices are special:
    hed[v01].type = OUT;
    hed[v02].type = OUT;
    hed[v03].type = OUT;
    //FaceVector incident = get_incident_faces();
    BOOST_FOREACH(HEFace m, incident_faces ) {
            hed.set_face_type(m, NONINCIDENT);
    }
    incident_faces.clear();
}

void VoronoiDiagram::remove_vertex_set(VertexVector& v0 , HEFace newface) {
    //std::cout << " remove_vertex_set () for new face "<< newface << "\n";
    
    HEEdge current_edge = hed.face_edge(newface);
    HEVertex current_target; 
    HEEdge start_edge = current_edge;
    bool done = false;
    
    // this repairs the next-pointers for newface that are broken.
    while (!done) {
        current_target = hed.target(  current_edge );
        //std::cout << " finding next for " << current_edge << "\n";
        // find the correct next edge for current_edge
        HEVertex current_source = hed.source( current_edge );
        HEOutEdgeItr it, it_end;
        boost::tie( it, it_end ) = boost::out_edges( current_target, hed);
        for ( ; it != it_end ; ++it ) {
            HEVertex out_target = hed.target( *it);
            if ( hed[out_target].type == NEW ) {
                if ( out_target != current_source ) { 
                    // this is the edge we want to take
                    hed[current_edge].next = *it;
                    assert( hed[current_edge].face ==  hed[ hed[current_edge].next ].face );
                    //std::cout << " setting next for " << current_edge << " to " << vd[current_edge].next <<"\n";
                }
            }
        }
        if ( hed[current_edge].next == start_edge )
            done = true;
        // jump to the next edge
        current_edge = hed[current_edge].next;
        //char c;
        //std::cin >> c;
    }
    
    // it should now be safe to delete v0
    // remove vertices V0 and edges
    BOOST_FOREACH( HEVertex v, v0 ) {
        boost::clear_vertex( v, hed);
        hed[v].type = OUT;
    }
    BOOST_FOREACH( HEVertex v, v0 ) {
        boost::remove_vertex( v, hed);    //
    }
    
}

// split the face f
void VoronoiDiagram::split_face(HEFace newface, HEFace f) {
    //std::cout << "split_face() on face = " << f << "\n";
    HEVertex new_source; // this is found as OUT-NEW-IN
    HEVertex new_target; // this is found as IN-NEW-OUT
    // the new vertex on face f connects new_source -> new_target
    
    HEEdge current_edge = hed.face_edge(f);
        assert( f == hed[current_edge].face );
        
    HEEdge start_edge = current_edge;
    VoronoiVertexType currentType = OUT;
    VoronoiVertexType nextType  = NEW;
    HEEdge new_previous;
    HEEdge new_next;
    HEEdge twin_next;
    HEEdge twin_previous;
    bool found = false;
    while (!found) {
        HEVertex current_vertex = hed.target( current_edge );
        HEEdge next_edge = hed[current_edge].next;
        HEVertex next_vertex = hed.target( next_edge );
        if ( hed[current_vertex].type == currentType ) {
            if ( hed[next_vertex].type == nextType ) {
                new_source = next_vertex;
                new_previous = next_edge;
                twin_next = hed[next_edge].next;
                found = true;
            }
        }
        //assert( current_and_next_on_same_face( current_edge ) );
        current_edge = hed[current_edge].next;   
    }
    found = false;
    currentType = IN;
    nextType = NEW;
    current_edge = hed.face_edge(f);
    while (!found) {
        HEVertex current_vertex = hed.target( current_edge );
        HEEdge next_edge = hed[current_edge].next;
        HEVertex next_vertex = hed.target( next_edge );
        if ( hed[current_vertex].type == currentType ) {
            if ( hed[next_vertex].type == nextType ) {
                new_target = next_vertex;
                new_next = hed[next_edge].next;
                twin_previous = next_edge;
                found = true;
            }
        }
        current_edge = hed[current_edge].next;
    }
    
    // now connect new_previous -> new_source -> new_target -> new_next
    HEEdge e_new = hed.add_edge( new_source, new_target ); // face,next,twin
    hed[new_previous].next = e_new;
    hed[e_new].next = new_next;
    hed[e_new].face = f;
    
    
    // the twin edge that bounds the new face
    HEEdge e_twin = hed.add_edge( new_target, new_source );
    hed[e_twin].twin = e_new;
    hed[e_new].twin = e_twin;
    hed[e_twin].face = newface;
    hed.set_face_edge(newface, e_twin);
    // faces[newface].edge = e_twin;
    hed[twin_previous].next = e_twin;
    hed[e_twin].next = twin_next;
    hed.set_face_edge(f,  e_new);  // make sure faces are ok
    
    //assert( isDegreeThree() );
}

//FaceVector VoronoiDiagram::get_incident_faces() {
//    return incident_faces;
//}

void VoronoiDiagram::add_new_voronoi_vertices(VertexVector& v0, Point& p) {
    // generate new vertices on all edges in V0 connecting to OUT-vertices
    //std::cout << "add_new_voronoi_vertices() \n";
    assert( !v0.empty() );
    std::vector<HEEdge> q_edges; // new vertices generated on these edges
    BOOST_FOREACH( HEVertex v, v0 ) {
        assert( hed[v].type == IN ); // all verts in v0 are IN
        HEOutEdgeItr edgeItr, edgeItrEnd;
        boost::tie( edgeItr, edgeItrEnd ) = boost::out_edges(v, hed);
        // among the out-edges of v, find the one that is OUT
        for ( ; edgeItr != edgeItrEnd ; ++edgeItr ) {
            HEVertex adj_vertex = hed.target( *edgeItr );
            if ( hed[adj_vertex].type == OUT ) 
                q_edges.push_back(*edgeItr);// adding vertices here invalidates the iterator edgeItr, so we can't do it here...
        }
    }
    assert( !q_edges.empty() );
    
    for( unsigned int m=0; m<q_edges.size(); ++m)  {  // create new vertices on edges q_edges[]
        HEVertex q = hed.add_vertex();
        hed[q].type = NEW;
        in_vertices.push_back(q);
        
        HEFace face = hed[q_edges[m]].face;
        HEEdge twin = hed[q_edges[m]].twin;
        HEFace twin_face = hed[twin].face;
        Point generator1 = hed.face_generator(face);
        Point generator2 = hed.face_generator(twin_face);
        hed[q].set_J( generator1, generator2, p); // ordering of generators shoud be CCW !?
        hed[q].set_position();
        //std::cout << " added vertex "<< q << " on edge " << q_edges[m] <<" at  " << vd[q].position << " to vd. \n";
        
        if ( hed.face_type(face) != INCIDENT ) {
            std::cout << "ERROR new vertex " << q << " on edge " << q_edges[m] << " on face " << face << "\n";
            std::cout << "ERROR left_face " << face << " not INCIDENT\n";
        }
        if ( hed.face_type(twin_face) != INCIDENT ) {
            std::cout << "ERROR new vertex " << q << " on edge " << q_edges[m] << " on face " << twin_face << "\n";
            std::cout << "ERROR twin_face " << twin_face << " not INCIDENT\n";
        }
        
        assert(  hed.face_type(face)  == INCIDENT);
        assert( hed.face_type(twin_face) == INCIDENT);
        
        
        hed.insert_vertex_in_edge( q, q_edges[m] );
        

    }
}


/* void VoronoiDiagram::insert_vertex_in_edge(VoronoiVertex v, VoronoiEdge e) {
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
    if ( source != twin_target )
        std::cout << " ERROR " << e << " source is " << source << " but " << twin << " target is " << twin_target << "\n"; 
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
    //std::cout << "  " << e << " split into " << e1 << " and " << e2 << "\n";
    //std::cout << "  " << twin << " split into " << te1 << " and " << te2 << "\n";
    
    // update the faces
    faces[face].edge = e1;
    faces[twin_face].edge = te1;
    
    // finally, remove the old edge
    boost::remove_edge( e, vd);
    boost::remove_edge( twin, vd);
}*/

/* VoronoiEdge VoronoiDiagram::find_previous_edge(VoronoiEdge e) {
    VoronoiEdge previous = vd[e].next;
    while ( vd[previous].next != e ) {
        previous = vd[previous].next;
    }
    return previous;
}*/

/* FaceVector VoronoiDiagram::get_adjacent_faces( VoronoiVertex q ) {
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
}*/

void VoronoiDiagram::augment_vertex_set(VertexVector& q, Point& p) {
    // RB2   voronoi-point q[0] = q( a, b, c ) is the seed
    // add faces Ca,Cb,Cc to the stack
    FaceVector adjacent_faces = hed.get_adjacent_faces( q[0] );
    in_vertices.push_back( q[0] );
    
    assert( adjacent_faces.size()==3 ); // in a degree three graph every vertex has three adjacent faces
    
    std::stack<HEFace> S; // B1.3  we push all the adjacent faces onto the stack, and label them INCIDENT
    //std::cout << "   seed vertex " << q[0] << " has adjacent faces: ";
    BOOST_FOREACH(HEFace f, adjacent_faces) {
        //faces[f].type = INCIDENT;
        hed.set_face_type(f, INCIDENT);
        incident_faces.push_back(f);
        //std::cout << " " << f << " ";
        //std::cout << " faces[f].type = " << faces[f].type  << "  \n";
        S.push(f);
    }
    //std::cout << "\n";
    
    while ( !S.empty() ) { 
        HEFace f = S.top();
        S.pop();
        //std::cout << "    augmenting from face = " << f << "\n";
        HEEdge current_edge = hed.face_edge(f); //faces[f].edge;
        HEEdge start_edge = current_edge;
        bool done=false;
        while (!done) {
            //std::cout << current_edge << "\n";
            
            //assert( faces[ vd[current_edge].face ].type == INCIDENT );
            //assert( vd[current_edge].face == f );
            
            // G( V, E, C) is the voronoi graph, C=cycles=faces
            // v0 is the set of vertices to be removed E(v0) are all edges that connect two v0 vertices
            // (T4) G( v0 , E(v0) ) is a tree
            // (T5) for any face c such that (v0 intersect V(c)) != 0 then 
            //       graph G(  v0 intersect V(c) , E( v0 int V(c) ) ) is connected
            // E0, the cut edges, is set of edges from V0 to V-V0
            HEVertex v = hed.target( current_edge );
            if ( hed[v].type == UNDECIDED ) {
                // B2.1  mark "out"  v in cycle_alfa if 
                //  (T6) v is adjacent to an IN vertex in V-Vin(alfa)
                //  (T7) v is on an "incident" cycle other than this cycle and is not adjacent to a vertex in Vin
                in_vertices.push_back( v );
                
                if ( hed[v].detH( p ) < 0.0 ) {
                    //std::cout << "     found IN vertex " << v << " on edge " << current_edge <<" face " << f <<"\n";
                    hed[v].type = IN;
                    q.push_back(v);
                    
                    // also set the adjacent faces to incident
                    FaceVector new_adjacent_faces = hed.get_adjacent_faces( v );
                    BOOST_FOREACH( HEFace adj_face, new_adjacent_faces ) {
                        if ( hed.face_type(adj_face) != INCIDENT ) {
                            hed.set_face_type(adj_face, INCIDENT);
                            incident_faces.push_back(adj_face);
                            //std::cout << "     setting twin face " << adj_face << " INCIDENT \n";
                            S.push(adj_face);
                        }
                    }
                } else {
                    hed[v].type = OUT;
                    //in_vertices.push_back( v );
                }
            }
            
            current_edge = hed[current_edge].next;
            //assert( current_and_next_on_same_face(current_edge) );
            if ( current_edge == start_edge )
                done = true;
        }

        
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
/*
bool VoronoiDiagram::current_and_next_on_same_face(VoronoiEdge current_edge) {
    return ( vd[current_edge].face == vd[ vd[current_edge].next ].face );
}*/

VertexVector VoronoiDiagram::find_seed_vertex(HEFace face_idx, const Point& p) {
    // evaluate H on all vertices along facet edges and return
    // vertex with the lowest H
    
    // find vertices that bound face_idx 
    VertexVector q_verts = hed.get_face_vertices(face_idx);                 assert( q_verts.size() >= 3 );
    double minimumH = 1; // safe, because we expect the min H to be negative...
    HEVertex minimalVertex;
    double h;
    BOOST_FOREACH( HEVertex q, q_verts) {
        if ( hed[q].type != OUT ) {
            h = hed[q].detH( p ); 
            if (h<minimumH) { // find minimum H value among q_verts
                minimumH = h;
                minimalVertex = q;
            }
        }
    }
    if (!(minimumH < 0) ) {
        std::cout << " ERROR: searching for seed when inserting " << p  << "  \n";
        std::cout << " ERROR: closest face is  " << face_idx  << " with generator " << hed.face_generator(face_idx) << " \n";
        std::cout << " ERROR: minimal vd-vertex " << hed[minimalVertex].position << " has \n";
        std::cout << " ERROR: detH = " << minimumH << " ! \n";
    }
    assert( minimumH < 0 );
    hed[minimalVertex].type = IN;
    VertexVector output;
    output.push_back(minimalVertex);
    return output;
}

// traverse face and return all vertices found
/* VertexVector VoronoiDiagram::get_face_vertices(FaceIdx face_idx) {
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
}*/

boost::python::list VoronoiDiagram::getGenerators()  {
    boost::python::list plist;
    for ( HEFace m=0;m<hed.face_size();++m ) {
        plist.append( hed.face_generator(m) );
    }
    return plist;
}



boost::python::list VoronoiDiagram::getVoronoiVertices() const {
    boost::python::list plist;
    HEVertexItr it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( hed );
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        if ( boost::degree( *itr, hed ) == 6 ) {
            plist.append( hed[*itr].position );
        }
    }
    return plist;
}

boost::python::list VoronoiDiagram::getFarVoronoiVertices() const {
    boost::python::list plist;
    HEVertexItr it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( hed );
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        if ( boost::degree( *itr, hed ) == 4 ) {
            plist.append( hed[*itr].position );
        }
    }
    return plist;
}

// return edge and the generator corresponding to its face
boost::python::list VoronoiDiagram::getEdgesGenerators()  {
    boost::python::list edge_list;
    HEEdgeItr itr, it_end;
    boost::tie( itr, it_end ) = boost::edges( hed );
    for (  ; itr != it_end ; ++itr ) { // loop through each edge
            boost::python::list point_list; // the endpoints of each edge
            HEVertex v1 = boost::source( *itr, hed  );
            HEVertex v2 = boost::target( *itr, hed  );
            Point src = hed[v1].position;
            Point tar = hed[v2].position;
            // shorten the edge, for visualization
            //double cut_amount = 0.0;
            //Point src_short = src + (cut_amount/((tar-src).norm())) * (tar-src);
            //Point tar_short = src + (1-cut_amount/((tar-src).norm())) * (tar-src);
            point_list.append( src );
            point_list.append( tar );
            //FaceIdx f = vd[*itr].face;
            //Point gen = faces[f].generator;
            //Point orig = gen.xyClosestPoint(src, tar);
            //Point dir = gen-orig;
            //dir.xyNormalize(); 
            //point_list.append( dir );
            edge_list.append(point_list);
    }
    return edge_list;
}

boost::python::list VoronoiDiagram::getVoronoiEdges() const {
    boost::python::list edge_list;
    HEEdgeItr itr, it_end;
    boost::tie( itr, it_end ) = boost::edges( hed );
    for (  ; itr != it_end ; ++itr ) { // loop through each edge
            boost::python::list point_list; // the endpoints of each edge
            HEVertex v1 = boost::source( *itr, hed  );
            HEVertex v2 = boost::target( *itr, hed  );
            point_list.append( hed[v1].position );
            point_list.append( hed[v2].position );
            edge_list.append(point_list);
    }
    return edge_list;
}

std::string VoronoiDiagram::str() const {
    std::ostringstream o;
    o << "VoronoiDiagram (nVerts="<< boost::num_vertices( hed ) << " , nEdges="<< boost::num_edges( hed ) <<"\n";
    return o.str();
}

} // end namespace
// end file weave.cpp
