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
#ifndef WEAVE_HPP
#define WEAVE_HPP

#include <vector>

#include "point.hpp"
#include "fiber.hpp"
#include "weave_typedef.hpp"
#include "halfedgediagram.hpp"

namespace ocl {

namespace weave {

/// vertex properties
struct VertexProps {
    VertexProps() {
        init();
    }
    /// construct vertex at position p with type t
    VertexProps( Point p, VertexType t, std::vector<Interval>::iterator x, std::vector<Interval>::iterator y )
    : xi( x ), yi( y ) {
        position=p;
        type=t;
        init();
    }

    VertexProps( Point p, VertexType t ) {
        position = p;
        type = t;
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
    // x interval
    std::vector<Interval>::iterator xi;
    // y interval
    std::vector<Interval>::iterator yi;
};

/// edge properties
struct EdgeProps {
    EdgeProps() {}
    /// the next edge, counterclockwise, from this edge
    Edge next;
    /// previous edge, to make Weave::build() faster, since we avoid calling hedi::previous_edge() 
    Edge prev;
    /// the twin edge
    Edge twin;

};


/// properties of a face in the weave
struct FaceProps {
    /// create face with given edge, generator, and type
    FaceProps( Edge e ) {
        edge = e;
    }
    /// face index
    Face idx;
    /// one edge that bounds this face
    Edge edge;
};

// comparison functor for x-fibers
struct XFiberCompare {
    /// comparison operator
    bool operator() (const Fiber& lhs, const Fiber& rhs) const
    { return lhs.p1.y < rhs.p1.y ;} // sort X-fibers by their y-coordinate
};

// comparison functor for y-fibers
struct YFiberCompare {
    /// comparison operator
    bool operator() (const Fiber& lhs, const Fiber& rhs) const
    { return lhs.p1.x < rhs.p1.x ;} // sort Y-fibers by their x-coordinate
};

                 
/// weave-graph, 2nd impl. based on HEDIGraph
/// see http://www.anderswallin.net/2011/05/weave-notes/
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
        
        /// new smarter version of build() 
        void build2();

        //void add_interval(Fiber& xf, Interval& xi);
        
        void add_vertices_x();
        void add_vertices_y();
        bool crossing_x( Fiber& yf, std::vector<Interval>::iterator& yi, Interval& xi, Fiber& xf );
        bool crossing_y( Fiber& xf, std::vector<Interval>::iterator& xi, Interval& yi, Fiber& yf );
        std::vector<Interval>::iterator find_interval_crossing_x( Fiber& xf, Fiber& yf );
        std::vector<Interval>::iterator find_interval_crossing_y( Fiber& xf, Fiber& yf );
        bool add_vertex(    Fiber& xf, Fiber& yf,
                            std::vector<Interval>::iterator xi, std::vector<Interval>::iterator yi,
                            enum VertexType type );
        void add_all_edges();

        /// run planar_face_traversal to get the waterline loops
        void face_traverse();
        
        /// retrun list of loops
        std::vector< std::vector<Point> > getLoops() const;
        
        /// string representation
        std::string str() const;
        
        /// print out information about the graph
        void printGraph() const;
        
    protected:       
    
        /// add CL vertex to weave
        /// sets position, type, and inserts the VertexPair into Interval::intersections
        /// also adds the CL-vertex to clVertices, a list of cl-verts to be processed during face_traverse()
        Vertex add_cl_vertex( const Point& position, Interval& interv, double ipos);
        
        /// add INT vertex to weave
        /// the new vertex at v_position has neighbor vertices x_lower and x_upper in the x-direction on interval xi
        /// and y_lower, y_upper in the y-direction of interval yi
        /// Create new edges and delete old ones
        void add_int_vertex(    const Point& v_position,
                                Vertex& x_l, 
                                Vertex& x_u, 
                                Vertex& y_l,
                                Vertex& y_u,
                                Interval& xi,
                                Interval& yi );

        
        /// given a vertex in the graph, find its upper and lower neighbor vertices
        std::pair<Vertex,Vertex> find_neighbor_vertices( VertexPair v_pair, Interval& ival, bool above_equality );
         
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
        std::set<Vertex> clVertexSet;
};

} // end weave namespace

} // end ocl namespace
#endif
// end file weave.hpp
