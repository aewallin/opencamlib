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
    // Euler formula for planar graphs
    // v - e + f = 2
    // in a half-edge diagram all edges occur twice, so:
    // f = 2-v+e
    int vertex_count = hed.num_vertices();
    int face_count = (vertex_count- 4)/2 + 3; // degree three graph
    //int face_count = hed.num_faces();
    if (face_count != gen_count) {
        std::cout << " face_count_equals_generator_count() ERROR:\n";
        std::cout << " num_vertices = " << vertex_count << "\n";
        std::cout << " gen_count = " << gen_count << "\n";
        std::cout << " face_count = " << face_count << "\n";
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
// this is roughly "algorithm A" from the paper, page 15/50
void VoronoiDiagram::addVertexSite(Point p) {
    assert( p.xyNorm() < far_radius );
    
    //std::cout << "VD: addVertexSite()\n";
    gen_count++;
    HEFace closest_face = fgrid->grid_find_closest_face( p );

    HEVertex v_seed = find_seed_vertex(closest_face, p);
    hed[v_seed].type = IN;
    VertexVector v0;
    v0.push_back(v_seed); 
    
// THREE choices for algorithm here:
    //augment_vertex_set_B(v0, p); // robust(?) traverses each face
    augment_vertex_set_M(v0, p); // build tree by breadth-first search
    //augment_vertex_set_RB(v0, p); // naive, assumes exact H calculation
    
    // add new vertices on all edges that connect v0 IN edges to OUT edges
    add_new_voronoi_vertices(v0, p);
    // generate new edges that form a loop around the region to be deleted
    HEFace newface = split_faces(p);
    // fix the next-pointers in newface, then remove set v0
    remove_vertex_set(v0, newface);
    // reset IN/OUT/UNDECIDED for verts, and INCIDENT/NONINCIDENT for faces
    reset_labels();
    //std::cout << "VD: addVertexSite() done.\n";
    assert( hed.checkFaces() );
    assert( isValid() );
}

HEFace VoronoiDiagram::split_faces(Point& p) {
    HEFace newface =  hed.add_face( FaceProps( HEEdge(), p, NONINCIDENT ) );
    fgrid->add_face( hed[newface] );
    BOOST_FOREACH( HEFace f, incident_faces ) {
        split_face(newface, f); // each INCIDENT face is split into two parts: newface and f
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
    hed[v01].type = OUT; // the outer vertices are special:
    hed[v02].type = OUT;
    hed[v03].type = OUT;
    BOOST_FOREACH(HEFace f, incident_faces ) { hed[f].type = NONINCIDENT; }
    incident_faces.clear();
}

// remove vertices in the set v0
void VoronoiDiagram::remove_vertex_set(VertexVector& v0 , HEFace newface) {
    HEEdge current_edge = hed[newface].edge; 
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
                        std::cout << " VD remove_vertex_set() ERROR.\n";
                        std::cout << "current.face = " << hed[current_edge].face << " IS NOT next_face = " << hed[ hed[current_edge].next ].face << std::endl;
                        HEVertex c_trg = hed.target( current_edge );
                        HEVertex c_src = hed.source( current_edge );
                        HEVertex n_trg = hed.target( hed[current_edge].next );
                        HEVertex n_src = hed.source( hed[current_edge].next );
                        
                        std::cout << "current_edge = " << hed[c_src].index << " - " << hed[c_trg].index << "\n";
                        std::cout << "next_edge = " << hed[n_src].index << " - " << hed[n_trg].index << "\n";
                        
                        printFaceVertexTypes(hed[current_edge].face);
                        printFaceVertexTypes(hed[ hed[current_edge].next ].face);
                        
                        std::cout << " printing all incident faces for debug: \n";
                        BOOST_FOREACH( HEFace f, incident_faces ) {
                            printFaceVertexTypes( f );
                        } 
                    }
                    assert( hed[current_edge].face ==  hed[ hed[current_edge].next ].face );
                }
            }
        }

        current_edge = hed[current_edge].next; // jump to the next edge
        if ( hed[current_edge].next == start_edge )
            done = true;
    }
    // it should now be safe to delete v0
    BOOST_FOREACH( HEVertex v, v0 ) { 
        assert( hed[v].type == IN );
        hed.delete_vertex(v); // this also removes edges connecting to v
    }
}

// split the face f into one part which is newface, and the other part is the old f
void VoronoiDiagram::split_face(HEFace newface, HEFace f) {
    HEVertex new_source; // this is found as OUT-NEW-IN
    HEVertex new_target; // this is found as IN-NEW-OUT
    // the new vertex on face f connects new_source -> new_target
    HEEdge current_edge = hed[f].edge;                             
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
    hed[f].edge = e_new; 
    
    // the twin edge that bounds the new face
    HEEdge e_twin = hed.add_edge( new_target, new_source );
    hed[twin_previous].next = e_twin;
    hed[e_twin].next = twin_next;
    hed[e_twin].face = newface;
    hed[newface].edge = e_twin; 
    
    hed[e_twin].twin = e_new;
    hed[e_new].twin = e_twin;
    
    //assert( isDegreeThree() );
}

EdgeVector VoronoiDiagram::find_edges(VertexVector& inVertices, VoronoiVertexType vtype) {
    assert( !inVertices.empty() );
    EdgeVector output; // new vertices generated on these edges
    BOOST_FOREACH( HEVertex v, inVertices ) {                                   
        assert( hed[v].type == IN ); // all verts in v0 are IN
        BOOST_FOREACH( HEEdge edge, hed.out_edges( v ) ) {
            HEVertex adj_vertex = hed.target( edge );
            if ( hed[adj_vertex].type == vtype ) 
                output.push_back(edge); // this is an IN-vtype edge
        }
    }
    return output;
}

// the set v0 are IN vertices that should be removed
// generate new voronoi-vertices on all edges connecting v0 to OUT-vertices
void VoronoiDiagram::add_new_voronoi_vertices(VertexVector& v0, Point& p) {
    assert( !v0.empty() );
    EdgeVector q_edges = find_edges(v0, OUT); // new vertices generated on these IN-OUT edges
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
        
        // check new vertex position, should lie between endpoints of q_edges[m]
        HEVertex trg = hed.target(q_edges[m]);
        HEVertex src = hed.source(q_edges[m]);
        Point trgP = hed[trg].position;
        Point srcP = hed[src].position;
        Point newP = hed[q].position;
        if (( trgP - srcP ).xyNorm() <= 0 ) {
            /*
            std::cout << "add_new_voronoi_vertices() WARNING ( trgP - srcP ).xyNorm()= " << ( trgP - srcP ).xyNorm() << "\n";
            std::cout << " src = " << srcP << "\n";
            std::cout << " trg= " << trgP << "\n";
            */
            //std::cout << "add_new_voronoi_vertices() WARNING zero-length edge! \n";
            hed[q].position = srcP;
        } else {
            assert( ( trgP - srcP ).xyNorm() > 0.0 ); // edge has finite length
            assert( ( trgP - srcP ).dot( trgP - srcP ) > 0.0 ); // length squared
            double t = ((newP - srcP).dot( trgP - srcP )) / ( trgP - srcP ).dot( trgP - srcP ) ;
            bool warn = false;
            //double t_orig=t;
            if (t < 0.0) {
                warn = true;
                t=0.0;
            } else if (t> 1.0) {
                warn = true;
                t=1.0;
            }
            if ( warn ) {
                //std::cout << "add_new_voronoi_vertices() WARNING positioning vertex outside edge! t_orig= " << t_orig << "\n";
                // CORRECT the position....
                hed[q].position = srcP + t*( trgP-srcP);
                t = ( hed[q].position - srcP).dot( trgP - srcP ) / ( trgP - srcP ).dot( trgP - srcP ) ;
                //std::cout << "add_new_voronoi_vertices() CORRECTED t= " << t << "\n";
            }
            assert( t >= 0.0 );
            assert( t <= 1.0 );
            
            double dtl = hed[q].position.xyDistanceToLine(srcP, trgP);
            if (dtl > 1e-3* ( trgP - srcP ).xyNorm() ) {
                //std::cout << "add_new_voronoi_vertices() WARNING new point far from edge!\n";
                //std::cout << "add_new_voronoi_vertices() WARNING edge length= " << ( trgP - srcP ).xyNorm()  << "\n";
                //std::cout << "add_new_voronoi_vertices() WARNING distance to edge= " << dtl  << "\n";
                t = ( hed[q].position - srcP).dot( trgP - srcP ) / ( trgP - srcP ).dot( trgP - srcP ) ;
                hed[q].position = srcP + t*( trgP-srcP);
                //newP = hed[q].position;
                dtl = hed[q].position.xyDistanceToLine(srcP, trgP);
                //std::cout << "add_new_voronoi_vertices() WARNING corrected distance to edge= " << dtl  << "\n";
            }
            assert( dtl < 1e-3* ( trgP - srcP ).xyNorm() );
        }

        hed.insert_vertex_in_edge( q, q_edges[m] );
        // sanity check on new vertex
        assert( hed[q].position.xyNorm() < 6.1*far_radius); // see init() for placement of the three initial vertices
    }
}


// return true if w is adjacent to an IN-vertex not in f.
bool VoronoiDiagram::adjacentInVertexNotInFace( HEVertex w, HEFace f ) {
    //assert( hed[w].type == UNDECIDED );
    VertexVector adj_verts = hed.adjacent_vertices(w);
    VertexVector face_verts = hed.face_vertices(f);
    BOOST_FOREACH( HEVertex v, adj_verts ) {
        if ( hed[v].type == IN ) { // v is an adjacent IN vertex
            assert( w != v );
            // check if v belongs to f
            bool in_face = false;
            BOOST_FOREACH( HEVertex face_vert, face_verts ) {
                if ( face_vert == v ) { // v is in f
                    in_face = true;
                }
            }
            if (!in_face)
                return true; // IN vertex not in f
        }
    }
    return false;
}

// return true if w is adjacent to an IN vertex in f
bool VoronoiDiagram::adjacentInVertexInFace( HEVertex w, HEFace f ) {
    //assert( hed[w].type == UNDECIDED );
    
    VertexVector adj_verts = hed.adjacent_vertices(w);
    VertexVector face_verts = hed.face_vertices(f);
    BOOST_FOREACH( HEVertex v, adj_verts ) {
        if ( hed[v].type == IN  ) { // adjacent IN vertex
            BOOST_FOREACH( HEVertex face_vert, face_verts ) {
                if ( face_vert == v ) {
                    return true; // IN vertex in f
                }
            }
        }
        /*
        if (hed[v].type == UNDECIDED){
            BOOST_FOREACH( HEVertex face_vert, face_verts ) {
                if ( face_vert == v ) {
                    return true; // IN vertex in f
                }
            }
        }*/
    }
    return false;
}

// return true if v is on an INCIDENT face which is not f
bool VoronoiDiagram::onOtherIncidentFace( HEVertex v , HEFace f) {
    //assert( hed[v].type == UNDECIDED );
    assert( hed[f].type == INCIDENT );
    FaceVector adjacent_faces = hed.adjacent_faces( v );
    //assert( adjacent_faces.size()==3 );
    BOOST_FOREACH( HEFace fa, adjacent_faces ) {
        if ( hed[fa].type == INCIDENT ) {
            if ( fa != f ) {
                return true;
            }
        }
    }
    return false;
} 

int VoronoiDiagram::outVertexCount(HEFace f) {
    int outCount = 0;
    VertexVector face_verts = hed.face_vertices(f);
    BOOST_FOREACH( HEVertex v, face_verts ) {
        if (hed[v].type == OUT )
            ++outCount;
    }
    return outCount;
}

// this is "Algorithm B" from Sugihara&Iri 1994, page 20-21
// http://dx.doi.org/10.1142/S0218195994000124
void VoronoiDiagram::augment_vertex_set_B(VertexVector& q, Point& p) {
    //std::cout << "augment_vertex_set_B()\n";
    assert( q.size()==1 ); // RB2   voronoi-point q[0] = q( a, b, c ) is the seed
    assert( hed[ q[0] ].type == IN ); 
    assert( allIn(q) );
    
    std::stack<HEFace> S;
    markAdjecentFacesIncident(S, q[0]);
    assert( allIn(q) );
    in_vertices.push_back( q[0] );
    while ( !S.empty() ) { // B2 process stack.
        HEFace f = S.top();
        std::cout << " Augment from ";
        printFaceVertexTypes(f);
        S.pop();
        VertexVector face_verts = hed.face_vertices(f);
        
        // (T4) tree
        // (T5) for each face, in-vertices connected
        BOOST_FOREACH( HEVertex v, face_verts ) { // go through all vertices in face and run B2.1 
            // B2.1  mark "out"  v in cycle_alfa if 
            if ( hed[v].type == UNDECIDED ) {
                if ( adjacentInVertexNotInFace( v, f ) ) { 
                    //  (T6) v is adjacent to an IN vertex in V-Vin(alfa)
                    std::cout << hed[v].index << " B2.1 T6 OUT \n";
                    hed[v].type = OUT;
                    in_vertices.push_back( v );
                } else if ( onOtherIncidentFace( v, f ) ) {
                    //  (T7) v is on an "incident" cycle other than this cycle and is not adjacent to a vertex in Vin(alfa)
                    if ( !adjacentInVertexInFace( v, f ) ) {
                        std::cout << hed[v].index << " B2.1 T7 OUT ";
                            FaceVector adjacent_faces = hed.adjacent_faces( v );
                            assert( adjacent_faces.size()==3 );
                            BOOST_FOREACH( HEFace fa, adjacent_faces ) {
                                if ( hed[fa].type == INCIDENT ) {
                                    if ( fa != f ) {
                                        std::cout << " INC-face: " << fa << "\n";
                                    }
                                }
                            }
    
                        hed[v].type = OUT;
                        in_vertices.push_back( v );
                    }
                }
            }
        }
        
        // B2.2 if OUT-graph is disconnected, find minimal set that makes it connected and mark OUT
        if ( outVertexCount(f) > 0 ) {
            while( !faceVerticesConnected(  f, OUT ) ) {
                //std::cout << "OUT-vertices: UNCONNECTED OUT Face: ";
                //printFaceVertexTypes(f);
                VertexVector repair_verts = findRepairVerts(f, OUT);
                BOOST_FOREACH( HEVertex r, repair_verts ) {
                    assert( hed[r].type == UNDECIDED );
                    hed[r].type = OUT;
                    std::cout << hed[r].index << " B2.2 OUT-repair \n";
                    in_vertices.push_back( r );
                }
            }
            assert( faceVerticesConnected(  f, OUT ) );
        }
        
        // B2.3 if no OUT-vertices exist, find UNDECIDED vertex with maximum H and mark OUT
        // at least one OUT-vertex must exist, since we do not want to delete faces.
        if ( outVertexCount(f) == 0  ) {
            //std::cout << " B2.3 no OUT-vertices case.\n";
            double maxH;
            bool first = true;
            HEVertex maximal_v;
            BOOST_FOREACH( HEVertex v, face_verts ) {
                if ( hed[v].type == UNDECIDED ) {
                    double h = hed[v].detH( p );
                    
                    if (first) {
                        maxH = h;
                        maximal_v = v;
                        first = false;
                    }
                    if ( h > maxH) {
                        //std::cout << "det= " << h << " is max so far!\n";
                        maxH = h;
                        maximal_v = v;
                    }
                }
            }
            //std::cout << " max H vertex is " << hed[maximal_v].position << "with detH= " << maxH << std::endl;
            hed[maximal_v].type = OUT; // mark OUT
            std::cout << hed[maximal_v].index << " B2.3 maxH OUT \n";
            in_vertices.push_back( maximal_v );
        }
        assert( !q.empty() );
        // now we have at least one OUT-vertex, and if we have many they are connected.
        assert( allIn(q) );
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
                //std::cout << uverts.size() << " UNDECIDED remain. DONE\n";
                done = true;
            } else {
                //std::cout << uverts.size() << " UNDECIDED remain, look for max abs(det) \n";
                // find uvert with highest abs(H)
                double max_abs_h;
                double max_h;
                double det_v;
                HEVertex max_v;
                bool first = true;
                BOOST_FOREACH( HEVertex v, uverts ) {
                    det_v = hed[v].detH(p);
                    
                    if ( first ) {
                        max_abs_h = abs(det_v);
                        max_h = det_v;
                        max_v = v;
                        first = false;
                    }
                    //std::cout << hed[v].index << ": det= " << det_v << " abs(det_v)= " << abs(det_v) << " max_abs= " << max_abs_h << "\n";
                    if ( abs(det_v) >= max_abs_h) {
                        max_abs_h = abs(det_v);
                        max_h = det_v;
                        max_v=v;
                    }
                }

                //std::cout << " max abs(H) vertex is " << hed[max_v].index << " with max_abs= " << max_h << std::endl;
                // now max_v is the one with highest abs(H)
                assert( allIn(q) );
                if ( max_h >= 0.0 ) {
                    hed[max_v].type = OUT;
                    std::cout << hed[max_v].index << " B2.4.1 H>0 OUT \n";
                    in_vertices.push_back( max_v );
                    //std::cout << "OUT vertex found.\n";
                    // check if OUT-graph is disconnected. repair with UNDECIDED vertices
                    
                    while( !faceVerticesConnected( f, OUT ) ) {
                        //std::cout << " h>=0.0 decision, Repair UNCONNECTED OUT Face: " << f << " : ";
                        //printFaceVertexTypes(f);
                        VertexVector repair_verts = findRepairVerts(f, OUT);
                        BOOST_FOREACH( HEVertex r, repair_verts ) {
                            assert( hed[r].type == UNDECIDED );
                            hed[r].type = OUT;
                            std::cout << hed[r].index << " B2.4.1 OUT-repair \n";
                            in_vertices.push_back( r );
                        }
                        //std::cout << "OUTRepair done: ";
                        //printFaceVertexTypes(f);
                        
                    }
                    
                    assert( faceVerticesConnected( f, OUT ) ); // should be repaired now
                    assert( allIn(q) );
                } else {
                    assert( max_h < 0.0 );
                    hed[max_v].type = IN;
                    std::cout << hed[max_v].index << " B2.4.2 H<0 IN \n";
                    in_vertices.push_back( max_v );
                    q.push_back(max_v);
                    markAdjecentFacesIncident(S, max_v);

                    // check if IN-graph is disconnected. repair with UNDECIDED vertices
                    while( !faceVerticesConnected(  f, IN ) ) {
                        //std::cout << "h<0.0 decision, Repair UNCONNECTED IN Face: " << f << " : ";
                        //printFaceVertexTypes(f);
                        VertexVector repair_verts = findRepairVerts(f, IN);
                        BOOST_FOREACH( HEVertex r, repair_verts ) {
                            assert( hed[r].type == UNDECIDED );
                            hed[r].type = IN;
                            std::cout << hed[r].index << " B2.4.2 IN-repair \n";
                            in_vertices.push_back( r );
                            q.push_back( r );
                            markAdjecentFacesIncident(S, r);
                        }
                    }
                    assert( faceVerticesConnected( f, IN ) ); // should be repaired now
                    assert( allIn(q) );
                }
            } // end UNDECIDED processing
        }
        
        std::cout << " Augment done:";
        //std::cout << " augment_RB Face DONE: ";
        printFaceVertexTypes(f);
        
        // we should now be done with face f
        assert( faceVerticesConnected(  f, IN ) );   // IN vertices should be connected
        assert( faceVerticesConnected(  f, OUT ) );  // OUT vertices should be connected
        assert( noUndecidedInFace( f ) );            // no UNDECIDED vertices should remain
    } // end stack while-loop
    std::cout << "augment_vertex_set_B() DONE. q.size()= " << q.size() << "\n";
    
    // done with all faces
    assert( allIncidentFacesOK() );
    assert( allIn(q) );
    //printVertices(q);
}

bool VoronoiDiagram::allIncidentFacesOK() {
    // all incident faces should pass the sanity-check
    BOOST_FOREACH( HEFace f, incident_faces ) {
        if ( !faceVerticesConnected(  f, IN ) )
            return false; // IN vertices should be connected
        if ( !faceVerticesConnected(  f, OUT ) )  // OUT vertices should be connected
            return false;
        if ( !noUndecidedInFace( f ) )            // no UNDECIDED vertices should remain
            return false;
    }
    return true;
}

bool VoronoiDiagram::allIn(VertexVector& q) {
    BOOST_FOREACH( HEVertex v, q) {
        if ( hed[v].type != IN )
            return false;
    }
    return true;
}

VertexVector VoronoiDiagram::findRepairVerts(HEFace f, VoronoiVertexType Vtype) {
    assert( !faceVerticesConnected(  f, Vtype ) ); // Vtype is not connected.
    // example 2-0-1-2-0
    
    // repair pattern: Vtype - U - Vtype
    // find potential U-sets
    HEEdge currentEdge = hed[f].edge;
    HEVertex endVertex = hed.source(currentEdge); // stop when target here
    EdgeVector startEdges;
    bool done = false;
    while (!done) { 
        HEVertex src = hed.source( currentEdge );
        HEVertex trg = hed.target( currentEdge );
        if ( hed[src].type == Vtype ) { // seach Vtype - U
            if ( hed[trg].type == UNDECIDED ) {
                // we have found Vtype-U
                startEdges.push_back( currentEdge );
            }
        }
        currentEdge = hed[currentEdge].next;
        if ( trg == endVertex ) {
            done = true;
        }
    }
    //std::cout << "findRepairVerts(): startEdges.size() = " << startEdges.size() << "\n";
    
    if (startEdges.empty()) {
        std::cout << "findRepairVerts(): ERROR repairing face= " << f << " type= "<< Vtype << "\n";
        std::cout << "findRepairVerts(): ERROR can't find Vtype-U start edge!\n";
        printFaceVertexTypes(f);
        assert(0); // repair is not possible if we don't have a startEdge
    }
    std::vector<VertexVector> repair_sets;
    
    BOOST_FOREACH( HEEdge uedge, startEdges ) {
        HEEdge ucurrent = uedge;
        bool set_done = false;
        VertexVector uset;
        while (!set_done) {
            HEVertex trg = hed.target( ucurrent );
            if ( hed[trg].type == UNDECIDED ) {
                uset.push_back(trg);
                ucurrent = hed[ucurrent].next;
            } else if ( hed[trg].type == Vtype ) {
                repair_sets.push_back(uset); // done with this set
                set_done = true;
            } else { // doesn't end in Vtype, so not a valid set
                set_done = true;
            }
        }
    }
    
    // among the repair sets, find the minimal one
    if (repair_sets.empty()) {
        std::cout << "findRepairVerts(): Vtype-U found, but cannot repair:\n";
        printFaceVertexTypes(f);
        assert(0);
    }
    
    std::size_t min_idx = 0;
    std::size_t min_size = repair_sets[min_idx].size();
    for( std::size_t idx = 0; idx<repair_sets.size(); ++idx ) {
        if ( repair_sets[idx].size() < min_size ) {
            min_idx = idx;
            min_size = repair_sets[idx].size();
        }
    }
    //std::cout << "findRepairVerts(): repair_sets[min_idx].size() = " << repair_sets[min_idx].size() << "\n";
    return repair_sets[min_idx];
}

bool VoronoiDiagram::noUndecidedInFace(HEFace f) {
    VertexVector face_verts = hed.face_vertices(f);
    BOOST_FOREACH( HEVertex v, face_verts ) {
        if ( hed[v].type == UNDECIDED )
            return false;
    }
    return true;
}

// check that the vertices TYPE are connected
bool VoronoiDiagram::faceVerticesConnected( HEFace f, VoronoiVertexType Vtype ) {
    VertexVector face_verts = hed.face_vertices(f);
    VertexVector type_verts;
    BOOST_FOREACH( HEVertex v, face_verts ) {
        if ( hed[v].type == Vtype )
            type_verts.push_back(v);
    }
    assert( !type_verts.empty() );
    if (type_verts.size()==1) // set of 1 is allways connected
        return true;
    
    // check that type_verts are connected
    HEEdge currentEdge = hed[f].edge;
    HEVertex endVertex = hed.source(currentEdge); // stop when target here
    EdgeVector startEdges;
    bool done = false;
    while (!done) { 
        HEVertex src = hed.source( currentEdge );
        HEVertex trg = hed.target( currentEdge );
        if ( hed[src].type != Vtype ) { // seach ?? - Vtype
            if ( hed[trg].type == Vtype ) {
                // we have found ?? - Vtype
                startEdges.push_back( currentEdge );
            }
        }
        currentEdge = hed[currentEdge].next;
        if ( trg == endVertex ) {
            done = true;
        }
    }
    assert( !startEdges.empty() );
    if ( startEdges.size() != 1 ) // when the Vtype vertices are connected, there is exactly one startEdge
        return false;
    else 
        return true;
}

void VoronoiDiagram::printFaceVertexTypes(HEFace f) {
    std::cout << " Face " << f << ": ";
    VertexVector face_verts = hed.face_vertices(f);    
    unsigned count=1;
    BOOST_FOREACH( HEVertex v, face_verts ) {
        std::cout << hed[v].index  << "(" << hed[v].type  << ")";
        if (count != face_verts.size() )
            std::cout << "-";
        count++;
    }
    std::cout << "\n";
}

void VoronoiDiagram::printVertices(VertexVector& q) {
    BOOST_FOREACH( HEVertex v, q) {
        std::cout << hed[v].index << " ";
    }
    std::cout << std::endl;
}

void VoronoiDiagram::pushAdjacentVertices(  HEVertex v , std::queue<HEVertex>& Q) {
    VertexVector adj_verts = hed.adjacent_vertices(v);
    BOOST_FOREACH( HEVertex w, adj_verts ) {
        if ( hed[w].type == UNDECIDED ) {
            if (not_in_queue(w,Q)) 
                Q.push(w); // push adjacent undecided verts for testing.
        }
    }
}

// from the "one million" paper, growing the tree by breadth-first search
void VoronoiDiagram::augment_vertex_set_M(VertexVector& v0, Point& p) {
    assert(v0.size()==1);
    std::queue<HEVertex> Q;
    // a priority_queue could be used here instead.
    // allways examine and decide on the vertex with largest detH, since that vertex has the most reliable detH sign.
    
    in_vertices.push_back( v0[0] );
    markAdjecentFacesIncident( v0[0] );
    assert( Q.empty() );
    pushAdjacentVertices( v0[0] , Q);
    
    while( !Q.empty() ) {
        HEVertex v = Q.front();
        assert( hed[v].type == UNDECIDED );
        
        // add to v0 if detH<0 and passes tests. otherwise OUT
        double h = hed[v].detH( p );
        if ( h < 0.0 ) {
            // try to mark IN
            // (C4) v should not be adjacent to two or more IN vertices
            VertexVector adj_v = hed.adjacent_vertices(v);
            int in_count=0;
            BOOST_FOREACH( HEVertex w, adj_v) {
                if ( hed[w].type == IN )
                    in_count++;
            }
            
            if ( in_count >= 2 ) {
                hed[v].type = OUT;
                //std::cout << " v " << hed[v].index << " decision is " << hed[v].type << " IN_COUNT>=2 \n";
                in_vertices.push_back( v );
            } else {
                // (C5) for an incident face containing v:
                //      v is adjacent to an IN vertex on this face
                FaceVector adj_faces = hed.adjacent_faces(v);
                assert( adj_faces.size() == 3 );
                FaceVector inc_faces;
                BOOST_FOREACH( HEFace f, adj_faces ) {
                    if ( hed[f].type == INCIDENT )
                        inc_faces.push_back( f );
                }
                assert( !inc_faces.empty() );
                bool all_found = true;
                BOOST_FOREACH( HEFace f, inc_faces ) {
                    // check each face f
                    // v should be adjacent to an IN vertex on the face
                    VertexVector face_verts = hed.face_vertices(f);
                    bool face_found=false;
                    BOOST_FOREACH( HEVertex w, face_verts ) {
                        if ( w != v ) {
                            if ( hed[w].type == IN ) {
                                assert( hed.edge(w,v) == hed.edge(v,w) );
                                if ( hed.edge(w,v) || hed.edge(v,w) ) {
                                    face_found = true;
                                }
                            }
                        }
                    }
                    if (!face_found) {
                        all_found=false;
                    }
                }
                if (all_found) {
                    hed[v].type = IN;
                    //std::cout << " v " << hed[v].index << " decision is " << hed[v].type << " (h<0)\n";
                    in_vertices.push_back( v );
                    v0.push_back(v);
                    markAdjecentFacesIncident(v);
                    pushAdjacentVertices( v , Q);
                } else {
                    assert( hed[v].type == UNDECIDED );
                    hed[v].type = OUT;
                    //std::cout << " v " << hed[v].index << " decision is " << hed[v].type << " NOT_FOUND (h<0)\n";
                    in_vertices.push_back( v );
                }
            }
        } else {
            assert( h >= 0.0 );
            assert( hed[v].type == UNDECIDED );
            // mark OUT
            hed[v].type = OUT;
            //std::cout << " v " << hed[v].index << " decision is " << hed[v].type << " (h>0)\n";
            in_vertices.push_back( v );
        }
        Q.pop(); // delete from queue
    }
    
    BOOST_FOREACH( HEFace f, incident_faces ) {
        if ( !faceVerticesConnected( f, IN ) ) {
            std::cout << " augment_vertex_set_M() ERROR, IN-vertices not connected.\n";
            std::cout << " printing all incident faces for debug: \n";
            BOOST_FOREACH( HEFace f, incident_faces ) {
                printFaceVertexTypes( f );
            } 
        }
    }
    
    BOOST_FOREACH( HEFace f, incident_faces ) {
        assert( faceVerticesConnected( f, IN ) );
    }
    //std::cout << " augment_M done:\n";
    //printVertices(v0);
}

bool VoronoiDiagram::not_in_queue(HEVertex w, std::queue<HEVertex> Q) {
    while( !Q.empty() ) {
        HEVertex v = Q.front();
        Q.pop();
        if ( w == v )
            return false;
    }
    return true;
}

// simple algorithm for finding vertex set
// relies on the correct computation of detH (this will fail due to floating point error - sooner or later)
// roughly "Algorithm RB" from Sugihara& Iri 1994, page 18
void VoronoiDiagram::augment_vertex_set_RB(VertexVector& q, Point& p) {
    assert(q.size()==1);
    std::stack<HEFace> S;
    in_vertices.push_back( q[0] );
    markAdjecentFacesIncident(S, q[0]); // B1.3  we push all the adjacent faces onto the stack, and label them INCIDENT
    while ( !S.empty() ) { // examine all incident faces until done.
        HEFace f = S.top();
        S.pop();
        HEEdge current_edge = hed[f].edge; 
        HEEdge start_edge = current_edge;
        bool done=false;
        while (!done) {
            assert( hed[ hed[current_edge].face ].type == INCIDENT );
            assert( hed[current_edge].face == f );
            // add v to V0 subject to:
            // (T4) V0 graph is a tree
            // (T5) the IN-vertices of face f are connected
            HEVertex v = hed.target( current_edge );
            if ( hed[v].type == UNDECIDED ) {
                in_vertices.push_back( v ); // this vertex needs to be reset by reset_labels()
                if ( hed[v].detH( p ) < 0.0 ) { // assume correct sign of H here
                    hed[v].type = IN;
                    q.push_back(v);
                    markAdjecentFacesIncident(S, v);
                } else {
                    hed[v].type = OUT;
                }
            }
            current_edge = hed[current_edge].next; // jump to the next edge on this face
            
            if ( current_edge == start_edge ) // when we are back where we started, stop.
                done = true;
        }
        // we should be done with face f
        if (!faceVerticesConnected( f, IN )) {
            std::cout << " augment_RB ERROR IN-verts not connected:\n";
            printFaceVertexTypes(f);
        }
        assert( faceVerticesConnected( f, IN ) );
        assert( faceVerticesConnected( f, OUT ) );
        assert( noUndecidedInFace( f ) ); 
    }
    
    assert( allIn(q) );
    //printVertices(q);
    
}

// used by augment_M
void VoronoiDiagram::markAdjecentFacesIncident( HEVertex v) {
    assert( hed[v].type == IN );
    FaceVector new_adjacent_faces = hed.adjacent_faces( v ); 
    assert( new_adjacent_faces.size()==3 );
    BOOST_FOREACH( HEFace adj_face, new_adjacent_faces ) {
        if ( hed[adj_face].type  != INCIDENT ) {
            hed[adj_face].type = INCIDENT; 
            incident_faces.push_back(adj_face);
        }
    }
}

void VoronoiDiagram::markAdjecentFacesIncident(std::stack<HEFace>& S, HEVertex v) {
    FaceVector new_adjacent_faces = hed.adjacent_faces( v ); // also set the adjacent faces to incident
    assert( new_adjacent_faces.size()==3 );
    BOOST_FOREACH( HEFace adj_face, new_adjacent_faces ) {
        if ( hed[adj_face].type  != INCIDENT ) {
            hed[adj_face].type = INCIDENT; 
            incident_faces.push_back(adj_face);
            S.push(adj_face);
        }
    }
}

// evaluate H on all face vertices and return
// vertex with the lowest H
HEVertex VoronoiDiagram::find_seed_vertex(HEFace f, const Point& p) {
    VertexVector face_verts = hed.face_vertices(f);                 
    assert( face_verts.size() >= 3 ); 
    double minimumH; // safe, because we expect the min H to be negative...
    HEVertex minimalVertex;
    double h;
    bool first = true;
    BOOST_FOREACH( HEVertex q, face_verts) {
        if ( hed[q].type != OUT ) {
            h = hed[q].detH( p ); 
            //std::cout << "  detH = " << h << " ! \n";
            if ( first ) {
                minimumH = h;
                minimalVertex = q;
                first = false;
            }
            if (h<minimumH) { // find minimum H value among q_verts
                minimumH = h;
                minimalVertex = q;
            }
        }
    }
    
    if (!(minimumH < 0) ) {
        std::cout << " VD find_seed_vertex() WARNING\n";
        std::cout << " WARNING: searching for seed when inserting " << p  << "  \n";
        std::cout << " WARNING: closest face is  " << f << " with generator " << hed[f].generator  << " \n";
        std::cout << " WARNING: minimal vd-vertex " << hed[minimalVertex].index << " has deth= " << hed[minimalVertex].detH( p ) << "\n";
    }
    //assert( minimumH < 0 );
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

std::string VoronoiDiagram::str() const {
    std::ostringstream o;
    o << "VoronoiDiagram (nVerts="<< hed.num_vertices() << " , nEdges="<< hed.num_edges() <<"\n";
    return o.str();
}

} // end namespace
// end file voronoidiagram.cpp
