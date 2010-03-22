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
#include "cutter.h"
#include "numeric.h"



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
            double h1 = radius2 - sqrt( radius2*radius2 - (q-radius1)*(q-radius1) );
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



// FIXME: this is the code for Spherical...
int BullCutter::facetDrop(Point &cl, CCPoint &cc, const Triangle &t) const
{
    // Drop cutter at (cl.x, cl.y) against facet of Triangle t

    Point normal; // facet surface normal
    
    if (t.n->z == 0)  {// vertical surface
        return -1;  //can't drop against vertical surface
    } else if (t.n->z < 0) {  // normal is pointing down
        normal = -1* (*t.n); // flip normal
    } else {
        normal = *t.n;
    }   
    
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
    
    // define the radiusvector which points from the 
    // ball-center to the cc-point.
    Point radiusvector = -radius*normal;
    
    // find the xy-coordinates of the cc-point
    Point cc_tmp = cl + radiusvector;
    
    // find the z-coordinate of the cc-point.
    // it lies in the plane.
    cc_tmp.z = (1.0/c)*(-d-a*cc_tmp.x-b*cc_tmp.y); // NOTE: potential for divide-by-zero (?!)
    
    // now find the z-coordinate of the cl-point
    double tip_z = cc_tmp.z - radiusvector.z - radius;
        
    if (cc_tmp.isInside(t)) { // NOTE: cc.z is ignored in isInside()       
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



// FIXME FIXME FIXME. this is totally wrong for now...
int BullCutter::edgeDrop(Point &cl, CCPoint &cc, const Triangle &t) const
{
    // Drop cutter at (p.x, p.y) against edges of Triangle t
    // strategy:
    // 1) calculate distance to infinite line
    // 2) calculate intersection points w. cutter
    // 3) pick the higher intersection point and test if it is in the edge
    //Point cc;
    int result = 0;
    
    for (int n=0;n<3;n++) { // loop through all three edges
    
        // 1) distance from point to line
        int start=n;
        int end=(n+1)%3;
        //std::cout << "testing poinst " << start<< " to " << end << " :";
        double d = cl.xyDistanceToLine(t.p[start],t.p[end]);
        //std::cout << "xyDistance=" << d ;
        
        if (d<=diameter/2) { // potential hit
            //std::cout << " potential hit\n";
            // 2) calculate intersection points w. cutter circle
            // points are on line and diameter/2 from cl
            // see http://mathworld.wolfram.com/Circle-LineIntersection.html
            double x1 = t.p[start].x - cl.x; // translate to cl=(0,0)
            double y1 = t.p[start].y - cl.y;
            double x2 = t.p[end].x - cl.x;
            double y2 = t.p[end].y - cl.y;
            double dx = x2-x1;
            double dy = y2-y1;
            double dr = sqrt( dx*dx + dy*dy);
            double D = x1*y2 - x2*y1;
            double discr = pow(diameter/2,2) * pow(dr,2) - pow(D,2);
            //std::cout << "discr=" << discr << "\n";
            
            if (Numeric::isNegative(discr)) {
                std::cout << "cutter.cpp ERROR: CylCutter::edgeTest discr= "<<discr<<" <0 !!\n";
                
                cc.type = ERROR;
                return -1;
                
            } else if (Numeric::isZero(discr)) {// tangent line
                cc.x = D*dy / pow(dr,2) + cl.x; // translate back to cl
                cc.y = -D*dx / pow(dr,2) + cl.y;
                // 3) check if cc is in edge
                if ( cc.isInsidePoints(t.p[start], t.p[end]) ) { 
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
                    if (x1 != x2) 
                        cc.z = z1 + ((z2-z1)/(x2-x1)) * (cc.x-x1);
                    else if (y1 != y2) 
                        cc.z = z1 + ((z2-z1)/(y2-y1)) * (cc.y-y1);
                       
                    if (cl.liftZ(cc.z))
                        cc.type = EDGE;
                }
            } else { // discr > 0, two intersection points
                Point cc1;
                Point cc2;
                // remember to translate back to cl
                cc1.x= (D*dy  + Numeric::sign(dy)*dx*sqrt(discr)) / pow(dr,2) + cl.x; 
                cc1.y= (-D*dx + fabs(dy)*sqrt(discr)   ) / pow(dr,2) + cl.y;
                cc1.z=0;
                cc2.x= (D*dy  - Numeric::sign(dy)*dx*sqrt(discr)) / pow(dr,2) + cl.x;
                cc2.y= (-D*dx - fabs(dy)*sqrt(discr)   ) / pow(dr,2) + cl.y;
                cc2.z=0;
                // 3) check if in edge
                double z1 = t.p[start].z;
                double z2 = t.p[end].z;
                double x1 = t.p[start].x;
                double x2 = t.p[end].x;
                double y1 = t.p[start].y;
                double y2 = t.p[end].y;
                if ( cc1.isInsidePoints(t.p[start], t.p[end]) ) {
                    // determine height of point. must be on line, so:
                    if (x1 != x2) 
                        cc1.z = z1 + ((z2-z1)/(x2-x1)) * (cc1.x-x1);
                    else if (y1 != y2) 
                        cc1.z = z1 + ((z2-z1)/(y2-y1)) * (cc1.y-y1);
           
                    if (cl.liftZ(cc1.z)) {
                        cc=cc1;
                        cc.type = EDGE;
                    }
                    //std::cout << "intersect case: cc1 isInside=true! cc1=" << cc1 << "\n";
                }
                if ( cc2.isInsidePoints(t.p[start], t.p[end]) ) {
                    // determine height of point. must be on line, so:
                    if (x1 != x2) 
                        cc2.z = z1 + ((z2-z1)/(x2-x1)) * (cc2.x-x1);
                    else if (y1 != y2) 
                        cc2.z = z1 + ((z2-z1)/(y2-y1)) * (cc2.y-y1);
                        
                    if (cl.liftZ(cc2.z)) {
                        cc = cc2;
                        cc.type = EDGE;
                    }
                    //std::cout << "intersect case: cc2 isInside=true! cc2=" << cc2 << "\n";
                }
                
                
            } //end two intersection points case
            
        }// end if(potential hit)
        else {
            // edge is too far away from cutter. nothing to do.
        }
        
    } // end loop through all edges
        
    return result;
}




//********  CylCutter string output ********************** */
std::string BullCutter::str()
{
    std::ostringstream o;
    o << "BullCutter"<< id <<"(d=" << diameter << ", radius1=" << radius1 << ", radius2=" << radius2 << ")";
    return o.str();
}

std::ostream& operator<<(std::ostream &stream, BullCutter c)
{
  stream << "BallCutter"<< c.id <<"(d=" << c.diameter << ", radius1=" << c.radius1 << " radius2=" << c.radius2 << ")";
  return stream;
}

