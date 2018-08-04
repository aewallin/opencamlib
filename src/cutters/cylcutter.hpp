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

#ifndef CYL_CUTTER_H
#define CYL_CUTTER_H

#include <iostream>
#include <string>
#include <vector>

#include "millingcutter.hpp"
#include "bullcutter.hpp"

namespace ocl
{

///
/// \brief Cylindrical MillingCutter (flat-endmill)
///
/// defined by one parameter, the cutter diameter
class CylCutter : public MillingCutter {
    public:
        CylCutter();
        /// create CylCutter with diameter d and length l
        explicit CylCutter(double d, double l);
        /// offset of Cylinder is BullCutter
        MillingCutter* offsetCutter(double d) const {return new BullCutter(diameter+2*d, d, length+d);}
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, CylCutter c);        
        std::string str() const;
    protected:
        bool vertexPush(const Fiber& f, Interval& i, const Triangle& t) const;
        CC_CLZ_Pair singleEdgeDropCanonical(const Point& u1, const Point& u2) const;
        double height(double r) const {return ( r <= radius ) ? 0.0 : -1.0;}
        double width(double h) const {return radius;} 
};

} // end namespace
#endif
