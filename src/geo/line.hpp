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
#ifndef LINE_H
#define LINE_H

#include <iostream>

#include "point.hpp"

namespace ocl
{
    

///
/// \brief A finite line segment in 3D space specified by its end points (p1, p2)
///

class Line {
    public:
        Line(){};
        /// create a line from p1 to p2
        Line(const Point &p1, const Point &p2);
        /// create a copy of line l.
        Line(const Line &l);
        virtual ~Line() {};
        
        /// text output
        friend std::ostream& operator<<(std::ostream &stream, const Line &l);

        /// start point
        Point p1;
        /// end point
        Point p2;
        
        /// return the length of the line-segment in the xy-plane
        double length2d()const;
        /// return a Point on the Line at parameter value t [0,1]
        Point getPoint(double t) const;
        /// return the point on the Line which is closest to Point p.
        Point Near(const Point& p) const;
};

} // end namespace
#endif
// end file line.h
