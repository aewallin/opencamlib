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

#include "voronoidiagram.h"
#include "numeric.h"

namespace ocl
{

VoronoiDiagram::VoronoiDiagram(double far, unsigned int n_bins) {
    fgrid = new FaceGrid(far, n_bins);
    far_radius=far;
    gen_count=3;
    init();
}


// sanity check
bool VoronoiDiagram::isValid() {
    if (!isDegreeThree() )
        return false;
    if (!face_count_equals_generator_count())
        return false;
    return true;
}

bool VoronoiDiagram::isDegreeThree() {
    // the outermost init() vertices have special degree, all others == 6
    BOOST_FOREACH(HEVertex v, hed.vertices() ) {
        if ( hed.degree( v ) != 6 ) {
            if ( (v != v01) && (v != v02) && (v != v03) )
                return false;
        }
    }
    return true;
}


bool VoronoiDiagram::face_count_equals_generator_count() {
    int vertex_count = hed.num_vertices();
    int face_count = (vertex_count- 4)/2 + 3;
    if (face_count != gen_count) {
        std::cout << " num_vertices = " << vertex_count << "\n";
        std::cout << " gen_count = " << gen_count << "\n";
        std::cout << " face_count = " << face_count << "\n";
        assert(0);
    }
    return ( face_count == gen_count );
}

// add one vertex at origo and three vertices at 'infinity' and their associated edges
void VoronoiDiagram::init() {
    //std::cout << "VD init() \n";
    double far_multiplier = 6;
    // add vertices
    HEVertex v0;
    v0  = hed.add_vertex( VertexProps(Point(0,0), UNDECIDED) );
    v01 = hed.add_vertex( VertexProps(Point(0, far_multiplier*far_radius), OUT) );
    v02 = hed.add_vertex( VertexProps(Point( cos(-5*PI/6)*far_multiplier*far_radius, sin(-5*PI/6)*far_multiplier*far_radius), OUT) );
    v03 = hed.add_vertex( VertexProps(Point( cos(-PI/6)*far_multiplier*far_radius, sin(-PI/6)*far_multiplier*far_radius), OUT) );

    // the locations of the initial generators:
    double gen_mutliplier = 3;
    gen2 = Point(cos(PI/6)*gen_mutliplier*far_radius, sin(PI/6)*gen_mutliplier*far_radius);
    gen3 = Point(cos(5*PI/6)*gen_mutliplier*far_radius, sin(5*PI/6)*gen_mutliplier*far_radius);
    gen1 = Point( 0,-gen_mutliplier*far_radius);
    hed[v0].set_J( gen1, gen2, gen3 ); // this sets J2,J3,J4 and pk, so that detH(pl) can be called later
        
    // add face 1: v0-v1-v2
    HEEdge e1 =  hed.add_edge( v0, v01  );   
    HEEdge e2 =  hed.add_edge( v01, v02 );
    HEEdge e3 =  hed.add_edge( v02, v0  ); 
    HEFace f1 =  hed.add_face( FaceProps(e2, gen3, NONINCIDENT) );
    fgrid->add_face( hed[f1] );
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
    HEFace f2 =  hed.add_face( FaceProps(e5, gen1, NONINCIDENT) );
    fgrid->add_face( hed[f2] );
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
    HEFace f3 =  hed.add_face( FaceProps(e8, gen2, NONINCIDENT) );
    fgrid->add_face( hed[f3] );
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
    
    assert( isValid() );
    //std::cout << " VD init() done.\n";
}


// comments relate to Sugihara-Iri paper
void VoronoiDiagram::addVertexSite(Point p) {
    //std::cout << "VD: addVertexSite()\n";
    gen_count++;
    // 1) find the generator-vertex/face closest to p
    // 2) amont the vd-vertices in the found face, find the one with the smallest H
    // 3) expand the tree of to-be-deleted vertices as far as possible:
    //   3.1 tree property
    //   3.2 H<0 property
    // 4) generate new cycle/face around new vertices
    // 5) delete to-be-deleted vertices and edges
    //std::cout << " VD: adding vertex site " << p << std::endl;
    // 1) find the face corresponding to the closest generator
    HEFace closest_face = fgrid->grid_find_closest_face( p );
    // B1.2 find seed vertex by evaluating H on the vertices of the found face
    HEVertex v_seed = find_seed_vertex(closest_face, p);
    hed[v_seed].type = IN;
    VertexVector v0;
    v0.push_back(v_seed); 
    // expand from seed v0[0] find the set V0
    
    
    //augment_vertex_set(v0, p);
    augment_vertex_set_RB(v0, p);
    
    
    // add new vertices on all edges that connect v0 IN edges to OUT edges
    add_new_voronoi_vertices(v0, p);
    // generate new edges that form a loop around the region to be deleted
    HEFace newface = split_faces(p);
    // fix the next-pointers in newface, then remove set v0
    remove_vertex_set(v0, newface);
    // reset IN/OUT/UNDECIDED for verts, and INCIDENT/NONINCIDENT for faces
    reset_labels();
    
    //assert( isValid() );
    //std::cout << "VD: addVertexSite() done.\n";
}

HEFace VoronoiDiagram::split_faces(Point& p) {
    HEFace newface =  hed.add_face( FaceProps( HEEdge(), p, NONINCIDENT ) );
    fgrid->add_face( hed[newface] );
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
    BOOST_FOREACH(HEFace f, incident_faces ) { hed[f].type = NONINCIDENT; }
    incident_faces.clear();
}

// remove vertices in the set v0
void VoronoiDiagram::remove_vertex_set(VertexVector& v0 , HEFace newface) {
    HEEdge current_edge = hed[newface].edge; 
    //HEVertex current_target; 
    HEEdge start_edge = current_edge;
    
    // this repairs the next-pointers for newface that are broken.
    bool done = false;
    while (!done) {
        HEVertex current_target = hed.target( current_edge ); // an edge on the new face
        HEVertex current_source = hed.source( current_edge );
        BOOST_FOREACH( HEEdge edge, hed.out_edges( current_target ) ) { // loop through potential "next" candidates
            HEVertex out_target = hed.target( edge );
            if ( hed[out_target].type == NEW ) { // the next vertex along the face should be "NEW"
                if ( out_target != current_source ) { // but not where we came from
                    hed[current_edge].next = edge; // this is the edge we want to take
                    
                    // current and next should belong on the same face
                    if (hed[current_edge].face !=  hed[ hed[current_edge].next ].face) {
                        std::cout << " VD remove_vertex_set() error.\n";
                        std::cout << "current.face = " << hed[current_edge].face << " IS NOT next_face = " << hed[ hed[current_edge].next ].face << std::endl;
                        printFaceVertexTypes(hed[current_edge].face);
                        printFaceVertexTypes(hed[ hed[current_edge].next ].face);
                    }
                    assert( hed[current_edge].face ==  hed[ hed[current_edge].next ].face );
                }
            }
        }
        if ( hed[current_edge].next == start_edge )
            done = true;
        current_edge = hed[current_edge].next; // jump to the next edge
    }
    // it should now be safe to delete v0
    BOOST_FOREACH( HEVertex v, v0 ) { hed.delete_vertex(v); }
}

// split the face f
void VoronoiDiagram::split_face(HEFace newface, HEFace f) {
    HEVertex new_source; // this is found as OUT-NEW-IN
    HEVertex new_target; // this is found as IN-NEW-OUT
    // the new vertex on face f connects new_source -> new_target
    HEEdge current_edge = hed[f].edge;                             assert( f == hed[current_edge].face );
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
    current_edge = hed[f].edge; 
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
    hed[newface].edge = e_twin; 
    hed[twin_previous].next = e_twin;
    hed[e_twin].next = twin_next;
    hed[f].edge = e_new; 
    
    //assert( isDegreeThree() );
}

// the set v0 are IN vertices that should be removed
// generate new voronoi-vertices on all edges connecting v0 to OUT-vertices
void VoronoiDiagram::add_new_voronoi_vertices(VertexVector& v0, Point& p) {
    assert( !v0.empty() );
    EdgeVector q_edges; // new vertices generated on these edges
    BOOST_FOREACH( HEVertex v, v0 ) {                                   assert( hed[v].type == IN ); // all verts in v0 are IN
        BOOST_FOREACH( HEEdge edge, hed.out_edges( v ) ) {
            HEVertex adj_vertex = hed.target( edge );
            if ( hed[adj_vertex].type == OUT ) 
                q_edges.push_back(edge);
        }
    }
    assert( !q_edges.empty() );
    
    for( unsigned int m=0; m<q_edges.size(); ++m )  {  // create new vertices on all edges q_edges[]
        HEVertex q = hed.add_vertex();
        hed[q].type = NEW;
        in_vertices.push_back(q);
        HEFace face = hed[q_edges[m]].face;     assert(  hed[face].type == INCIDENT);
        HEEdge twin = hed[q_edges[m]].twin;
        HEFace twin_face = hed[twin].face;      assert( hed[twin_face].type == INCIDENT);
        hed[q].set_J( hed[face].generator  , hed[twin_face].generator  , p); 
        hed[q].set_position();
        hed.insert_vertex_in_edge( q, q_edges[m] );
    }
}

// for visualizing the vertex set to be removed
boost::python::list getVertexSet() {
    boost::python::list plist;
    return plist;
}

// return true if w is adjacent to an IN-vertex not in f.
bool VoronoiDiagram::adjacentInVertexNotInFace( HEVertex w, HEFace f ) {
    VertexVector adj_verts = hed.adjacent_vertices(w);
    VertexVector face_verts = hed.face_vertices(f);
    BOOST_FOREACH( HEVertex v, adj_verts ) {
        if ( hed[v].type == IN ) {
            // check if v belongs to f
            BOOST_FOREACH( HEVertex face_vert, face_verts ) {
                if ( face_vert == v ) {
                    return false;
                }
            }
            return true; // IN vertex not in f
        }
    }
    return false;
}

// return true if w is adjacent to an IN vertex in f
bool VoronoiDiagram::adjacentInVertexInFace( HEVertex w, HEFace f ) {
    VertexVector adj_verts = hed.adjacent_vertices(w);
    VertexVector face_verts = hed.face_vertices(f);
    BOOST_FOREACH( HEVertex v, adj_verts ) {
        if ( hed[v].type == IN ) { // adjacent IN vertex
            BOOST_FOREACH( HEVertex face_vert, face_verts ) {
                if ( face_vert == v ) {
                    return true;// IN vertex in f
                }
            }
            return false; 
        }
    }
    return false;
}

// return true if only one of the adjacent faces are INCIDENT
bool VoronoiDiagram::onOtherIncidentFace( HEVertex v , HEFace f) {
    FaceVector adjacent_faces = hed.adjacent_faces( v );
    
    if (adjacent_faces.size()!=3) {
        std::cout << " adjacent_faces.size()!=3 " << adjacent_faces.size() << std::endl;
        std::cout << " vertex.position " << hed[v].position << std::endl;
    }
    //assert( adjacent_faces.size()==3 );
    int count = 0;
    BOOST_FOREACH( HEFace f, adjacent_faces ) {
        if ( hed[f].type == INCIDENT )
            ++count;
    }
    //std::cout << count << " INCIDENT adjacent faces \n";
    return ( count > 1 );
} 

bool VoronoiDiagram::noOutVertexInFace( HEFace f ) {
    VertexVector face_verts = hed.face_vertices(f);
    BOOST_FOREACH( HEVertex v, face_verts ) {
        if ( hed[v].type == OUT )
            return false;
    }
    return true;
}

VertexVector VoronoiDiagram::removeVertex( VertexVector verts, HEVertex v ) {
    VertexVector out;
    BOOST_FOREACH( HEVertex w, verts ) {
        if ( w!=v) 
            out.push_back(w);
    }
    return out;
}

// this is "Algorithm B" from Sugihara&Iri 1994, page 20-21
// http://dx.doi.org/10.1142/S0218195994000124
void VoronoiDiagram::augment_vertex_set_RB(VertexVector& q, Point& p) {
    std::cout << "augment_vertex_set_RB()\n";
    assert( q.size()==1 ); // RB2   voronoi-point q[0] = q( a, b, c ) is the seed
    assert( hed[ q[0] ].type == IN ); 
    FaceVector adjacent_faces = hed.adjacent_faces( q[0] );
    assert( adjacent_faces.size()==3 ); // degree three diagram
    in_vertices.push_back( q[0] );
    
    std::stack<HEFace> S;
    BOOST_FOREACH(HEFace f, adjacent_faces) { // push adjacent faces onto stack
        hed[f].type = INCIDENT; 
        incident_faces.push_back(f);
        S.push(f);
    }
    
    while ( !S.empty() ) { // B2 process stack.
        HEFace f = S.top();
        std::cout << "Augment from face= " << f << std::endl;
        printFaceVertexTypes(f);
        S.pop();
        VertexVector tmp_verts = hed.face_vertices(f);
        // remove q[0]
        VertexVector face_verts = removeVertex( tmp_verts, q[0] );
        
        //removeVertex( face_verts, q[0] );
        
        BOOST_FOREACH( HEVertex v, face_verts ) { // go through all vertices in face and run B2.1 
            // B2.1  mark "out"  v in cycle_alfa if 
            //  (T6) v is adjacent to an IN vertex in V-Vin(alfa)
            if ( hed[v].type == UNDECIDED ) {
                if ( adjacentInVertexNotInFace( v, f ) ) {
                    std::cout << " B2.1 T6 OUT " << hed[v].position << "\n";
                    hed[v].type = OUT;
                    in_vertices.push_back( v );
                } else if ( onOtherIncidentFace( v, f ) ) {
                    //  (T7) v is on an "incident" cycle other than this cycle and is not adjacent to a vertex in Vin(alfa)
                    if ( !adjacentInVertexInFace( v, f) ) {
                        std::cout << " B2.1 T7 OUT " << hed[v].position << "\n";
                        hed[v].type = OUT;
                        in_vertices.push_back( v );
                    }
                }
            }
        }
            
        // B2.2 if OUT-graph is disconnected, find minimal set that makes it connected and mark OUT
        // FIXME TODO TODO
        
        // B2.3 if no OUT-vertices exist, find UNDECIDED vertex with maximum H and mark OUT
        if ( noOutVertexInFace( f ) ) {
            std::cout << " B2.3 no OUT-vertices case.\n";
            double maxH = -1;
            HEVertex maximal_v;
            BOOST_FOREACH( HEVertex v, face_verts ) {
                if ( hed[v].type == UNDECIDED ) {
                    double h = hed[v].detH( p );
                    if ( h > maxH) {
                        maxH = h;
                        maximal_v = v;
                    }
                }
            }
            std::cout << " max H vertex is " << hed[maximal_v].position << "with detH= " << maxH << std::endl;
            // mark OUT
            hed[maximal_v].type = OUT;
            in_vertices.push_back( maximal_v );
        }
        // now we have at least one OUT-vertex, and if we have many they are connected.
        
        // B2.4 while UNDECIDED vertices remain, do B2.4.1, B2.4.2, and B2.4.3
        
        // B2.4.1 find UNDECIDED vertex v with maximal abs( detH )
        // B2.4.2 if detH >= 0, mark OUT.  
        //        if OUT-graph becomes disconnected, repair it with UNDECIDED vertices
        // B2.4.3 if detH < 0, mark IN
        //        mark adjacent faces INCIDENT, and add to stack
        //        if IN-graph becomes disconnected, repair with UNDECIDED vertices
        bool done = false;
        
        while (!done) {
            VertexVector uverts;
            BOOST_FOREACH( HEVertex v, face_verts ) {
                if ( hed[v].type == UNDECIDED ) {
                    uverts.push_back(v);
                }
            }
            if ( uverts.empty() ) {
                std::cout << uverts.size() << " UNDECIDED remain. DONE\n";
                done = true;
            } else {
                std::cout << uverts.size() << " UNDECIDED remain\n";
                // find uvert with highest abs(H)
                double max_abs_h = 0;
                double det_v;
                HEVertex max_v = HEVertex();
                BOOST_FOREACH( HEVertex v, uverts ) {
                    det_v = hed[v].detH(p);
                    //std::cout << det_v << " determinant\n";
                    //std::cout << abs(det_v) << " abs determinant\n";
                    if ( abs(det_v) >= max_abs_h) {
                        max_abs_h = abs(det_v);
                        max_v=v;
                    }
                }
                if (max_v == HEVertex() ) {
                    std::cout << " no max H vertex found! \n";
                    assert(0);
                }
                std::cout << " max abs(H) vertex is " << hed[max_v].position << " with detH= " << det_v << std::endl;
                // now max_v is the one with highest abs(H)
                if ( det_v >= 0.0 ) {
                    hed[max_v].type = OUT;
                    in_vertices.push_back( max_v );
                    std::cout << "OUT vertex found.\n";
                    // check if OUT-graph is disconnected. repair with UNDECIDED vertices
                } else {
                    assert( det_v < 0.0 );
                    hed[max_v].type = IN;
                    in_vertices.push_back( max_v );
                    std::cout << "IN vertex found.\n";
                    in_vertices.push_back( max_v );
                    q.push_back(max_v);
                    FaceVector new_adjacent_faces = hed.adjacent_faces( max_v ); // also set the adjacent faces to incident
                    BOOST_FOREACH( HEFace adj_face, new_adjacent_faces ) {
                        if ( hed[adj_face].type  != INCIDENT ) {
                            hed[adj_face].type = INCIDENT; 
                            incident_faces.push_back(adj_face);
                            S.push(adj_face);
                        }
                    }
                    // check if IN-graph is disconnected. repair with UNDECIDED vertices
                }
            } // end UNDECIDED processing
        }
        
        // we should now be done with face f
        // IN vertices should be connected
        assert( faceVerticesConnected(  f, IN ) );
        // OUT vertices should be connected
        assert( faceVerticesConnected(  f, OUT ) );
        // no UNDECIDED vertices should remain
        std::cout << "Face: " << f << " augment Done: ";
        printFaceVertexTypes(f);
    } // end stack while-loop
    std::cout << "augment_vertex_set_RB() DONE.\n";

}

// check that the vertices TYPE are connected
bool VoronoiDiagram::faceVerticesConnected( HEFace f, VoronoiVertexType Vtype ) {
    VertexVector face_verts = hed.face_vertices(f);
    VertexVector type_verts;
    BOOST_FOREACH( HEVertex v, face_verts ) {
        if ( hed[v].type == Vtype )
            type_verts.push_back(v);
    }
    if (type_verts.empty())
        return true;
    else if (type_verts.size()==1)
        return true;
    
    std::cout << type_verts.size() << " verts of type " << Vtype << " to check\n";
    printFaceVertexTypes(f);
    // check that type_verts are connected
    // each vertex should connect to another vertex in type_verts
    BOOST_FOREACH( HEVertex v, type_verts) {
        assert( hed[v].type == Vtype );
        bool found=false;
        BOOST_FOREACH( HEVertex w, type_verts ) {
            // v should connect to some w, but not itse
            if ( w != v ) {
                if ( hed.edge( v , w) ||  hed.edge( w , v) ) // v is connected to w
                    found = true;
            }
        }
        if (!found)
            return false;
    }
    return true;
}

void VoronoiDiagram::printFaceVertexTypes(HEFace f) {
    std::cout << " Face " << f << ": ";
    VertexVector face_verts = hed.face_vertices(f);
    BOOST_FOREACH( HEVertex v, face_verts ) {
        std::cout << hed[v].type << " ";
    }
    std::cout << "\n";
}

// simple algorithm for finding vertex set
// relies on the correct computation of detH (this will fail due to floating point error - sooner or later)
// roughly "Algorithm RB" from Sugihara& Iri 1994, page 18
void VoronoiDiagram::augment_vertex_set(VertexVector& q, Point& p) {
    assert(q.size()==1);
    // RB2   voronoi-point q[0] = q( a, b, c ) is the seed
    // add adjacent faces Ca,Cb,Cc to the stack
    FaceVector adjacent_faces = hed.adjacent_faces( q[0] );
    assert( adjacent_faces.size()==3 ); // in a degree three graph every vertex has three adjacent faces
    
    in_vertices.push_back( q[0] );
    
    std::stack<HEFace> S; // B1.3  we push all the adjacent faces onto the stack, and label them INCIDENT
    BOOST_FOREACH(HEFace f, adjacent_faces) {
        hed[f].type = INCIDENT; 
        incident_faces.push_back(f);
        S.push(f);
    }
    
    while ( !S.empty() ) { // examine all incident faces until done.
        HEFace f = S.top();
        S.pop();
        HEEdge current_edge = hed[f].edge; 
        HEEdge start_edge = current_edge;
        bool done=false;
        while (!done) {
            assert( hed[ hed[current_edge].face ].type == INCIDENT );
            assert( hed[current_edge].face == f );
            
            // G( V, E, C) is the voronoi graph, C=cycles=faces
            // v0 is the set of vertices to be removed E(v0) are all edges that connect two v0 vertices
            // (T4) G( v0 , E(v0) ) is a tree
            // (T5) for any face/cycle c such that (v0 intersect V(c)) != 0 then 
            //       graph G(  v0 intersect V(c) , E( v0 int V(c) ) ) is connected
            //     NOTE: this means a face cannot be split in two
            // E0, the cut edges, is set of edges from V0 to V-V0
            HEVertex v = hed.target( current_edge );
            if ( hed[v].type == UNDECIDED ) {
                // B2.1  mark "out"  v in cycle_alfa if 
                //  (T6) v is adjacent to an IN vertex in V-Vin(alfa)
                //  (T7) v is on an "incident" cycle other than this cycle and is not adjacent to a vertex in Vin
                in_vertices.push_back( v );
                
                if ( hed[v].detH( p ) < 0.0 ) {
                    hed[v].type = IN;
                    q.push_back(v);
                    
                    FaceVector new_adjacent_faces = hed.adjacent_faces( v ); // also set the adjacent faces to incident
                    BOOST_FOREACH( HEFace adj_face, new_adjacent_faces ) {
                        if ( hed[adj_face].type  != INCIDENT ) {
                            hed[adj_face].type = INCIDENT; 
                            incident_faces.push_back(adj_face);
                            S.push(adj_face);
                        }
                    }
                } else {
                    hed[v].type = OUT;
                    //in_vertices.push_back( v );
                }
            }
            current_edge = hed[current_edge].next; // jump to the next edge on this face
            
            if ( current_edge == start_edge ) // when we are back where we started, stop.
                done = true;
        }

        
        // B2.2 if subgraph (Vout,Eout) is disconnected, find minimal set V* of undecided vertices such that (Vout U V*) is connected
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
        
    }
}

// evaluate H on all face vertices and return
// vertex with the lowest H
HEVertex VoronoiDiagram::find_seed_vertex(HEFace face_idx, const Point& p) {
    VertexVector face_verts = hed.face_vertices(face_idx);                 
    assert( face_verts.size() >= 3 ); 
    double minimumH = 1; // safe, because we expect the min H to be negative...
    HEVertex minimalVertex;
    double h;
    BOOST_FOREACH( HEVertex q, face_verts) {
        if ( hed[q].type != OUT ) {
            h = hed[q].detH( p ); 
            std::cout << "  detH = " << h << " ! \n";
            if (h<minimumH) { // find minimum H value among q_verts
                minimumH = h;
                minimalVertex = q;
            }
        }
    }
    if (!(minimumH < 0) ) {
        std::cout << " VD find_seed_vertex() \n";
        std::cout << " ERROR: searching for seed when inserting " << p  << "  \n";
        std::cout << " ERROR: closest face is  " << face_idx  << " with generator " << hed[face_idx].generator  << " \n";
        std::cout << " ERROR: detH = " << minimumH << " ! \n";
        std::cout << " ERROR: minimal vd-vertex " << hed[minimalVertex].position << " has \n";
        
    }
    assert( minimumH < 0 );
    return minimalVertex;
}

//  voronoi-faces are dual to delaunay-vertices.
//  vornoi-vertices are dual to delaunay-faces 
//  voronoi-edges are dual to delaunay-edges(connect two faces)
HalfEdgeDiagram* VoronoiDiagram::getDelaunayTriangulation()  {
    //std::cout << "VD: getDelaunayTriangulation()()\n";
    HalfEdgeDiagram* dt = new HalfEdgeDiagram();
    // loop through faces and add vertices/generators
    typedef std::pair<HEFace, HEVertex> FaVePair;
    typedef std::map<HEFace, HEVertex> FaVeMap;
    FaVeMap map;
    //VertexVector verts;
    for ( HEFace f=0;f<hed.num_faces();++f ) {
        Point pos = hed[f].generator;
        if ( pos != gen1 && pos != gen2 && pos != gen3) { // don't add the special init()-generators
            HEVertex v = dt->add_vertex( VertexProps( pos , OUT)  );
            //verts.push_back(v); 
            map.insert( FaVePair(f,v) );
        }
    }
    
    // loop through voronoi-edges and add delaunay-edges
    FaVeMap::iterator itr;
    BOOST_FOREACH( HEEdge edge, hed.edges() ) {
            HEFace face = hed[edge].face;
            //std::cout << " vd faces: " << face << " \n";
            HEEdge twin_edge = hed[edge].twin;
            if (twin_edge != HEEdge() ) {
                HEFace twin_face = hed[twin_edge].face;
                //std::cout << " vd faces: " << face << " , " << twin_face << std::endl;
                
                itr = map.find(face);
                if (itr != map.end() ) {
                    HEVertex v1 = itr->second;
                    itr = map.find(twin_face);
                    if (itr != map.end() ) {
                        HEVertex v2 = itr->second;
                        //std::cout << " dt edge " << v1 << " , " << v2 << std::endl;
                        
                        dt->add_edge( v1, v2 );
                    }
                }
            }
    }
    //std::cout << "VD: getDelaunayTriangulation() done.\n";
    return dt;
}

boost::python::list VoronoiDiagram::getGenerators()  {
    boost::python::list plist;
    for ( HEFace f=0;f<hed.num_faces();++f ) {
        plist.append( hed[f].generator  );
    }
    return plist;
}

boost::python::list VoronoiDiagram::getVoronoiVertices() const {
    boost::python::list plist;
    BOOST_FOREACH( HEVertex v, hed.vertices() ) {
        if ( hed.degree( v ) == 6 ) {
            plist.append( hed[v].position );
        }
    }
    return plist;
}

boost::python::list VoronoiDiagram::getFarVoronoiVertices() const {
    boost::python::list plist;
    BOOST_FOREACH( HEVertex v, hed.vertices() ) {
        if ( hed.degree( v ) == 4 ) {
            plist.append( hed[v].position );
        }
    }
    return plist;
}

// return edge and the generator corresponding to its face
boost::python::list VoronoiDiagram::getEdgesGenerators()  {
    boost::python::list edge_list;
    BOOST_FOREACH( HEEdge edge, hed.edges() ) {
            boost::python::list point_list; // the endpoints of each edge
            HEVertex v1 = hed.source( edge );
            HEVertex v2 = hed.target( edge );
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

boost::python::list VoronoiDiagram::getDelaunayEdges()  {
    boost::python::list edge_list;
    BOOST_FOREACH( HEEdge edge, dt->edges() ) {
            boost::python::list point_list; // the endpoints of each edge
            HEVertex v1 = dt->source( edge );
            HEVertex v2 = dt->target( edge );
            Point src = (*dt)[v1].position;
            Point tar = (*dt)[v2].position;
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
    BOOST_FOREACH( HEEdge edge, hed.edges() ) { // loop through each edge
            boost::python::list point_list; // the endpoints of each edge
            HEVertex v1 = hed.source( edge );
            HEVertex v2 = hed.target( edge );
            point_list.append( hed[v1].position );
            point_list.append( hed[v2].position );
            edge_list.append(point_list);
    }
    return edge_list;
}

std::string VoronoiDiagram::str() const {
    std::ostringstream o;
    o << "VoronoiDiagram (nVerts="<< hed.num_vertices() << " , nEdges="<< hed.num_edges() <<"\n";
    return o.str();
}

} // end namespace
// end file voronoidiagram.cpp
