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

#include <boost/graph/adjacency_list.hpp>

#include "point.h"
#include "ccpoint.h"
#include "numeric.h"
#include "fiber.h"

namespace ocl
{


/// weave-graph
class Weave {
    public:
        Weave();
        virtual ~Weave() {};
        
        void addFiber(Fiber& f);
        void build();
        std::vector<Fiber> fibers;
        std::vector<Fiber> xfibers;
        std::vector<Fiber> yfibers;
        std::string str() const;
        void printGraph() const;
        
        typedef boost::adjacency_list< boost::listS,    // out-edges stored in a std::list
                               boost::vecS,     // vertex set stored in a std::vector
                               boost::undirectedS,  // an un directed  graph.
                               // vertex properties:
                               boost::property< boost::vertex_name_t , Point,
                                        boost::property< boost::vertex_color_t, bool > >,
                               // edge properties:
                               boost::property< boost::edge_weight_t, double >
                               > WeaveGraph; 
                               
                    // graph_traits< G >::edge_parallel_category
                    // tags: disallow_parallel_edge_tag
        typedef boost::graph_traits< WeaveGraph >::vertex_descriptor VertexDescriptor;
        
        WeaveGraph g;
        
                           
};



} // end namespace
#endif
// end file weave.h
