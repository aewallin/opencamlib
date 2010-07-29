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
#ifndef FIBER_H
#define FIBER_H

#include <vector>

#include <boost/graph/adjacency_list.hpp>

#include "point.h"
#include "ccpoint.h"
#include "numeric.h"
#include "weave_typedef.h"

namespace ocl
{

/// interval for use by fiber and weave
class Interval {
    public:
        Interval();
        Interval(double l, double u);
        virtual ~Interval();
        
        void updateUpper(double t, CCPoint& p);
        void updateLower(double t, CCPoint& p);
        bool outside(const Interval& i) const;
        bool inside(const Interval& i) const;
        bool empty() const;
        std::string str() const;
                
        CCPoint upper_cc;
        CCPoint lower_cc;
        double upper;
        double lower;
        bool in_weave; // flag for use by Weave::build()
        VertexDescriptor vert_lower; // weave vertex corresponding to lower CL-point
        VertexDescriptor vert_upper; // upper CL-point weave vertex
        std::set< VertexPair, VertexPairCompare > intersections; // store intersection coordinates with other fibers here
};



class Fiber {
    public:
        Fiber(){ };
        Fiber(const Point &p1, const Point &p2);
        virtual ~Fiber() {};
        /// add an interval to this Fiber
        void addInterval(Interval& i);
        /// return true if Fiber already has interval i in it
        bool contains(Interval& i) const;
        /// return true if Interval i is completely missing (no overlaps) from Fiber
        bool missing(Interval& i) const;
        void condense(); // get rid of this (??)
        /// t-value corresponding to Point p
        double tval(Point& p) const;
        /// Point corresponding to t-value
        Point point(double t) const;
        /// return intervals to python
        boost::python::list getInts() const;
        /// print the intervals
        void printInts() const;
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, const Fiber &f);
        
    // DATA
        /// start point
        Point p1;
        /// end point
        Point p2;
        /// direction vector (normalized)
        Point dir;
        /// the intervals in this Fiber
        std::vector<Interval> ints;
    private:
        // set the direction(tangent) vector
        void calcDir();
};

} // end namespace
#endif
// end file fiber.h
