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

#include <cmath>
#include "arc.hpp"

namespace ocl
{

Arc::Arc(const Point &p1in, const Point &p2in, const Point &cin, bool dirin) {
    p1=p1in;
    p2=p2in;
    c=cin;
    dir=dirin;
    setProperties();
}

Arc::Arc(const Arc &a) {
    p1=a.p1;
    p2=a.p2;
    c=a.c;
    dir=a.dir;
    setProperties();
}

void Arc::setProperties() {
    // arc properties
    Point vs = (p1 - c).xyPerp();
    Point ve = (p2 - c).xyPerp();
    radius = vs.xyNorm();
    vs.normalize();
    ve.normalize();
    length = fabs(xyIncludedAngle(vs, ve, dir)) * radius;
}

std::ostream& operator<<(std::ostream &stream, const Arc& a)
{
  stream << "(" << a.p1 << ", " << a.p2 << ", " << a.c << ", " << a.dir << ")";
  return stream;
}

Point Arc::getPoint(double t)const {
        /// returns a point which is 0-1 along span
        if(fabs(t) < 0.00000000000001)
        return p1;
        if(fabs(t - 1.0) < 0.00000000000001)
        return p2;

        double d = t * length;
        if(!dir)d = -d;
        Point v = p1 - c;
        v.xyRotate(d / radius);
        return v + c;
}

double Arc::xyIncludedAngle(const Point& v1, const Point& v2, bool dir) {
    // returns the absolute included angle between 2 vectors in 
    // the direction of dir ( true=acw  false=cw )
    int d = dir ? 1 : (-1);
    double inc_ang = v1.dot(v2);
    if(inc_ang > 1. - 1.0e-10) 
        return 0;
    if(inc_ang < -1. + 1.0e-10)
        inc_ang = PI;
    else {  // dot product,   v1 . v2  =  cos(alfa)
        if(inc_ang > 1.0) 
            inc_ang = 1.0;
        inc_ang = acos(inc_ang); // 0 to pi radians

        double x = v1.x * v2.y - v1.y * v2.x; 
        if(d * x < 0) 
            inc_ang = 2 * PI - inc_ang ; // cp
    }
    return d * inc_ang;
}

} // end namespace
// end file arc.cpp
