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
#ifndef ARC_H
#define ARC_H

#include "point.hpp"
#include "numeric.hpp"

namespace ocl
{

///
/// \brief a finite arc segment in 3D space specified by its end points (p1, p2)
///
class Arc {
        /// 2D length of the segment in the xy-plane
        double length; // 2d length
        /// radius of the arc
        double radius;

    public:
        Arc(){}
        /// create an arc from point p1 to point p2 with center c and direction dir.
        /// direction is true for anti-clockwise arcs.
        Arc(const Point &p1, const Point &p2, const Point &c, bool dir);
        /// copy constructor
        Arc(const Arc &a);
        virtual ~Arc() {};
        
        /// text output
        friend std::ostream& operator<<(std::ostream &stream, const Arc &a);

        /// start point
        Point p1;
        /// end point
        Point p2;
        /// centre point
        Point c;
        /// direction true for anti-clockwise
        bool dir;
        /// return the length of the arc
        double length2d()const{return length;}
        /// return a point along the arc at parameter value t [0,1]
        Point getPoint(double t)const;
                
        /// returns the absolute included angle (in radians) between 
        /// two vectors v1 and v2 in the direction of dir ( true=acw  false=cw)
        double xyIncludedAngle(const Point& v1, const Point& v2, bool dir = true);
    private:
        /// set arc-properties
        void setProperties();
};

} // end namespace
#endif
// end file arc.h
