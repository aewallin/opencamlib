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
#ifndef WEAVE_H
#define WEAVE_H

#include <vector>

#include <boost/graph/adjacency_list.hpp> // graph class
#include <boost/graph/planar_face_traversal.hpp>
#include <boost/python.hpp>

#include "point.h"
#include "ccpoint.h"
#include "numeric.h"
#include "fiber.h"
#include "weave_typedef.h"

namespace ocl
{

// see weave_typedef.h for boost-graph classes                         
                    
/// weave-graph
class Weave {
    public:
        Weave();
        virtual ~Weave() {};
        void add_vertex( Point& position, VertexType t, Interval& i, double ipos);
        void addFiber(Fiber& f);
        /// sort into X and Y fibers
        void sort_fibers(); 
        /// from the list of fibers, build a graph
        void build();
        /// build a planar embedding of the graph
        void build_embedding(PlanarEmbedding& e);
        void face_traverse();
        std::vector<Weave> split_components();
        
        // python debug/test interface:
        boost::python::list get_components();
        boost::python::list getCLPoints() const;
        boost::python::list getIPoints() const;
        boost::python::list getEdges() const;
        boost::python::list py_getLoops() const;
        std::vector< std::vector<Point> > getLoops() const;
        // string repr
        std::string str() const;
        void printGraph() const;
        void print_embedding(PlanarEmbedding& e);
        
        // DATA
        std::vector<Fiber> fibers;
        std::vector<Fiber> xfibers;
        std::vector<Fiber> yfibers;
        
        WeaveGraph g;
        std::vector< std::vector<VertexDescriptor> > loops;
};



} // end namespace
#endif
// end file weave.h
