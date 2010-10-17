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

#include "bullcutter.h"
#include "numeric.h"
#include "oellipse.h"

namespace ocl
{

BullCutter::BullCutter() {
    std::cout << " usage: BullCutter( double diameter, double corner_radius, double length ) \n";
    assert(0);
}

BullCutter::BullCutter(const double d, const double r, const double l) {
    assert( d > 0.0 );
    diameter = d;
    radius = d/2.0;        // total cutter radius
    radius1 = d/2.0 - r;   // cylindrical middle part radius
    assert( radius1 > 0.0 );
    radius2 = r;         // corner radius
    assert( l > 0.0 );
    length = l;
    xy_normal_length = radius1;
    normal_length = radius2;
    center_height = radius2;
}

// height of cutter at radius r
double BullCutter::height(const double r) const {
    if ( r <= radius1 )
        return 0.0;
    else if ( r <= radius )
        return radius2 - sqrt( square(radius2) - square(r-radius1) );
    else {
        assert(0);
        return -1;
    }
}

// width of cutter at height h
double BullCutter::width(const double h) const {
    if ( h >= radius2 )
        return radius; // cylindrical part
    else // toroid
        return radius1 + sqrt( square(radius2) - square(radius2-h) );
}

// drop-cutter: vertex and facet are handled in base-class

// drop-cutter: Toroidal cutter edge-test
CC_CLZ_Pair BullCutter::singleEdgeContact(const Point& u1, const Point& u2) const {
    if ( isZero_tol( u1.z - u2.z ) ) {  // horizontal edge special case
        if ( u1.y <= radius1) {             // horizontal edge, contact with cylindrical part of cutter 
            return CC_CLZ_Pair( 0 , u1.z);  
        } else { //if (u1.y <= diameter/2) { 
            assert( (u1.y <= diameter/2) );
            // horizontal edge, toroid region 
            // (q-r1)^2 + h2^2 = r2^2
            // h2 = sqrt( r2^2 - (q-r1)^2 )
            // h1 = r2 - h2   and  cutter_tip = p.z - h1   
            double h1 = radius2 - sqrt( square(radius2) - square( u1.y - radius1) );
            return CC_CLZ_Pair( 0 , u1.z - h1);  
        }
    } else {
        // the general offset-ellipse case
        //  short axis of ellipse = radius2
        //  long axis of ellipse = radius2/sin(theta)       (theta is the slope of the line)
        double b_axis = radius2;
        double theta = atan( (u2.z - u1.z) / (u2.x-u1.x) ); 
        double a_axis = fabs( radius2/sin(theta) );
        assert( a_axis > 0.0 );
        assert( b_axis > 0.0 );
        Point ellcenter(0,u1.y,0);
        Ellipse e = Ellipse( ellcenter, a_axis, b_axis, radius1);
        Point ucl(0,0,0); 
        int iters = e.solver_brent( ucl );
        assert( iters < 200 );
        Point ecen1 = e.calcEcenter( u1, u2, 1);
        Point ecen2 = e.calcEcenter( u1, u2, 2);
        Point ecen;
        Epos pos_hi;
        if (ecen1.z >= ecen2.z) { // we want the higher center
            ecen = ecen1;
            pos_hi = e.epos1;
        } else {
            ecen = ecen2;
            pos_hi = e.epos2;
        } 
        e.center = ecen; 
        Point ell_ccp = e.ePoint(pos_hi);  // pseudo cc-point on the ellipse/cylinder, in the CL=origo system
        if ( fabs( ell_ccp.xyNorm() - radius1 ) > 1E-5 ) { // ell_ccp should be on the cylinder-circle   
            std::cout << " eccen=" << e.eccen << " ell_cpp=" << ell_ccp << "radius1="<< radius1 <<"\n";
            std::cout << " ell_ccp.xyNorm() - radius1 =" << ell_ccp.xyNorm() - radius1 << "\n";
            assert(0);
        }
        // find real cc-point
        Point cc_tmp_u = ell_ccp.closestPoint(u1,u2);
        return CC_CLZ_Pair( cc_tmp_u.x , ecen.z-radius2);
    }
}

// push-cutter: vertex and facet handled by base-class

bool BullCutter::edgePush(const Fiber& f, Interval& i,  const Triangle& t) const {
    bool result = false;
    for (int n=0;n<3;n++) { // loop through all three edges
        int start=n;
        int end=(n+1)%3;
        const Point p1 = t.p[start]; // edge is from p1 to p2
        const Point p2 = t.p[end];
        const Point ufp1 = f.p1 + Point(0,0,radius2); // take a fiber which is raised up by radius
        const Point ufp2 = f.p2 + Point(0,0,radius2);
        // find where the plane slices the edge
        // edge: p1+t*(p2-p1)
        if ( isZero_tol( p2.z-p1.z ) ) { // this is the horizontal-edge special case
            double h = p1.z - f.p1.z;
            if ( h > 0.0 ) {
                // the cutter acts as a cylinder with radius:
                double eff_radius = this->width( h ) ;
                // contact this cylinder/circle against edge in xy-plane
                // fiber is f.p1 + qt*(f.p2-f.p1)
                // line  is p1 + qv*(p2-p1)
                double qt;
                double qv;
                if (xy_line_line_intersection( p1 , p2, qv, f.p1, f.p2, qt ) ) {
                    Point q = p1 + qv*(p2-p1); // the intersection point
                    // from q, go v-units along tangent, then eff_r*normal, and end up on fiber:
                    // q + ccv*tangent + r*normal = p1 + clt*(p2-p1)
                    
                    double ccv, clt;
                    Point xy_tang=p2-p1;
                    xy_tang.z=0;
                    xy_tang.xyNormalize();
                    Point xy_normal = xy_tang.xyPerp();
                    Point q1 = q+eff_radius*xy_normal;
                    Point q2 = q1+(p2-p1);
                    if ( xy_line_line_intersection( q1 , q2, ccv, f.p1, f.p2, clt ) ) {
                        double t_cl1 = clt;
                        double t_cl2 = qt + (qt - clt );
                        CCPoint cc_tmp1 = q+ccv*(p2-p1);
                        CCPoint cc_tmp2 = q-ccv*(p2-p1); 
                        cc_tmp1.type = EDGE;
                        cc_tmp2.type = EDGE;
                        if( cc_tmp1.isInsidePoints(p1,p2) && (cc_tmp1.z >= f.p1.z) ) {
                            i.update( t_cl1  , cc_tmp1 );
                            result = true;
                        }
                        if( cc_tmp2.isInsidePoints( p1,p2 ) && (cc_tmp2.z >= f.p1.z) ) {
                            i.update( t_cl2  , cc_tmp2 );
                            result = true;
                        }
                    }
                }
            }
        } else { // general non-horizontal-edge  case
            double tplane = (ufp1.z - p1.z ) / (p2.z-p1.z);
            Point ell_center = p1+tplane*(p2-p1);
            assert( isZero_tol( fabs(ell_center.z - ufp1.z) ) );
            // find the dimensions of the ellipse
            Point major_dir = p2-p1;
            major_dir.z = 0;
            major_dir.xyNormalize();
            Point minor_dir = major_dir.xyPerp();
            //Point minor_axis = radius2*minor_dir;
            assert( (p2-p1).xyNorm() > 0.0 ); // avoid divide-by-zero
            double theta = atan( (p2.z - p1.z) / (p2-p1).norm() ); 
            double major_axis_length = fabs( radius2/sin(theta) );
            //Point major_axis = major_axis_length*major_dir;
            AlignedEllipse e = AlignedEllipse(ell_center, major_axis_length, radius2, radius1, major_dir, minor_dir );
            // now we want the offset-ellipse point to lie on the fiber
            // take the distance along major_axis as the error to be minimized
            
        }
    }
    return result;
}

MillingCutter* BullCutter::offsetCutter(const double d) const {
    return new BullCutter(diameter+2*d, radius2+d, length+d) ;
}

std::string BullCutter::str() const {
    std::ostringstream o;
    o << *this;
    return o.str();
}

std::ostream& operator<<(std::ostream &stream, BullCutter c) {
  stream << "BullCutter(d=" << c.diameter << ", r1=" << c.radius1 << " r2=" << c.radius2 << ", L=" << c.length <<  ")";
  return stream;
}

} // end namespace
// end file bullcutter.cpp
