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

BullCutter::BullCutter(double d, double r, double l) {
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
double BullCutter::height(double r) const {
    if ( r <= radius1 )
        return 0.0; // cylinder
    else if ( r <= radius )
        return radius2 - sqrt( square(radius2) - square(r-radius1) ); // toroid
    else {
        assert(0);
        return -1;
    }
}

// width of cutter at height h
double BullCutter::width(double h) const {
    if ( h >= radius2 )
        return radius; // cylindrical part
    else 
        return radius1 + sqrt( square(radius2) - square(radius2-h) ); // toroid
}

// drop-cutter: vertex and facet are handled in base-class

// drop-cutter: Toroidal cutter edge-test
CC_CLZ_Pair BullCutter::singleEdgeContact( const Point& u1, const Point& u2 ) const {
    if ( isZero_tol( u1.z - u2.z ) ) {  // horizontal edge special case
        if ( u1.y <= radius1) {             // horizontal edge, contact with cylindrical part of cutter 
            return CC_CLZ_Pair( 0 , u1.z);  
        } else { //if (u1.y <= diameter/2) { 
            assert( (u1.y <= diameter/2) );
            // horizontal edge, toroid region (q-r1)^2 + h2^2 = r2^2  => h2 = sqrt( r2^2 - (q-r1)^2 )
            // h1 = r2 - h2   and  cutter_tip = p.z - h1   
            double h1 = radius2 - sqrt( square(radius2) - square( u1.y - radius1) ); // call height() instead??
            return CC_CLZ_Pair( 0 , u1.z - h1);  
        }
    } else { // the general offset-ellipse case
        double b_axis = radius2;                            // short axis of ellipse = radius2
        double theta = atan( (u2.z - u1.z) / (u2.x-u1.x) ); // theta is the slope of the line
        double a_axis = fabs( radius2/sin(theta) );         // long axis of ellipse = radius2/sin(theta)       
        assert( a_axis > 0.0 );
        assert( b_axis > 0.0 );
        Point ellcenter(0,u1.y,0);
        Ellipse e = Ellipse( ellcenter, a_axis, b_axis, radius1);
        Point ucl(0,0,0); 
        int iters = e.solver_brent( ucl );
        assert( iters < 200 );
        e.setEposHi(u1,u2); // this selects either epos1 or epos2 and sets it to epos_hi
        Point ell_ccp = e.ePointHi();  // pseudo cc-point on the ellipse/cylinder, in the CL=origo system
        if ( fabs( ell_ccp.xyNorm() - radius1 ) > 1E-5 ) { // ell_ccp should be on the cylinder-circle   
            std::cout << " eccen=" << e.eccen << " ell_cpp=" << ell_ccp << "radius1="<< radius1 <<"\n";
            std::cout << " ell_ccp.xyNorm() - radius1 =" << ell_ccp.xyNorm() - radius1 << "\n";
            assert(0);
        }
        Point cc_tmp_u = ell_ccp.closestPoint(u1,u2); // find real cc-point
        return CC_CLZ_Pair( cc_tmp_u.x , e.center.z-radius2);
    }
}

// push-cutter: vertex and facet handled by base-class

bool BullCutter::generalEdgePush(const Fiber& f, Interval& i,  const Point& p1, const Point& p2) const {
    bool result = false;
    if ( isZero_tol( p2.z-p1.z ) )
        return result;
    assert( fabs(p2.z-p1.z) > 0.0 ); // guard against horiz edges
    
    const Point ufp1 = f.p1 + Point(0,0,radius2); // take a fiber which is raised up by radius2
    const Point ufp2 = f.p2 + Point(0,0,radius2);
    // p1+t*(p2-p1) = ufp1.z   =>  
    double tplane = (ufp1.z - p1.z ) / (p2.z-p1.z); // intersect edge with plane at z = ufp1.z
    Point ell_center = p1+tplane*(p2-p1);       assert( isZero_tol( fabs(ell_center.z - ufp1.z) ) );
    Point major_dir = p2-p1;                    
    major_dir.z = 0;
    major_dir.xyNormalize();
    Point minor_dir = major_dir.xyPerp();
    double theta = atan( (p2.z - p1.z) / (p2-p1).xyNorm() ); 
    double major_length = fabs( radius2/sin(theta) ) ;
    double minor_length = radius2;                      //assert( major_length >= minor_length );
    AlignedEllipse e = AlignedEllipse(ell_center, major_length, minor_length, radius1,  major_dir, minor_dir );
    // now we want the offset-ellipse point to lie on the fiber
    Fiber fu(ufp1,ufp2);
    if ( e.aligned_solver( fu ) ) {
        Point pseudo_cc = e.ePoint1(); // pseudo cc-point on ellipse and cylinder
        Point pseudo_cc2 = e.ePoint2();
        CCPoint cc = pseudo_cc.closestPoint(p1,p2);
        CCPoint cc2 = pseudo_cc2.closestPoint(p1,p2);
        cc.type = EDGE_POS;
        cc2.type = EDGE_POS;
        Point cl = e.oePoint1() - Point(0,0,center_height);         assert( isZero_tol( fabs(cl.z - f.p1.z)) );
        Point cl2 = e.oePoint2() - Point(0,0,center_height);         assert( isZero_tol( fabs(cl2.z - f.p1.z)) );
        double cl_t = f.tval(cl);
        double cl_t2 = f.tval(cl2);
        if ( f.p1.y == f.p2.y ) {
            std::cout << " cl.y on fiber.y check \n";
            if ( !isZero_tol( fabs(cl.y - f.p1.y) ) )
                std::cout << " cl.y =" << cl.y << "  fiber.y=" << f.p1.y << " \n";
            assert( isZero_tol( fabs(cl.y - f.p1.y) ) );
            assert( isZero_tol( fabs(cl2.y - f.p1.y) ) );
        } 
            
        // bool update_ifCCinEdgeAndTrue( double t_cl, CCPoint& cc_tmp, const Point& p1, const Point& p2, bool condition);
        if ( i.update_ifCCinEdgeAndTrue( cl_t, cc, p1, p2, true ) )
            result = true;
        if ( i.update_ifCCinEdgeAndTrue( cl_t2, cc2, p1, p2, true ) )
            result = true;
    }
    return result;
}



MillingCutter* BullCutter::offsetCutter(double d) const {
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
