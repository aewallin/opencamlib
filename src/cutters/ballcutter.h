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

#ifndef BALL_CUTTER_H
#define BALL_CUTTER_H

#include <iostream>
#include <string>
#include <vector>

#include "millingcutter.h"

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
        MillingCutter* offsetCutter(double d) const;
        
        /// drop BallCutter against an edge u1-u2 
        CC_CLZ_Pair singleEdgeContact(const Point& u1, const Point& u2) const;
        
        /// push BallCutter along Fiber f against Triangle t, and update Interval i accordingly.   
        bool edgePush(const Fiber& f, Interval& i, const Triangle& t) const;
        
        friend std::ostream& operator<<(std::ostream &stream, BallCutter c);
        std::string str() const;
    protected:
        double height(double r) const;
        double width(double h) const; 
};

} // end namespace
#endif
// end ballcutter.h
