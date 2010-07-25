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
#include <boost/graph/graphviz.hpp>
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
        void addFiber(Fiber& f);
        void build();
        void build2();
        void invert();
        void sort_fibers();
        void add_xy_fibers_to_g2();
        std::vector<Fiber> fibers;
        std::vector<Fiber> xfibers;
        std::vector<Fiber> yfibers;
        std::string str() const;
        void printGraph() const;
        void printGraph2() const;
        void writeGraph() const; // write to dot file
        // python debug/test interface:
        boost::python::list getCLPoints() const;
        boost::python::list getIPoints() const;
        WeaveGraph g;
        InvWeaveGraph g2;
};



} // end namespace
#endif
// end file weave.h
