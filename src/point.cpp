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

#include "point.h"
#include "triangle.h"
#include "numeric.h"

namespace ocl
{
    

int Point::count = 0;

Point::Point() {
    x=0;
    y=0;
    z=0;
    setID();
}

Point::Point(double xin, double yin, double zin) {
    x=xin;
    y=yin;
    z=zin;
    setID();
}

Point::Point(const Point &p) {
    x=p.x;
    y=p.y;
    z=p.z;
    setID();
}


//********     methods ********************** */

void Point::setID() {
    id = count;
    count++;
}

double Point::norm() const {
    return sqrt( square(x) + square(y) + square(z) );
}

Point Point::cross(const Point &p) const {
    double xc = y * p.z - z * p.y;
    double yc = z * p.x - x * p.z;
    double zc = x * p.y - y * p.x;
    return Point(xc, yc, zc);
}

double Point::dot(const Point &p) const
{
    return x * p.x + y * p.y + z * p.z;
}

void Point::normalize() {
    if (this->norm() != 0.0)
        *this *=(1/this->norm());
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


double Point::xyDistance(const Point &p) const
{   
    return (*this - p).xyNorm();
    //return sqrt(pow(x - p.x, 2) + pow((y - p.y), 2));
}


int Point::liftZ(double zin)
{
    if (zin>z) {
        z=zin;
        return 1;
    } else {
        return 0;
    }
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
    // vector notation:
    // u = (p3-p1) dot v / (v dot v)
    double u = (*this - p1).dot(v) / v.dot(v);
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

bool Point::isInside(const Triangle &t) const
{
    // point in triangle test
    // http://www.blackpawn.com/texts/pointinpoly/default.html
    
    Point v0 = t.p[2] - t.p[0];
    Point v1 = t.p[1] - t.p[0];
    Point v2 = *this  - t.p[0];
    
    double dot00 = v0.dot(v0);
    double dot01 = v0.dot(v1);
    double dot02 = v0.dot(v2);
    double dot11 = v1.dot(v1);
    double dot12 = v1.dot(v2);
    
    double invD = 1.0 / ( dot00 *dot11 - dot01*dot01 );
    // barycentric coordinates
    double u = (dot11 * dot02 - dot01 * dot12) * invD;
    double v = (dot00 * dot12 - dot01 * dot02) * invD;

    // Check if point is in triangle
    return (u > 0.0) && (v > 0.0) && (u + v < 1.0);
}


#define TOLERANCE 0.000001 // FIXME: magic number constant...

bool Point::isInsidePoints(const Point &p1, const Point &p2) const
{
    double minx,maxx,miny,maxy;
    if (p1.x > p2.x) {
        minx = p2.x;
        maxx = p1.x;
    } else {
        minx = p1.x;
        maxx = p2.x;
    }
    if (p1.y > p2.y) {
        miny = p2.y;
        maxy = p1.y;
    } else {
        miny = p1.y;
        maxy = p2.y;
    }
    //std::cout << "minx=" << minx << "maxx=" << maxx << "miny=" << miny << "maxy=" << maxy << "\n";
    //std::cout << "x=" << x << " y=" << y << "\n";
    bool b1 = (x>= (minx-TOLERANCE));
    bool b2 = (x<= (maxx+TOLERANCE));
    bool b3 = (y>= (miny-TOLERANCE));
    bool b4 = (y<= (maxy+TOLERANCE));
    //std::cout << "b1=" << b1 << " b2=" << b2 << " b3=" << b3 << " b4=" << b4 << "\n";
    if ( b1 && b2 && b3 && b4) {
        //std::cout << "returning true\n";
        return true;
    } 
    else {
        //std::cout << "returning false\n";
        return false;
    }
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

// scalar multiplication
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

const Point operator*(const double &a, const Point &p) {
    return Point(p) *= a;
}

bool Point::operator==(const Point &p) const {
    return (this == &p) || (x==p.x && y==p.y && z==p.z);
}

bool Point::operator!=(const Point &p) const {
    return !(*this == p);
}

/// return true if the vector has only a z-component
bool Point::zParallel() const 
{
    if (x != 0.0)
        return false;
    else if (y != 0.0)
        return false;
    else
        return true;
}

std::string Point::str() const
{
        std::ostringstream o;
        o << *this;
        return o.str();
}

std::ostream& operator<<(std::ostream &stream, const Point& p)
{
  stream << "(" << p.x << ", " << p.y << ", " << p.z << ")"; // no ID
  return stream;
}


} // end namespace
// end file point.cpp
