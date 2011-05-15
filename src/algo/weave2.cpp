/*  $Id$
 * 
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

#include "weave2.h"


namespace ocl
{

namespace weave2
{

int VertexProps::count = 0;

void Weave::addFiber(Fiber& f) {
    if ( f.dir.xParallel() && !f.empty() ) {
        xfibers.push_back(f);
    } else if ( f.dir.yParallel() && !f.empty() ) {
        yfibers.push_back(f);
    } else if (!f.empty()) {
        assert(0); // fiber must be either x or y
    }
}

void Weave::face_traverse() { 
    std::cout << " traversing graph with " << clVertices.size() << " cl-points\n";
    // traverse the graph putting loops of vertices into the loops variable
    // std::vector< std::vector<Vertex> > loops;
    while ( !clVertices.empty() ) {
        std::vector<Vertex> loop;
        Vertex current = *(clVertices.begin());
        Vertex first = current;
        do {
            assert( g[current].type == CL );
            loop.push_back(current);
            clVertices.erase(current);
            // find the edge to follow
            std::vector<Edge> outEdges = hedi::out_edges(current, g);
            if (outEdges.size() != 1 )
                std::cout << " outEdges.size() = " << outEdges.size() << "\n";
            assert( outEdges.size() == 1 );
            // traverse to the next cl-point using next
            Edge currentEdge = outEdges[0];
            do { // following next, find a CL point 
                current = hedi::target( currentEdge, g);
                currentEdge = g[currentEdge].next;
            } while ( g[current].type != CL );
        } while (current!=first); // end the loop when we arrive at the start
        loops.push_back(loop);
    }
}
        

Vertex Weave::add_vertex( Point& position, VertexType type, Interval& interv, double ipos) {
    Vertex  v = hedi::add_vertex(VertexProps( position, type ), g);
    interv.intersections2.insert( VertexPair( v, ipos) ); // ?? this makes Interval depend on the WeaveGraph type
    clVertices.insert(v);
    return v;
}

/// from the list of fibers, build a graph
void Weave::build() {
    // 1) add CL-points of X-fiber (if not already in graph)
    // 2) add CL-points of Y-fiber (if not already in graph)
    // 3) add intersection point (if not already in graph) (will allways be new??)
    // 4) add edges. (if they provide new connections)
    //      ycl_lower <-> intp <-> ycl_upper
    //      xcl_lower <-> intp <-> xcl_upper
    // if this connects points that are already connected, then remove old edge and
    // provide this "via" connection
    //int n_xfiber=0;
    BOOST_FOREACH( Fiber& xf, xfibers) {
        assert( !xf.empty() ); // no empty fibers please
        BOOST_FOREACH( Interval& xi, xf.ints ) {
            //std::cout << "x-fiber " << n_xfiber++ << "\n";
            double xmin = xf.point(xi.lower).x;
            double xmax = xf.point(xi.upper).x;
            assert( !xi.in_weave ); // this is the first time the x-interval is added!
            xi.in_weave = true;
            // add the X interval end-points to the weave
            Point p1( xf.point(xi.lower) );
            Vertex xv1 = add_vertex( p1, CL , xi, p1.x ); 
            Point p2( xf.point(xi.upper) );
            Vertex xv2 = add_vertex( p2, CL , xi, p2.x );
            Edge e1 = hedi::add_edge( xv1, xv2, g);
            Edge e2 = hedi::add_edge( xv2, xv1, g);

            //std::cout << " add_edge " << v1 << "("<< p1.x<< ") - " << v2 <<"("<< p2.x << ")\n";
            g[e1].next = e2;
            g[e2].next = e1;
            g[e1].prev = e2;
            g[e2].prev = e1;
            
            BOOST_FOREACH( Fiber& yf, yfibers ) { // loop through all y-fibers for all x-intervals
                if ( (xmin <= yf.p1.x) && ( yf.p1.x <= xmax ) ) {// potential intersection between y-fiber and x-interval
                    BOOST_FOREACH( Interval& yi, yf.ints ) {
                        double ymin = yf.point(yi.lower).y ;
                        double ymax = yf.point(yi.upper).y ;
                        if ( (ymin <= xf.p1.y) && (xf.p1.y <= ymax) ) { 
                            // there is an actual intersection btw x-interval and y-interval
                            // X interval xi on fiber xf intersects with Y interval yi on fiber yf
                            // intersection is at ( yf.p1.x, xf.p1.y , xf.p1.z )
                            if (!yi.in_weave) { // add y-interval endpoints to weave
                                Point p1( yf.point(yi.lower) );
                                add_vertex( p1, CL , yi, p1.y );
                                Point p2( yf.point(yi.upper) );
                                add_vertex( p2, CL , yi, p2.y );
                                yi.in_weave = true;
                            }
                            // 3) intersection point (this is always new, no need to check for existence??)
                            Vertex v = hedi::add_vertex(g);
                            Point v_position( yf.p1.x, xf.p1.y , xf.p1.z );
                            g[v].position = v_position;
                            g[v].type = INT;
                            
                            
                            // 4) add edges 
                            // find the X-vertices above and below the new vertex v.
                            VertexPair v_pair_x( v , v_position.x );
                            //std::cout << " search for X-neigbors to " << v << "("<<v_position.x << ") \n";
                            VertexPairIterator x_tmp = xi.intersections2.lower_bound( v_pair_x ); // returns first that is not less than argument (equal or greater)
                            //assert(x_tmp != xi.intersections2.end() ); // we must find a lower_bound
                            //std::cout << " lower_bound is " << x_tmp->first << " at " << x_tmp->second << "\n";
                            VertexPairIterator x_above = x_tmp;
                            VertexPairIterator x_below = --x_tmp;
                            //std::cout << " x-above is " << x_above->first << " at " << x_above->second << "\n";
                            //std::cout << " x-below is " << x_below->first << " at " << x_below->second << "\n";
                            Vertex x_u = x_above->first; // vertex above v
                            Vertex x_l = x_below->first; // vertex below v
                            // if x_above and x_below should be already connected
                            //assert( hedi::has_edge( x_l, x_u, g ) );
                            //assert( hedi::has_edge( x_u, x_l, g ) );
                            
                            // these original edges will eventually be deleted!
                            Edge xe_lu = hedi::edge( x_l, x_u, g);
                            Edge xe_ul = hedi::edge( x_u, x_l, g);
                            Edge xe_lu_next = g[xe_lu].next;
                            Edge xe_lu_prev = g[xe_lu].prev; // hedi::previous_edge( xe_lu, g );
                            Edge xe_ul_next = g[xe_ul].next;
                            Edge xe_ul_prev = g[xe_ul].prev; // hedi::previous_edge( xe_ul, g );
                            
                            // now do the same thing in the y-direction.
                            VertexPair v_pair_y( v , v_position.y );
                            VertexPairIterator y_tmp = yi.intersections2.lower_bound( v_pair_y );
                            //assert(y_tmp != yi.intersections2.end() );
                            VertexPairIterator y_above = y_tmp;
                            VertexPairIterator y_below = --y_tmp;
                            Vertex y_u = y_above->first;
                            Vertex y_l = y_below->first;
                            //std::cout << " search for Y-neigbors to " << v << "("<< v_position.y << ") \n";
                            //std::cout << " y-above is " << y_above->first << " at " << y_above->second << "\n";
                            //std::cout << " y-below is " << y_below->first << " at " << y_below->second << "\n";

                            // the next/prev data we need
                            Edge ye_lu_next ;
                            Edge ye_lu_prev ;
                            Edge ye_ul_next ;
                            Edge ye_ul_prev ;
                            Edge ye_lu;
                            Edge ye_ul;
                            bool y_lu_edge = hedi::has_edge( y_l, y_u, g ); // flag indicating existing y_l - y_u edge 
                            // the case where y_l and y_u are alread already connected.
                            if ( y_lu_edge ) {
                                //assert( hedi::has_edge( y_u, y_l, g ) ); // twin must also exist
                                ye_lu = hedi::edge( y_l, y_u, g);
                                ye_ul = hedi::edge( y_u, y_l, g);
                                ye_lu_next = g[ye_lu].next;
                                ye_lu_prev = g[ye_lu].prev; // hedi::previous_edge( ye_lu, g );
                                ye_ul_next = g[ye_ul].next;
                                ye_ul_prev = g[ye_ul].prev; // hedi::previous_edge( ye_ul, g );
                            } 
                            // and now eight new edges to add
                            Edge xl_v = hedi::add_edge(x_l, v  , g);
                            Edge v_yl = hedi::add_edge(v  , y_l, g);
                            Edge yl_v = hedi::add_edge(y_l, v  , g);
                            Edge v_xu = hedi::add_edge(v  , x_u, g);
                            Edge xu_v = hedi::add_edge(x_u, v  , g);
                            Edge v_yu = hedi::add_edge(v  , y_u, g);
                            Edge yu_v = hedi::add_edge(y_u, v  , g);
                            Edge v_xl = hedi::add_edge(v  , x_l, g);
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
                            // delete the old edges
                            boost::remove_edge( x_l, x_u, g);
                            boost::remove_edge( x_u, x_l, g);
                            if ( y_lu_edge ) {
                                boost::remove_edge( y_l, y_u, g);
                                boost::remove_edge( y_u, y_l, g);
                            }
                            
                            // finally add new intersection vertex to the interval sets
                            xi.intersections2.insert( VertexPair( v, v_position.x ) );
                            yi.intersections2.insert( VertexPair( v, v_position.y ) );

                        } // end intersection case
                    } // end y interval loop
                } // end if(potential intersection)
            } // end y fiber loop
        } // x interval loop
       
    } // end X-fiber loop
     
}
        
 
} // end weave2 namespace

} // end ocl namespace
// end file weave2.cpp
