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

#include "point.h"
#include "ccpoint.h"
#include "numeric.h"
#include "fiber.h"
#include "weave_typedef.h"

namespace ocl
{

// see weave_typedef.h for boost-graph typedefs                       
                    
/// weave-graph
class Weave {
    public:
        Weave();
        virtual ~Weave() {}

        /// add Fiber f to the graph
        void addFiber(Fiber& f);
        /// from the list of fibers, build a graph
        void build();
        /// split this Weave into its disconnected components
        std::vector<Weave> split_components();
        /// run planar_face_traversal to get the waterline points in correct order
        void face_traverse();
        /// retrun list of loops
        std::vector< std::vector<Point> > getLoops() const;
        
        /// string representation
        std::string str() const;
        /// print out information about the graph
        void printGraph() const;
        /// print out the planar embedding
        void print_embedding(WeavePlanarEmbedding& e);
        
    // DATA
        /// output: list of loops in this weave
        std::vector< std::vector<WeaveVertex> > loops;
    protected:
        /// build a planar embedding of the graph, for the BGL planarity test
        void build_embedding(WeavePlanarEmbedding& e);
        
        /// add vertex to weave
        /// sets position, type, and inserts the VertexPair into Interval::intersections
        void add_vertex( Point& position, WeaveVertexType t, Interval& i, double ipos);
        
        /// run a planarity test on the graph
        bool isPlanar() const;
        /// the weave-graph
        WeaveGraph g;
        /// the X-fibers
        std::vector<Fiber> xfibers;
        /// the Y-fibers
        std::vector<Fiber> yfibers;
       
};

} // end namespace
#endif
// end file weave.h
