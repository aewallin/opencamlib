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
#ifndef LINE_H
#define LINE_H

#include <iostream>
#include "point.h"

///
/// \brief a finite line in 3D space specified by its end points (p1, p2)
///

///
/// longer documentation here.
///
class Line {
    public:
		Line(){}
        Line(const Point &p1, const Point &p2);
        Line(const Line &l);

        // text output
        friend std::ostream& operator<<(std::ostream &stream, const Line &l);

        /// start point
        Point p1;
        /// end point
        Point p2;

		double length2d()const;
		Point getPoint(double fraction)const;
		Point Line::Near(const Point& p)const;
};

#endif
