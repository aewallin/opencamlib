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
#include <cmath> // sqrt, sin, cos, fabs
#include <cassert>
#include <sstream>

#include "point.hpp"
#include "triangle.hpp"
#include "numeric.hpp"

namespace ocl
{
    
Point::Point() {
    x=0;
    y=0;
    z=0;
}

Point::Point(double xin, double yin, double zin) {
    x=xin;
    y=yin;
    z=zin;
}

Point::Point(double xin, double yin) {
    x=xin;
    y=yin;
    z=0.0;
}

Point::Point(const Point &p) {
    x=p.x;
    y=p.y;
    z=p.z;
}


//********     methods ********************** */


double Point::norm() const {
    return sqrt( square(x) + square(y) + square(z) );
}

Point Point::cross(const Point &p) const {
    double xc = y * p.z - z * p.y;
    double yc = z * p.x - x * p.z;
    double zc = x * p.y - y * p.x;
    return Point(xc, yc, zc);
}

double Point::dot(const Point &p) const {
    return x * p.x + y * p.y + z * p.z;
}

void Point::normalize() {
    double norm = this->norm();
    if (norm != 0.0)
        *this *=(1/norm);
}

double Point::xyNorm() const {
    return sqrt( square(x) + square(y) );
}

void Point::xyNormalize() {
    if (this->xyNorm() != 0.0)
        *this *=(1/this->xyNorm());
}

Point Point::xyPerp() const
{
        return Point(-y, x, z);
}

void Point::xyRotate(double cosa, double sina) {                                                                                                                        // rotate vector by angle
    double temp = -y * sina + x * cosa;
    y = x * sina + cosa * y;
    x = temp;
}

void Point::xyRotate(double angle) {
    xyRotate(cos(angle), sin(angle));
}

void Point::xRotate(double theta) {
    matrixRotate(1, 0         , 0          ,
                 0, cos(theta), -sin(theta),
                 0, sin(theta), cos(theta)  );
}

void Point::yRotate(double theta) {
    matrixRotate(cos(theta) , 0 , sin(theta) ,
                          0 , 1 , 0          ,
                 -sin(theta), 0 , cos(theta)  );
}

void Point::zRotate(double theta) {
    matrixRotate(cos(theta), -sin(theta) , 0 ,
                 sin(theta), cos(theta), 0,
                 0, 0, 1  );
}

// http://en.wikipedia.org/wiki/Rotation_matrix
void Point::matrixRotate(double a,double b, double c,
                         double d,double e, double f,
                         double g,double h, double i) {
    // multiply point with matrix
    double xr = a*x + b*y + c*z;
    double yr = d*x + e*y + f*z;
    double zr = g*x + h*y + i*z;
    x=xr;
    y=yr;
    z=zr;
}

double Point::xyDistance(const Point &p) const
{   
    return (*this - p).xyNorm();
    //return sqrt(pow(x - p.x, 2) + pow((y - p.y), 2));
}




double Point::xyDistanceToLine(const Point &p1, const Point &p2) const
{
    // see for example
    // http://mathworld.wolfram.com/Point-LineDistance2-Dimensional.html
    if ((p1.x == p2.x) && (p1.y == p2.y)) {// no line in xy plane
        std::cout << "point.cpp: xyDistanceToLine ERROR!: can't calculate distance from \n";
        std::cout << "point.cpp: xyDistanceToLine ERROR!: *this ="<<*this <<" to line through\n";
        std::cout << "point.cpp: xyDistanceToLine ERROR!: p1="<<p1<<" and \n";
        std::cout << "point.cpp: xyDistanceToLine ERROR!: p2="<<p2<< "\n";
        std::cout << "point.cpp: xyDistanceToLine ERROR!: in the xy-plane\n";
        return -1;
    }
    else {
        Point v = Point(p2.y-p1.y, -(p2.x-p1.x), 0 );
        v.normalize();
        Point r = Point(p1.x - x, p1.y - y, 0);
        return fabs( v.dot(r));
    }
}

/// return Point on p1-p2 line which is closest in 3D to this.
Point Point::closestPoint(const Point &p1, const Point &p2) const
{
    Point v = p2 - p1;
    assert( v.norm() > 0.0 );
    double u = (*this - p1).dot(v) / v.dot(v);  // u = (p3-p1) dot v / (v dot v)
    return p1 + u*v;
}

/// return Point on p1-p2 line which is closest in XY-plane to this
Point Point::xyClosestPoint(const Point &p1, const Point &p2) const
{
    // one explanation is here
    // http://local.wasp.uwa.edu.au/~pbourke/geometry/pointline/
    Point pt1 = p1; // this required because of "const" arguments above.
    Point pt2 = p2;
    Point v = pt2 - pt1;
    if ( isZero_tol( v.xyNorm() ) ) { // if p1 and p2 do not make a line in the xy-plane
        std::cout << "point.cpp: xyClosestPoint ERROR!: can't calculate closest point from \n";
        std::cout << "point.cpp: xyClosestPoint ERROR!: *this ="<<*this <<" to line through\n";
        std::cout << "point.cpp: xyClosestPoint ERROR!: p1="<<p1<<" and \n";
        std::cout << "point.cpp: xyClosestPoint ERROR!: p2="<<p2<< "\n";
        std::cout << "point.cpp: xyClosestPoint ERROR!: in the xy-plane\n";
        assert(0);
        return Point(0,0,0); 
    }
        
    double u;
    // vector notation:
    // u = (p3-p1) dot v / (v dot v)
    u = (this->x - p1.x) * (v.x) + (this->y - p1.y)*(v.y);
    u = u/ (v.x*v.x + v.y*v.y);
    // coordinates for closest point
    double x = p1.x + u*v.x;
    double y = p1.y + u*v.y;
    return Point(x,y,0);
}


bool Point::isRight(const Point &p1, const Point &p2) const
{
    // is Point right of line through points p1 and p2 ?, in the XY plane.
    // this is an ugly way of doing a determinant
    // should be prettyfied sometime...
    /// \todo FIXME: what if p1==p2 ? (in the XY plane)
    double a1 = p2.x - p1.x;
    double a2 = p2.y - p1.y;
    double t1 = a2;
    double t2 = -a1;
    double b1 = x - p1.x;
    double b2 = y - p1.y;

    double t = t1 * b1 + t2 * b2;
    if (t > 0.00000000000001) /// \todo FIXME: hardcoded magic number...
        return true;
    else
        return false;
}

bool Point::isInside(const Triangle &t) const {
    Point p = *this;
    Point a = t.p[0];
    Point b = t.p[1];
    Point c = t.p[2];

    // Compute barycentric coordinates
    double u = (a.y * c.x - a.x * c.y + (c.y - a.y) * p.x + (a.x - c.x) * p.y)
             / (a.y * c.x - a.x * c.y + (c.y - a.y) * b.x + (a.x - c.x) * b.y);

    double v = (a.x * b.y - a.y * b.x + (a.y - b.y) * p.x + (b.x - a.x) * p.y)
             / (a.x * b.y - a.y * b.x + (a.y - b.y) * c.x + (b.x - a.x) * c.y);

    // Check if point is inside triangle
    return u > 0.0 && v > 0.0 && (u + v) < 1.0;
}

bool Point::isInside(const Point& p1, const Point& p2) const {
    // segment is p1 + t*(p2-p1)
    // p1 + t*(p2-p1) = p
    // p1*(p2-p1) + t * (p2-p1)*(p2-p1) = p*(p2-p1)
    // t = (p - p1 )*(p2-p1) / (p2-p1)*(p2-p1)
    Point p2minusp1 = p2 - p1;
    Point thisminusp1 = (*this - p1);
    double t = thisminusp1.dot(p2minusp1) / p2minusp1.dot(p2minusp1);
    if (t > 1.0 || t < 0.0) {
        return false;
    }
    return true;
}

/* **************** Operators ***************  
 *  see
 *  http://www.cs.caltech.edu/courses/cs11/material/cpp/donnie/cpp-ops.html
*/

Point& Point::operator=(const Point &p) {
    if (this == &p)
        return *this;
    x=p.x;
    y=p.y;
    z=p.z;
    return *this;
}

// Point*scalar multiplication
Point& Point::operator*=(const double &a) {
    x*=a;
    y*=a;
    z*=a;
    return *this;
}

Point& Point::operator+=(const Point &p) {
    x+=p.x;
    y+=p.y;
    z+=p.z;
    return *this;
}

Point& Point::operator-=(const Point &p) {
    x-=p.x;
    y-=p.y;
    z-=p.z;
    return *this;
}

const Point Point::operator+(const Point &p) const {
    return Point(*this) += p;
}

const Point Point::operator-(const Point &p) const {
    return Point(*this) -= p;
}

const Point Point::operator*(const double &a) const {
    return Point(*this) *= a;
}

// scalar*Point
const Point operator*(const double &a, const Point &p) {
    return Point(p) *= a;
}

bool Point::operator==(const Point &p) const {
    return (this == &p) || (x==p.x && y==p.y && z==p.z);
}

bool Point::operator!=(const Point &p) const {
    return !(*this == p);
}

bool Point::zParallel() const  {
    if (x != 0.0)
        return false;
    else if (y != 0.0)
        return false;
    else
        return true;
}

bool Point::xParallel() const  {
    if ( isZero_tol( y ) && isZero_tol( z ) )
        return true;
    return false;
}

bool Point::yParallel() const  {
    if ( isZero_tol( x ) && isZero_tol( z ) )
        return true;
    return false;
}

void Point::z_projectOntoEdge(const Point& p1, const Point& p2) {
    // edge is p1+t*(p2-p1)
    // now locate z-coord of *this on edge
    double t;
    if ( fabs(p2.x-p1.x) > fabs(p2.y-p1.y) ) {
        t = (this->x - p1.x) / (p2.x-p1.x); 
    } else {
        t = (this->y - p1.y) / (p2.y-p1.y);
    }
    this->z = p1.z + t*(p2.z-p1.z);
}


std::string Point::str() const {
    std::ostringstream o;
    o << *this;
    return o.str();
}

std::ostream& operator<<(std::ostream &stream, const Point& p) {
  stream << "(" << p.x << ", " << p.y << ", " << p.z << ")"; 
  return stream;
}

} // end namespace
// end file point.cpp
