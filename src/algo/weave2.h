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
#ifndef WEAVE2_H
#define WEAVE2_H

#include <vector>

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
    /// previous edge, to make Weave::build() faster, since we avoid calling hedi::previous_edge() 
    Edge prev;
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
        void addFiber(Fiber& f);
        
        /// from the list of fibers, build a graph
        void build();
        
        /// add vertex to weave
        /// sets position, type, and inserts the VertexPair into Interval::intersections
        Vertex add_vertex( Point& position, VertexType type, Interval& interv, double ipos);

        /// run planar_face_traversal to get the waterline loops
        void face_traverse() { 
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
        /// set of CL-points
        std::set<Vertex> clVertices;
};

} // end weave2 namespace

} // end ocl namespace
#endif
// end file weave2.h
