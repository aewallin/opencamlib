/*  $Id$
 * 
 *  Copyright (c) 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com).
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

#include "conecutter.hpp"
#include "compositecutter.hpp" // for offsetCutter()
#include "numeric.hpp"

namespace ocl
{

ConeCutter::ConeCutter() {
    assert(0);
}

ConeCutter::ConeCutter(double d, double a, double l) {
    diameter = d;
    radius = d/2.0;
    angle = a;
    assert( angle > 0.0 );
    length = radius/tan(angle) + l;
    center_height = radius/tan(angle);
    assert( center_height > 0.0 );
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
                   //   normal_length, center_height, xy_normal_length, fiber, interval, triangle
    if ( generalFacetPush( 0, this->center_height, this->xy_normal_length , fib, i ,t) ) // BASE
        result = true;

    return result;
}

// cone is pushed along Fiber f into contact with edge p1-p2
bool ConeCutter::generalEdgePush(const Fiber& f, Interval& i,  const Point& p1, const Point& p2) const {
    bool result = false;
    
    if ( isZero_tol(p2.z-p1.z) ) // guard against horizontal edge
        return result;
    assert( (p2.z-p1.z) != 0.0 );
    // idea: as the ITO-cone slides along the edge it will pierce a z-plane at the height of the fiber
    // the shaped of the pierced area is either a circle if the edge is steep
    // or a 'half-circle' + cone shape if the edge is shallow (ice-cream cone...)
    // we can now intersect this 2D shape with the fiber and get the CL-points.

    // this is where the ITO cone pierces the z-plane of the fiber
    // edge-line: p1+t*(p2-p1) = zheight
    // => t = (zheight - p1)/ (p2-p1)  
    double t_tip = (f.p1.z - p1.z) / (p2.z-p1.z);
    Point p_tip = p1 + t_tip*(p2-p1);
    assert( isZero_tol( abs(p_tip.z-f.p1.z) ) ); // p_tip should be in plane of fiber
    
    // this is where the ITO cone base exits the plane
    double t_base = (f.p1.z+center_height - p1.z) / (p2.z-p1.z);
    Point p_base = p1 + t_base*(p2-p1);
    p_base.z = f.p1.z; // project to plane of fiber
    double L = (p_base-p_tip).xyNorm(); 
    
    //if ( L <= radius ){ // this is where the ITO-slice is a circle
        // find intersection points, if any, between the fiber and the circle
        // fiber is f.p1 - f.p2
        // circle is centered at p_base 
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
            if (discr >= 0.0 ) {
                if ( discr == 0.0 ) { // tangent case
                    double x_tang =  ( det*dy  )/ square(dr);
                    double y_tang = -( det*dx  )/ square(dr);
                    Point p_tang(x_tang+p_base.x, y_tang+p_base.y); // translate back from (0,0) system!
                    double t_tang = f.tval( p_tang );
                    if ( circle_CC( t_tang, p1, p2, f, i) ) {
                        result = true;
                    }
                } else {
                    // two intersection points with the base-circle
                    double x_pos = (  det*dy + sign(dy) * dx * sqrt( discr ) ) / square(dr);
                    double y_pos = ( -det*dx + fabs(dy)  * sqrt( discr ) ) / square(dr); 
                    Point cl_pos(x_pos+p_base.x, y_pos+p_base.y);
                    double t_pos = f.tval( cl_pos );
                    // the same with "-" sign:
                    double x_neg = (  det*dy - sign(dy) * dx * sqrt( discr ) ) / square(dr);
                    double y_neg = ( -det*dx - fabs(dy)  * sqrt( discr ) ) / square(dr); 
                    Point cl_neg(x_neg+p_base.x, y_neg+p_base.y);
                    double t_neg = f.tval( cl_neg );
                    if ( circle_CC( t_pos, p1, p2, f, i) ) 
                        result = true;

                    if ( circle_CC( t_neg, p1, p2, f, i) ) 
                        result = true;

                }
            }
        }
        
    //} // circle-case
    
    if ( L > radius ) {
        // ITO-slice is cone + "half-circle"        
        // lines from p_tip to tangent points of the base-circle
        
        // this page has an analytic solution:
        // http://mathworld.wolfram.com/CircleTangentLine.html
        // this page has a geometric construction:
        // http://www.mathopenref.com/consttangents.html
        
        // circle p_base, radius
        // top    p_tip
        
        // tangent point at intersection of base-circle with this circle:
        Point p_mid = 0.5*( p_base + p_tip );
        p_mid.z = f.p1.z;
        double r_tang = L/2;
        
        // circle-circle intersection to find tangent-points
        // three cases: no intersection point
        //              one intersection point
        //              two intersection points

        //d is the distance between the circle centers
        Point pd = p_mid - p_base; 
        pd.z = 0;
        double dist = pd.xyNorm(); //distance between the circles
        
        //Check for special cases which do not lead to solutions we want
        bool case1 = ( isZero_tol( dist )  && isZero_tol( fabs(radius - r_tang) ) );
        bool case2 = (dist > (radius + r_tang) );  //no solution. circles do not intersect
        bool case3 = ( dist < fabs(radius - r_tang));   //no solution. one circle is contained in the other
        bool case4 = ( isZero_tol( dist - (radius+r_tang) ) );  // tangent case
        if (case1 || case2 || case3 || case4 ) {
            
        } else {
            // here we know we have two solutions.

            //Determine the distance from point 0 to point 2
            // law of cosines (http://en.wikipedia.org/wiki/Law_of_cosines)
            // rt^2 = d^2 + r^2 -2*d*r*cos(gamma)
            // so cos(gamma) = (rt^2-d^2-r^2)/ -(2*d*r)
            // and the sought distance is
            // a = r*cos(gamma) = (-rt^2+d^2+r^2)/ (2*d) 
            double a = ( -square(r_tang) + square(radius) + square(dist) ) / (2.0 * dist);
            assert( a >= 0.0 );

            Point v2 = p_base + (a/dist)*pd; // v2 is the point where the line through the circle intersection points crosses the line between the circle centers.  
            //Determine the distance from v2 to either of the intersection points
            double h = sqrt( square(radius) - square(a) );
            //Now determine the offsets of the intersection points from point 2
            Point ofs( -pd.y * (h / dist), pd.x * (h / dist) );
            // now we know the tangent-points
            Point tang1 = v2 + ofs;
            Point tang2 = v2 - ofs;
            if ( cone_CC( tang1, p_tip, p_base, p1, p2, f, i ) ) 
                result = true;
            if ( cone_CC( tang2, p_tip, p_base, p1, p2, f, i ) ) 
                result = true;
        }
    } // end circle+cone case
    
    return result;
}

// test for intersection with the fiber and a tip-tang line
// if there is an intersection in tip-tang, calculate the cc-point and update the interval
bool ConeCutter::cone_CC(const Point& tang, 
                         const Point& tip, 
                         const Point& base, 
                         const Point& p1, 
                         const Point& p2,
                         const Fiber& f, 
                         Interval& i) const {
    double u,t;
    if ( xy_line_line_intersection( f.p1, f.p2, u, tang, tip, t) ) {
        if ( (t>=0.0) && (t<=1.0) ) {
            CCPoint cc_tmp = base + t*(tip-base);
            cc_tmp.z_projectOntoEdge(p1,p2);
            cc_tmp.type = EDGE_CONE;
            return i.update_ifCCinEdgeAndTrue( u, cc_tmp, p1, p2, (true) ) ;
        }
    }
    return false;
}

// t is a position along the fiber
// p1-p2 is the edge
// Interval& i is updated
bool ConeCutter::circle_CC( double t, const Point& p1, const Point& p2, const Fiber& f, Interval& i) const {
    // cone base circle is center_height above fiber
    double t_cc = (f.p1.z+center_height - p1.z) / (p2.z-p1.z);  // t-parameter of the cc-point, center_height above fiber
    CCPoint cc_tmp = p1 + t_cc*(p2-p1); // cc-point on the edge
    cc_tmp.type = EDGE_CONE_BASE;
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
