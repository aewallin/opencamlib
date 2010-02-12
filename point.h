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
#ifndef POINT_H
#define POINT_H

#include <iostream>
#include <string>

// forward declaration
class Triangle;
///
/// \brief a point or vector in 3D space specified by its coordinate (p.x, p.y, p.z)
///

///
/// longer documentation here.
///
class Point {
	public:
		Point();
		Point(double x, double y, double z);
        Point(const Point &p);
        
        /// dot product
        double dot(const Point &p) const;
        
        /// cross product
        Point cross(const Point &p);
        
        /// distance to point from (0,0,0)
        double norm() const; // norm of vector, or distance to (0,0,0) for a point
        
        /// scales vector so that norm()==1.0
        void normalize();
        
        /// distance from Point to another Point p in the XY plane
        double xyDistance(Point &p) const;
        
        /// length of vector in xy plane
        double xyNorm() const;
        
        /// normalize so that length in xy plane is 1
        void xyNormalize();
	    
        /// if z < zin, lift point so that z=zin. Used by drop-cutter etc.
        void liftZ(double zin);
        
        /// distance from Point to infinite line through p1 and p2. In the XY plane.
        double xyDistanceToLine(const Point &p1, const Point &p2) const;
		
        Point &operator=(const Point &p);
        Point &operator+=(const Point &p);
        Point &operator-=(const Point &p);
        const Point operator+(const Point &p);
        const Point operator-(const Point &p);
                
        Point &operator*=(const double &a);  // scalar multiplication
        const Point operator*(const double &a);     // Point*scalar 
        
        bool operator==(const Point &p);
        bool operator!=(const Point &p);

        // text output
        friend std::ostream& operator<<(std::ostream &stream, Point p);
        std::string str();
        
        // geometric predicates
        /// returns true if point is right of line through p1 and p2 (works in the XY-plane)
        bool isRight(const Point &p1, const Point &p2) const;
         
        /// retruns true if point is inside Triangle t (works in the XY-plane)
        bool isInside(const Triangle &t) const; // is point inside triangle t?
        
        bool isInsidePoints(const Point &p1, const Point &p2) const;

		/// X coordinate
        double x;
        /// Y coordinate
        double y;
        /// Z coordinate
        double z;

        static int count;
        int id;
	private:
		void setID();

};

// scalar multiplication   scalar*Point
const Point operator*(const double &a, const Point &p);

// call it Vector also just for fun (?)
typedef Point Vector;

#endif
