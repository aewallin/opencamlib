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
#include "compoundcutter.h" // for offsetCutter()

namespace ocl
{
    

//********   constructors ********************** */
ConeCutter::ConeCutter() {
    setDiameter(1.0);
    angle = 45;
    length = radius/tan(angle);
}

ConeCutter::ConeCutter(const double d, const double a) {
    setDiameter(d);
    angle = a;
    length = radius/tan(angle);
}

double ConeCutter::height(const double r) const {
    assert( tan(angle) > 0.0 ); // guard against division by zero
    return r/tan(angle);
}

//********   drop-cutter methods ********************** */

// we either hit the tip, when the slope of the plane is smaller than angle
// or when the slope is steep, the circular edge between the cone and the cylindrical shaft
int ConeCutter::facetDrop(CLPoint &cl, const Triangle &t) const {
    int result = 0;
    Point normal; // facet surface normal    
    if ( isZero_tol( t.n->z ) )  {// vertical surface
        return -1;  //can't drop against vertical surface
    } else if (t.n->z < 0) {  // normal is pointing down
        normal = -1* (*t.n); // flip normal
    } else {
        normal = *t.n;
    }   
    assert( isPositive( normal.z ) );
    
    // horizontal plane special case
    if ( (isZero_tol(normal.x)) && (isZero_tol(normal.y)) ) { 
        // so any vertex is at the correct height
        CCPoint* cc_tmp = new CCPoint();
        cc_tmp->x = cl.x;
        cc_tmp->y = cl.y;
        cc_tmp->z = t.p[0].z;
        cc_tmp->type=FACET_TIP;
        if (cc_tmp->isInside(t)) { // cc-point is on the axis of the cutter       
            if ( cl.liftZ(cc_tmp->z) ) {
                cl.cc = cc_tmp;
                return 1;
            } else {
                delete cc_tmp;
            }
        } else { // not inside facet
            delete cc_tmp;
            return 0;
        }
    } // end horizontal plane case.
    
    
    // define plane containing facet
    // a*x + b*y + c*z + d = 0, so
    // d = -a*x - b*y - c*z, where
    // (a,b,c) = surface normal
    double a = normal.x;
    double b = normal.y;
    double c = normal.z;
    double d = - normal.dot(t.p[0]); //double d = - a * t.p[0].x - b * t.p[0].y - c * t.p[0].z;
    normal.xyNormalize(); // make length of normal == 1.0
    
    // cylindrical contact point case
    // find the xy-coordinates of the cc-point
    CCPoint* cyl_cc_tmp = new CCPoint();
    *cyl_cc_tmp = cl - radius*normal;
    cyl_cc_tmp->z = (1.0/c)*(-d-a*cyl_cc_tmp->x-b*cyl_cc_tmp->y);
    double cyl_cl_z = cyl_cc_tmp->z - length; // tip positioned here
    cyl_cc_tmp->type = FACET_CYL;
    
    // tip contact with facet
    CCPoint* tip_cc_tmp = new CCPoint(cl.x,cl.y,0.0);
    tip_cc_tmp->z = (1.0/c)*(-d-a*tip_cc_tmp->x-b*tip_cc_tmp->y);
    double tip_cl_z = tip_cc_tmp->z;
    tip_cc_tmp->type = FACET_TIP;
          
    if (tip_cc_tmp->isInside(t)) { // TIP case     
        if ( cl.liftZ(tip_cl_z) ) {
            cl.cc = tip_cc_tmp;
            result = 1;
        } else {
            delete tip_cc_tmp;
        }
    } else {
        delete tip_cc_tmp;
    }
    
    if (cyl_cc_tmp->isInside(t))  { // CYLINDER case
        if ( cl.liftZ( cyl_cl_z) ) {
            cl.cc = cyl_cc_tmp;
            result = 1; 
        } else {
            delete cyl_cc_tmp;
        }
    } else {
        delete cyl_cc_tmp;
    }
    
    return result; 
}

// cone sliced with vertical plane results in a hyperbola as the intersection curve
// find point where hyperbola and line slopes match
int ConeCutter::edgeDrop(CLPoint &cl, const Triangle &t) const
{
    int result = 0;
    for (int n=0;n<3;n++) { // loop through all three edges
        // 1) distance from point to line in xy plane
        int start=n;      // index of the start-point of the edge
        int end=(n+1)%3;  // index of the end-point of the edge
        Point p1 = t.p[start];
        Point p2 = t.p[end];
        // check that there is an edge in the xy-plane
        // can't drop against vertical edges!
        if ( !isZero_tol( p1.x - p2.x) || !isZero_tol( p1.y - p2.y) ) {
            double d = cl.xyDistanceToLine(p1, p2);
            assert( d >= 0.0 );
                
            if ( d <= radius ) { // potential hit
                // math/geometry from Yau et al. 2004 paper (Int. J. Prod. Res. vol42 no13)
                // closest point to cl on line lies at
                Point sc = cl.xyClosestPoint( p1, p2 );   
                Point v = p2 - p1;
                v.z=0;
                v.xyNormalize();
                double p2u = (p2-sc).dot(v); // u-coord of p2 in plane coordinates.
                double p1u = (p1-sc).dot(v);
                // in (u,v) coordinates 
                // cl is at (0, 0)
                // p1 is at (p1u,   d)
                // p2 is at (p2u, d)
                // closest point sc is at u=0
                // edge rotated so it is at y=d 
                // if R2 < d < R we contact the upper cone of the APT-tool
                // intersection curve is
                // z = ( sqrt( x^2 + d^2) / (R-R2) ) * L
                // L = height of upper cone region
                //
                // the slope of this curve is 
                // dz/dx = (L/(R-R2)) * x /(sqrt( x^2 + l^2 ))
                // set this equal to the slope, m, of the line:
                // m = (z2 - z1) / (x2 - x1)
                double m = (p2.z-p1.z) / (p2u-p1u);
                // the outermost point on the cutter is at
                // xu = sqrt( R^2 - d^2 )
                double xu = sqrt( square(radius) - square(d) ); 
                assert( xu <= radius );
                // here the slope has a maximum
                // mu = (L/(R-R2)) * xu /(sqrt( xu^2 + d^2 ))
                double mu = (length/radius ) * xu / sqrt( square(xu) + square(d) ) ;
                // find contact point where slopes match.
                // there are two cases:
                // 1) if abs(m) <= abs(mu)  we contact the curve at
                // xp = sign(m) * sqrt( R^2 m^2 d^2 / (h^2 - R^2 m^2) )
                double ccu;
                if (fabs(m) <= fabs(mu) ) { // 1) hyperbola case
                    ccu = sign(m) * sqrt( square(radius)*square(m)*square(d) / 
                                         (square(length) -square(radius)*square(m) ) );
                } else if ( fabs(m)>fabs(mu) ) {
                    // 2) if abs(m) > abs(mu) there is contact with the circular edge
                    // xp = sign(m) * xu
                    ccu = sign(m)*xu;
                } else {
                    assert(0);
                }
                
                // now the cc-point can be found: (in the XY plane)
                CCPoint* cc_tmp = new CCPoint();
                *cc_tmp = sc + ccu*v; 
                // locate cc_tmp on the line (find the z-coord)
                // cc_tmp = p1 + t*(p2-p1)
                // t = (cc_tmp-p1).dot(p2-p1) / (p2-p1).dot(p2-p1)
                double t;
                if ( fabs(p2.x-p1.x) > fabs(p2.y-p1.y) ) { // locate along coordinate with the bigger "spread"
                    t = (cc_tmp->x - p1.x) / (p2.x-p1.x);
                } else {
                    t = (cc_tmp->y - p1.y) / (p2.y-p1.y);
                }
                cc_tmp->z = p1.z + t*(p2.z-p1.z);
                
                // find the CL-height
                double cl_z;
                if (fabs(m) <= fabs(mu) ) {
                    // 1) zc = zp - Lc + (R - sqrt(xp^2 + d^2)) / tan(beta2)
                    cl_z = cc_tmp->z - length + (radius-sqrt(square(ccu) + square(d)))/ tan(angle);
                } else if ( fabs(m)>fabs(mu) ) {
                    // 2) zc = zp - Lc
                    cl_z = cc_tmp->z - length; // case where we hit the edge of the cone
                } else {
                    assert(0);
                }
                
                cc_tmp->type = EDGE;
                // test if cc-point is in edge
                if ( cc_tmp->isInsidePoints( p1, p2 ) ) {
                    if (cl.liftZ(cl_z)) {
                        cl.cc = cc_tmp;
                        result = 1;
                    } else {
                        delete cc_tmp;
                    }
                } else {
                    delete cc_tmp;
                }
                
            }// end if(potential hit)
        }// end if(vertical edge)
    } // end loop through all edges
    return result;
}

/// offset of cone is BallConeCutter
MillingCutter* ConeCutter::offsetCutter(const double d) const {
    return new BallConeCutter(2*d,  diameter+2*d, angle) ;
}

//******** string output ********************** */
std::string ConeCutter::str() const {
    std::ostringstream o;
    o << *this;
    return o.str();
}

std::ostream& operator<<(std::ostream &stream, ConeCutter c) {
  stream << "ConeCutter (d=" << c.diameter << ", angle=" << c.angle << ", length=" << c.length << ")";
  return stream;
}

} // end namespace
// end file conecutter.cpp
