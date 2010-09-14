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

#include "point.h"
#include "ccpoint.h"
#include "interval.h"

namespace ocl
{

/// a fiber is an infinite line in space along which the cutter can be pushed
/// into contact with a triangle. A Weave is built from many X-fibers and Y-fibers
class Fiber {
    public:
        Fiber(){ };
        /// create a Fiber between points p1 and p2
        Fiber(const Point &p1, const Point &p2);
        virtual ~Fiber() {};
        /// add an interval to this Fiber
        void addInterval(Interval& i);
        /// return true if Fiber already has interval i in it
        bool contains(Interval& i) const;
        /// return true if Interval i is completely missing (no overlaps) from Fiber
        bool missing(Interval& i) const;
        /// get rid of this (??)
        void condense();  // REMOVE??
        /// t-value corresponding to Point p
        double tval(Point& p) const;
        /// Point corresponding to t-value
        Point point(const double t) const;
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
    protected:
        /// set the direction(tangent) vector
        void calcDir();
};

} // end namespace
#endif
// end file fiber.h
