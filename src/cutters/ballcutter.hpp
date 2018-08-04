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
#ifndef BALL_CUTTER_H
#define BALL_CUTTER_H

#include <iostream>
#include <string>
#include <vector>

#include "millingcutter.hpp"
#include "numeric.hpp"

namespace ocl
{

/// \brief Ball or Spherical MillingCutter (ball-nose endmill)
///
class BallCutter : public MillingCutter {
    public:
        BallCutter();
        /// create a BallCutter with diameter d (radius d/2) and length l
        explicit BallCutter(double d, double l);
        /// offset of Ball is Ball
        MillingCutter* offsetCutter(double d) const {return  new BallCutter(diameter+2*d, length+d);}
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, BallCutter c);
        std::string str() const;
    protected:
        CC_CLZ_Pair singleEdgeDropCanonical(const Point& u1, const Point& u2) const;
        bool generalEdgePush(const Fiber& f, Interval& i,  const Point& p1, const Point& p2) const;
        /// calculate CC-point and update Interval i
        bool calcCCandUpdateInterval( double t, const Point& p1, const Point& p2, const Fiber& f, Interval& i) const;
        double height(double r) const {return radius - sqrt( square(radius) - square(r) );}
        double width(double h) const {return ( h >= radius ) ? radius : sqrt( square(radius) - square(radius-h) ); }
};

} // end namespace
#endif
// end ballcutter.h
