/*  Copyright 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com)
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
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <math.h>
#include <boost/progress.hpp>

#include "cutter.h"
#include "point.h"
#include "triangle.h"
#include "numeric.h"
#include "oellipse.h"

namespace ocl
{

//********   CylCutter ********************** */
BullCutter::BullCutter()
{
    setDiameter(1.0);
    radius1= 0.3;
    radius2= 0.2;
    setRadius();
}

BullCutter::BullCutter(const double d, const double r)
{
    setDiameter(d);
    radius = d/2;
    radius1 = d/2 - r;
    radius2 = r;
}

void BullCutter::setRadius()
{
    radius= radius1+radius2;
}


//********   drop-cutter methods ********************** */
int BullCutter::vertexDrop(Point &cl, CCPoint &cc, const Triangle &t) const
{
    // some math here: http://www.anderswallin.net/2007/06/drop-cutter-part-13-cutter-vs-vertex/
    int result = 0;
    BOOST_FOREACH( const Point& p, t.p)
    {
        // distance in XY-plane from cl to p
        double q = cl.xyDistance(p);
        assert( q >= 0.0 );
    
        if ( q <= radius1 ) { // p is inside the cylindrrical part of the cutter
            if (cl.liftZ(p.z)) { // we need to lift the cutter
                cc = p;
                cc.type = VERTEX;
                result = 1;
            }
        }
        else if ( q <= radius ) { // p is in the toroidal part of the cutter
            // (q-r1)^2 + h2^2 = r2^2
            // h2 = sqrt( r2^2 - (q-r1)^2 )
            // h1 = r2 - h2
            // cutter_tip = p.z - h1
            double h1 = radius2 - sqrt( square(radius2) - square(q-radius1) );
            if ( cl.liftZ(p.z - h1) ) { // we need to lift the cutter
                cc = p;
                cc.type = VERTEX;
                result = 1;
            }
        }
        else {
            // point outside cutter, nothing to do.
        }
    }
    return result;
}


int BullCutter::facetDrop(Point &cl, CCPoint &cc, const Triangle &t) const
{
    // Drop cutter at (cl.x, cl.y) against facet of Triangle t
    Point normal; // facet surface normal
    
    if ( isZero_tol( t.n->z ) )  {// vertical surface
        return -1;  //can't drop against vertical surface
    } else if (t.n->z < 0) {  // normal is pointing down
        normal = -1 * (*t.n); // flip normal
    } else {
        normal = *t.n;
    }   
    assert( isPositive( normal.z ) );
    
    
    if ( ( isZero_tol(normal.x) ) && ( isZero_tol(normal.y) ) ) { // horizontal plane
        // so any vertex is at the correct height
        Point cc_tmp;
        cc_tmp.x = cl.x;
        cc_tmp.y = cl.y;
        cc_tmp.z = t.p[0].z;
        if (cc_tmp.isInside(t)) { // assuming cc-point is on the axis of the cutter...       
            if ( cl.liftZ(cc_tmp.z) ) {
                cc = cc_tmp;
                cc.type = FACET;
                return 1;
            }
        } else { // not inside facet
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
    //double d = - a * t.p[0].x - b * t.p[0].y - c * t.p[0].z;
    double d = - normal.dot(t.p[0]);
        
    normal.normalize(); // make length of normal == 1.0
    Point xyNormal = normal;
    xyNormal.z = 0;
    xyNormal.xyNormalize();
    
    // define the radiusvector which points from the 
    // torus-center to the cc-point.
    Point radiusvector = -radius2*normal - radius1*xyNormal;
    
    // find the xy-coordinates of the cc-point
    Point cc_tmp = cl + radiusvector; // NOTE xy-coords right, z-coord is not.
        
    if (cc_tmp.isInside(t)) {   
        // find the z-coordinate of the cc-point.
        // it lies in the plane.
        cc_tmp.z = (1.0/c)*(-d-a*cc_tmp.x-b*cc_tmp.y); 
        
        // now find the z-coordinate of the cl-point
        double tip_z = cc_tmp.z + radius2*normal.z - radius2;
        
        if ( cl.liftZ(tip_z) ) {
            cc = cc_tmp;
            cc.type = FACET;
            return 1;
        }
    } else {
        return 0;
    }
    
    return 0; // we never get here (?)
}



/// Toroidal cutter edge-test
int BullCutter::edgeDrop(Point &cl, CCPoint &cc, const Triangle &t) const
{
    // Drop cutter at (cl.x, cl.y) against the three edges of Triangle t
    int result = 0;    
    for (int n=0;n<3;n++) { // loop through all three edges
        int start=n;
        int end=(n+1)%3;
        Point p1 = t.p[start];
        Point p2 = t.p[end];
        
        // check that there is an edge in the xy-plane
        // can't drop against vertical edges!
        if ( !isZero_tol( p1.x - p2.x) || !isZero_tol( p1.y - p2.y) ) {
            double d = cl.xyDistanceToLine(p1,p2);
            assert( d >= 0.0 ); // FIXME d==0.0 is another special case...
            
            // is the edge close enough?
            if (d<=diameter/2) { // potential hit
                if ( isZero_tol( p1.z - p2.z ) ) {
                    // horizontal edge special case
                    // std::cout << "BullCutter:edgeDrop(): horizontal edge\n";
                    if ( d<= radius1){
                        // horizontal edge, contact with cylindrical part of cutter
                        Point cc_tmp = cl.xyClosestPoint(p1,p2);
                        cc_tmp.z = p1.z;                
                        // test if cc-point is in edge
                        if ( cc_tmp.isInsidePoints( p1, p2 ) ) {
                            if (cl.liftZ(p1.z)) {
                                cc = cc_tmp;
                                cc.type = EDGE_HORIZ_CYL;
                                result = 1;
                            }
                        }                        
                    } 
                    else if (d <= diameter/2) { // we are in toroid part of cutter
                        // horizontal edge, toroid region special case
                        // (q-r1)^2 + h2^2 = r2^2
                        // h2 = sqrt( r2^2 - (q-r1)^2 )
                        // h1 = r2 - h2
                        // cutter_tip = p.z - h1
                        Point cc_tmp = cl.xyClosestPoint(p1,p2);
                        cc_tmp.z = p1.z;        
                        if ( cc_tmp.isInsidePoints( p1, p2 ) ) { 
                            double h1 = radius2 - sqrt( square(radius2) - square(d-radius1) );
                            if ( cl.liftZ(p1.z - h1) ) { 
                                cc = cc_tmp;
                                cc.type = EDGE_HORIZ_TOR;
                                result = 1;
                            }
                        }
                    }
                } // end horizontal edge special cases
                else {
                    // general case (potential hit and not a horizontal edge)
                    // this is the offset-ellipse case
                    // instead of dropping a Toroid(r1,r2) against a line(p1,p2)
                    // we drop a cylinder Cylinder(r1) against a Tube(radius=r2) around line(p1,p2)
                    // std::cout << "BullCutter:edgeDrop(): general case\n";
                    // the "canonical" case for which the offset-ellipse solver is written
                    // requires p1.y == p2.y, i.e. the edge needs to be rotated so that it is parallel
                    // to the X-axis.
                    // and the cutter is positioned at (0,0) in this coordinate system.
                    
                    Point v = p2 - p1; // vector along edge, from p1 -> p2
                    Point vxy = v;
                    vxy.z = 0;
                    vxy.xyNormalize(); // normalized XY edge vector
                     
                    // figure out u-coordinates of p1 and p2
                    Point sc = cl.xyClosestPoint( p1, p2 );   
                    assert( ( (cl-sc).xyNorm() - d ) < 1E-6 );
                    
                    
                    double p1u = (p1-sc).dot(vxy);
                    double p2u = (p2-sc).dot(vxy); 
                
                    // edge endpoints in the new coordinate system
                    Point up1 = Point( p2u, d, p2.z);
                    Point up2 = Point( p1u, d, p1.z);
                                       
                    // in these coordinates, CL is at origo
                    Point ucl = Point( 0.0 , 0.0, cl.z );
                    
                    //  short axis of ellipse = radius2
                    //  long axis of ellipse = radius2/sin(theta)
                    //  where theta is the slope of the line
                    double b_axis = radius2;
                    assert( fabs(up2.x-up1.x) > 0.0 ); // avoid divide-by-zero
                    double theta = atan( (up2.z - up1.z) / (up2.x-up1.x) ); 
                    double a_axis = fabs( radius2/sin(theta) );
                    
                    // locate ellipse in the XY plane
                    Point ellcenter = Point(0,d,0); 
                    
                    // the ellipse and the solver
                    Ellipse e = Ellipse( ellcenter, a_axis, b_axis, radius1);
                    int iters = Ellipse::solver(e, ucl);
                    assert( ellcenter.x == 0);
                    assert( ellcenter.z == 0);
                    assert( iters < 20 ); // it's probably an error if the solver takes too long...
                    
                    // the corresponding solved ellipse-centers
                    Point ecen1 = e.calcEcenter( ucl, up1, up2, 1);
                    Point ecen2 = e.calcEcenter( ucl, up1, up2, 2);
                    assert( ecen1.y == d ); // ellipse-centers are on the edge
                    assert( ecen2.y == d );
                    // s,t sign info must be got here also!
                    
                    // choose the one with higher z-value
                    Point ecen;
                    Epos pos_hi;
                    int ep_sign = 0;
                    if (ecen1.z > ecen2.z) {
                        ecen = ecen1;
                        pos_hi = e.epos1;
                        ep_sign = -1;
                    }
                    else {
                        ecen = ecen2;
                        pos_hi = e.epos2;
                        ep_sign = 1;
                    }
                         
                        
                    // a new ellipse in the right place        
                    // this is at the correct z-height 
                    Ellipse e_hi = Ellipse(ecen, a_axis, b_axis, radius1);
                    assert( ecen.y == d );
                    
                    // cc-point on the ellipse/cylinder, in the CL=origo system
                    Point ell_ccp = e_hi.ePoint(pos_hi);
                    assert( fabs( ell_ccp.xyNorm() - radius1 ) < 1E-6 );  // should be on the cylinder-circle                  
                    
                    Point cc_tmp_u = ell_ccp.closestPoint(up1,up2);
                                        
                    Point cc_tmp = sc + cc_tmp_u.x*vxy; //locate in XY plane
                    // now find the z-coord of cc_tmp
                    double t;
                    if ( fabs(p2.x-p1.x) > fabs(p2.y-p1.y) ) {
                        t = (cc_tmp.x - p1.x) / (p2.x-p1.x);
                    } else {
                        t = (cc_tmp.y - p1.y) / (p2.y-p1.y);
                    }
                    cc_tmp.z = p1.z + t*(p2.z-p1.z);
                    
                    if ( cc_tmp.isInsidePoints( p1, p2 ) ) {
                        if ( cl.liftZ(ecen.z-radius2) ) {
                            cc = cc_tmp;
                            if (ep_sign > 0)
                                cc.type = EDGE_POS;
                            else
                                cc.type = EDGE_NEG;
                            result = 1;
                        }
                    }

                    
                } // end general case
                
            }// end if(potential hit)
            else {
                // edge is too far away from cutter. nothing to do.
            }
            
        } // end if( not vertical edge) 
        
    } // end loop through all edges
        
    return result;
}

/// offset of bull is bull
MillingCutter* BullCutter::offsetCutter(double d) const {
    return new BullCutter(diameter+2*d, radius2+d) ;
}

//********  CylCutter string output ********************** */
std::string BullCutter::str() const
{
    std::ostringstream o;
    o << "BullCutter(d=" << diameter << ", radius1=" << radius1 << ", radius2=" << radius2 << ")";
    return o.str();
}

std::ostream& operator<<(std::ostream &stream, BullCutter c)
{
  stream << "BallCutter"<< c.id <<"(d=" << c.diameter << ", radius1=" << c.radius1 << " radius2=" << c.radius2 << ")";
  return stream;
}

} // end namespace
// end file bullcutter.cpp
