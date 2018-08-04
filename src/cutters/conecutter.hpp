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
#ifndef CONE_CUTTER_HPP
#define CONE_CUTTER_HPP

#include <iostream>
#include <string>
#include <vector>

#include "millingcutter.hpp"

namespace ocl
{

/// \brief Conical MillingCutter 
///
/// cone defined by diameter and the cone half-angle(in radians). sharp tip. 
/// 60 degrees or 90 degrees are common
class ConeCutter : public MillingCutter {
    public:
        ConeCutter();
        /// create a ConeCutter with specified maximum diameter and cone-angle
        /// for a 90-degree cone specify the half-angle  angle= pi/4
        ConeCutter(double d, double angle, double l=10);
        /// offset of ConeCutter is BallConeCutter (should be Ball-Cone-Bull??)
        MillingCutter* offsetCutter(double d) const;
        /// Cone facet-drop is special, since we can make contact with either the tip or the circular rim
        bool facetDrop(CLPoint &cl, const Triangle &t) const; 
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, ConeCutter c);
        std::string str() const;
        
    protected:
        CC_CLZ_Pair singleEdgeDropCanonical(const Point& u1, const Point& u2) const;
        
        bool facetPush(const Fiber& fib, Interval& i,  const Triangle& t) const;
            
        bool generalEdgePush(const Fiber& f, Interval& i,  const Point& p1, const Point& p2) const;
        
        bool circle_CC( double t, const Point& p1, const Point& p2, const Fiber& f, Interval& i) const;
        bool cone_CC(const Point& tang, 
                         const Point& tip, 
                         const Point& base, 
                         const Point& p1, 
                         const Point& p2,
                         const Fiber& f, 
                         Interval& i) const; 
                         
        double height(double r) const;
        double width(double h) const;
        /// the half-angle of the cone, in radians
        double angle;
};

} // end ocl namespace
#endif
// end conecutter.hpp
