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
#include "oellipse.h"

namespace ocl
{

//********   CylCutter ********************** */
BullCutter::BullCutter() {
    setDiameter(1.0);
    radius1= 0.3;
    radius2= 0.2;
    setRadius();
}

BullCutter::BullCutter(const double d, const double r) {
    setDiameter(d);
    radius = d/2;        // total cutter radius
    radius1 = d/2 - r;   // cylindrical middle part radius
    radius2 = r;         // corner radius
}

void BullCutter::setRadius() {
    radius= radius1+radius2;
}

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


//********   drop-cutter methods ********************** */

int BullCutter::facetDrop(CLPoint &cl, const Triangle &t) const {
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
    
    // horizontal plane special case
    if ( ( isZero_tol(normal.x) ) && ( isZero_tol(normal.y) ) ) { 
        CCPoint* cc_tmp = new CCPoint(cl.x,cl.y,t.p[0].z);
        
        if (cc_tmp->isInside(t)) { // assuming cc-point is on the axis of the cutter...       
            if ( cl.liftZ(cc_tmp->z) ) {
                cc_tmp->type = FACET;
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
    CCPoint* cc_tmp = new CCPoint();
    *cc_tmp = cl + radiusvector; // NOTE xy-coords right, z-coord is not.
    cc_tmp->z = (1.0/c)*(-d-a*cc_tmp->x-b*cc_tmp->y); // cc-point lies in the plane.
    cc_tmp->type = FACET;
    if (cc_tmp->isInside(t)) {   
        // now find the z-coordinate of the cl-point
        double tip_z = cc_tmp->z + radius2*normal.z - radius2;
        
        if ( cl.liftZ(tip_z) ) {
            cl.cc = cc_tmp;
            return 1;
        } else {
            delete cc_tmp;
        }
    } else {
        delete cc_tmp;
        return 0;
    }
    
    return 0; // we never get here (?)
}



/// Toroidal cutter edge-test
int BullCutter::edgeDrop(CLPoint &cl, const Triangle &t) const {
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
                if ( isZero_tol( p1.z - p2.z ) ) {  // horizontal edge special case
                    if ( d<= radius1) {             // horizontal edge, contact with cylindrical part of cutter
                        CCPoint* cc_tmp = new CCPoint();
                        *cc_tmp = cl.xyClosestPoint(p1,p2);
                        cc_tmp->z = p1.z;   
                        cc_tmp->type = EDGE_HORIZ_CYL;           
                        if ( cc_tmp->isInsidePoints( p1, p2 ) ) { // test if cc-point is in edge
                            if (cl.liftZ(p1.z)) {
                                cl.cc = cc_tmp;
                                result = 1;
                            } else {
                                delete cc_tmp;
                            }
                        } else {
                            delete cc_tmp;
                        }   
                    } 
                    else if (d <= diameter/2) { // we are in toroid part of cutter
                        // horizontal edge, toroid region special case
                        // (q-r1)^2 + h2^2 = r2^2
                        // h2 = sqrt( r2^2 - (q-r1)^2 )
                        // h1 = r2 - h2
                        // cutter_tip = p.z - h1
                        CCPoint* cc_tmp = new CCPoint();
                        *cc_tmp = cl.xyClosestPoint(p1,p2);
                        cc_tmp->z = p1.z;   
                        cc_tmp->type = EDGE_HORIZ_TOR;     
                        if ( cc_tmp->isInsidePoints( p1, p2 ) ) { 
                            double h1 = radius2 - sqrt( square(radius2) - square(d-radius1) );
                            if ( cl.liftZ(p1.z - h1) ) {
                                cl.cc = cc_tmp;                                 
                                result = 1;
                            } else {
                                delete cc_tmp;
                            }
                        } else {
                            delete cc_tmp;
                        }
                    }
                } // end horizontal edge special cases
                else {
                    // general case (potential hit and not a horizontal edge)
                    // this is the offset-ellipse case
                    // instead of dropping a Toroid(r1,r2) against a line(p1,p2)
                    // we drop a cylinder Cylinder(r1) against a Cylinder(radius=r2) around line(p1,p2)
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
                    assert( a_axis > 0.0 );
                    assert( b_axis > 0.0 );
                    
                    // double eccen = a_axis/b_axis;
                    // bool debugNR = false;
                    // if (eccen > 370000) {
                    //    std::cout << "high eccen case eccen=" << eccen << "\n";
                    //    std::cout << " theta=" << theta << "\n";
                    //    std::cout << " up1=" << up1 << " up2="<< up2 << "\n";
                    //    debugNR = true;
                    // }
                    
                    // locate ellipse in the XY plane
                    Point ellcenter = Point(0,d,0); // only ellcenter.y != 0.0
                                          
                    // the ellipse and the solver
                    Ellipse e = Ellipse( ellcenter, a_axis, b_axis, radius1);
                    // if (debugNR) {
                    //if (e.eccen > 10000)
                    //    std::cout << " ellipse=" << e << "\n";
                    //    std::cout << " ucl=" << ucl << "\n";
                    // }
                    // run the solver
                    //int iters = e.solver_nr(ucl);  // old Newton-Rhapson solver
                    int iters = e.solver_brent(ucl);
                    // now epos1 and epos2 are the correct Epos positions where error is zero
                    /*
                    if (e.epos1.s == e.epos2.s && e.epos1.t==e.epos2.t) {
                        if ( !isZero_tol(e.epos1.s) || !isZero_tol(e.epos1.t) ) {
                            std::cout << "identical solutions!\n";
                            std::cout << "epos1=" << e.epos2 << " epos2="<< e.epos1 << "\n";
                        }
                    }*/
                        
                    assert( iters < 200 ); // it's probably an error if the solver takes too long...
                    
                    // the corresponding solved ellipse-centers
                    Point ecen1 = e.calcEcenter( ucl, up1, up2, 1);
                    Point ecen2 = e.calcEcenter( ucl, up1, up2, 2);
                    assert( ecen1.y == d ); // ellipse-centers are on the edge
                    assert( ecen2.y == d );
                    
                    // choose the one with higher z-value
                    Point ecen;
                    Epos pos_hi;
                    int ep_sign = 0;
                    if (ecen1.z >= ecen2.z) {
                        ecen = ecen1;
                        pos_hi = e.epos1;
                        ep_sign = -1;
                        // std::cout << "epos1 chosen\n";
                    } else {
                        ecen = ecen2;
                        pos_hi = e.epos2;
                        ep_sign = 1;
                        // std::cout << "epos2 chosen\n";
                    }
                    //if (e.eccen > 1000)
                    //    std::cout << "ecen1.z=" << ecen1.z << " ecen2.z=" << ecen2.z << " chosen="<< ecen.z <<"\n";
                        
                    // a new ellipse in the right place        
                    // this is at the correct z-height 
                    Ellipse e_hi = Ellipse(ecen, a_axis, b_axis, radius1);
                    assert( ecen.y == d );
                    assert( ecen.z >= ecen1.z );
                    assert( ecen.z >= ecen2.z );
                    
                    // cc-point on the ellipse/cylinder, in the CL=origo system
                    Point ell_ccp = e_hi.ePoint(pos_hi);
                    if ( fabs( ell_ccp.xyNorm() - radius1 ) > 1E-5 ) { // should be on the cylinder-circle   
                        std::cout << " eccen=" << e.eccen << " ell_cpp=" << ell_ccp << "radius1="<< radius1 <<"\n";
                        std::cout << " ell_ccp.xyNorm() - radius1 =" << ell_ccp.xyNorm() - radius1 << "\n";
                        assert(0);
                    }
                    assert( fabs( ell_ccp.xyNorm() - radius1 ) < 1E-5 );                 
                    
                    Point cc_tmp_u = ell_ccp.closestPoint(up1,up2);
                                        
                    CCPoint* cc_tmp = new CCPoint();
                    *cc_tmp = sc + cc_tmp_u.x*vxy; //locate in XY plane
                    
                    double t;
                    if ( fabs(p2.x-p1.x) > fabs(p2.y-p1.y) ) {
                        t = (cc_tmp->x - p1.x) / (p2.x-p1.x); // now find the z-coord of cc_tmp
                    } else {
                        t = (cc_tmp->y - p1.y) / (p2.y-p1.y);
                    }
                    cc_tmp->z = p1.z + t*(p2.z-p1.z);
                    
                    if (ep_sign > 0) // sign matters only for cc.type
                        cc_tmp->type = EDGE_POS;
                    else
                        cc_tmp->type = EDGE_NEG;
                        
                    if ( cc_tmp->isInsidePoints( p1, p2 ) ) {

                        if ( cl.liftZ(ecen.z-radius2) ) {
                            cl.cc = cc_tmp;
                            result = 1;
                        } else {
                            delete cc_tmp;
                        }
                    } else {
                        delete cc_tmp;
                    }
                } // end general case
                
            }// end if(potential hit)            
        } // end if( not vertical edge ) 
    } // end loop through all edges
        
    return result;
}

                        
//********  BullCutter push-cutter methods ****************************/


bool BullCutter::facetPush(const Fiber& fib, Interval& i,  const Triangle& t) const {
    bool result = false;
    
    Point normal; // facet surface normal 
    if ( t.n->zParallel() ) { // normal points in z-dir   
        return result; //can't push against horizontal plane, stop here.
    }
    else if (t.n->z < 0) {  // normal is pointing down
        normal = -1* (*t.n); // flip normal
    } else {
        normal = *t.n;
    }
    // now we know the normal points upwards
    normal.normalize();
    Point xy_normal = normal;
    xy_normal.z = 0;
    xy_normal.xyNormalize();
    
    //   find a point on the plane from which radius2*normal+radius1*xy_normal lands on the fiber+radius2*Point(0,0,1) 
    //   (u,v) locates a point on the triangle facet    v0+ u*(v1-v0)+v*(v2-v0)    u,v in [0,1]
    //   t locates a point along the fiber:             p1 + t*(p2-p1)             t in [0,1]
    // 
    //   facet-point + r2 * n + r1* xy_n = fiber-point + r2*Point(0,0,1)
    //   =>
    //   v0+ u*(v1-v0)+v*(v2-v0) + r2 * n + r1* xy_n  = p1 + t*(p2-p1) + r2*Point(0,0,1)
    //
    //   v0x + u*(v1x-v0x) + v*(v2x-v0x) + r2*nx + r1*xy_n.x  = p1x + t*(p2x-p1x)          p2x-p1x==0 for Y-fiber
    //   v0y + u*(v1y-v0y) + v*(v2y-v0y) + r2*ny + r1*xy_n.y  = p1y + t*(p2y-p1y)          p2y-p1y==0 for X-fiber
    //   v0z + u*(v1z-v0z) + v*(v2z-v0z) + r2*nz              = p1z + t*(p2z-p1z) + r2    (p2z-p1z)==0 for XY-fibers!!
    //   X-fiber:
    //   v0x + u*(v1x-v0x) + v*(v2x-v0x) + r2*nx + r1*xy_n.x  = p1x + t*(p2x-p1x)         
    //   v0y + u*(v1y-v0y) + v*(v2y-v0y) + r2*ny + r1*xy_n.y  = p1y                    solve these  two for (u,v)
    //   v0z + u*(v1z-v0z) + v*(v2z-v0z) + r2*nz              = p1z + r2               and substitute above for t
    //   or
    //   [ (v1y-v0y)    (v2y-v0y) ] [ u ] = [ -v0y - r2*ny - r1*xy_n.y + p1y     ]
    //   [ (v1z-v0z)    (v2z-v0z) ] [ v ] = [ -v0z - r2*nz + p1z + r2            ]
    //
    //   Y-fiber:
    //   [ (v1x-v0x)    (v2x-v0x) ] [ u ] = [ -v0x - r2*nx - r1*xy_n.x + p1x     ]
    
    double a;
    double b;
    double c = t.p[1].z - t.p[0].z;
    double d = t.p[2].z - t.p[0].z;
    double e;
    double f = -t.p[0].z - radius2*normal.z + fib.p1.z + radius2;
    double u;
    double v;
    // a,b,e depend on the fiber:
    if ( fib.p1.y == fib.p2.y ) { // XFIBER
        a = t.p[1].y - t.p[0].y;
        b = t.p[2].y - t.p[0].y;
        e = -t.p[0].y - radius2*normal.y - radius1*xy_normal.y + fib.p1.y;
        if (!two_by_two_solver(a,b,c,d,e,f,u,v))
            return result;
        CCPoint cc = t.p[0] + u*(t.p[1]-t.p[0]) + v*(t.p[2]-t.p[0]);
        cc.type = FACET;
        if ( ! cc.isInside( t ) ) 
            return result;
        // v0x + u*(v1x-v0x) + v*(v2x-v0x) + r2*nx + r1*xy_n.x = p1x + t*(p2x-p1x) 
        // =>
        // t = 1/(p2x-p1x) * ( v0x + r2*nx + r1*xy_n.x - p1x +  u*(v1x-v0x) + v*(v2x-v0x)       )
        assert( !isZero_tol( fib.p2.x - fib.p1.x )  );
        double tval = (1.0/( fib.p2.x - fib.p1.x )) * ( t.p[0].x + radius2*normal.x + radius1*xy_normal.x - fib.p1.x 
                                                        + u*(t.p[1].x-t.p[0].x)+v*(t.p[2].x-t.p[0].x) );
        if ( tval < 0.0 || tval > 1.0  ) {
            std::cout << "BullCutter::facetPush() tval= " << tval << " error!?\n";
        } 
        assert( tval > 0.0 && tval < 1.0 );
        i.updateUpper( tval  , cc );
        i.updateLower( tval  , cc );
        result = true;
    } else if (fib.p1.x == fib.p2.x) { // YFIBER
        a = t.p[1].x - t.p[0].x;
        b = t.p[2].x - t.p[0].x;
        e = -t.p[0].x - radius2*normal.x - radius1*xy_normal.x + fib.p1.x;
        if (!two_by_two_solver(a,b,c,d,e,f,u,v))
            return result;
        CCPoint cc = t.p[0] + u*(t.p[1]-t.p[0]) + v*(t.p[2]-t.p[0]);
        cc.type = FACET;
        if ( ! cc.isInside( t ) ) 
            return result;
        assert( !isZero_tol( fib.p2.y - fib.p1.y )  );
        double tval = (1.0/( fib.p2.y - fib.p1.y )) * ( t.p[0].y + radius2*normal.y + radius1*xy_normal.y - fib.p1.y 
                                                        + u*(t.p[1].y-t.p[0].y)+v*(t.p[2].y-t.p[0].y) );
        if ( tval < 0.0 || tval > 1.0  ) {
            std::cout << "BullCutter::facetPush() tval= " << tval << " error!?\n";
        } 
        assert( tval > 0.0 && tval < 1.0 );
        i.updateUpper( tval  , cc );
        i.updateLower( tval  , cc );
        result = true;    
    } else {
        assert(0);
    }
    
    
    return result;
}

bool BullCutter::edgePush(const Fiber& f, Interval& i,  const Triangle& t) const {
    bool result = false;
    return result;
}

//*********************************************************************/


/// offset of bull is bull
MillingCutter* BullCutter::offsetCutter(const double d) const {
    return new BullCutter(diameter+2*d, radius2+d) ;
}

//********  BullCutter string output ********************** */
std::string BullCutter::str() const
{
    std::ostringstream o;
    o << *this;
    return o.str();
}

std::ostream& operator<<(std::ostream &stream, BullCutter c)
{
  stream << "BullCutter(d=" << c.diameter << ", r1=" << c.radius1 << " r2=" << c.radius2 << ")";
  return stream;
}

} // end namespace
// end file bullcutter.cpp
