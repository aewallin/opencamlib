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

/*
struct VertexProps; 
struct EdgeProps;
struct FaceProps;
typedef unsigned int Face;  
  
// the graph type for the weave
typedef HEDIGraph<     boost::listS,             // out-edges stored in a std::list
                       boost::listS,             // vertex set stored here
                       boost::bidirectionalS,    // bidirectional graph.
                       VertexProps,              // vertex properties
                       EdgeProps,                // edge properties
                       FaceProps,                // face properties
                       boost::no_property,       // graph properties
                       boost::listS             // edge storage
                       > WeaveGraph;

typedef boost::graph_traits< WeaveGraph >::vertex_descriptor  Vertex;
typedef boost::graph_traits< WeaveGraph >::vertex_iterator    VertexItr;
typedef boost::graph_traits< WeaveGraph >::edge_descriptor    Edge;
typedef boost::graph_traits< WeaveGraph >::edge_iterator      EdgeItr;
typedef boost::graph_traits< WeaveGraph >::out_edge_iterator  OutEdgeItr;
typedef boost::graph_traits< WeaveGraph >::adjacency_iterator AdjacencyItr;

/// intersections between intervals are stored as a VertexPair
/// pair.first is a vertex descriptor of the weave graph
/// pair.second is the coordinate along the fiber of the intersection
typedef std::pair< Vertex, double > VertexPair;

/// compare based on pair.second, the coordinate of the intersection
struct VertexPairCompare {
    /// comparison operator
    bool operator() (const VertexPair& lhs, const VertexPair& rhs) const
    { return lhs.second < rhs.second ;}
};

/// intersections stored in this set (for rapid finding of neighbors etc)
typedef std::set< VertexPair, VertexPairCompare > VertexIntersectionSet;

typedef VertexIntersectionSet::iterator VertexPairIterator;    


/// vertex type: CL-point, internal point, adjacent point
enum VertexType {CL, CL_DONE, ADJ, TWOADJ, INT };
*/

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
class Weave2 {
    public:
        Weave2() {}
        virtual ~Weave2() {}

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
                    assert( !xi.in_weave );
                    // add the interval end-points to the weave
                    Point p = xf.point(xi.lower);
                    hedi::add_vertex( VertexProps(p, CL ), g ); 
                    // p, CL , xi, p.x ); // add_vertex( Point& position, VertexType t, Interval& i, double ipos)
                    p = xf.point(xi.upper);
                    hedi::add_vertex( VertexProps(p, CL ), g ); 
                    // add_vertex( p, CL , xi, p.x );
                    // xi.intersections std::set of intersections with this interval.

                    BOOST_FOREACH( Fiber& yf, yfibers ) { // loop through all y-fibers for all x-intervals
                        if ( (xmin <= yf.p1.x) && ( yf.p1.x <= xmax ) ) {// potential intersection between y-fiber and x-interval
                            BOOST_FOREACH( Interval& yi, yf.ints ) {
                                double ymin = yf.point(yi.lower).y;
                                double ymax = yf.point(yi.upper).y;
                                if ( (ymin <= xf.p1.y) && (xf.p1.y <= ymax) ) { 
                                    // there is an actual intersection btw x-interval and y-interval
                                    // X interval xi on fiber xf intersects with Y interval yi on fiber yf
                                    // intersection is at ( yf.p1.x, xf.p1.y , xf.p1.z )
                                    if (!yi.in_weave) { // add y-interval endpoints to weave
                                        Point p = yf.point(yi.lower);
                                        hedi::add_vertex( VertexProps(p, CL ), g ); 
                                        //add_vertex( p, CL , yi, p.y );
                                        p = yf.point(yi.upper);
                                        hedi::add_vertex( VertexProps(p, CL ), g ); 
                                        //add_vertex( p, CL , yi, p.y );
                                        yi.in_weave = true;
                                    }
                                    // 3) intersection point (this is always new, no need to check for existence??)
                                    Vertex  v;
                                    v = hedi::add_vertex(g);
                                    Point v_position = Point( yf.p1.x, xf.p1.y , xf.p1.z) ;
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
                                    ++x_above;
                                    --x_below;
                                    // if x_above and x_below are already connected, we need to remove that edge.
                                    boost::remove_edge( x_above->first, x_below->first, g);
                                    boost::add_edge( x_above->first , v , g );
                                    boost::add_edge( x_below->first , v , g );
                                    
                                    // now do the same thing in the y-direction.
                                    VertexPair v_pair_y( v , v_position.y );
                                    VertexPairIterator y_tmp = yi.intersections2.lower_bound( v_pair_y );
                                    assert(y_tmp != yi.intersections2.end() );
                                    VertexPairIterator y_above = y_tmp;
                                    VertexPairIterator y_below = y_tmp;
                                    ++y_above;
                                    --y_below;
                                    // if y_above and y_below are already connected, we need to remove that edge.
                                    boost::remove_edge( y_above->first, y_below->first, g);
                                    boost::add_edge( y_above->first , v , g );
                                    boost::add_edge( y_below->first , v , g );
                                    
                                }
                            } // end y interval loop
                        } // end if(potential intersection)
                    } // end y fiber loop
                } // x interval loop
               
            } // end X-fiber loop
             
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
