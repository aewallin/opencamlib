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

#include "cylcutter.hpp"
#include "bullcutter.hpp" // for offsetCutter()
#include "numeric.hpp"

namespace ocl
{

CylCutter::CylCutter() {
    std::cout << " usage: CylCutter( double diameter, double length ) \n";
    assert(0);
}

CylCutter::CylCutter(double d, double l) {
    diameter= d;                    assert( d > 0.0);
    radius= d/2.0;
    length = l;                     assert( l > 0.0 );
    xy_normal_length = radius;
    normal_length = 0.0;
    center_height = 0.0;
}

// drop-cutter vertexDrop is handled by the base-class method in MillingCutter
// drop-cutter facetDrop is handled by the base-class method in MillingCutter

// we handle the edge-drop here.
CC_CLZ_Pair CylCutter::singleEdgeDropCanonical(const Point& u1, const Point& u2) const {
    // along the x-axis the cc-point is at x-coord s or -s:
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

// general purpose vertexPush, delegates to this->width(h) 
bool CylCutter::vertexPush(const Fiber& f, Interval& i, const Triangle& t) const {
    bool result = false;
    BOOST_FOREACH( const Point& p, t.p) {
        if (this->singleVertexPush(f,i,p, VERTEX) )
            result = true;
    }

    Point p1, p2;
    if ( t.zslice_verts(p1, p2, f.p1.z) ) {
        p1.z = f.p1.z; // z-coord should be very close to f.p1.z, but set it exactly anyway.
        p2.z = f.p1.z;
        if ( this->singleVertexPush(f,i,p1, VERTEX_CYL) )
            result = true;
        if (this->singleVertexPush(f,i,p2, VERTEX_CYL))
            result = true;
    }
    
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
