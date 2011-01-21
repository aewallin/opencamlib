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

#include "conecutter.h"
#include "compositecutter.h" // for offsetCutter()
#include "numeric.h"

namespace ocl
{

ConeCutter::ConeCutter() {
    assert(0);
}

ConeCutter::ConeCutter(double d, double a, double l) {
    diameter = d;
    radius = d/2.0;
    angle = a;
    length = radius/tan(angle) + l;
    center_height = radius/tan(angle);
    xy_normal_length = radius;
    normal_length = 0.0;
}

double ConeCutter::height(double r) const {
    assert( tan(angle) > 0.0 ); // guard against division by zero
    return r/tan(angle);
}

double ConeCutter::width(double h) const {
    return (h<center_height) ? h*tan(angle) : radius ;
}

// ?? Ball-Cone-Bull ??
MillingCutter* ConeCutter::offsetCutter(double d) const {
    return new BallConeCutter(2*d,  diameter+2*d, angle) ;
}

// because this checks for contact with both the tip and the circular edge it is hard to move to the base-class
// we either hit the tip, when the slope of the plane is smaller than angle
// or when the slope is steep, the circular edge between the cone and the cylindrical shaft
bool ConeCutter::facetDrop(CLPoint &cl, const Triangle &t) const {
    bool result = false;
    Point normal = t.upNormal(); // facet surface normal    
    if ( isZero_tol( normal.z ) )  // vertical surface
        return false;  //can't drop against vertical surface
    
    if ( (isZero_tol(normal.x)) && (isZero_tol(normal.y)) ) {  // horizontal plane special case
        CCPoint cc_tmp( cl.x, cl.y, t.p[0].z, FACET_TIP );  // so any vertex is at the correct height
        return cl.liftZ_if_inFacet(cc_tmp.z, cc_tmp, t);
    } else {
        // define plane containing facet
        // a*x + b*y + c*z + d = 0, so
        // d = -a*x - b*y - c*z, where  (a,b,c) = surface normal
        double a = normal.x;
        double b = normal.y;
        double c = normal.z;
        double d = - normal.dot(t.p[0]); 
        normal.xyNormalize(); // make xy length of normal == 1.0
        // cylindrical contact point case
        // find the xy-coordinates of the cc-point
        CCPoint cyl_cc_tmp =  cl - radius*normal;
        cyl_cc_tmp.z = (1.0/c)*(-d-a*cyl_cc_tmp.x-b*cyl_cc_tmp.y);
        double cyl_cl_z = cyl_cc_tmp.z - length; // tip positioned here
        cyl_cc_tmp.type = FACET_CYL;
        
        // tip contact with facet
        CCPoint tip_cc_tmp(cl.x,cl.y,0.0);
        tip_cc_tmp.z = (1.0/c)*(-d-a*tip_cc_tmp.x-b*tip_cc_tmp.y);
        double tip_cl_z = tip_cc_tmp.z;
        tip_cc_tmp.type = FACET_TIP;
              
        result = result || cl.liftZ_if_inFacet( tip_cl_z, tip_cc_tmp, t);
        result = result || cl.liftZ_if_inFacet( cyl_cl_z, cyl_cc_tmp, t);
        return result; 
    }
}

// cone sliced with vertical plane results in a hyperbola as the intersection curve
// find point where hyperbola and line slopes match
CC_CLZ_Pair ConeCutter::singleEdgeDropCanonical( const Point& u1, const Point& u2) const {
    double d = u1.y;
    double m = (u2.z-u1.z) / (u2.x-u1.x); // slope of edge
    // the outermost point on the cutter is at   xu = sqrt( R^2 - d^2 )
    double xu = sqrt( square(radius) - square(u1.y) );                  assert( xu <= radius );
    // max slope at xu is mu = (L/(R-R2)) * xu /(sqrt( xu^2 + d^2 ))
    double mu = (center_height/radius ) * xu / sqrt( square(xu) + square(d) ) ;
    bool hyperbola_case = (fabs(m) <= fabs(mu));
    // find contact point where slopes match, there are two cases:
    // 1) if abs(m) <= abs(mu)  we contact the curve at xp = sign(m) * sqrt( R^2 m^2 d^2 / (h^2 - R^2 m^2) )
    // 2) if abs(m) > abs(mu) there is contact with the circular edge at +/- xu
    double ccu;
    if ( hyperbola_case ) { 
        ccu = sign(m) * sqrt( square(radius)*square(m)*square(d) / (square(length) -square(radius)*square(m) ) );
    } else { 
        ccu = sign(m)*xu;
    } 
    Point cc_tmp( ccu, d, 0.0); // cc-point in the XY plane
    cc_tmp.z_projectOntoEdge(u1,u2);
    double cl_z;
    if ( hyperbola_case ) {  // 1) zc = zp - Lc + (R - sqrt(xp^2 + d^2)) / tan(beta2)
        cl_z = cc_tmp.z - center_height + (radius-sqrt(square(ccu) + square(d)))/ tan(angle);
    } else {  // 2) zc = zp - Lc
        cl_z = cc_tmp.z - center_height; // case where we hit the edge of the cone
    } 
    return CC_CLZ_Pair( ccu , cl_z);
}

// cone is pushed along Fiber f into contact with edge p1-p2
bool ConeCutter::generalEdgePush(const Fiber& f, Interval& i,  const Point& p1, const Point& p2) const {
    bool result = false;
    double m = (p2.z-p1.z) / (p2-p1).xyNorm() ; // edge slope
    double tanangle = tan(angle);
    // general quadratic form:
    // Ax^2 + Bxy + Cy^2 + Dx + Ey + F =0
    // =>
    // Ax^2 + Bxy + Cy^2 = -(Dx + Ey + F)
    // or intersection btw plane z = -(Dx + Ey + F)
    // and quadratic z = Ax^2 + Bxy + Cy^2 
    
    // cone quadratic is (1/c^2)*(x^2+y^2) = (z-z0)^2     
    //          where c=r/h ratio of radius to height (opening angle), and z0= height above z=0
    // sqrt(x^2 +y^2) = c ( z -  z0 ) 
    // => z = z0 + (1/c)*sqrt(x^2+y^2)   CONE
    //
    // plane   D*x + E*y + G*z + F = 0, so
    // F = -A*x - E*y - G*z, where  (A,E,G) = surface normal
    // p1 and p2 are in plane, so is p1+f.dir
    // normal = (p2-p1).cross(p1+f.dir)
    Point planeNormal = (p2-p1).cross(p1+f.dir);
    // now
    // z = -(1/G)*(Dx + Ey + F)   PLANE
    //
    // set equal:  CONE = PLANE
    //
    //  z = z0 + (1/c)*sqrt(x^2+y^2) = -(1/G)*(Dx + Ey + F)   
    // aĺfa = cD/G
    // beta = cE/G
    // lam  = cF/G + cz0
    // quadratic:
    // (1-alfa^2)x^2 + (1-beta^2)y^2 + 2*aflfa*beta -2*alfa*lam*x - 2*beta*lam*y - lam^2 = 0
    // Ax^2 + Bxy + Cy^2 + Dx + Ey + F =0   (standard form)
    // identify coefficients as:
    // A = 1-alfa^2
    // B = 2*alfa*beta
    // C = 1-beta^2
    // D = -2*alfa*lam
    // E = -2*beta*lam
    // F = -lam^2
    // discriminant = B^2 - 4AC
    // < 0 ellipse
    // ==0 parabola
    // > 0 hyperbola
    
    if (fabs(m) > tanangle ) {
        // hyperbola case
    } else {
        // ellipse case
    }

    return result;
}



std::string ConeCutter::str() const {
    std::ostringstream o;
    o << *this;
    return o.str();
}

std::ostream& operator<<(std::ostream &stream, ConeCutter c) {
  stream << "ConeCutter (d=" << c.diameter << ", angle=" << c.angle << ", L=" << c.length << ")";
  return stream;
}

} // end namespace
// end file conecutter.cpp
