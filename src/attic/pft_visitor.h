/*  $Id$
 * 
 *  Copyright (c) 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com).
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

#ifndef PFT_VISITOR_H
#define PFT_VISITOR_H

#include <vector>
#include <boost/graph/planar_face_traversal.hpp>

#include "weave_typedef.h"
#include "weave.h"

namespace ocl
{
    
/// \brief a visitor-object for the Boost Graph Library planar_face_traversal() algorithm.
/// the visitor starts recording vertices when a face with a CL-vertex is encountered
/// and pushes the complete loop back to Weave when the face traversal is done. 
/// Faces with non-CL-vertices are traversed but nothing is done/recorded.
struct vertex_output_visitor : public boost::planar_face_traversal_visitor
{
    /// create a visitor-object
    vertex_output_visitor(Weave* w, WeaveGraph& g) : g_(g) , w_(w) {}
    
    /// called when we begin traversing a new face, clear contents of current_loop
    void begin_face() { 
        current_loop.clear();
    } 
    
    /// called when we encouter a new vertex. if it is of CL-type we push it into the loop
    void next_vertex(WeaveVertex v) {
        if ( g_[v].type == CL ) { // could instead check for vertex degree?
            current_loop.push_back(v);
        }
    }
    /// called when face-traversal of one face is complete. we then push the current loop into the list of loops
    void end_face() { 
        if (!current_loop.empty())
            w_->loops.push_back( current_loop );
    } 
    
    /// the graph we operate on
    WeaveGraph& g_;
    /// the corresponding weave-object which holds the graph
    Weave* w_;
    /// a list of CL-vertices in the current face
    std::vector<WeaveVertex> current_loop;
};

} // end namespace
#endif
