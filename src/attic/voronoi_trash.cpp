


// set the adjacent faces to incident, and push onto stack
void VoronoiDiagram::markAdjecentFacesIncident(std::stack<HEFace>& S, HEVertex v) {
    FaceVector new_adjacent_faces = hedi::adjacent_faces( v, g ); 
    assert( new_adjacent_faces.size()==3 );
    BOOST_FOREACH( HEFace adj_face, new_adjacent_faces ) {
        if ( g[adj_face].type  != INCIDENT ) {
            g[adj_face].type = INCIDENT; 
            incident_faces.push_back(adj_face);
            S.push(adj_face);
        }
    }
}



// return true if v is on an INCIDENT face which is not f
bool VoronoiDiagram::onOtherIncidentFace( HEVertex v , HEFace f) {
    //assert( hed[v].type == UNDECIDED );
    assert( g[f].type == INCIDENT );
    FaceVector adjacent_faces = hedi::adjacent_faces( v, g );
    //assert( adjacent_faces.size()==3 );
    BOOST_FOREACH( HEFace fa, adjacent_faces ) {
        if ( g[fa].type == INCIDENT ) {
            if ( fa != f ) {
                return true;
            }
        }
    }
    return false;
} 


// return true if w is adjacent to an IN-vertex not in f.
bool VoronoiDiagram::adjacentInVertexNotInFace( HEVertex w, HEFace f ) {
    //assert( hed[w].type == UNDECIDED );
    VertexVector adj_verts = hedi::adjacent_vertices(w, g);
    VertexVector face_verts = hedi::face_vertices(f, g);
    BOOST_FOREACH( HEVertex v, adj_verts ) {
        if ( g[v].type == IN ) { // v is an adjacent IN vertex
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



// NOTE: unused helper-function
// return true if w is adjacent to an IN vertex in f
bool VoronoiDiagram::adjacentInVertexInFace( HEVertex w, HEFace f ) {
    //assert( hed[w].type == UNDECIDED );
    
    VertexVector adj_verts = hedi::adjacent_vertices(w, g);
    VertexVector face_verts = hedi::face_vertices(f, g);
    BOOST_FOREACH( HEVertex v, adj_verts ) {
        if ( g[v].type == IN  ) { // adjacent IN vertex
            BOOST_FOREACH( HEVertex face_vert, face_verts ) {
                if ( face_vert == v ) {
                    return true; // IN vertex in f
                }
            }
        }
        
        //if (hed[v].type == UNDECIDED){
        //    BOOST_FOREACH( HEVertex face_vert, face_verts ) {
        //        if ( face_vert == v ) {
        //            return true; // IN vertex in f
        //        }
        //    }
        //}
    }
    return false;
}


// NOTE: the augment-algorithm that adds vertices to the delete-set by
// traversing each face in turns needs this repair-vertex function as a helper
// not used anymore.
VertexVector VoronoiDiagram::findRepairVerts(HEFace f, VoronoiVertexType Vtype) {
    assert( !faceVerticesConnected(  f, Vtype ) ); // Vtype is not connected.
    // example 2-0-1-2-0
    
    // repair pattern: Vtype - U - Vtype
    // find potential U-sets
    HEEdge currentEdge = g[f].edge;
    HEVertex endVertex = hedi::source(currentEdge, g); // stop when target here
    EdgeVector startEdges;
    bool done = false;
    while (!done) { 
        HEVertex src = hedi::source( currentEdge , g);
        HEVertex trg = hedi::target( currentEdge , g);
        if ( g[src].type == Vtype ) { // search Vtype - U
            if ( g[trg].type == UNDECIDED ) {
                // we have found Vtype-U
                startEdges.push_back( currentEdge );
            }
        }
        currentEdge = g[currentEdge].next;
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
            HEVertex trg = hedi::target( ucurrent , g);
            if ( g[trg].type == UNDECIDED ) {
                uset.push_back(trg);
                ucurrent = g[ucurrent].next;
            } else if ( g[trg].type == Vtype ) {
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
//----------------------------------------------------------------------




// NOTE: this is a naive implementation of the augment-algorithm
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
//----------------------------------------------------------------------




// NOTE: this is an experimental augment_vertex_set algorithm from:
// this is "Algorithm B" from Sugihara&Iri 1994, page 20-21
// http://dx.doi.org/10.1142/S0218195994000124
// it does not seem to work robustly so probably contains unknown bugs.
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

//----------------------------------------------------------------------


// NOTE: since the voronoi diagram construction starts with three initial 
// generator-points it is not correct to return the delaunay triangulation
// of the voronoi-diagram!!

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
//----------------------------------------------------------------------
