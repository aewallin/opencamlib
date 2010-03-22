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
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <math.h>
#include "line.h"

Line::Line(const Point &p1in, const Point &p2in)
{
    p1=p1in;
	p2=p2in;
}

Line::Line(const Line &l)
{
    p1=l.p1;
	p2=l.p2;
}

std::ostream& operator<<(std::ostream &stream, const Line& l)
{
  stream << "(" << l.p1 << ", " << l.p2 << ")";
  return stream;
}

double Line::length2d()const
{
	return Point(p1, p2).xyNorm();
}

Point Line::getPoint(double t)const
{
	return Point(p1, p2) * t + p1;
}

Point Line::Near(const Point& p)const{
	// returns the near point from a line on the extended line
	Point v(p1, p2);
	v.normalize();
	double dp = Point(p1, p).dot(v);
	return p1 + (v * dp);
}
