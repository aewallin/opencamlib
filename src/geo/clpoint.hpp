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
#ifndef CLPOINT_H
#define CLPOINT_H

#include <atomic>
#include <string>
#include <iostream>

#include "point.hpp"
#include "ccpoint.hpp"
#include "triangle.hpp"

namespace ocl
{

///
/// \brief Cutter-Location (CL) point.
///
class CLPoint : public Point {
    public:
        /// CLPoint at (0,0,0)
        CLPoint();
        /// CLPoint at (x,y,z)
        CLPoint(double x, double y, double z);
        /// CLPoint at (x,y,z) with CCPoint ccp
        CLPoint(double x, double y, double z, CCPoint& ccp);
        /// copy constructor
        CLPoint(const CLPoint& cl);
        /// cl-point at Point p
        CLPoint(const Point& p);
        virtual ~CLPoint();
        /// Atomic pointer to the corresponding CCPoint, protected against
        /// concurrent replacement in liftZ.
        std::atomic<CCPoint*> cc;
        /// string repr
        std::string str() const;

        /// if cc is in the edge p1-p2, test if clpoint needs to be lifted to z
        /// if so, set cc = cc_tmp and return true
        bool liftZ_if_InsidePoints(double z, CCPoint& cc_tmp, const Point& p1,const Point& p2);

        /// if cc in in Triangle facet, test if clpoint needs to be lifted
        /// if so, set cc=cc_tmp and return true
        bool liftZ_if_inFacet(double z, CCPoint& cc_tmp, const Triangle& t);

        /// if zin > z, lift CLPoint and update cc-point, and return true
        bool liftZ(double zin, CCPoint& ccp);

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
