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
#ifndef POINT_H
#define POINT_H

#include <string>
#include <iostream>

namespace ocl
{

class Triangle; 

///
/// \brief a point or vector in 3D space specified by its coordinates (x, y, z)
///
class Point {
    public:
        /// create a point at (0,0,0)
        Point();
        /// create a point at (x,y,z)
        Point(double x, double y, double z);
        /// create a point at (x,y,0)
        Point(double x, double y);
        /// create a point at p
        Point(const Point &p);
        /// destructor. empty.
        virtual ~Point() {};
        
        /// dot product
        double dot(const Point &p) const;
        
        /// cross product
        Point cross(const Point &p) const;
        
        /// norm of vector, or distance from (0,0,0) to *this
        double norm() const; 
        
        /// scales vector so that norm()==1.0
        void normalize();
        
        /// distance from Point to another Point p in the XY plane
        double xyDistance(const Point &p) const;
        
        /// length of vector in the XY plane
        double xyNorm() const;
        
        /// normalize so that length xyNorm == 1.0
        void xyNormalize();

        /// return perpendicular in the xy plane, rotated 90 degree to the left
        Point xyPerp() const;
        
        /// move *this along z-axis so it lies on p1-p2 line-segment 
        void z_projectOntoEdge(const Point& p1, const Point& p2);
        
        /// rotate point in the xy-plane by angle theta
        /// inputs are cos(theta) and sin(theta)
        void xyRotate(double cosa, double sina);
        /// rotate point in xy-plane bu angle theta (radians or degrees??)
        void xyRotate(double angle);
        
        /// rotate around x-axis
        void xRotate(double theta);
        /// rotate around y-axis
        void yRotate(double theta);
        /// rotate around z-axis
        void zRotate(double theta);
        void matrixRotate(double a,double b, double c,
                          double d,double e, double f,
                          double g,double h, double i);
                          
        /// distance from Point to infinite line through p1 and p2. In the XY plane.
        double xyDistanceToLine(const Point &p1, const Point &p2) const;
        
        /// return closest Point to line through p1 and p2. in 3D.
        Point closestPoint(const Point &p1, const Point &p2) const;
        
        /// return closest Point to line through p1 and p2. Works in the XY plane.
        Point xyClosestPoint(const Point &p1, const Point &p2) const;
        
        /// returns true if point is right of line through p1 and p2 (works in the XY-plane)
        bool isRight(const Point &p1, const Point &p2) const;
         
        /// returns true if Point *this is inside Triangle t 
        bool isInside(const Triangle &t) const; 
        /// return true if Point within line segment p1-p2
        bool isInside(const Point& p1, const Point& p2) const;
            

        /// return true if the x and y components are both zero.
        bool xParallel() const;
        /// return true if vector parallel to y-axis
        bool yParallel() const;
        /// return true if vector parallel to z-axis
        bool zParallel() const;
        
        /// assignment
        Point &operator=(const Point &p);
        /// addition
        Point &operator+=(const Point &p);
        /// subtraction
        Point &operator-=(const Point &p);
        /// addition
        const Point operator+(const Point &p)const;
        /// subtraction
        const Point operator-(const Point &p) const;

        /// scalar multiplication
        Point &operator*=(const double &a);  // scalar multiplication with Point *= scalar
        /// Point * scalar
        const Point operator*(const double &a)const;     // Point*scalar 
        /// equality
        bool operator==(const Point &p) const;
        /// inequality
        bool operator!=(const Point &p) const;

        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, const Point &p);
        /// string repr
        std::string str() const;

        /// X coordinate
        double x;
        /// Y coordinate
        double y;
        /// Z coordinate
        double z;
};

/// scalar multiplication   scalar*Point
const Point operator*(const double &a, const Point &p);

} // end namespace
#endif
// end file point.h
