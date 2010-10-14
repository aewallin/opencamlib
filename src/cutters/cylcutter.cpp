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

#include "cylcutter.h"
#include "bullcutter.h" // for offsetCutter()
#include "numeric.h"

namespace ocl
{

CylCutter::CylCutter() {
    std::cout << " usage: CylCutter( double diameter, double length ) \n";
    assert(0);
}

CylCutter::CylCutter(const double d, const double l) {
    assert( d > 0.0);
    diameter= d;
    radius= d/2.0;
    assert( l > 0.0 );
    length = l;
    xy_normal_length = radius;
    normal_length = 0.0;
    center_height = 0.0;
}

MillingCutter* CylCutter::offsetCutter(const double d) const {
    return new BullCutter(diameter+2*d, d, length+d) ; // offset of CylCutter is BullCutter
}

// height of cutter at radius r
double CylCutter::height(const double r) const {
    if ( r <= radius )
        return 0.0;
    else {
        assert(0);
        return -1;
    }
}

// width of cutter at height h
double CylCutter::width(const double h) const {
    return radius;
}

// drop-cutter vertexDrop is handled by the base-class
// drop-cutter facetDrop is handled by the base-class

bool CylCutter::singleEdgeDrop(CLPoint& cl, const Point& p1, const Point& p2, const double d) const {
    bool result=false;
    // 2) calculate intersection points with cutter circle.
    // points are on line and radius from cl.
    // see http://mathworld.wolfram.com/Circle-LineIntersection.html
    double x1 = p1.x - cl.x; // translate to cl=(0,0)
    double y1 = p1.y - cl.y;
    double x2 = p2.x - cl.x;
    double y2 = p2.y - cl.y;
    double dx = x2-x1;
    double dy = y2-y1;
    double dr_sq = dx*dx + dy*dy;
    double dr = sqrt( dr_sq );
    double D = x1*y2 - x2*y1;
    double discr = square( radius ) * square(dr) - square(D);
    if ( !isZero_tol(discr) && isNegative(discr) ) {
        std::cout << "cutter.cpp ERROR: CylCutter::edgeTest discr= "<<discr<<" <0 !!\n";
        assert(0);
        return false;
    } else if ( isZero_tol(discr) ) {// discr==0.0 means line is tangent to cutter circle
        CCPoint cc_tmp;
        cc_tmp.x =  D*dy / dr_sq + cl.x; // translate back to cl
        cc_tmp.y = -D*dx / dr_sq + cl.y;
        cc_tmp.z_projectOntoEdge(p1,p1);
        cc_tmp.type = EDGE;
        result = result || cl.liftZ_if_InsidePoints(cc_tmp.z, cc_tmp, p1, p2);
    } else { // discr > 0, two intersection points
        assert( discr > 0.0 );
        CCPoint cc1;
        CCPoint cc2;
        double sqrt_discr = sqrt(discr);
        cc1.x= ( D*dy + sign(dy)*dx*sqrt_discr) / dr_sq + cl.x; // remember to translate back to cl
        cc1.y= (-D*dx + fabs(dy)*sqrt_discr   ) / dr_sq + cl.y;
        cc1.z_projectOntoEdge(p1,p2);
        cc1.type = EDGE;
        
        cc2.x= ( D*dy - sign(dy)*dx*sqrt_discr) / dr_sq + cl.x;
        cc2.y= (-D*dx - fabs(dy)*sqrt_discr   ) / dr_sq + cl.y;
        cc2.z_projectOntoEdge(p1,p2);
        cc2.type = EDGE;
        
        result = result || cl.liftZ_if_InsidePoints(cc1.z, cc1, p1, p2);
        result = result || cl.liftZ_if_InsidePoints(cc2.z, cc2, p1, p2);
    } //end two intersection points case
    return result;
}

// push cutter: facet handled in base-class

// push cutter along Fiber against vertices of Triangle t
// update Interval i 
bool CylCutter::vertexPush(const Fiber& f, Interval& i, const Triangle& t) const {
    bool result = false;
    std::vector<Point> verts;
    verts.push_back(t.p[0]);
    verts.push_back(t.p[1]);
    verts.push_back(t.p[2]);
    // if the triangle is sliced, we have two more verts to test:
    Point p1,p2;
    if ( t.zslice_verts(p1, p2, f.p1.z) ) {
        p1.z = p1.z + 1E-3; // dirty trick...
        p2.z = p2.z + 1E-3; // ...which will not affect results, unless cutter.length < 1E-3
        verts.push_back(p1);
        verts.push_back(p2);
    }
    BOOST_FOREACH( const Point& p, verts) {
        if ( (p.z >= f.p1.z) && (p.z <= (f.p1.z+getLength())) ) { // p.z is within cutter
            Point pq = p.xyClosestPoint(f.p1, f.p2);
            double q = (p-pq).xyNorm(); // distance in XY-plane from fiber to p
            if ( q <= radius ) { // p is inside the cutter
                double ofs = sqrt( square(radius) - square(q) ); // distance along fiber 
                Point start = pq - ofs*f.dir;
                Point stop  = pq + ofs*f.dir;
                CCPoint cc_tmp = CCPoint(p);
                cc_tmp.type = VERTEX;
                //std::cout << "updating with " << f.tval(stop) << " to " << f.tval(start) << "\n";
                i.updateUpper( f.tval(stop) , cc_tmp );
                i.updateLower( f.tval(start) , cc_tmp );
                result = true;                
            }             
        }
    }
    return result;
}

bool CylCutter::edgePush(const Fiber& f, Interval& i,  const Triangle& t) const {
    bool result = false;
    for (int n=0;n<3;n++) { // loop through all three edges
        int start=n;
        int end=(n+1)%3;
        Point p1 = t.p[start];
        Point p2 = t.p[end];
        // check that there is an edge in the xy-plane
        // can't push against vertical edges ??
        if ( !isZero_tol( p1.x - p2.x ) || !isZero_tol( p1.y - p2.y) ) {
            // fiber is f.p1 + t*(f.p2-f.p1)
            // line  is p1 + v*(p2-p1)
            double tq, v;
            if ( xy_line_line_intersection(p1, p2, v, f.p1, f.p2, tq ) ) {  // find XY-intersection btw fiber and edge
                Point q = p1 + v*(p2-p1); // intersection point, on edge
                // Point q = f.p1 + tq*(f.p2-f.p1);
                // from q, go v-units along tangent, then r*normal, and end up on fiber:
                // q + t*tangent + r*normal = p1 + t*(p2-p1)
                double v_cc, t_cl;
                Point xy_tang=p2-p1;
                xy_tang.z=0;
                xy_tang.xyNormalize();
                Point xy_normal = xy_tang.xyPerp();
                Point q1 = q+radius*xy_normal;
                Point q2 = q1+(p2-p1);
                if ( xy_line_line_intersection( q1 , q2, v_cc, f.p1, f.p2, t_cl ) ) {
                    double t_cl1 = t_cl;
                    double t_cl2 = tq + (tq-t_cl );
                    CCPoint cc_tmp1 = q+v_cc*(p2-p1);
                    CCPoint cc_tmp2 = q-v_cc*(p2-p1); 
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
            } // end if(fiber and edge intersect)
        } // end if(vertical edge)
    } // end loop through all edges
    return result;
}   

std::string CylCutter::str() const {
    std::ostringstream o;
    o << *this;
    return o.str();
}

std::ostream& operator<<(std::ostream &stream, CylCutter c) {
  stream << "CylCutter (d=" << c.diameter << ", L=" << c.length <<  ")";
  return stream;
}

} // end namespace
// end file cylcutter.cpp
