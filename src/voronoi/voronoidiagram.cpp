/* 
 *  Copyright 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com)
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

#include "voronoidiagram.hpp"
#include "numeric.hpp"

namespace ocl
{



VoronoiDiagram::VoronoiDiagram(double far, unsigned int n_bins) {
    fgrid = new FaceGrid(far, n_bins);
    far_radius=far;
    gen_count=3;
    init();
}

VoronoiDiagram::~VoronoiDiagram() { 
    delete fgrid; 
}

// add one vertex at origo and three vertices at 'infinity' and their associated edges
void VoronoiDiagram::init() {
    // add init vertices
    HEVertex v0  = hedi::add_vertex( VoronoiVertex() , g );
    double far_multiplier = 6;
    HEVertex v01 = hedi::add_vertex( VoronoiVertex(Point(             0                 , -3.0*far_radius*far_multiplier    )               , OUT, OUTER) , g );
    HEVertex v02 = hedi::add_vertex( VoronoiVertex(Point(  +3.0*sqrt(3.0)*far_radius*far_multiplier/2.0, +3.0*far_radius*far_multiplier/2.0), OUT, OUTER), g );
    HEVertex v03 = hedi::add_vertex( VoronoiVertex(Point(  -3.0*sqrt(3.0)*far_radius*far_multiplier/2.0, +3.0*far_radius*far_multiplier/2.0), OUT, OUTER), g );
    out_verts[0]=v01; out_verts[1]=v02; out_verts[2]=v03;
    // the locations of the initial generators:
    Point gen1 = Point( 0, 3.0*far_radius);
    Point gen2 = Point( -3.0*sqrt(3.0)*far_radius/2.0, -3.0*far_radius/2.0 );
    Point gen3 = Point( +3.0*sqrt(3.0)*far_radius/2.0, -3.0*far_radius/2.0 );
    g[v0].set_generators( gen1, gen2, gen3 ); 
    HEVertex g1 = hedi::add_vertex( VoronoiVertex( gen1 , OUT, VERTEXGEN), g);
    HEVertex g2 = hedi::add_vertex( VoronoiVertex( gen2 , OUT, VERTEXGEN), g);
    HEVertex g3 = hedi::add_vertex( VoronoiVertex( gen3 , OUT, VERTEXGEN), g);
    
    // add face 1: v0-v1-v2 which encloses gen3
    HEEdge e1 =  hedi::add_edge( v0 , v01 , g);   
    HEEdge e2 =  hedi::add_edge( v01, v02 , g);
    HEEdge e3 =  hedi::add_edge( v02, v0  , g); 
    HEFace f1 =  hedi::add_face( FaceProps(e2, gen3, NONINCIDENT), g ); 
    fgrid->add_face( g[f1] );
    g[e1].face = f1;
    g[e2].face = f1;
    g[e3].face = f1;
    g[e1].next = e2;
    g[e2].next = e3;
    g[e3].next = e1;
    g[g3].face = f1;
    
    // add face 2: v0-v2-v3 which encloses gen1
    HEEdge e4 = hedi::add_edge( v0, v02  , g );   
    HEEdge e5 = hedi::add_edge( v02, v03 , g );
    HEEdge e6 = hedi::add_edge( v03, v0  , g ); 
    HEFace f2 =  hedi::add_face( FaceProps(e5, gen1, NONINCIDENT), g );
    fgrid->add_face( g[f2] );
    g[e4].face = f2;
    g[e5].face = f2;
    g[e6].face = f2;
    g[e4].next = e5;
    g[e5].next = e6;
    g[e6].next = e4;
    g[g1].face = f2;
    
    // add face 3: v0-v3-v1 which encloses gen2
    HEEdge e7 = hedi::add_edge( v0 , v03 , g);   
    HEEdge e8 = hedi::add_edge( v03, v01 , g);
    HEEdge e9 = hedi::add_edge( v01, v0  , g); 
    HEFace f3 =  hedi::add_face( FaceProps(e8, gen2, NONINCIDENT), g );
    fgrid->add_face( g[f3] );
    g[e7].face = f3;
    g[e8].face = f3;
    g[e9].face = f3;
    g[e7].next = e8;
    g[e8].next = e9;
    g[e9].next = e7;
    g[g2].face = f3;
    
    // twin edges
    g[e1].twin = e9;
    g[e9].twin = e1;
    g[e2].twin = HEEdge(); // the outermost edges have invalid twins
    g[e5].twin = HEEdge();
    g[e8].twin = HEEdge();
    g[e3].twin = e4;
    g[e4].twin = e3;
    g[e6].twin = e7;
    g[e7].twin = e6;

    assert( vdChecker.isValid(this) );
}



void VoronoiDiagram::addLineSite(int idx1, int idx2) {
    // find the vertices corresponding to idx1 and idx2
    HEVertex start, end;
    bool start_found=false;
    bool end_found=false;
    BOOST_FOREACH( HEVertex v, hedi::vertices(g) ) {
        if ( g[v].index == idx1 ) {
            start = v;
            start_found = true;
        }
        if (g[v].index == idx2) {
            end = v;
            end_found = true;
        }
    }
    assert( start_found );
    assert( end_found );
    std::cout << " start = " << g[start].index << " at " << g[start].position << "\n";
    std::cout << " end   = " << g[end].index   << " at " << g[end].position << "\n";
    HEVertex seed1 = find_seed_vertex(start,end);
    HEVertex seed2 = find_seed_vertex(end,start);
    std::cout << " seed1   = " << g[seed1].index   << " at " << g[seed1].position << "\n";
    std::cout << " seed2   = " << g[seed2].index   << " at " << g[seed2].position << "\n";
    //std::pair<HEVertex, HEVertex>
    augment_vertex_set(seed1, start, end);
    std::cout << " v0.size() = " << v0.size() << "\n";
    
}

HEVertex VoronoiDiagram::find_seed_vertex(HEVertex start, HEVertex end) {
    // find the seed-vertex
    assert( g[start].type == VERTEXGEN );
    assert( g[end].type == VERTEXGEN );
    HEFace start_face = g[start].face;
    
    VertexVector face_verts = hedi::face_vertices(start_face,g);
    assert( face_verts.size() >= 3 );
    double minimumH; 
    HEVertex minimalVertex;
    bool first = true;
    BOOST_FOREACH( HEVertex q, face_verts) { // go thorugh all the vertices and find the one with smallest detH
        if ( g[q].status != OUT ) {
            Point apex_point = get_apex_point( start, end, q);
            // find the closest point to q on the start-end line segment
            double h = g[q].detH( apex_point ); // closest to start-end line segment 
            if ( first || (h<minimumH) ) {
                minimumH = h;
                minimalVertex = q;
                first = false;
            }
        }
    }
    //assert( vdChecker.detH_is_negative( this, p, f, minimalVertex ) );
    return minimalVertex;
    
    //return start;
}

void VoronoiDiagram::augment_vertex_set(HEVertex& v_seed, HEVertex start, HEVertex end) {
    std::queue<HEVertex> Q; // FIXME: a priority_queue could/should be used here instead.
    mark_vertex_in( v_seed, Q);
    modified_vertices.push_back( v_seed );
    while( !Q.empty() ) {
        HEVertex v = Q.front();      assert( g[v].status == UNDECIDED );
        // mark IN and add to v0 if detH<0 and passes tests. otherwise OUT
        Point apex_point = get_apex_point(start,end,v);
        double h = g[v].detH( apex_point ); // replace with INPredicate(PointGen) INPredicate(LineGen)
        if ( h < 0.0 ) { // try to mark IN
            // (C4) v should not be adjacent to two or more IN vertices (this would result in a loop/cycle!)
            // (C5) for an incident face containing v: v is adjacent to an IN vertex on this face
            if ( (adjacentInCount(v) >= 2) || (!incidentFacesHaveAdjacentInVertex(v)) ) 
                g[v].status = OUT;
            else
                mark_vertex_in( v, Q);
        } else { // detH was positive, so mark OUT
            g[v].status = OUT;
        }
        modified_vertices.push_back( v );
        Q.pop(); // delete from queue
    }
    // sanity-check: for all incident_faces the IN-vertices should be connected
    assert( vdChecker.incidentFaceVerticesConnected( this,  IN ) );
}


// return the closest point to q on the start-end line-segment
Point VoronoiDiagram::get_apex_point(HEVertex start, HEVertex end, HEVertex q) {
    Point p1 = g[start].position;
    Point p2 = g[end].position;
    Point pq = g[q].position;
    
    Point p1pq = pq-p1;
    Point p1p2 = p2-p1;
    double t = p1pq.dot(p1p2) / p1p2.dot(p1p2);
    if (t<0)
        return p1;
    if (t>1)
        return p2;
    else {
        return p1 + t*p1p2;
    }
     
    //return pq;
}

// comments relate to Sugihara-Iri 1994 paper
// this is roughly "algorithm A" from the paper, page 15/50
int VoronoiDiagram::addVertexSite(const Point& p) {
    HEVertex new_vert = hedi::add_vertex( VoronoiVertex(p, OUT, VERTEXGEN), g);
    assert( p.xyNorm() < far_radius );     // only add vertices within the far_radius circle
    gen_count++;
    HEFace closest_face = fgrid->grid_find_closest_face( p );
    HEVertex v_seed = find_seed_vertex( closest_face, p );    
    augment_vertex_set(v_seed, p); 
    add_new_voronoi_vertices( p );    
    HEFace newface = split_faces( p );
    remove_vertex_set( newface );
    g[new_vert].face = newface;
    reset_status();
    assert( vdChecker.isValid(this) );
    return g[new_vert].index;
}

// evaluate H on all face vertices and return vertex with the lowest H
HEVertex VoronoiDiagram::find_seed_vertex(HEFace f, const Point& p) {
    VertexVector face_verts = hedi::face_vertices(f,g);
    assert( face_verts.size() >= 3 );
    double minimumH; 
    HEVertex minimalVertex;
    bool first = true;
    BOOST_FOREACH( HEVertex q, face_verts) { // go thorugh all the vertices and find the one with smallest detH
        if ( g[q].status != OUT ) {
            double h = g[q].detH( p ); 
            if ( first || (h<minimumH) ) {
                minimumH = h;
                minimalVertex = q;
                first = false;
            }
        }
    }
    assert( vdChecker.detH_is_negative( this, p, f, minimalVertex ) );
    return minimalVertex;
}

// from the "one million" paper, growing the v0-tree of "IN" vertices by "breadth-first search"
void VoronoiDiagram::augment_vertex_set(HEVertex& v_seed, const Point& p) {
    std::queue<HEVertex> Q; // FIXME: a priority_queue could/should be used here instead.
    // this woiuld allways examine and decide on the vertex with largest detH, 
    // since that vertex has the most reliable detH sign.
    mark_vertex_in( v_seed, Q);
    modified_vertices.push_back( v_seed );
    while( !Q.empty() ) {
        HEVertex v = Q.front();      assert( g[v].status == UNDECIDED );
        // mark IN and add to v0 if detH<0 and passes tests. otherwise OUT
        double h = g[v].detH( p ); // replace with INPredicate(PointGen) INPredicate(LineGen)
        if ( h < 0.0 ) { // try to mark IN
            // (C4) v should not be adjacent to two or more IN vertices (this would result in a loop/cycle!)
            // (C5) for an incident face containing v: v is adjacent to an IN vertex on this face
            if ( (adjacentInCount(v) >= 2) || (!incidentFacesHaveAdjacentInVertex(v)) ) 
                g[v].status = OUT;
            else
                mark_vertex_in( v, Q);
        } else { // detH was positive, so mark OUT
            g[v].status = OUT;
        }
        modified_vertices.push_back( v );
        Q.pop(); // delete from queue
    }
    // sanity-check: for all incident_faces the IN-vertices should be connected
    assert( vdChecker.incidentFaceVerticesConnected( this,  IN ) );
}

void VoronoiDiagram::mark_vertex_in(HEVertex& v, std::queue<HEVertex>& Q) {
    g[v].status = IN;
    v0.push_back( v );
    markAdjecentFacesIncident( v );
    pushAdjacentVertices( v , Q);
}

// the set v0 are IN vertices that should be removed
// generate new voronoi-vertices on all edges connecting v0 to OUT-vertices
void VoronoiDiagram::add_new_voronoi_vertices( const Point& p ) {
    assert( !v0.empty() );
    EdgeVector q_edges = find_in_out_edges(); //, OUT); // new vertices generated on these IN-OUT edges
    assert( !q_edges.empty() );
    
    for( unsigned int m=0; m<q_edges.size(); ++m )  {  // create new vertices on all edges q_edges[]
        HEVertex q = hedi::add_vertex(g);
        g[q].status = NEW;
        modified_vertices.push_back(q);
        HEFace face = g[q_edges[m]].face;     assert(  g[face].status == INCIDENT);
        HEEdge twin = g[q_edges[m]].twin;
        HEFace twin_face = g[twin].face;      assert( g[twin_face].status == INCIDENT);
        g[q].set_generators( g[face].generator  , g[twin_face].generator  , p); 
        // Point,Point,Point
        // should be: Generator*, Generator*, Generator*
        
        check_vertex_on_edge(q, q_edges[m]);

        hedi::insert_vertex_in_edge( q, q_edges[m] , g);
    }
}

// check that vertex q is positioned on the edge e
void VoronoiDiagram::check_vertex_on_edge(HEVertex q, HEEdge e) {

    // sanity check on new vertex
    if (!(g[q].position.xyNorm() < 18*far_radius)) {
        std::cout << "WARNING check_vertex_on_edge() new vertex outside far_radius! \n";
        std::cout << g[q].position << " norm=" << g[q].position.xyNorm() << " 6.1*far_radius=" << 6.1*far_radius << "\n"; //"WARNING check_vertex_on_edge() new vertex outside far_radius! \n";
    }
    assert( g[q].position.xyNorm() < 18*far_radius);
    
    HEVertex trg = hedi::target(e, g);
    HEVertex src = hedi::source(e, g);
    Point trgP = g[trg].position;
    Point srcP = g[src].position;
    Point newP = g[q].position;
    if (( trgP - srcP ).xyNorm() <= 0 ) {
        std::cout << "WARNING check_vertex_on_edge() zero-length edge! \n";
        g[q].position = srcP;
    } else {
        assert( ( trgP - srcP ).xyNorm() > 0.0 ); // edge has finite length
        assert( ( trgP - srcP ).dot( trgP - srcP ) > 0.0 ); // length squared
        double torig = ((newP - srcP).dot( trgP - srcP )) / ( trgP - srcP ).dot( trgP - srcP ) ;
        bool warn = false;
        double t(torig);
        if (torig < 0.0) { // clamp the t-parameter to [0,1]
            warn = true;
            t=0.0;
        } else if (torig> 1.0) {
            warn = true;
            t=1.0;
        }
        if ( warn ) {
            std::cout << "WARNING: check_vertex_on_edge() t_old= " << torig << " CORRECTED t_new= " << t << "\n";
            std::cout << "src= " << srcP << " new= " << newP << " trg= " << trgP << "\n";
            g[q].position = srcP + t*( trgP-srcP);
            t = ( g[q].position - srcP).dot( trgP - srcP ) / ( trgP - srcP ).dot( trgP - srcP ) ;
            
        }
        // now we are clamped:
        assert( t >= 0.0 );
        assert( t <= 1.0 );        
        double dtl_orig = g[q].position.xyDistanceToLine(srcP, trgP);
        double dtl(dtl_orig);
        if (dtl_orig > 1e-3* ( trgP - srcP ).xyNorm() ) {
            t = ( g[q].position - srcP).dot( trgP - srcP ) / ( trgP - srcP ).dot( trgP - srcP ) ;
            g[q].position = srcP + t*( trgP-srcP);
            dtl = g[q].position.xyDistanceToLine(srcP, trgP);
            std::cout << "WARNING check_vertex_on_edge()  old_dtl= " << dtl_orig << " new_dtl= " << dtl  << "\n";
        }
        assert( dtl < 1e-3* ( trgP - srcP ).xyNorm() );
    }
    

    
}

HEFace VoronoiDiagram::split_faces(const Point& p) {
    HEFace newface =  hedi::add_face( FaceProps( HEEdge(), p, NONINCIDENT ), g );
    fgrid->add_face( g[newface] );
    BOOST_FOREACH( HEFace f, incident_faces ) {
        split_face(newface, f); // each INCIDENT face is split into two parts: newface and f
    }
    return newface;
}

// 1) repair the next-pointers for newface that are broken.
// 2) remove IN vertices in the set v0
void VoronoiDiagram::remove_vertex_set( HEFace newface ) {
    HEEdge current_edge = g[newface].edge; 
    HEEdge start_edge = current_edge;
    do {
        HEVertex current_target = hedi::target( current_edge , g); // an edge on the new face
        HEVertex current_source = hedi::source( current_edge , g);
        BOOST_FOREACH( HEEdge edge, hedi::out_edges( current_target, g ) ) { // loop through potential "next" candidates
            HEVertex out_target = hedi::target( edge , g);
            if ( g[out_target].status == NEW ) { // the next vertex along the face should be "NEW"
                if ( out_target != current_source ) { // but not where we came from
                    g[current_edge].next = edge; // this is the edge we want to take
                    assert( vdChecker.current_face_equals_next_face( this, current_edge ) );
                }
            }
        }
        current_edge = g[current_edge].next; // jump to the next edge
    } while (g[current_edge].next != start_edge);
    BOOST_FOREACH( HEVertex v, v0 ) {      // it should now be safe to delete all IN vertices
        assert( g[v].status == IN );
        hedi::delete_vertex(v,g); // this also removes edges connecting to v
    }
}

// reset status of modified_vertices and incident_faces
void VoronoiDiagram::reset_status() {
    BOOST_FOREACH( HEVertex v, modified_vertices ) {
        g[v].reset();
    }
    modified_vertices.clear();
    g[out_verts[0]].status = OUT; // the outer vertices are special.
    g[out_verts[1]].status = OUT;
    g[out_verts[2]].status = OUT;
    BOOST_FOREACH(HEFace f, incident_faces ) { 
        g[f].status = NONINCIDENT; 
    }
    incident_faces.clear();
    v0.clear();
}


// amont the edges of HEFace f
// find the NEW vertex with signature NEW->s
boost::tuple<HEEdge, HEVertex, HEEdge> VoronoiDiagram::find_new_vertex(HEFace f, VoronoiVertexStatus s) {
    HEVertex v;
    HEEdge prev;
    HEEdge twin_next;
    HEEdge current_edge = g[f].edge;
    bool found = false;                             
    while (!found) {
        HEVertex current_vertex = hedi::target( current_edge , g);
        HEEdge next_edge = g[current_edge].next;
        HEVertex next_vertex = hedi::target( next_edge , g);
        if ( g[current_vertex].status == s ) {
            if ( g[next_vertex].status == NEW) {
                v = next_vertex;
                prev = next_edge;
                twin_next = g[next_edge].next;
                found = true;
            }
        }
        current_edge = g[current_edge].next;   
    }
    return boost::tuple<HEEdge, HEVertex, HEEdge>( prev, v, twin_next );
}


// split the face f into one part which is newface, and the other part is the old f
void VoronoiDiagram::split_face(HEFace newface, HEFace f) {
    HEVertex new_source; // this Vertex is found as OUT-NEW-IN
    HEVertex new_target; // this Vertex is found as IN-NEW-OUT
    HEEdge new_previous, new_next, twin_next, twin_previous;
    boost::tie( new_previous, new_source, twin_next) = find_new_vertex(f, OUT);
    boost::tie( twin_previous, new_target, new_next) = find_new_vertex(f, IN);
    // now connect:   new_previous -> new_source -> new_target -> new_next
    // and:              twin_next <- new_source <- new_target <- twin_previous    
    HEEdge e_new = hedi::add_edge( new_source, new_target , EdgeProps(new_next, f), g); 
    g[new_previous].next = e_new;
    g[f].edge = e_new; 
    // the twin edge that bounds the new face
    HEEdge e_twin = hedi::add_edge( new_target, new_source , EdgeProps(twin_next, newface), g);
    g[twin_previous].next = e_twin;
    g[newface].edge = e_twin; 
    hedi::twin_edges(e_new,e_twin,g);
}

// given a list inVertices of "IN" vertices, find the adjacent IN-OUT edges 
EdgeVector VoronoiDiagram::find_in_out_edges() { 
    assert( !v0.empty() );
    EdgeVector output; // new vertices generated on these edges
    BOOST_FOREACH( HEVertex v, v0 ) {                                   
        assert( g[v].status == IN ); // all verts in v0 are IN
        BOOST_FOREACH( HEEdge edge, hedi::out_edges( v , g) ) {
            HEVertex adj_vertex = hedi::target( edge , g);
            if ( g[adj_vertex].status == OUT ) 
                output.push_back(edge); // this is an IN-OUT edge
        }
    }
    return output;
}

void VoronoiDiagram::pushAdjacentVertices( HEVertex v, std::queue<HEVertex>& Q) {
    BOOST_FOREACH( HEVertex w, hedi::adjacent_vertices(v,g) ) {
        if ( g[w].status == UNDECIDED ) {
            if ( !g[w].in_queue ) { 
                Q.push(w); // push adjacent undecided verts for testing.
                g[w].in_queue=true;
            }
        }
    }
}

int VoronoiDiagram::adjacentInCount(HEVertex v) {
    int in_count=0;
    BOOST_FOREACH( HEVertex w, hedi::adjacent_vertices(v,g) ) {
        if ( g[w].status == IN )
            in_count++;
    }
    return in_count;
}

FaceVector VoronoiDiagram::adjacentIncidentFaces(HEVertex v) {
    FaceVector adj_faces = hedi::adjacent_faces(v,g);
    assert( adj_faces.size() == 3 );
    FaceVector inc_faces;
    BOOST_FOREACH( HEFace f, adj_faces ) {
        if ( g[f].status == INCIDENT )
            inc_faces.push_back( f );
    }
    assert( !inc_faces.empty() );
    return inc_faces;
}

bool VoronoiDiagram::incidentFacesHaveAdjacentInVertex(HEVertex v) {
    bool all_found = true;
    BOOST_FOREACH( HEFace f, adjacentIncidentFaces(v) ) { // check each face f
        // v should be adjacent to an IN vertex on the face
        bool face_found=false;
        BOOST_FOREACH( HEVertex w, hedi::face_vertices(f,g) ) {
            if ( w != v && g[w].status == IN && hedi::has_edge(w,v,g) ) 
                face_found = true;
        }
        if (!face_found)
            all_found=false;
    }
    return all_found;
}


// IN-Vertex v has three adjacent faces, mark nonincident faces incident
// and push them to incident_faces
void VoronoiDiagram::markAdjecentFacesIncident( HEVertex v) {
    assert( g[v].status == IN );
    FaceVector new_adjacent_faces = hedi::adjacent_faces( v, g ); 
    assert( new_adjacent_faces.size()==3 );
    BOOST_FOREACH( HEFace adj_face, new_adjacent_faces ) {
        if ( g[adj_face].status  != INCIDENT ) {
            g[adj_face].status = INCIDENT; 
            incident_faces.push_back(adj_face);
        }
    }
}

void VoronoiDiagram::printFaceVertexTypes(HEFace f) {
    std::cout << " Face " << f << ": ";
    VertexVector face_verts = hedi::face_vertices(f,g);    
    unsigned count=1;
    BOOST_FOREACH( HEVertex v, face_verts ) {
        std::cout << g[v].index  << "(" << g[v].status  << ")";
        if (count != face_verts.size() )
            std::cout << "-";
        count++;
    }
    std::cout << "\n";
}

void VoronoiDiagram::printVertices(VertexVector& q) {
    BOOST_FOREACH( HEVertex v, q) {
        std::cout << g[v].index << " ";
    }
    std::cout << std::endl;
}

std::string VoronoiDiagram::str() const {
    std::ostringstream o;
    o << "VoronoiDiagram (nVerts="<< hedi::num_vertices(g) << " , nEdges="<< hedi::num_edges(g) <<"\n";
    return o.str();
}

} // end namespace
// end file voronoidiagram.cpp
