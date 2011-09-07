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
#ifndef SIMPLE_WEAVE_HPP
#define SIMPLE_WEAVE_HPP

#include <vector>

#include "weave.hpp"

#include "point.hpp"
#include "fiber.hpp"
#include "weave_typedef.hpp"
#include "halfedgediagram.hpp"

namespace ocl {

namespace weave {

/// weave-graph, 2nd impl. based on HEDIGraph
/// see http://www.anderswallin.net/2011/05/weave-notes/
class SimpleWeave : public Weave {
    public:
        SimpleWeave() {}
        
        virtual ~SimpleWeave() {} // this special descrutctor required because hedi inherits from BGL::adjacency_list. FIXME.

        //void addFiber(Fiber& f);
        
        void build();
        
        /// new smarter version of build() 
        //void build2();
        
        //void add_vertices_x();
        //void add_vertices_y();
        //bool crossing_x( Fiber& yf, std::vector<Interval>::iterator& yi, Interval& xi, Fiber& xf );
        //bool crossing_y( Fiber& xf, std::vector<Interval>::iterator& xi, Interval& yi, Fiber& yf );
        //std::vector<Interval>::iterator find_interval_crossing_x( Fiber& xf, Fiber& yf );
        //std::vector<Interval>::iterator find_interval_crossing_y( Fiber& xf, Fiber& yf );
        //bool add_vertex(    Fiber& xf, Fiber& yf,
        //                    std::vector<Interval>::iterator xi, std::vector<Interval>::iterator yi,
        //                    enum VertexType type );
        //void add_all_edges();

        /// run planar_face_traversal to get the waterline loops
        //void face_traverse();
        
        /// retrun list of loops
        //std::vector< std::vector<Point> > getLoops() const;
        
        /// string representation
        //std::string str() const;
        
        /// print out information about the graph
        //void printGraph() const;
        
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
        //std::pair<Vertex,Vertex> find_neighbor_vertices( VertexPair v_pair, Interval& ival, bool above_equality );
        std::pair<Vertex,Vertex> find_neighbor_vertices( VertexPair v_pair, Interval& ival);
// DATA
        /// the weave-graph
        //WeaveGraph g;
        /// output: list of loops in this weave
        //std::vector< std::vector<Vertex> > loops;
        /// the X-fibers
        //std::vector<Fiber> xfibers;
        /// the Y-fibers
        //std::vector<Fiber> yfibers;
        
        /// set of CL-points
        //std::set<Vertex> clVertexSet;
};

} // end weave namespace

} // end ocl namespace
#endif
// end file simple_weave.hpp
