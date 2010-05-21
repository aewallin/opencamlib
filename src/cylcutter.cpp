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
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <math.h>
#include <cassert>

#include "cutter.h"
#include "point.h"
#include "triangle.h"
#include "cutter.h"
#include "numeric.h"
#include "fiber.h"

// #define EDGEDROP_DEBUG

namespace ocl
{

//********   CylCutter ********************** */
CylCutter::CylCutter()
{
    setDiameter(1.0);
}

CylCutter::CylCutter(const double d)
{
    setDiameter(d);
}

//********   drop-cutter methods ********************** */
int CylCutter::vertexDrop(Point &cl, CCPoint &cc, const Triangle &t) const
{
    /// loop through each vertex p of Triangle t
    /// drop down cutter at (cl.x, cl.y) against Point p
    
    int result = 0;
    BOOST_FOREACH( const Point& p, t.p)
    {
        double q = cl.xyDistance(p); // distance in XY-plane from cl to p
        if (q<= radius) { // p is inside the cutter
            if (cl.liftZ(p.z)) { // we need to lift the cutter
                cc = p;
                cc.type = VERTEX;
                result = 1;
            }
        } else {
            // point outside cutter, nothing to do.
        }
    }
    return result;
}

int CylCutter::facetDrop(Point &cl, CCPoint &cc, const Triangle &t) const
{
    // Drop cutter at (cl.x, cl.y) against facet of Triangle t
    Point normal; // facet surface normal
    
    if ( isZero_tol( t.n->z ) )  { // vertical surface
        return -1;  // can't drop against vertical surface
    } else if (t.n->z < 0) {  // normal is pointing down
        normal = -1* (*t.n); // flip normal
    } else {
        normal = *t.n;
    }
    
    assert( isPositive( normal.z ) ); // we are in trouble if n.z is not positive by now...
    
    // define plane containing facet
    // a*x + b*y + c*z + d = 0, so
    // d = -a*x - b*y - c*z, where
    // (a,b,c) = surface normal
    double a = normal.x;
    double b = normal.y;
    double c = normal.z;
    //double d = - a * t.p[0].x - b * t.p[0].y - c * t.p[0].z;
    double d = - normal.dot(t.p[0]);
        
    normal.xyNormalize(); // make length of normal in xy plane == 1.0
    
    // the contact point with the plane is on the periphery
    // of the cutter, a length radius from cl in the direction of -n
    Point cc_tmp = cl - (radius)*normal; // Note: at this point the z-coord is rubbish.
    
    if (cc_tmp.isInside(t)) { // NOTE: cc.z is ignored in isInside()
        cc_tmp.z = (1.0/c)*(-d-a*cc_tmp.x-b*cc_tmp.y); // NOTE: potential for divide-by-zero (?!)
        if (cl.liftZ(cc_tmp.z)) {
            cc = cc_tmp;
            cc.type = FACET;
            return 1;
        }
    } else {
        //std::cout << " NOT isInside!, cc="<<cc<<"\n";
        return 0;
    }
    return 0;
}


int CylCutter::edgeDrop(Point &cl, CCPoint &cc, const Triangle &t) const
{
    // Drop cutter at (p.x, p.y) against edges of Triangle t
    // strategy:
    // 1) calculate distance to infinite line
    // 2) calculate intersection points w. cutter
    // 3) pick the higher intersection point and test if it is in the edge
    int result = 0;
    
    for (int n=0;n<3;n++) { // loop through all three edges
        // 1) distance from point to line
        int start=n;
        int end=(n+1)%3;
        #ifdef EDGEDROP_DEBUG
            std::cout << "testing points " << start<< " to " << end << " :";
        #endif 
        Point p1 = t.p[start];
        Point p2 = t.p[end];
        
        // check that there is an edge in the xy-plane
        // can't drop against vertical edges!
        if ( !isZero_tol( p1.x - p2.x ) || !isZero_tol( p1.y - p2.y) ) {
            
            double d = cl.xyDistanceToLine(t.p[start],t.p[end]);
            #ifdef EDGEDROP_DEBUG
                std::cout << "xyDistance=" << d ;
            #endif
            
            if (d<=radius) { // potential hit
                //std::cout << " potential hit\n";
                // 2) calculate intersection points with cutter circle.
                // points are on line and diameter/2 from cl.
                // see http://mathworld.wolfram.com/Circle-LineIntersection.html
                double x1 = t.p[start].x - cl.x; // translate to cl=(0,0)
                double y1 = t.p[start].y - cl.y;
                double x2 = t.p[end].x - cl.x;
                double y2 = t.p[end].y - cl.y;
                double dx = x2-x1;
                double dy = y2-y1;
                double dr = sqrt( dx*dx + dy*dy);
                double D = x1*y2 - x2*y1;
                double discr = square( radius ) * square(dr) - square(D);
                #ifdef EDGEDROP_DEBUG
                    std::cout << "discr=" << discr << "\n";
                #endif
                
                if ( !isZero_tol(discr) && isNegative(discr) ) {
                    std::cout << "cutter.cpp ERROR: CylCutter::edgeTest discr= "<<discr<<" <0 !!\n";
                    cc.type = ERROR;
                    assert(0);
                    return 0;
                    
                } else if ( isZero_tol(discr) ) {// tangent line
                    #ifdef EDGEDROP_DEBUG
                        std::cout << "discr= zero, tangent case.\n";
                    #endif
                    CCPoint cc_tmp;
                    cc_tmp.x = D*dy / pow(dr,2) + cl.x; // translate back to cl
                    cc_tmp.y = -D*dx / pow(dr,2) + cl.y;
                    
                    // 3) check if cc is in edge
                    if ( cc_tmp.isInsidePoints(t.p[start], t.p[end]) ) { 
                        // determine height of point. must be on line, so:
                        // std::cout << "tangent-case: isInside=true!\n";
                        // two point formula for line:
                        // z-z1 = ((z2-z1)/(x2-x1)) * (x - x1)
                        // z = z1 + ((z2-z1)/(x2-x1)) * (x-x1)
                        double z1 = t.p[start].z;
                        double z2 = t.p[end].z;
                        double x1 = t.p[start].x;
                        double x2 = t.p[end].x;
                        double y1 = t.p[start].y;
                        double y2 = t.p[end].y;
                        
                        // use either x-coord or y-coord to calculate z-height
                        if ( fabs(x1 - x2) > fabs(y2 - y1) ) 
                            cc_tmp.z = z1 + ((z2-z1)/(x2-x1)) * (cc_tmp.x-x1);
                        else if ( !isZero_tol( y2-y1) ) // guard against division by zero
                            cc_tmp.z = z1 + ((z2-z1)/(y2-y1)) * (cc_tmp.y-y1);
                        else 
                            assert(0); // trouble.
                           
                        if (cl.liftZ(cc_tmp.z)) {
                            cc = cc_tmp;
                            cc.type = EDGE;
                            result = 1;
                        }
                    }
                } else { // discr > 0, two intersection points
                    #ifdef EDGEDROP_DEBUG
                        std::cout << "discr>0, two intersections\n";
                    #endif
                    Point cc1;
                    Point cc2;
                    // remember to translate back to cl
                    cc1.x= (D*dy  + sign(dy)*dx*sqrt(discr)) / pow(dr,2) + cl.x; 
                    cc1.y= (-D*dx + fabs(dy)*sqrt(discr)   ) / pow(dr,2) + cl.y;
                    cc1.z=0;
                    cc2.x= (D*dy  - sign(dy)*dx*sqrt(discr)) / pow(dr,2) + cl.x;
                    cc2.y= (-D*dx - fabs(dy)*sqrt(discr)   ) / pow(dr,2) + cl.y;
                    cc2.z=0;
                    #ifdef EDGEDROP_DEBUG
                        std::cout << "cc1= " << cc1 << "\n";
                        std::cout << "cc2= " << cc2 << "\n";
                    #endif
                    // 3) check if in edge
                    double z1 = t.p[start].z;
                    double z2 = t.p[end].z;
                    double x1 = t.p[start].x;
                    double x2 = t.p[end].x;
                    double y1 = t.p[start].y;
                    double y2 = t.p[end].y;
                    if ( cc1.isInsidePoints(t.p[start], t.p[end]) ) {
                        #ifdef EDGEDROP_DEBUG
                            std::cout << "cc1 is in edge!\n";
                        #endif
                        // determine height of point. must be on line, so:
                        if ( !isZero_tol( fabs(x1 - x2) > fabs(y1 - y2) )  ) {  // can compute using x-coords
                            #ifdef EDGEDROP_DEBUG
                                std::cout << "computing cc1 height from x-coords\n";
                            #endif
                            cc1.z = z1 + ((z2-z1)/(x2-x1)) * (cc1.x-x1);
                        }
                        else if ( !isZero_tol( fabs(y1 - y2) ) ) {
                            #ifdef EDGEDROP_DEBUG
                                std::cout << "computing cc1 height from y-coords\n";
                            #endif
                            cc1.z = z1 + ((z2-z1)/(y2-y1)) * (cc1.y-y1);
                        }
                        else {// we are in trouble.
                            std::cout << "cyclutter edge-test, unable to compute cc-point. stop.\n";
                            assert(0);
                        }
                        
                        if (cl.liftZ(cc1.z)) {
                            cc=cc1;
                            cc.type = EDGE;
                            result = 1;
                        }
                        //std::cout << "intersect case: cc1 isInside=true! cc1=" << cc1 << "\n";
                    }
                    if ( cc2.isInsidePoints(t.p[start], t.p[end]) ) {
                        #ifdef EDGEDROP_DEBUG
                            std::cout << "cc2 is in edge!\n";
                        #endif
                        // determine height of point. must be on line, so:
                        if ( fabs(x1 - x2) > fabs(y1 - y2)  ) {
                            #ifdef EDGEDROP_DEBUG
                                std::cout << "computing cc2 height from x-coords\n";
                            #endif
                            cc2.z = z1 + ((z2-z1)/(x2-x1)) * (cc2.x-x1);
                        }
                        else if ( !isZero_tol( fabs(y1 - y2) )  ) {
                            #ifdef EDGEDROP_DEBUG
                                std::cout << "computing cc2 height from y-coords\n";
                            #endif
                            cc2.z = z1 + ((z2-z1)/(y2-y1)) * (cc2.y-y1);
                        }
                        else {// we are in trouble.
                            std::cout << "cyclutter edge-test, unable to compute cc-point. stop.\n";
                            assert(0);
                        }
                            
                        if (cl.liftZ(cc2.z)) {
                            cc = cc2;
                            cc.type = EDGE;
                            result=1;
                        }
                        //std::cout << "intersect case: cc2 isInside=true! cc2=" << cc2 << "\n";
                    }
                    
                } //end two intersection points case
                
            }// end if(potential hit)
            else {
                // edge is too far away from cutter. nothing to do.
            }
        } // end if(vertical edge)
        
    } // end loop through all edges

    return result;
}


//************** push cutter methods **********************************/
//************** push cutter methods **********************************/


/// push cutter along Fiber against vertices of Triangle t
/// add interfering intervals to the Fiber
int CylCutter::vertexPush(Fiber& f, Interval& i, const Triangle& t) const {
    int result = 0;
    BOOST_FOREACH( const Point& p, t.p)
    {
        //std::cout << "clength=" << getLength() << "\n";
        //std::cout << "zmax=" << f.p1.z+getLength()<< "\n";
        //std::cout << "p.z=" << p.z << "\n";
        if ( ( p.z >= f.p1.z ) && ( p.z <= (f.p1.z+getLength()) ) ) {
            Point pq = p.xyClosestPoint(f.p1, f.p2);
            double q = (p-pq).xyNorm(); // distance in XY-plane from fiber to p
            if ( q <= radius ) { // p is inside the cutter
                double ofs = sqrt( square(radius) - square(q) ); // distance along fiber 
                Point start = pq - ofs*f.dir;
                Point stop  = pq + ofs*f.dir;
                f.addInt(f.tval(start) , f.tval(stop)) ;
                    i.start_cc = p;
                    i.start_cc.type = VERTEX;
                    result=1;
                
            }             
        }
    }
    return result;
}

/// push cutter along Fiber against facet of Triangle t
/// add an interval to Fiber where the cutter interferes
int CylCutter::facetPush(Fiber& f, Interval& i,  const Triangle& t) const {
    int result = 0;
    
    
    
    return result;
}    

int CylCutter::edgePush(Fiber& f, Interval& i,  const Triangle& t) const {
    int result = 0;
    
    
    
    return result;
}   

MillingCutter* CylCutter::offsetCutter(double d) const {
    return new BullCutter(diameter+2*d, d) ;
}
    
//********  CylCutter string output ********************** */
std::string CylCutter::str() const
{
    std::ostringstream o;
    o << *this;
    return o.str();
}

std::ostream& operator<<(std::ostream &stream, CylCutter c)
{
  stream << "CylCutter (d=" << c.diameter << ")";
  return stream;
}

} // end namespace
// end file cylcutter.cpp
