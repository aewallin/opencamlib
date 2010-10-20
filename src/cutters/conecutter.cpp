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
    if (h< height(radius) )
        return h*tan(angle);
    else
        return radius;
}

// offset of cone is BallConeCutter
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
    assert( isPositive( normal.z ) );
    
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
CC_CLZ_Pair ConeCutter::singleEdgeContact( const Point& u1, const Point& u2) const {
    double d = u1.y;
    double m = (u2.z-u1.z) / (u2.x-u1.x); // slope
    // the outermost point on the cutter is at
    // xu = sqrt( R^2 - d^2 )
    double xu = sqrt( square(radius) - square(u1.y) ); 
    assert( xu <= radius );
    // max slope at xu is mu = (L/(R-R2)) * xu /(sqrt( xu^2 + d^2 ))
    double mu = (center_height/radius ) * xu / sqrt( square(xu) + square(d) ) ;
    // find contact point where slopes match.
    // there are two cases:
    // 1) if abs(m) <= abs(mu)  we contact the curve at xp = sign(m) * sqrt( R^2 m^2 d^2 / (h^2 - R^2 m^2) )
    // 2) if abs(m) > abs(mu) there is contact with the circular edge at +/- xu
    double ccu;
    if (fabs(m) <= fabs(mu) ) { 
        ccu = sign(m) * sqrt( square(radius)*square(m)*square(d) / 
                             (square(length) -square(radius)*square(m) ) );
    } else if ( fabs(m)>fabs(mu) ) { 
        ccu = sign(m)*xu;
    } else { assert(0); }
    Point cc_tmp( ccu, d, 0.0); // cc-point in the XY plane
    cc_tmp.z_projectOntoEdge(u1,u2);
    double cl_z;
    if (fabs(m) <= fabs(mu) ) {  // 1) zc = zp - Lc + (R - sqrt(xp^2 + d^2)) / tan(beta2)
        cl_z = cc_tmp.z - center_height + (radius-sqrt(square(ccu) + square(d)))/ tan(angle);
    } else if ( fabs(m)>fabs(mu) ) {  // 2) zc = zp - Lc
        cl_z = cc_tmp.z - center_height; // case where we hit the edge of the cone
    } else { assert(0); }
    return CC_CLZ_Pair( ccu , cl_z);
}

bool ConeCutter::generalEdgePush(const Fiber& f, Interval& i,  const Point& p1, const Point& p2) const {
    bool result = false;
    // Inverse-Tool-Offset approach: intersect the fiber with planes two sloping planes that contain the edge.
    // fiber: f.p1 + t*(f.p2 - f.p1)
    // plane: ( p - p1 ) dot n = 0       (p1 is in the plane, n is a normal)
    // solve for t:  t= ( p1 - f.p1)dot(n) / (f.p2-f.p1)dot(n)
    Point edge = p2 - p1;
    Point edge_xy(edge.x,edge.y,0.0); // this has the correct direction in the xy-plane
    edge_xy.xyNormalize(); // unit length
    Point tang_xy = edge_xy.xyPerp(); // unit length edge-normal in xy
    Point tangent( tang_xy.x, tang_xy.y, -cos(angle) ) ; // plane tangent?
    assert( isZero_tol( tangent.dot(edge) ) );
    Point n1 = edge.cross( tangent-p1 );
    //Point n1( normal_xy.x, normal_xy.y, cos(angle) ); // flip up at angle 
    double fiber_dot_n = (f.p2-f.p1).dot(n1);
    if (!isZero_tol( fiber_dot_n ) ) {
        double up = (p1-f.p1).dot(n1);
        if ( !isZero_tol( up ) ) {
            double t_cl = up / fiber_dot_n;
            //bool update_ifCCinEdgeAndTrue( double t_cl, CCPoint& cc_tmp, const Point& p1, const Point& p2, bool condition);
            //Point cl = f.point(t);
            CCPoint cc = 0.5*(p1+p2); // DUMMY
            cc.type = EDGE_POS;
            if ( i.update_ifCCinEdgeAndTrue( t_cl , cc , p1 , p2 , (cc.z > f.p1.z) ) )
                result = true;
        } 
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
