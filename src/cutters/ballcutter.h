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

/* ********************************************************************
 *  SPHERE      ballcutter.cpp
 * ********************************************************************/
/// \brief Ball or Spherical MillingCutter (ball-nose endmill)
///
class BallCutter : public MillingCutter {
    public:
        BallCutter();
        /// create a BallCutter with diameter d (radius d/2) and length l
        explicit BallCutter(const double d, const double l);
        /// offset of Ball is Ball
        MillingCutter* offsetCutter(const double d) const;
        
        /// drop BallCutter against an edge p1-p2 at xy-distance d from CLPoint cl
        bool singleEdgeDrop(CLPoint& cl, const Point& p1, const Point& p2, const double d) const;
        
        /// push BallCutter along Fiber f against Triangle t, and update Interval i accordingly.   
        bool edgePush(const Fiber& f, Interval& i, const Triangle& t) const;
        
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, BallCutter c);
        /// string repr
        std::string str() const;
    protected:
        /// the height of the cutter at radius r
        double height(const double r) const;
        /// the width/radius of the cutter at height h
        double width(const double h) const; 
};

} // end namespace
#endif
