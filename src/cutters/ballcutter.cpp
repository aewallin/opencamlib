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

#include <iostream>
#include <sstream>
#include <string>

#include <boost/foreach.hpp>

#include "ballcutter.hpp"

namespace ocl
{

BallCutter::BallCutter() {
    std::cout << " usage: BallCutter( double diameter, double length )\n";
    assert(0);
}

BallCutter::BallCutter(double d, double l) {
    diameter = d;               assert( diameter>0.0 );
    radius = d/2.0;
    length = l;                 assert( length>0.0 );
    normal_length = radius;
    xy_normal_length = 0.0;
    center_height = radius;
}

// drop-cutter methods: vertex and facet are handled in base-class

// drop-cutter edgeDrop 
CC_CLZ_Pair BallCutter::singleEdgeDropCanonical(const Point& u1, const Point& u2) const {
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
bool BallCutter::generalEdgePush(const Fiber& f, Interval& i,  const Point& p1, const Point& p2) const {
    bool result = false;
    if ( isZero_tol( (p2-p1).xyNorm() ) ) { // this would be a vertical edge
        return result;
    }
    
    const Point ufp1 = f.p1 + Point(0,0,radius); // take a fiber which is raised up by radius
    const Point ufp2 = f.p2 + Point(0,0,radius); // and intersect it with a cylinder around the edge p1-p2
    // Ray : P(t) = O + t*V    from point O, in direction V
    // Cylinder [A, B, r]   from point A to point B, radius r
    // Point P on infinite cylinder if ((P - A) x (B - A))^2 = r^2 * (B - A)^2
    // expand : ((O - A) x (B - A) + t * (V x (B - A)))^2 = r^2 * (B - A)^2
    // equation in the form (X + t * Y)^2 = d , where: 
    //  X = (O - A) x (B - A)
    //  Y = V x (B - A)
    //  d = r^2 * (B - A)^2
    // expand the equation :
    // t^2 * (Y . Y) + t * (2 * (X . Y)) + (X . X) - d = 0
    // => second order equation in the form : a*t^2 + b*t + c = 0 where
    // a = (Y . Y)
    // b = 2 * (X . Y)
    // c = (X . X) - d
    Point ab = p2-p1;                           // axis of the cylinder
    Point ao = (ufp1 - p1);                     // cyl start to ray start
    Point ao_x_ab = ao.cross(ab);               // cross product
    Point v_x_ab  = (ufp2-ufp1).cross(ab);      // cross product
    double ab2 = ab.dot(ab);                    // dot product
    double a = v_x_ab.dot(v_x_ab);              // dot product
    double b = 2 * ( v_x_ab.dot(ao_x_ab) );     // dot product
    double c = ao_x_ab.dot(ao_x_ab) - (radius*radius * ab2);
    // solve second order equation : a*t^2 + b*t + c = 0
    // t = (-b +/- sqrt( b^2 - 4ac ) )   / 2a
    double discr = b*b-4*a*c;
    double t1, t2;
    if ( isZero_tol( discr ) ) { // tangent case, only one root
        t1= -b/(2*a); 
        if ( calcCCandUpdateInterval( t1, p1, p2, f, i ) ) 
            result = true;
    } else if ( discr > 0.0 ) { // two roots
        t1 = (-b + sqrt( discr))/(2*a);
        t2 = (-b - sqrt( discr))/(2*a);
        if ( calcCCandUpdateInterval( t1, p1, p2, f, i ) ) 
            result = true;
        if ( calcCCandUpdateInterval( t2, p1, p2, f, i ) ) 
            result = true;
    } 
    return result;
}

// t is a position along the fiber
// p1-p2 is the edge
// Interval& i is updated
bool BallCutter::calcCCandUpdateInterval( double t, const Point& p1, const Point& p2, const Fiber& f, Interval& i) const {
    Point cl_center = f.point(t) + Point(0,0,radius); // center of ball is here
    CCPoint cc_tmp = cl_center.closestPoint(p1,p2); // cc-point on the edge, point on edge closest to center
    cc_tmp.type = EDGE_BALL;
    // require contact with lower hemishphere
    return i.update_ifCCinEdgeAndTrue( t, cc_tmp, p1, p2, ((cl_center-cc_tmp).z >=0) );
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
