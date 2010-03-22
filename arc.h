/*  Copyright 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com)
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
#ifndef ARC_H
#define ARC_H

#include <iostream>
#include "point.h"

///
/// \brief a finite line in 3D space specified by its end points (p1, p2)
///

///
/// longer documentation here.
///
class Arc {
		double length; // 2d length
		double radius;

    public:
		Arc(){}
        Arc(const Point &p1, const Point &p2, const Point &c, bool dir);
        Arc(const Arc &a);

        // text output
        friend std::ostream& operator<<(std::ostream &stream, const Arc &a);

        /// start point
        Point p1;
        /// end point
        Point p2;
        /// centre point
        Point c;
		/// direction true for anti-clockwise
		bool dir;

		double length2d()const{return length;}
		Point getPoint(double fraction)const;
		void setProperties();
		double xyIncludedAngle(const Point& v1, const Point& v2, bool dir = true);
};

#endif
