/*  $Id$
 * 
 *  Copyright (c) 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com).
 *  
 *  This file is part of OpenCAMlib 
 *  (see https://github.com/aewallin/opencamlib).
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <sstream>
#include <string>

#include "simple_weave.hpp"

namespace ocl
{

namespace weave
{

std::pair<Vertex,Vertex> SimpleWeave::find_neighbor_vertices( VertexPair v_pair, Interval& ival) {
    Interval::VertexPairIterator itr = ival.intersections2.lower_bound( v_pair ); // returns first that is not less than argument (equal or greater)
    assert( itr != ival.intersections2.end() ); // we must find a lower_bound
    Interval::VertexPairIterator v_above = itr; // lower_bound returns one beyond the give key, i.e. what we want
    Interval::VertexPairIterator v_below = --itr; // this is the vertex below the give vertex
    std::pair<Vertex,Vertex> out;
    out.first = v_above->first; // vertex above v (xu)
    out.second = v_below->first; // vertex below v (xl)
    assert( out.first != out.second );
    return out;
}
                            
// from the list of fibers, build a graph
// FIXME: the problem here is that from N x-fibers and N y-fibers
// this builds a graph with roughly N*N vertices/edges. This consumes a lot of RAM even
// for small parts, and unacceptably much RAM for large parts.
// a smarter build() would keep track of the important faces of the planar graph
// (faces that produce toolpath-loops)
// vertices/edges belonging to non-toolpath-producing faces could then be deleted
// and the RAM consumption should be limited to N+N (i.e. the length/perimeter of the part/toolpath
// in contrast to the area for the naive implementation)
void SimpleWeave::build() {
    // 1) add CL-points of X-fiber (if not already in graph)
    // 2) add CL-points of Y-fiber (if not already in graph)
    // 3) add intersection point (if not already in graph) (will always be new??)
    // 4) add edges. (if they provide new connections)
    // ycl_lower <-> intp <-> ycl_upper
    // xcl_lower <-> intp <-> xcl_upper
    // if this connects points that are already connected, then remove old edge and
    // provide this "via" connection
    //int n_xfiber=0;
    // std::cout << " SimpleWeave::build()... \n";
    BOOST_FOREACH( Fiber& xf, xfibers) {
        assert( !xf.empty() ); // no empty fibers please
        BOOST_FOREACH( Interval& xi, xf.ints ) {
            //std::cout << "x-fiber " << n_xfiber++ << "\n";
            double xmin = xf.point(xi.lower).x;
            double xmax = xf.point(xi.upper).x;
            if ( (xmax-xmin) > 0) {
            assert( !xi.in_weave ); // this is the first time the x-interval is added!
            xi.in_weave = true;
            // add the X interval end-points to the weave
            Point p1( xf.point(xi.lower) );
            Vertex xv1 = add_cl_vertex( p1, xi, p1.x );
            Point p2( xf.point(xi.upper) );
            Vertex xv2 = add_cl_vertex( p2, xi, p2.x );
            Edge e1 = g.add_edge(xv1,xv2); 
            Edge e2 = g.add_edge(xv2,xv1); 

            //std::cout << " add_edge " << xv1 << "("<< p1.x<< ") - " << xv2 <<"("<< p2.x << ")\n";
            g[e1].next = e2;
            g[e2].next = e1;
            g[e1].prev = e2;
            g[e2].prev = e1;
            //int n_yfiber=0;
            BOOST_FOREACH( Fiber& yf, yfibers ) { // loop through all y-fibers for all x-intervals
                if ( (xmin < yf.p1.x) && ( yf.p1.x < xmax ) ) {// potential intersection between y-fiber and x-interval
                    BOOST_FOREACH( Interval& yi, yf.ints ) {
                        //std::cout << "y-fiber " << n_yfiber++ << "\n";
                        double ymin = yf.point(yi.lower).y ;
                        double ymax = yf.point(yi.upper).y ;
                        if ( (ymin < xf.p1.y) && (xf.p1.y < ymax) ) {
                            // there is an actual intersection btw x-interval and y-interval
                            // X interval xi on fiber xf intersects with Y interval yi on fiber yf
                            // intersection is at ( yf.p1.x, xf.p1.y , xf.p1.z )
                            if (!yi.in_weave) { // add y-interval endpoints to weave
                                Point yp1( yf.point(yi.lower) );
                                add_cl_vertex( yp1, yi, yp1.y );
                                Point yp2( yf.point(yi.upper) );
                                add_cl_vertex( yp2, yi, yp2.y );
                                yi.in_weave = true;
                            }
                            // 3) intersection point, of type INT
                            
                            Vertex v = g.null_vertex(); 
                            
                            Point v_position( yf.p1.x, xf.p1.y , xf.p1.z );
                            // find neighbor to v
                            Vertex x_u, x_l;
                            
                            //std::cout << " fins neighbor to x= " << v_position.x << "\n";
                            boost::tie( x_u, x_l ) = find_neighbor_vertices( VertexPair(v, v_position.x), xi );
                            //std::cout << "found: x_u , x_l : " << x_u << " , " << x_l << "\n";
                            Vertex y_u, y_l;
                            boost::tie( y_u, y_l ) = find_neighbor_vertices( VertexPair(v, v_position.y), yi );
                            
                            //std::cout << "found: y_u , y_l : " << y_u << " , " << y_l << "\n";
                            
                            add_int_vertex(v_position,x_l,x_u,y_l,y_u,xi,yi);
                        } // end intersection case
                    } // end y interval loop
                } // end if(potential intersection)
            } // end y fiber loop
            
            // now we've added an x-interval, we've gone through all the y-intervals
            // if there isn't a single intersecting interval, then remove the x-interval as it is useless
            assert( xi.intersections2.size() >= 2  );
            if ( xi.intersections2.size() == 2 ) {
                clVertexSet.erase(xv1);
                clVertexSet.erase(xv2);
                g.clear_vertex(xv1); 
                g.clear_vertex(xv2); 
                g.remove_vertex(xv1); 
                g.remove_vertex(xv2);  
            }
        
        } // end zero-length interval check

        } // x interval loop
    } // end X-fiber loop
}

// add a new CL-vertex to Weave, also adding it to the interval intersection-set, and to clVertices
Vertex SimpleWeave::add_cl_vertex( const Point& position, Interval& ival, double ipos) {
    Vertex  v = g.add_vertex(); 
    g[v].position = position;
    g[v].type = CL;
    ival.intersections2.insert( VertexPair( v, ipos) ); // ?? this makes Interval depend on the WeaveGraph type
    clVertexSet.insert(v);
    return v;
}

// add an internal vertex to the weave
void SimpleWeave::add_int_vertex(  const Point& v_position, // position of new vertex
                             Vertex& x_l, // the x-lower vertex
                             Vertex& x_u, // the x-upper vertex
                             Vertex& y_l, // y-lower
                             Vertex& y_u, // y-upper
                             Interval& x_int,  // the x-interval
                             Interval& y_int ) // the y-interval
{
    //std::cout << " add_int_vertex " << "\n";
    Vertex v = g.add_vertex(); //hedi::add_vertex( VertexProps( v_position, INT ), g);
    g[v].position = v_position;
    g[v].type = INT;
    assert( g.has_edge(x_l,x_u) ); 
    assert( g.has_edge(x_u,x_l) ); 
    Edge xe_lu = g.edge(x_l,x_u); 
    Edge xe_ul = g.edge(x_u,x_l); 
    Edge xe_lu_next = g[xe_lu].next;
    Edge xe_lu_prev = g[xe_lu].prev; 
    Edge xe_ul_next = g[xe_ul].next;
    Edge xe_ul_prev = g[xe_ul].prev; 
    
    // the next/prev data we need
    Edge ye_lu_next, ye_lu_prev ;
    Edge ye_ul_next, ye_ul_prev ;
    Edge ye_lu, ye_ul;

    bool y_lu_edge = g.has_edge(y_l,y_u); // flag indicating existing y_l - y_u edge
    // the case where y_l and y_u are already connected.

    if ( y_lu_edge ) {
        assert( g.has_edge( y_u, y_l ) ); // twin must also exist
        ye_lu = g.edge( y_l, y_u);
        ye_ul = g.edge( y_u, y_l);
        ye_lu_next = g[ye_lu].next;
        ye_lu_prev = g[ye_lu].prev; 
        ye_ul_next = g[ye_ul].next;
        ye_ul_prev = g[ye_ul].prev; 
    } 

    // and now eight new edges to add
    Edge xl_v = g.add_edge(x_l, v  );
    Edge v_yl = g.add_edge(v  , y_l);
    Edge yl_v = g.add_edge(y_l, v  );
    Edge v_xu = g.add_edge(v  , x_u);
    Edge xu_v = g.add_edge(x_u, v  );
    Edge v_yu = g.add_edge(v  , y_u);
    Edge yu_v = g.add_edge(y_u, v  );
    Edge v_xl = g.add_edge(v  , x_l);
    // checks for special cases:
    if (xe_lu_prev == xe_ul) // xl hairpin
        xe_lu_prev = v_xl;
    if (xe_lu_next == xe_ul) // xu hairpin
        xe_lu_next = xu_v;
    if (xe_ul_prev == xe_lu)
        xe_ul_prev = v_xu;
    if (xe_ul_next == xe_lu)
        xe_ul_next = xl_v;
    if ( y_lu_edge ) {
        // the same checks for the y-edge
        if (ye_lu_prev == ye_ul)
            ye_lu_prev = v_yl;
        if (ye_lu_next == ye_ul)
            ye_lu_next = yu_v;
        if (ye_ul_prev == ye_lu)
            ye_ul_prev = v_yu;
        if (ye_ul_next == ye_lu)
            ye_ul_next = yl_v;
    } else { // we should form a y-hairpin
        ye_lu_next = yu_v;
        ye_lu_prev = v_yl;
        ye_ul_next = yl_v;
        ye_ul_prev = v_yu;
    }
    // now set next/prev edges (there are 2*12=24 of these to do)
    g[xe_lu_prev].next = xl_v;  g[xl_v].prev = xe_lu_prev;
    g[xl_v].next = v_yl;        g[v_yl].prev = xl_v;
    g[v_yl].next = ye_ul_next;  g[ye_ul_next].prev = v_yl;
    g[ye_lu_prev].next = yl_v;  g[yl_v].prev = ye_lu_prev;
    g[yl_v].next = v_xu;        g[v_xu].prev = yl_v;
    g[v_xu].next = xe_lu_next;  g[xe_lu_next].prev = v_xu;
    g[xe_ul_prev].next = xu_v;  g[xu_v].prev = xe_ul_prev;
    g[xu_v].next = v_yu;        g[v_yu].prev = xu_v;
    g[v_yu].next = ye_lu_next;  g[ye_lu_next].prev = v_yu;
    g[ye_ul_prev].next = yu_v;  g[yu_v].prev = ye_ul_prev;
    g[yu_v].next = v_xl;        g[v_xl].prev = yu_v;
    g[v_xl].next = xe_ul_next;  g[xe_ul_next].prev = v_xl;
    // delete the old x-edges
    g.remove_edge( x_l, x_u);
    g.remove_edge( x_u, x_l);
    if( y_lu_edge ) { // delete old y-edges
        assert( g.has_edge( y_l, y_u) );
        assert( g.has_edge( y_u, y_l) );
        g.remove_edge( y_l, y_u);
        g.remove_edge( y_u, y_l);
    }
    
    // finally add new intersection vertex to the interval sets
    x_int.intersections2.insert( VertexPair( v, v_position.x ) );
    y_int.intersections2.insert( VertexPair( v, v_position.y ) );
}


} // end weave namespace

} // end ocl namespace
// end file simple_weave.cpp
