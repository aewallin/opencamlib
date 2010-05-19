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
#ifndef POINT_H
#define POINT_H

#include <iostream>
#include <string>

namespace ocl
{

// forward declaration
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

        /// create a point at p
        Point(const Point &p);
        
        /// dot product
        double dot(const Point &p) const;
        
        /// cross product
        Point cross(const Point &p) const;
        
        /// distance to point from (0,0,0)
        double norm() const; // norm of vector, or distance to (0,0,0) for a point
        
        /// scales vector so that norm()==1.0
        void normalize();
        
        /// distance from Point to another Point p in the XY plane
        double xyDistance(const Point &p) const;
        
        /// length of vector in xy plane
        double xyNorm() const;
        
        /// normalize so that length in xy plane is 1
        void xyNormalize();

        /// return perpendicular in the xy plane, rotated 90 degree to the left
        Point xyPerp() const;

        // rotate the vector around x0 y0
        /// rotate point in the xy-plane by angle theta
        /// inputs are cos(theta) and sin(theta)
        void xyRotate(double cosa, double sina);
        /// rotate point in xy-plane bu angle theta (radians or degrees??)
        void xyRotate(double angle);
        
        /// if z < zin, lift point so that z=zin. Used by drop-cutter etc.
        int liftZ(double zin);
        
        /// distance from Point to infinite line through p1 and p2. In the XY plane.
        double xyDistanceToLine(const Point &p1, const Point &p2) const;
        
        /// return closest Point to line through p1 and p2. in 3D.
        Point closestPoint(const Point &p1, const Point &p2);
        
        /// return closest Point to line through p1 and p2. Works in the XY plane.
        Point xyClosestPoint(const Point &p1, const Point &p2) const;
        
        /// returns true if point is right of line through p1 and p2 (works in the XY-plane)
        bool isRight(const Point &p1, const Point &p2) const;
         
        /// retruns true if point is inside Triangle t (works in the XY-plane)
        bool isInside(const Triangle &t) const; 
        
        /// retruns true if point is inside p1-p2 line (xy-plane)
        bool isInsidePoints(const Point &p1, const Point &p2) const;
        
        /// assignment
        Point &operator=(const Point &p);
        /// addition
        Point &operator+=(const Point &p);
        /// subtraction
        Point &operator-=(const Point &p);
        /// addition
        const Point operator+(const Point &p)const;
        /// subtraction
        const Point operator-(const Point &p)const;
        /// what is this??
        const Point operator-(void)const;
        /// scalar multiplication
        Point &operator*=(const double &a);  // scalar multiplication
        /// Point * scalar
        const Point operator*(const double &a)const;     // Point*scalar 
        /// equality
        bool operator==(const Point &p);
        /// inequality
        bool operator!=(const Point &p);

        // text output
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, const Point &p);
        /// string repr
        std::string str();
        


        /// X coordinate
        double x;
        /// Y coordinate
        double y;
        /// Z coordinate
        double z;
        
        /// id-count
        static int count;
        /// id-number
        int id;
    private:
        /// set id-number
        void setID();

};

/// scalar multiplication   scalar*Point
const Point operator*(const double &a, const Point &p);

/// type of cc-point
enum CCType {NONE, VERTEX, 
             EDGE, EDGE_HORIZ_CYL, EDGE_HORIZ_TOR, 
             EDGE_POS, EDGE_NEG,  
             FACET, FACET_TIP, FACET_CYL, 
             ERROR};

///
/// \brief Cutter-Contact (CC) point. A Point wiht a CCType.
///
/// Cutter-Contact (CC) Point.
/// A Point which also contains the type of cutter-contact.
class CCPoint : public Point {
    public:
        /// create a CCPoint at (0,0,0)
        CCPoint();
        /// specifies the type of the Cutter Contact point. Possible values are NONE, VERTEX, EDGE, FACET, ERROR.
        CCType type;
        /// assignment
        CCPoint &operator=(const Point &p);
        /// string repr
        std::string str();
        
    private:        
};

} // end namespace
#endif
// end file point.h
