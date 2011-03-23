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
    // grows from zero up to radius
    // above that (cutter shaft) return radius
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

bool ConeCutter::facetPush(const Fiber& fib, Interval& i,  const Triangle& t) const {
    // push two objects: tip, and base-circle
    bool result = false;
    if ( generalFacetPush( 0, 0, 0, fib, i, t) ) // TIP
        result = true;
    if ( generalFacetPush( 0, this->center_height, this->xy_normal_length , fib, i ,t) ) // BASE
        result = true;
        
    return result;
}

// cone is pushed along Fiber f into contact with edge p1-p2
bool ConeCutter::generalEdgePush(const Fiber& f, Interval& i,  const Point& p1, const Point& p2) const {
    bool result = false;
    
    if ( isZero_tol(p2.z-p1.z) ) // guard agains horizontal edge
        return result;
    assert( (p2.z-p1.z) != 0.0 );
    // idea: as the ITO-cone slides along the edge it will pierce a z-plane at the height of the fiber
    // the shaped of the pierced area is either a circle if the edge is steep
    // or a 'half-circle' + cone shape if the edge is shallow (ice-cream cone...)
    // we can now intersect this 2D shape with the fiber and get the CL-points.
    // how to get the CC-point? (point on edge closest to z-axis of cutter? closest to CL?)


    
    // this is where the ITO cone pierces the plane
    // edge-line: p1+t*(p2-p1) = zheight
    // => t = (zheight - p1)/ (p2-p1)  
    double t_tip = (f.p1.z - p1.z) / (p2.z-p1.z);
    if (t_tip < 0.0 )
        t_tip = 0.0;
    Point p_tip = p1 + t_tip*(p2-p1);
    assert( isZero_tol( abs(p_tip.z-f.p1.z) ) ); // p_tip should be in plane of fiber
    
    // this is where the ITO cone base exits the plane
    double t_base = (f.p1.z+center_height - p1.z) / (p2.z-p1.z);
    Point p_base = p1 + t_base*(p2-p1);
    p_base.z = f.p1.z; // project to plane of fiber
    //std::cout << "(t0, t1) (" << t0 << " , " << t1 << ") \n";
    double L = (p_base-p_tip).xyNorm(); 
    
    if ( L <= radius ) { // this is where the ITO-slice is a circle
        // find intersection points, if any, between the fiber and the circle
        // fiber is f.p1 - f.p2
        // circle is centered at p_base and radius
        double d = p_base.xyDistanceToLine(f.p1, f.p2);
        if ( d <= radius ) {
            // we know there is an intersection point.
            // http://mathworld.wolfram.com/Circle-LineIntersection.html
            
            // subtract circle center, math is for circle centered at (0,0)
            double dx = f.p2.x - f.p1.x;
            double dy = f.p2.y - f.p1.y;
            double dr = sqrt( square(dx) + square(dy) );
            double det = (f.p1.x-p_base.x) * (f.p2.y-p_base.y) - (f.p2.x-p_base.x) * (f.p1.y-p_base.y);
            
            // intersection given by:
            //  x = det*dy +/- sign(dy) * dx * sqrt( r^2 dr^2 - det^2 )   / dr^2
            //  y = -det*dx +/- abs(dy)  * sqrt( r^2 dr^2 - det^2 )   / dr^2
            
            double discr = square(radius) * square(dr) - square(det);
            assert( discr > 0.0 ); // this means we have an intersection
            if ( discr == 0.0 ) { // tangent case
                double x_tang =  ( det*dy  )/ square(dr);
                double y_tang = -( det*dx  )/ square(dr);
                Point p_tang(x_tang+p_base.x, y_tang+p_base.y); // translate back from (0,0) system!
                double t_tang = f.tval( p_tang );
                if ( circle_CC( t_tang, p1, p2, f, i) )
                    result = true;
            } else {
                // two intersection points
                double x_pos = (  det*dy + sign(dy)* dx * sqrt( discr ) ) / square(dr);
                double y_pos = ( -det*dx + abs(dy)  * sqrt( discr ) ) / square(dr); 
                Point p_pos(x_pos+p_base.x, y_pos+p_base.y);
                double t_pos = f.tval( p_pos );
                // the same with "-" sign:
                double x_neg = (  det*dy - sign(dy) * dx * sqrt( discr ) ) / square(dr);
                double y_neg = ( -det*dx - abs(dy)  * sqrt( discr ) ) / square(dr); 
                Point p_neg(x_neg+p_base.x, y_neg+p_base.y);
                double t_neg = f.tval( p_neg );
                if ( circle_CC( t_pos, p1, p2, f, i) ) 
                    result = true;
                if ( circle_CC( t_neg, p1, p2, f, i) ) 
                    result = true;
            }
        }
        return result;
    } else {
        // ITO-slice is cone + half-circle        
        // lines from p_tip to tangent points
        assert( L > radius );
        // http://mathworld.wolfram.com/CircleTangentLine.html
        // circle centered at x0, y0, radius a
        // tangent through (0,0)
        // t = +/- acos(  -a*x0 +/- y0*sqrt(x0^2+y0^2-a^2) / (x0^2+y0^2) )
        // translate so p_mid is at (0,0)
        //Point c = p_base - p_mid;
        //double cos1 = (-radius*c.x + c.y*sqrt(square(c.x)+square(c.y)+square(radius)) )/ (square(c.x) + square(c.y) );
        //double cos2 = (-radius*c.x - c.y*sqrt(square(c.x)+square(c.y)+square(radius)) )/ (square(c.x) + square(c.y) );
        
        
        return result;
    }
}


// t is a position along the fiber
// p1-p2 is the edge
// Interval& i is updated
bool ConeCutter::circle_CC( double t, const Point& p1, const Point& p2, const Fiber& f, Interval& i) const {
    // cone base circle is center_height above fiber
    double t_cc = (f.p1.z+center_height - p1.z) / (p2.z-p1.z); 
    CCPoint cc_tmp = p1 + t_cc*(p2-p1); // cc-point on the edge
    cc_tmp.type = EDGE_CONE;
    return i.update_ifCCinEdgeAndTrue( t, cc_tmp, p1, p2, (true) );
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
