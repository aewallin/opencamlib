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
#ifndef FIBER_HPP
#define FIBER_HPP

#include <vector>

#include "point.hpp"
#include "interval.hpp"

namespace ocl
{

/// a fiber is an infinite line in space along which the cutter can be pushed
/// into contact with a triangle. A Weave is built from many X-fibers and Y-fibers.
/// might be called a Dexel also in some papers/textbooks.
class Fiber {
    public:
        Fiber() {ints.clear();}
        /// create a Fiber between points p1 and p2
        Fiber(const Point &p1, const Point &p2);
        virtual ~Fiber() {}
        /// add an interval to this Fiber
        void addInterval(Interval& i);
        /// return true if Fiber already has interval i in it
        bool contains(Interval& i) const;
        /// return true if Interval i is completely missing (no overlaps) from Fiber
        bool missing(Interval& i) const;
       
        /// t-value corresponding to Point p
        double tval(Point& p) const;
        /// Point corresponding to t-value
        Point point(double t) const;
        /// print the intervals
        void printInts() const;
        /// return true if the Fiber contains no intervals
        bool empty() const {return ints.empty();}
        /// return number of intervals
        unsigned int size() const {return ints.size();}
        
        /// return the upper cl-point of interval n
        Point upperCLPoint(unsigned int n) const {
            return point(ints[n].upper);
        }
        /// return the lower cl-point of interval n
        Point lowerCLPoint(unsigned int n) const {
            return point(ints[n].lower);
        }
        
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, const Fiber &f);
        
        bool operator==(const Fiber& other ) const { 
            if( (this->p1 == other.p1) && (this->p2 == other.p2) ) 
                return true; 
            else 
                return false; 
        }

    // DATA
        Point p1;  ///< start point
        Point p2;  ///< end point
        Point dir; ///< direction vector (normalized)
        std::vector<Interval> ints; ///< the intervals in this Fiber
    protected:
        /// set the direction(tangent) vector
        void calcDir();
};

} // end namespace
#endif
// end file fiber.h
