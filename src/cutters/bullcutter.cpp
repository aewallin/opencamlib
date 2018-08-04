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

#include "bullcutter.hpp"
#include "numeric.hpp"

namespace ocl
{

BullCutter::BullCutter() {
    std::cout << " usage: BullCutter( double diameter, double corner_radius, double length ) \n";
    assert(0);
}

BullCutter::BullCutter(double d, double r, double l) {
    diameter = d;               assert( d > 0.0 );
    radius = d/2.0;        // total cutter radius
    radius1 = d/2.0 - r;   // cylindrical middle part radius
    radius2 = r;                assert( radius1 > 0.0 ); // corner radius
    length = l;                 assert( l > 0.0 );
    xy_normal_length = radius1;
    normal_length = radius2;
    center_height = radius2;
}

MillingCutter* BullCutter::offsetCutter(double d) const {
    return new BullCutter(diameter+2*d, radius2+d, length+d) ;
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
    return ( h >= radius2 ) ? radius : radius1 + sqrt(square(radius2)-square(radius2-h)) ;
}

// drop-cutter: vertex and facet are handled in base-class

// drop-cutter: Toroidal cutter edge-test handled here
// called from MillingCutter::singleEdgeDrop()
//
// the canonical geometry is one where (cl.x, cl.y) = (0,0)
// and the edge is rotated to lie along the x-axis.
// since the u1-u2 edge is along the x-axis, the points u1 u2 have the same y-coordinate.
//
// when a radius2 cylinder around the edge is sliced by an XY plane this
// results in an ellipse with a shorter axis of radius2, and a longer axis of radius2/sin(theta)
// where theta is the slope of the edge in the XZ plane
CC_CLZ_Pair BullCutter::singleEdgeDropCanonical( const Point& u1, const Point& u2 ) const {
    if ( isZero_tol( u1.z - u2.z ) ) {  // horizontal edge special case
        return CC_CLZ_Pair( 0 , u1.z - height(u1.y) );
    } else { // the general offset-ellipse case
        double b_axis = radius2;                            // short axis of ellipse = radius2
        double theta = atan( (u2.z - u1.z) / (u2.x-u1.x) ); // theta is the slope of the line
        double a_axis = fabs( radius2/sin(theta) );         // long axis of ellipse = radius2/sin(theta)       
        Point ellcenter(0,u1.y,0);
        Ellipse e = Ellipse( ellcenter, a_axis, b_axis, radius1); // radius1 is the ellipse-offset
        int iters = e.solver_brent(); // numerical solver that searches for a point on the ellipse
                                      // such that a radius1-length normal-vector from this ellipse-point
                                      // is at the CL point (0,0)
        if (iters > 200)
            std::cout << "WARNING: BullCutter::singleEdgeDropCanonical() iters>200 !!\n";
        assert( iters < 200 );
        e.setEllipsePositionHi(u1,u2); // this selects either EllipsePosition1 or 
                                       // EllipsePosition2 and sets it to EllipsePosition_hi
        // pseudo cc-point on the ellipse/cylinder, in the CL=(0,0) system
        Point ell_ccp = e.ePointHi();
        assert( fabs( ell_ccp.xyNorm() - radius1 ) < 1E-5); // ell_ccp should be on the cylinder-circle  
        Point cc_tmp_u = ell_ccp.closestPoint(u1,u2); // find cc-point on u1-u2 edge
        return CC_CLZ_Pair( cc_tmp_u.x , e.getCenterZ()-radius2);
    }
}

// push-cutter: vertex and facet handled by base-class
//  edge handled here
bool BullCutter::generalEdgePush(const Fiber& f, Interval& i,  const Point& p1, const Point& p2) const {
    //std::cout << " BullCutter::generalEdgePush() \n";
    bool result = false;
    
    if ( isZero_tol( (p2-p1).xyNorm() ) ) { // this would be a vertical edge
        return result;
    }
    
    if ( isZero_tol( p2.z-p1.z ) ) // this would be a horizontal edge
        return result;
    assert( fabs(p2.z-p1.z) > 0.0 ); // no horiz edges allowed hereafter
    
    // p1+t*(p2-p1) = f.p1.z+radius2   =>  
    double tplane = (f.p1.z + radius2 - p1.z ) / (p2.z-p1.z); // intersect edge with plane at z = ufp1.z
    Point ell_center = p1+tplane*(p2-p1);                               
    assert( isZero_tol( fabs(ell_center.z - (f.p1.z+radius2)) ) );
    Point major_dir = (p2-p1);     
    assert( major_dir.xyNorm() > 0.0 );               
    
    major_dir.z = 0;
    major_dir.xyNormalize();
    Point minor_dir = major_dir.xyPerp();
    double theta = atan( (p2.z - p1.z) / (p2-p1).xyNorm() ); 
    double major_length = fabs( radius2/sin(theta) ) ;
    double minor_length = radius2;
    AlignedEllipse e(ell_center, major_length, minor_length, radius1,  major_dir, minor_dir );
    if ( e.aligned_solver( f ) ) { // now we want the offset-ellipse point to lie on the fiber
        Point pseudo_cc  = e.ePoint1(); // pseudo cc-point on ellipse and cylinder
        Point pseudo_cc2 = e.ePoint2();
        CCPoint cc  = pseudo_cc.closestPoint(p1,p2);
        CCPoint cc2 = pseudo_cc2.closestPoint(p1,p2);
        cc.type  = EDGE_POS;
        cc2.type = EDGE_POS;
        Point cl  = e.oePoint1() - Point(0,0,center_height);            
        assert( isZero_tol( fabs(cl.z - f.p1.z)) );
        Point cl2 = e.oePoint2() - Point(0,0,center_height);            
        assert( isZero_tol( fabs(cl2.z - f.p1.z)) );
        double cl_t  = f.tval(cl);
        double cl_t2 = f.tval(cl2);
        if ( i.update_ifCCinEdgeAndTrue( cl_t, cc, p1, p2, true ) )
            result = true;
        if ( i.update_ifCCinEdgeAndTrue( cl_t2, cc2, p1, p2, true ) )
            result = true;
    }
    //std::cout << " BullCutter::generalEdgePush() DONE result= " << result << "\n";
    return result;
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
