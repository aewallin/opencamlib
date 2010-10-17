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

CC_CLZ_Pair CylCutter::singleEdgeContact(const Point& u1, const Point& u2) const {
    // edge is u1-u2  along the X-axis and the ycoord is 0<d<radius 
    // u1 = (u1x, d, u1z)
    // u2 = (u2x, d, u2z)
    assert( u1.y == u2.y);
    assert( u1.y >= 0.0 );
    assert( u1.y <= this->radius );
    
    // along the x-axis the cc-point is at x-coord s:
    // in the xy-plane the cc-point is at:
    double s = sqrt( square( radius ) - square( u1.y ) );
    Point cc1(  s, u1.y, 0);
    Point cc2( -s, u1.y, 0);
    cc1.z_projectOntoEdge(u1,u2);
    cc2.z_projectOntoEdge(u1,u2);
    // pick the higher one
    double cc_u;
    double cl_z;
    if (cc1.z > cc2.z) {
        cc_u = cc1.x;
        cl_z = cc1.z;
    } else {
        cc_u = cc2.x;
        cl_z = cc2.z;
    }
    return CC_CLZ_Pair( cc_u, cl_z);
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
                CCPoint cc_tmp( p, VERTEX);
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
