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

#ifndef CONE_CUTTER_H
#define CONE_CUTTER_H

#include <iostream>
#include <string>
#include <vector>

#include "millingcutter.h"

namespace ocl
{

/// \brief Conical MillingCutter 
///
/// cone defined by diameter and the cone half-angle(in radians). sharp tip. 
/// 60 degrees or 90 degrees are common
class ConeCutter : public MillingCutter {
    public:
        /// default constructor
        ConeCutter();
        /// create a ConeCutter with specified maximum diameter and cone-angle
        /// for a 90-degree cone specify the half-angle  angle= pi/4
        ConeCutter(const double d, const double angle);
        
        MillingCutter* offsetCutter(const double d) const;
        // drop-cutter
        bool facetDrop(CLPoint &cl, const Triangle &t) const; // FIXME, handle in base-class?
        bool singleEdgeDrop(CLPoint& cl, const Point& p1, const Point& p2, const double d) const;
        
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, ConeCutter c);
        std::string str() const;
        
    protected:
        double height(const double r) const;
        
        /// the half-angle of the cone, in radians
        double angle;
};

} // end namespace
#endif
// end conecutter.h
