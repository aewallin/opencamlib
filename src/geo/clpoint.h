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
#ifndef CLPOINT_H
#define CLPOINT_H

#include <string>
#include <iostream>

#include "point.h"
#include "ccpoint.h"
#include "triangle.h"

namespace ocl
{

///
/// \brief Cutter-Location (CL) point.
///
class CLPoint : public Point {
    public:
        CLPoint();
        /// CLPoint at (x,y,z)
        CLPoint(double x, double y, double z);
        /// CLPoint at (x,y,z) with CCPoint ccp
        CLPoint(double x, double y, double z, CCPoint& ccp);
        /// copy constructor
        CLPoint(const CLPoint& cl);
        virtual ~CLPoint();
        /// Pointer to the corresponding CCPoint
        CCPoint* cc; 
        /// string repr
        std::string str() const;
        /// if zin > z, lift CLPoint and update cc-point, and return true 
        int liftZ(double zin, CCPoint& ccp);
        /// if zin > z, lift CLPoint and return true.
        bool liftZ(const double zin);
        /// return true if cl-point above triangle
        bool below(const Triangle& t) const;
        /// return the CCPoint (for python)
        CCPoint getCC();
        /// assignment
        CLPoint &operator=(const CLPoint &p);
        /// addition
        const CLPoint operator+(const CLPoint &p) const;
        const CLPoint operator+(const Point &p) const;
};

} // end namespace
#endif
// end file clpoint.h
