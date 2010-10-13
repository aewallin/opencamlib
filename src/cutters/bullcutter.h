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

#ifndef BULL_CUTTER_H
#define BULL_CUTTER_H

#include <iostream>
#include <string>
#include <vector>

#include "millingcutter.h"

namespace ocl
{

/// \brief Bull-nose or Toroidal MillingCutter (filleted endmill)
///
/// defined by the cutter diameter and by the corner radius
///
class BullCutter : public MillingCutter {
    public:
        BullCutter();
        /// Create bull-cutter with diamter d, corner radius r, and length l.
        BullCutter(const double diameter, const double radius, const double length);
        /// offset of Bull is Bull
        MillingCutter* offsetCutter(const double offset) const;
        
        bool singleEdgeDrop(CLPoint& cl, const Point& p1, const Point& p2, const double d) const;
        bool edgePush(const Fiber& f, Interval& i, const Triangle& t) const;
        
        friend std::ostream& operator<<(std::ostream &stream, BullCutter c);
        std::string str() const;
        
    protected:
        double height(const double r) const;
        double width(const double h) const; 
        /// radius of cylindrical part of cutter
        double radius1;
        /// tube radius of torus
        double radius2;
};

} // end namespace
#endif
