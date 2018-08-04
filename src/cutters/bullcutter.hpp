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

#ifndef BULL_CUTTER_H
#define BULL_CUTTER_H

#include <iostream>
#include <string>
#include <vector>

#include "millingcutter.hpp"
#include "ellipse.hpp"

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
        BullCutter(double diameter, double radius, double length);
        /// offset of Bull is Bull
        MillingCutter* offsetCutter(double offset) const;
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, BullCutter c);
        std::string str() const;
        
    protected:
        
        bool generalEdgePush(const Fiber& f, Interval& i,  const Point& p1, const Point& p2) const;
        CC_CLZ_Pair singleEdgeDropCanonical(const Point& u1, const Point& u2) const;
        double height(double r) const;
        double width(double h) const; 
        /// radius of cylindrical part of cutter
        double radius1;
        /// tube radius of torus
        double radius2;
};

} // end namespace
#endif
