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


#include "halfedgediagram.h"

namespace ocl
{


void VertexProps::set_position() {
    double w = J4;
    double x = - J2/w+pk.x;
    double y = J3/w+pk.y;
    position =  Point(x,y);
}
double VertexProps::detH(const Point& pl) {
    return J2*(pl.x-pk.x) - J3*(pl.y-pk.y) + 0.5*J4*(square(pl.x-pk.x) + square(pl.y-pk.y));
}
void VertexProps::set_J(Point& pi, Point& pj, Point& pk) { 
    // 1) i-j-k should come in CCW order
    Point pi_,pj_,pk_;
    if ( pi.isRight(pj,pk) ) {
        pi_ = pj;
        pj_ = pi;
        pk_ = pk;
    } else {
        pi_ = pi;
        pj_ = pj;
        pk_ = pk;
    }
    // 2) point pk should have the largest angle TODO TODO
    this->pk = pk_;
    J2 = detH_J2( pi_, pj_, pk_);
    J3 = detH_J3( pi_, pj_, pk_);
    J4 = detH_J4( pi_, pj_, pk_);

}
/// the J2 determinant
double VertexProps::detH_J2(Point& pi, Point& pj, Point& pk) {
    return (pi.y-pk.y)*(square(pj.x-pk.x)+square(pj.y-pk.y))/2 - (pj.y-pk.y)*(square(pi.x-pk.x)+square(pi.y-pk.y))/2;
}
/// the J3 determinant
double VertexProps::detH_J3(Point& pi, Point& pj, Point& pk) {
    return (pi.x-pk.x)*(square(pj.x-pk.x)+square(pj.y-pk.y))/2 - (pj.x-pk.x)*(square(pi.x-pk.x)+square(pi.y-pk.y))/2;
}
/// the J4 determinant
double VertexProps::detH_J4(Point& pi, Point& pj, Point& pk) {
    return (pi.x-pk.x)*(pj.y-pk.y) - (pj.x-pk.x)*(pi.y-pk.y);
}


/* ****************** FaceList ******************************** */



FaceList::FaceList() {
    far_radius = 1;
    nbins = 100;
    binwidth = 2*far_radius/nbins;
    faces.clear();
}
FaceList::FaceList(double far, unsigned int n_bins) {
    far_radius = 3.1*far;
    nbins = n_bins;
    faces.clear();
    binwidth = 2*far_radius/nbins;
    grid = new Grid( boost::extents[n_bins][n_bins] );
    for ( GridIndex m=0 ; m<nbins ; ++m ) {
        for ( GridIndex n=0 ; n<nbins ; ++n ) {
            (*grid)[m][n] = new FaceVector();
        }
    }
}

HEFace FaceList::add_face(FaceProps f_prop) {
    faces.push_back(f_prop);
    HEFace idx = faces.size()-1;
    // insert into correct bin here
    GridIndex row = get_grid_index( f_prop.generator.x );
    GridIndex col = get_grid_index( f_prop.generator.y );
    FaceVector* bucket = (*grid)[row][col];
    bucket->push_back(idx);
    return idx;
} 



GridIndex FaceList::get_grid_index( double x ) {
    GridIndex idx;
    idx = (int)( floor( (x+far_radius)/binwidth ) );                
        assert( idx >= 0 );     
        assert( idx <= nbins );
    return idx;
}

FaceProps& FaceList::operator[](const unsigned int m) {
    return faces[m];
}

const FaceProps& FaceList::operator[](const unsigned int m) const {
    return faces[m];
}

unsigned int FaceList::size() const {
    return faces.size();
}
    
/// simple implementation to find the closest face to the new generator p
HEFace FaceList::find_closest_face(const Point& p) {
    HEFace closest_face;
    double closest_distance = 3*far_radius; // a big number...
    double d;
    for (HEFace  m=0;m<faces.size();++m) {
        d = ( faces[m].generator - p).norm();
        if (d<closest_distance ) {
            closest_distance=d;
            closest_face=m;
        }
    }
    return closest_face;
}
    
/// grid-based search for the closest face to generator p
// grid search algorithm:
// - find the closest grid-cell
// - move in a spiral outward to find the first point
// - when the first point is found we need only search grid cells within a radius = distance-to-found-point
// - find the cells within a radius = distance-to-found-point
// -- in these cells, search all points and find the closest one
HEFace FaceList::grid_find_closest_face(const Point& p) {
    //std::cout << " grid_find_closest_face( "<< p << " ) \n";
    HEFace closest_face;
    std::set<HEFace> fset;
    GridIndex row = get_grid_index( p.x );
    GridIndex col = get_grid_index( p.y );
    //std::cout << "closest cell is ( " << row << " , " << col << " ) \n" ;
    insert_faces_from_bucket( fset, row, col ); // the closest bucket
    GridIndex dist = 0;
    do {
        dist++;
        insert_faces_from_neighbors( fset, row, col , dist );
        //assert( dist < nbins );
    } while (fset.empty());
    //std::cout << " fset.size() = " << fset.size() << "  at dist= " << dist << "\n";
    GridIndex max_dist = (int)( ceil( sqrt((double)2)*dist ) ); // expand up to this radius, to be sure to find the closest point
    for (GridIndex d = dist; d<=max_dist;d++)
        insert_faces_from_neighbors( fset, row, col , d );

    closest_face = find_closest_in_set( fset , p ); // among the faces found, find the closest one
    return closest_face;
}
/// go through the HEFace set and return the one closest to p
HEFace FaceList::find_closest_in_set( std::set<HEFace>& set, const Point&p ) {
    HEFace closest_face;
    double closest_distance = 3*far_radius; // a big number...
    double d;
    BOOST_FOREACH( HEFace f, set ) {
        d = ( faces[f].generator - p).norm();
        if (d<closest_distance ) {
            closest_distance=d;
            closest_face=f;
        }
    }
    return closest_face;
}
    
void FaceList::insert_faces_from_neighbors( std::set<HEFace>& set, GridIndex row, GridIndex col , GridIndex dist ) {
    // insert faces from neighbors of (row,col) at distance dist
    GridIndex min_row = (row >= dist ? row-dist : 0); // N
    GridIndex max_row = (row <= nbins-dist-1 ? row+dist : nbins-1); // S
    GridIndex max_col = (col <= nbins-dist-1 ? col+dist : nbins-1); // E
    GridIndex min_col = (col >= dist  ? col-dist : 0); // W
    for (GridIndex c = min_col; c<=max_col; c++) {
        insert_faces_from_bucket( set, min_row , c );
        insert_faces_from_bucket( set, max_row , c );
    }
    for (GridIndex r = min_row; r<=max_row; r++) {
        insert_faces_from_bucket( set, r, min_col );
        insert_faces_from_bucket( set, r, max_col );
    }
}
    
void FaceList::insert_faces_from_bucket( std::set<HEFace>& set, GridIndex row, GridIndex col ) {
    FaceVector* bucket = (*grid)[row][col];
    BOOST_FOREACH( HEFace f, *bucket ) {
        set.insert(f);
    }
}

/* ****************** HalfEdgeDiagram ******************************** */



HalfEdgeDiagram::HalfEdgeDiagram(double far, unsigned int n_bins) {
    faces = FaceList(far, n_bins);
}

HEEdge HalfEdgeDiagram::add_edge(HEVertex v1, HEVertex v2) {
    HEEdge e;
    bool b;
    boost::tie( e , b ) = boost::add_edge( v1, v2, *this);
    return e;
}

HEEdge HalfEdgeDiagram::add_edge(HEVertex v1, HEVertex v2, EdgeProps prop) {
    HEEdge e;
    bool b;
    boost::tie( e , b ) = boost::add_edge( v1, v2, prop, *this);
    return e;
}

HEFace HalfEdgeDiagram::add_face( FaceProps f_prop ) {
    return faces.add_face( f_prop );
}
HEFace HalfEdgeDiagram::find_closest_face(const Point& p){
    return faces.grid_find_closest_face(p);
}


EdgeVector HalfEdgeDiagram::edges() const {
    EdgeVector ev;
    HEEdgeItr it, it_end;
    boost::tie( it, it_end ) = boost::edges( *this );
    for ( ; it != it_end ; ++it ) {
        ev.push_back(*it);
    }
    return ev;
}
        
EdgeVector HalfEdgeDiagram::out_edges( HEVertex v ) const {
    EdgeVector ev;
    HEOutEdgeItr it, it_end;
    boost::tie( it, it_end ) = boost::out_edges( v, *this );
    for ( ; it != it_end ; ++it ) {
        ev.push_back(*it);
    }
    return ev;
}


VertexVector HalfEdgeDiagram::vertices() const {
    VertexVector vv;
    HEVertexItr it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( *this );
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        vv.push_back( *itr );
    }
    return vv;
}

HEEdge HalfEdgeDiagram::previous_edge(HEEdge e) {
    HEEdge previous = (*this)[e].next;
    while ( (*this)[previous].next != e ) {
        previous = (*this)[previous].next;
    }
    return previous;
}

// traverse face and return all vertices found
VertexVector HalfEdgeDiagram::face_vertices(HEFace face_idx) {
    VertexVector verts;
    HEEdge startedge = faces[face_idx].edge; // the edge where we start
    HEVertex start_target = boost::target( startedge, *this); 
    verts.push_back(start_target);
    HEEdge current = (*this)[startedge].next;
    do {
        HEVertex current_target = boost::target( current, *this); 
        verts.push_back(current_target);
        current = (*this)[current].next;
    } while ( current != startedge );
    return verts;
}
FaceVector HalfEdgeDiagram::adjacent_faces( HEVertex q ) {
    // given the vertex q, find the three adjacent faces
    std::set<HEFace> face_set;
    HEOutEdgeItr itr, itr_end;
    boost::tie( itr, itr_end) = boost::out_edges(q, *this);
    for ( ; itr!=itr_end ; ++itr ) {
        face_set.insert( (*this)[*itr].face );
    }
    assert( face_set.size() == 3); // degree of q is three, so has three faces
    FaceVector fv;
    BOOST_FOREACH(HEFace m, face_set) {
        fv.push_back(m);
    }
    return fv;
}

/// insert vertex v into edge e
void HalfEdgeDiagram::insert_vertex_in_edge(HEVertex v, HEEdge e) {
    // the vertex v is in the middle of edge e
    //                    face
    //                    e1   e2
    // previous-> source  -> v -> target
    //            tw_targ <- v <- tw_sour <- tw_previous
    //                    te2  te1
    //                    twin_face
    
    HEEdge twin = (*this)[e].twin;
    HEVertex source = boost::source( e , *this );
    HEVertex target = boost::target( e , *this );
    HEVertex twin_source = boost::source( twin , *this );
    HEVertex twin_target = boost::target( twin , *this );
    if ( source != twin_target )
        std::cout << " ERROR " << e << " source is " << source << " but " << twin << " target is " << twin_target << "\n"; 
    assert( source == twin_target );    assert( target == twin_source );
    
    HEFace face = (*this)[e].face;
    HEFace twin_face = (*this)[twin].face;
    HEEdge previous = previous_edge(e);
    HEEdge twin_previous = previous_edge(twin);
    
    HEEdge e1 = add_edge( source, v  );
    HEEdge e2 = add_edge( v, target  );
    
    // preserve the left/right face link
    (*this)[e1].face = face;
    (*this)[e1].next = e2;
    (*this)[e2].face = face;
    (*this)[e2].next = (*this)[e].next;
    (*this)[previous].next = e1;
    
    HEEdge te1 = add_edge( twin_source, v  );
    HEEdge te2 = add_edge( v, twin_target  );
    (*this)[te1].face = twin_face;
    (*this)[te1].next = te2;
    (*this)[te2].face = twin_face;
    (*this)[te2].next = (*this)[twin].next;
    (*this)[twin_previous].next = te1;
    // TWINNING (note indices 'cross', see ASCII art above)
    (*this)[e1].twin = te2;
    (*this)[te2].twin = e1;
    (*this)[e2].twin = te1;
    (*this)[te1].twin = e2;
    
    // update the faces
    (*this)[face].edge = e1;
    (*this)[twin_face].edge = te1;
    
    // finally, remove the old edge
    boost::remove_edge( e   , (*this));
    boost::remove_edge( twin, (*this));
}


} // end namespace
// end file halfedgediagram.cpp

