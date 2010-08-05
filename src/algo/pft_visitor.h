/*  $Id$
 * 
 *  Copyright 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com)
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

#ifndef PFT_VISITOR_H
#define PFT_VISITOR_H

#include <vector>
#include <boost/graph/planar_face_traversal.hpp>

#include "weave_typedef.h"
#include "weave.h"

namespace ocl
{
    
    
struct vertex_output_visitor : public boost::planar_face_traversal_visitor
{
    vertex_output_visitor(Weave* w, WeaveGraph& g) : g_(g) , w_(w) {}
    
    void begin_face() { 
        current_loop.clear();
    } 
    
    void next_vertex(VertexDescriptor v) { 
        if ( boost::get( boost::vertex_type, g_, v) == CL ) { // could instead check for vertex degree?
            current_loop.push_back(v);
        }
    }
    
    void end_face() { 
        if (!current_loop.empty())
            w_->loops.push_back( current_loop );
    } 
    
    WeaveGraph& g_;
    Weave* w_;
    std::vector<VertexDescriptor> current_loop;
};

} // end namespace
#endif
