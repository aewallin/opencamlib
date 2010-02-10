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
#include "point.h"
#include "triangle.h"

int Point::count = 0;

Point::Point()
{
    x=0;
    y=0;
    z=0;
    setID();
}

Point::Point(double xin, double yin, double zin)
{
    x=xin;
    y=yin;
    z=zin;
    setID();
}

Point::Point(const Point &p)
{
    x=p.x;
    y=p.y;
    z=p.z;
    setID();
}


//********     methods ********************** */

void Point::setID()
{
		id = count;
		count++;
}

double Point::norm()
{
    return sqrt(x*x+y*y+z*z);
}

const Point Point::cross(const Point &p)
{
    double xc = y * p.z - z * p.y;
    double yc = z * p.x - x * p.z;
    double zc = x * p.y - y * p.x;
    return Point(xc, yc, zc);
}

double Point::dot(const Point &p)
{
    return x * p.x + y * p.y + z * p.z;
}

void Point::normalize()
{
	if (this->norm() != 0) {
		*this *=(1/this->norm());
	}
}

double Point::xyNorm()
{
    return sqrt(x*x+y*y);
}

void Point::xyNormalize()
{
    if (this->xyNorm() != 0) {
		*this *=(1/this->xyNorm());
	}
}


double Point::xyDistance(Point p)
{
    return sqrt(pow(x - p.x, 2) + pow((y - p.y), 2));
}

void Point::liftZ(double zin)
{
    if (zin>z)
        z=zin;
}

double Point::xyDistanceToLine(const Point &p1, const Point &p2)
{
        // see for example
        // http://mathworld.wolfram.com/Point-LineDistance2-Dimensional.html
        if ((p1.x == p2.x) && (p2.y == p2.y)) {// no line in xy plane
            return -1;
        }
        else {
            Vector v = Vector(p2.y-p1.y, -(p2.x-p1.x), 0 );
            v.normalize();
            Vector r = Vector(p1.x - x, p1.y - y, 0);
            return fabs( v.dot(r));
        }
}

// geometric predicates

bool Point::isRight(const Point p1, const Point p2)
{
    // is point right of line through points p1 and p2 ?
	// this is an ugly way of doing a determinant
	// should be prettyfied sometime...
    // FIXME: what if p1==p2 ? (in the XY plane)
	double a1 = p2.x - p1.x;
	double a2 = p2.y - p1.y;
	double t1 = a2;
	double t2 = -a1;
	double b1 = x - p1.x;
	double b2 = y - p1.y;

	double t = t1 * b1 + t2 * b2;
	if (t > 0.00000000000001) //FIXME: hardcoded magic number...
		return true;
	else
		return false;    
}

bool Point::isInside(const Triangle t)
{
    // point in triangle test
    
	// a new Triangle projected onto the XY plane
    Point p1 = Point(t.p[0].x, t.p[0].y, 0.0);
    Point p2 = Point(t.p[1].x, t.p[1].y, 0.0);
    Point p3 = Point(t.p[2].x, t.p[2].y, 0.0);
	//double p1[3] = {t.m_p[0], t.m_p[1], 0};
	//double p2[3] = {t.m_p[3], t.m_p[4], 0};
	//double p3[3] = {t.m_p[6], t.m_p[7], 0};
    
    // a new point projected onto the XY plane
	Point p = Point(x, y, 0.0);
    
	bool b1 = p.isRight(p1, p2);
	bool b2 = p.isRight(p3, p1);
	bool b3 = p.isRight(p2, p3);

    
	if ((b1) && (b2) && (b3)) {
		return true;
    }
	else if ((!b1) && (!b2) && (!b3)) {
		return true;
    }
	else {
		return false;
    }
}

/* **************** Operators ***************  
/*  see
/*  http://www.cs.caltech.edu/courses/cs11/material/cpp/donnie/cpp-ops.html
*/

Point& Point::operator=(const Point &p)
{
    x=p.x;
    y=p.y;
    z=p.z;
    return *this;
}

// scalar multiplication
Point& Point::operator*=(const double &a)
{
    x*=a;
    y*=a;
    z*=a;
    return *this;
}

Point& Point::operator+=(const Point &p)
{
    x+=p.x;
    y+=p.y;
    z+=p.z;
    return *this;
}

Point& Point::operator-=(const Point &p)
{
    x-=p.x;
    y-=p.y;
    z-=p.z;
    return *this;
}

const Point Point::operator+(const Point &p)
{
    return Point(*this) += p;
}

const Point Point::operator-(const Point &p)
{
    return Point(*this) -= p;
}

const Point Point::operator*(const double &a)
{
    return Point(*this) *= a;
}

const Point operator*(const double &a, const Point &p)
{
    return Point(p) *= a;
}

bool Point::operator==(const Point &p)
{
    return (x==p.x && y==p.y && z==p.z) ? true : false ;
}

bool Point::operator!=(const Point &p)
{
    return !(*this == p);
}

std::string Point::str()
{
	std::ostringstream o;
	o << "P"<< id <<"(" << x << ", " << y << ", " << z << ")";
	return o.str();
}

std::ostream& operator<<(std::ostream &stream, Point p)
{
  stream << "P" << p.id << "(" << p.x << ", " << p.y << ", " << p.z << ")";
  return stream;
}
