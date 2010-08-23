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

#include <boost/python.hpp>
#include "millingcutter.h"

namespace ocl
{

/* ********************************************************************
 *  SPHERE      ballcutter.cpp
 * ********************************************************************/
/// \brief Ball or Spherical MillingCutter (ball-nose endmill)
///
/// defined by one parameter. the cutter diameter.
/// the sphere radius will be diameter/2
class BallCutter : public MillingCutter {
    public:
        BallCutter();
        /// create a BallCutter with diameter d and radius d/2
        explicit BallCutter(const double d);
        
        MillingCutter* offsetCutter(const double d) const;
        
        // dropCutter methods
        int vertexDrop(CLPoint &cl, const Triangle &t) const;
        int facetDrop(CLPoint &cl, const Triangle &t) const;
        int edgeDrop(CLPoint &cl, const Triangle &t) const;
        
        // pushCutter methods   
        bool vertexPush(const Fiber& f, Interval& i, const Triangle& t) const;
        bool facetPush(const Fiber& f, Interval& i, const Triangle& t) const;
        bool edgePush(const Fiber& f, Interval& i, const Triangle& t) const;
        
        
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, BallCutter c);
        /// string repr
        std::string str() const;
        
};

} // end namespace
#endif
