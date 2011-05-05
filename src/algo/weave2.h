/*  $Id:  $
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
#ifndef WEAVE2_H
#define WEAVE2_H

#include <vector>

// #include <boost/graph/adjacency_list.hpp> // graph class

#include "point.h"
#include "ccpoint.h"
#include "numeric.h"
#include "fiber.h"
#include "weave2_typedef.h"

#include "halfedgediagram2.h"


namespace ocl
{

namespace weave2
{

/// vertex properties
struct VertexProps {
    VertexProps() {
        init();
    }
    /// construct vertex at position p with type t
    VertexProps( Point p, VertexType t) {
        position=p;
        type=t;
        init();
    }
    void init() {
        index = count;
        count++;
    }
    VertexType type;
// HE data
    /// the position of the vertex
    Point position;
    /// index of vertex
    int index;
    /// global vertex count
    static int count;
};
int VertexProps::count = 0;

/// edge properties
struct EdgeProps {
    EdgeProps() {}
    /// create edge with given next, twin, and face
    EdgeProps(Edge n, Edge t, Face f) { 
        next = n;
        twin = t;
        face = f;
    }
    /// the next edge, counterclockwise, from this edge
    Edge next; 
    /// the twin edge
    Edge twin;
    /// the face to which this edge belongs
    Face face; 
};

/// types of faces in the weave
enum FaceType {INCIDENT, NONINCIDENT};

/// properties of a face in the weave
struct FaceProps {
    /// create face with given edge, generator, and type
    FaceProps( Edge e , Point gen, FaceType t) {
        edge = e;
        type = t;
    }
    /// face index
    Face idx;
    /// one edge that bounds this face
    Edge edge;
    /// face type
    FaceType type;
};

                 
/// weave-graph, 2nd impl. based on HEDIGraph
class Weave {
    public:
        Weave() {}
        virtual ~Weave() {}

        /// add Fiber f to the graph
        /// each fiber should be either in the X or Y-direction
        /// FIXME: seprate addXFiber and addYFiber methods?
        void addFiber(Fiber& f) {
            if ( f.dir.xParallel() && !f.empty() ) {
                xfibers.push_back(f);
            } else if ( f.dir.yParallel() && !f.empty() ) {
                yfibers.push_back(f);
            } else if (!f.empty()) {
                assert(0); // fiber must be either x or y
            }
        }
        
        /// from the list of fibers, build a graph
        void build() {
            // 1) add CL-points of X-fiber (if not already in graph)
            // 2) add CL-points of Y-fiber (if not already in graph)
            // 3) add intersection point (if not already in graph) (will allways be new??)
            // 4) add edges. (if they provide new connections)
            //      ycl_lower <-> intp <-> ycl_upper
            //      xcl_lower <-> intp <-> xcl_upper
            // if this connects points that are already connected, then remove old edge and
            // provide this "via" connection
            BOOST_FOREACH( Fiber& xf, xfibers) {
                assert( !xf.empty() ); // no empty fibers please
                BOOST_FOREACH( Interval& xi, xf.ints ) {
                    double xmin = xf.point(xi.lower).x;
                    double xmax = xf.point(xi.upper).x;
                    assert( !xi.in_weave ); // this is the first time the x-interval is added!
                    xi.in_weave = true;
                    // add the X interval end-points to the weave
                    Point p( xf.point(xi.lower) );
                    Vertex v1 = add_vertex( p, CL , xi, p.x ); 
                    p = xf.point(xi.upper);
                    Vertex v2 = add_vertex( p, CL , xi, p.x );
                    Edge e1 = hedi::add_edge( v1, v2, g);
                    Edge e2 = hedi::add_edge( v2, v1, g);
                    hedi::twin_edges(e1, e2, g);
                    g[e1].next = e2;
                    g[e2].next = e1;
                    
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
                                        Point p( yf.point(yi.lower) );
                                        add_vertex( p, CL , yi, p.y );
                                        p = yf.point(yi.upper);
                                        add_vertex( p, CL , yi, p.y );
                                        yi.in_weave = true;
                                    }
                                    // 3) intersection point (this is always new, no need to check for existence??)
                                    Vertex  v;
                                    v = hedi::add_vertex(g);
                                    Point v_position( yf.p1.x, xf.p1.y , xf.p1.z );
                                    g[v].position = v_position;
                                    g[v].type = INT;
                                    
                                    xi.intersections2.insert( VertexPair( v, v_position.x ) );
                                    yi.intersections2.insert( VertexPair( v, v_position.y ) );
                                    
                                    // 4) add edges 
                                    // find the X-vertices above and below the new vertex v.
                                    VertexPair v_pair_x( v , v_position.x );
                                    VertexPairIterator x_tmp = xi.intersections2.lower_bound( v_pair_x );
                                    assert(x_tmp != xi.intersections2.end() );
                                    VertexPairIterator x_above = x_tmp;
                                    VertexPairIterator x_below = x_tmp;
                                    Vertex x_u = (++x_above)->first;
                                    Vertex x_l = (--x_below)->first;
                                    // if x_above and x_below are already connected, we need to remove that edge.
                                    assert( hedi::has_edge( x_l, x_u, g ) );
                                    Edge e = hedi::edge( x_l, x_u, g);
                                    
                                    //hedi::insert_vertex_in_edge(v, e, g);

                                    // now do the same thing in the y-direction.
                                    VertexPair v_pair_y( v , v_position.y );
                                    VertexPairIterator y_tmp = yi.intersections2.lower_bound( v_pair_y );
                                    assert(y_tmp != yi.intersections2.end() );
                                    VertexPairIterator y_above = y_tmp;
                                    VertexPairIterator y_below = y_tmp;
                                    //++y_above;
                                    //--y_below;
                                    Vertex y_u = (++y_above)->first;
                                    Vertex y_l = (--y_below)->first;
                                    
                                    // if y_l and y_u are already connected, we need to remove that edge.
                                    if ( hedi::has_edge( y_l, y_u, g ) ) {
                                        assert( hedi::has_edge( y_u, y_l, g ) ); // twin must also exist
                                        Edge e_lu = hedi::edge( y_l, y_u, g);
                                        Edge e_ul = g[e_lu].twin;
                                        Edge y_lu_next = g[e_lu].next;
                                        Edge y_lu_prev = hedi::previous_edge(e_lu,g);
                                        
                                        Edge y_ul_next = g[e_ul].next;
                                        Edge y_ul_prev = hedi::previous_edge(e_ul,g);
                                        
                                        boost::remove_edge( y_l, y_u, g);
                                        boost::remove_edge( y_u, y_l, g);
                                    }
                                    Edge e_lv = hedi::add_edge( y_l, v, g);
                                    
                                    //boost::remove_edge( y_above->first, y_below->first, g);
                                    //boost::add_edge( y_above->first , v , g );
                                    //boost::add_edge( y_below->first , v , g );
                                    
                                }
                            } // end y interval loop
                        } // end if(potential intersection)
                    } // end y fiber loop
                } // x interval loop
               
            } // end X-fiber loop
             
        }
        
        /// add vertex to weave
        /// sets position, type, and inserts the VertexPair into Interval::intersections
        Vertex add_vertex( Point& position, VertexType type, Interval& interv, double ipos) {
            Vertex  v = hedi::add_vertex(VertexProps( position, type ), g);
            interv.intersections2.insert( VertexPair( v, ipos) ); // ?? this makes Interval depend on the WeaveGraph type
            return v;
        }
        
        
        /// run planar_face_traversal to get the waterline loops
        void face_traverse() { }
        
        /// retrun list of loops
        std::vector< std::vector<Point> > getLoops() const {
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
        
        /// string representation
        std::string str() const {
            std::ostringstream o;
            o << "Weave2\n";
            o << "  " << xfibers.size() << " X-fibers\n";
            o << "  " << yfibers.size() << " Y-fibers\n";
            return o.str();
        }
        
        /// print out information about the graph
        void printGraph() const {
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
        
    protected:        
// DATA
        /// the weave-graph
        WeaveGraph g;
        /// output: list of loops in this weave
        std::vector< std::vector<Vertex> > loops;
        /// the X-fibers
        std::vector<Fiber> xfibers;
        /// the Y-fibers
        std::vector<Fiber> yfibers;
       
};

} // end weave2 namespace

} // end ocl namespace
#endif
// end file weave2.h
