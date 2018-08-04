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
#ifndef SIMPLE_WEAVE_HPP
#define SIMPLE_WEAVE_HPP

#include <vector>

#include "weave.hpp"

namespace ocl {

namespace weave {

class SimpleWeave : public Weave {
    public:
        SimpleWeave() {}
        virtual ~SimpleWeave() {} 
        void build();
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
        std::pair<Vertex,Vertex> find_neighbor_vertices( VertexPair v_pair, Interval& ival);
};

} // end weave namespace

} // end ocl namespace
#endif
// end file simple_weave.hpp
