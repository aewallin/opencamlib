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
#ifndef SMART_WEAVE_HPP
#define SMART_WEAVE_HPP

#include <vector>

#include "weave.hpp"

namespace ocl {

namespace weave {

class SmartWeave : public Weave {
    public:
        SmartWeave() {}
        virtual ~SmartWeave() {}
        void build();
    protected:       
        void add_vertices_x();
        void add_vertices_y();
        bool crossing_x( Fiber& yf, std::vector<Interval>::iterator& yi, Interval& xi, Fiber& xf );
        bool crossing_y( Fiber& xf, std::vector<Interval>::iterator& xi, Interval& yi, Fiber& yf );
        std::vector<Interval>::iterator find_interval_crossing_x( Fiber& xf, Fiber& yf );
        std::vector<Interval>::iterator find_interval_crossing_y( Fiber& xf, Fiber& yf );
        Vertex add_cl_vertex( const Point& position, Interval& ival, double ipos);
        bool add_vertex(    Fiber& xf, 
                            Fiber& yf,
                            std::vector<Interval>::iterator xi, 
                            std::vector<Interval>::iterator yi,
                            enum VertexType type );
        void add_all_edges();
        std::pair<Vertex,Vertex> find_neighbor_vertices( VertexPair v_pair, Interval& ival, bool above_equality );
};

} // end weave namespace

} // end ocl namespace
#endif
// end file smart_weave.hpp
