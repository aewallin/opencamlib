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

#include <boost/foreach.hpp>

#include "ballcutter.h"
#include "numeric.h"

namespace ocl
{

BallCutter::BallCutter() {
    std::cout << " usage: BallCutter( double diameter, double length )\n";
    assert(0);
}

BallCutter::BallCutter(double d, double l) {
    assert( d>0.0 );
    diameter = d;
    radius = d/2.0;
    assert( l>0.0 );
    length = l;
    normal_length = radius;
    xy_normal_length = 0.0;
    center_height = radius;
}

// height of cutter at radius r
double BallCutter::height(double r) const {
    return radius - sqrt( square(radius) - square(r) );
}

// width of cutter at height h
double BallCutter::width(double h) const {
    if ( h >= radius )
        return radius;
    else 
        return sqrt( square(radius) - square(radius-h) );
}

// offset of ball is a bigger ball
MillingCutter* BallCutter::offsetCutter(double d) const {
    return  new BallCutter(diameter+2*d, length+d) ;
}

// drop-cutter methods: vertex and facet are handled in base-class

// drop-cutter edgeDrop 
CC_CLZ_Pair BallCutter::singleEdgeContact(const Point& u1, const Point& u2) const {
    // the plane of the line will slice the spherical cutter at
    // a distance d==u1.y==u2.y from the center of the cutter
    // here the radius of the circular section is s:
    double s = sqrt( square(radius) - square( u1.y ) );            
    Point normal(u2.z - u1.z, -(u2.x - u1.x), 0); // (dz, -du) is a normal to the line 
    normal.xyNormalize();
    if (normal.y < 0)  // flip normal so it points upward
        normal = -1*normal;
    Point cc( -s*normal.x, u1.y, 0); // from (0,u1.y,0) we go a distance -s in the normal direction
    cc.z_projectOntoEdge(u1,u2);
    double cl_z = cc.z + s*normal.y - radius;
    return CC_CLZ_Pair( cc.x , cl_z);
}


// push-cutter: vertex and facet handled in base-class

bool BallCutter::singleEdgePush(const Fiber& f, Interval& i,  const Point& p1, const Point& p2) const {
    bool result = false;
    const Point ufp1 = f.p1 + Point(0,0,radius); // take a fiber which is raised up by radius
    const Point ufp2 = f.p2 + Point(0,0,radius);
    // and intersect it with a cylinder around the edge p1-p2
    //--------------------------------------------------------------------------
    // Ray : P(t) = O + t*V    from point O, in direction V
    // Cylinder [A, B, r]   from point A to point B, radius r
    // Point P on infinite cylinder if ((P - A) x (B - A))^2 = r^2 * (B - A)^2
    // expand : ((O - A) x (B - A) + t * (V x (B - A)))^2 = r^2 * (B - A)^2
    // equation in the form (X + t * Y)^2 = d
    // where : 
    //  X = (O - A) x (B - A)
    //  Y = V x (B - A)
    //  d = r^2 * (B - A)^2
    // expand the equation :
    // t^2 * (Y . Y) + t * (2 * (X . Y)) + (X . X) - d = 0
    // => second order equation in the form : a*t^2 + b*t + c = 0 where
    // a = (Y . Y)
    // b = 2 * (X . Y)
    // c = (X . X) - d
    //--------------------------------------------------------------------------
    Point ab = p2-p1; // axis of the cylinder
    Point ao = (ufp1 - p1); // cyl start to ray start
    Point ao_x_ab = ao.cross(ab); // cross product
    Point v_x_ab  = (ufp2-ufp1).cross(ab); // cross product
    double ab2 = ab.dot(ab); // dot product
    double a = v_x_ab.dot(v_x_ab); // dot product
    double b = 2 * ( v_x_ab.dot(ao_x_ab) ); // dot product
    double c = ao_x_ab.dot(ao_x_ab) - (radius*radius * ab2);
    // solve second order equation : a*t^2 + b*t + c = 0
    // t = (-b +/- sqrt( b^2 - 4ac ) )   / 2a
    double discr = b*b-4*a*c;
    double t1;
    double t2;
    if ( isZero_tol( discr ) ) { // tangent case
        //std::cout << "TANGENT CASE!\n";
        t1= -b/(2*a); // only one root
        Point cl1_center = f.point(t1) + Point(0,0,radius);
        CCPoint cc_tmp = cl1_center.closestPoint(p1,p2);
        cc_tmp.type = EDGE;
        double cct = (cc_tmp-p1).dot(p2-p1) / (p2-p1).dot(p2-p1) ;
        if ( cct > 0.0 && cct < 1.0 && ((cl1_center-cc_tmp).z >=0) ) {
            i.update( t1  , cc_tmp );
            result = true;
        }
    } else if ( discr > 0.0 ) { // two roots
        t1 = (-b + sqrt( discr))/(2*a);
        t2 = (-b - sqrt( discr))/(2*a);
        Point cl1_center = f.point(t1) + Point(0,0,radius);
        Point cl2_center = f.point(t2) + Point(0,0,radius);
        // cc-point is on p1-p2 line, closest to CL
        CCPoint cc_tmp1 = cl1_center.closestPoint(p1,p2);
        CCPoint cc_tmp2 = cl2_center.closestPoint(p1,p2);
        // edge: p1 + t*(p2-p1) = cc_tmp
        // so t = (cc_tmp-p1)dot(p2-p1) / (p2-p1).dot(p2-p1)
        double cct1 = (cc_tmp1-p1).dot(p2-p1) / (p2-p1).dot(p2-p1) ;
        double cct2 = (cc_tmp2-p1).dot(p2-p1) / (p2-p1).dot(p2-p1) ;
        cc_tmp1.type = EDGE;
        cc_tmp2.type = EDGE;
        if ( cct1 > 0.0 && cct1 < 1.0 && ((cl1_center-cc_tmp1).z >=0) ) {
            i.update( t1  , cc_tmp1 );
            result = true;
        }
        // edge: p1 + t*(p2-p1) = cc_tmp
        // so t = (cc_tmp-p1)dot(p2-p1) / (p2-p1).dot(p2-p1)
        if ( cct2 > 0.0 && cct2 < 1.0 && ((cl2_center-cc_tmp2).z >=0) ) {
            i.update( t2  , cc_tmp2 );
            result = true;
        }
    } 
    
    // no solution to quadratic(?), i.e. no contact with the ball
    // instead we test for contact with the cylindrical shaft
    // fiber is f.p1 + v*(f.p2-f.p1)  and line  is p1 + u*(p2-p1)
    if ( shaftEdgePush(f,i,p1,p2) )
        result = true;
        
    return result;
}

std::string BallCutter::str() const {
    std::ostringstream o;
    o << *this; 
    return o.str();
}

std::ostream& operator<<(std::ostream &stream, BallCutter c) {
  stream << "BallCutter(d=" << c.diameter << ", r=" << c.radius << ", L=" << c.length << ")";
  return stream;
}

} // end namespace
// end file ballcutter.cpp
