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

// traverse the graph putting loops of vertices into the loops variable
// this figure illustrates next-pointers: http://www.anderswallin.net/wp-content/uploads/2011/05/weave2_zoom.png
void Weave::face_traverse() { 
    std::cout << " traversing graph with " << clVertices.size() << " cl-points\n";
    while ( !clVertices.empty() ) { // while unprocessed cl-vertices remain
        std::vector<Vertex> loop; // start on a new loop
        Vertex current = *(clVertices.begin());
        Vertex first = current;
        
        do { // traverse around the loop
            assert( g[current].type == CL ); // we only want cl-points in the loop
            loop.push_back(current);
            clVertices.erase(current); // remove from list of unprocesser cl-verts
            std::vector<Edge> outEdges = hedi::out_edges(current, g); // find the edge to follow
            //if (outEdges.size() != 1 )
            //    std::cout << " outEdges.size() = " << outEdges.size() << "\n";
            assert( outEdges.size() == 1 ); // cl-points are allways at ends of intervals, so they have only one out-edge
            Edge currentEdge = outEdges[0]; 
            do { // following next, find a CL point 
                current = hedi::target( currentEdge, g);
                currentEdge = g[currentEdge].next;
            } while ( g[current].type != CL );
        } while (current!=first); // end the loop when we arrive at the start
        
        loops.push_back(loop); // add the processed loop to the master list of all loops
    }
}
        
// add a new CL-vertex to Weave, also adding it to the interval intersection-set, and to clVertices
Vertex Weave::add_cl_vertex( Point& position, Interval& ival, double ipos) {
    Vertex  v = hedi::add_vertex( VertexProps( position, CL ), g);
    ival.intersections2.insert( VertexPair( v, ipos) ); // ?? this makes Interval depend on the WeaveGraph type
    clVertices.insert(v);
    return v;
}

// given a vertex pair, find the vertex above and below the given vertex
std::pair<Vertex,Vertex> Weave::find_neighbor_vertices( VertexPair v_pair, Interval& ival) { 
    //VertexPair v_pair_x( v , v_position.x );
    VertexPairIterator x_tmp = ival.intersections2.lower_bound( v_pair ); // returns first that is not less than argument (equal or greater)
    assert(x_tmp != ival.intersections2.end() ); // we must find a lower_bound
    VertexPairIterator x_above = x_tmp;
    VertexPairIterator x_below = --x_tmp;
    std::pair<Vertex,Vertex> out;
    out.first = x_above->first; // vertex above v (xu)
    out.second = x_below->first; // vertex below v (xl)
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
            Vertex xv1 = add_cl_vertex( p1, xi, p1.x ); 
            Point p2( xf.point(xi.upper) );
            Vertex xv2 = add_cl_vertex( p2, xi, p2.x );
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
                                add_cl_vertex( p1, yi, p1.y );
                                Point p2( yf.point(yi.upper) );
                                add_cl_vertex( p2, yi, p2.y );
                                yi.in_weave = true;
                            }
                            // 3) intersection point, of type INT
                            Point v_position( yf.p1.x, xf.p1.y , xf.p1.z );
                            Vertex v = hedi::add_vertex( VertexProps( v_position, INT ), g);
                            
                            // 4) add edges 
                            
                            // find x-neighbors
                            Vertex x_u, x_l;
                            boost::tie( x_u, x_l ) = find_neighbor_vertices( VertexPair(v, v_position.x) , xi); 
                            
                            // these original edges will eventually be deleted!
                            Edge xe_lu = hedi::edge( x_l, x_u, g);
                            Edge xe_ul = hedi::edge( x_u, x_l, g);
                            Edge xe_lu_next = g[xe_lu].next;
                            Edge xe_lu_prev = g[xe_lu].prev; 
                            Edge xe_ul_next = g[xe_ul].next;
                            Edge xe_ul_prev = g[xe_ul].prev; 
                            
                            // find y-neighbors
                            Vertex y_u, y_l;
                            boost::tie( y_u, y_l ) = find_neighbor_vertices( VertexPair(v, v_position.y) , yi); 
                            
                            // the next/prev data we need
                            Edge ye_lu, ye_ul;
                            Edge ye_lu_next, ye_lu_prev ;
                            Edge ye_ul_next, ye_ul_prev ;
             
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

std::vector< std::vector<Point> > Weave::getLoops() const {
    std::vector< std::vector<Point> > loop_list;
    BOOST_FOREACH( std::vector<Vertex> loop, loops ) {
        std::vector<Point> point_list;
        BOOST_FOREACH( Vertex v, loop ) {
            point_list.push_back( g[v].position );
        }
        loop_list.push_back(point_list);
    }
    return loop_list;
}


// this can cause a build error when both face and vertex descriptors have the same type
// i.e. unsigned int (?)
// operator[] below "g[*itr].type" then looks for FaceProps.type which does not exist...
void Weave::printGraph() const {
    std::cout << " number of vertices: " << boost::num_vertices( g ) << "\n";
    std::cout << " number of edges: " << boost::num_edges( g ) << "\n";
    VertexItr it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( g );
    int n=0, n_cl=0, n_internal=0;
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        if ( g[*itr].type == CL )
            ++n_cl;
        else
            ++n_internal;
        ++n;
    }
    std::cout << " counted " << n << " vertices\n";
    std::cout << "          CL-nodes: " << n_cl << "\n";
    std::cout << "    internal-nodes: " << n_internal << "\n";
}
        
// string representation
std::string Weave::str() const {
    std::ostringstream o;
    o << "Weave2\n";
    o << "  " << xfibers.size() << " X-fibers\n";
    o << "  " << yfibers.size() << " Y-fibers\n";
    return o.str();
}
        

} // end weave2 namespace

} // end ocl namespace
// end file weave2.cpp
