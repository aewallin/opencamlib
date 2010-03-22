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
BallCutter::BallCutter()
{
    setDiameter(1.0);
}

BallCutter::BallCutter(const double d)
{
    setDiameter(d);
    radius = d/2;
}

//********   drop-cutter methods ********************** */
int BallCutter::vertexDrop(Point &cl, CCPoint &cc, const Triangle &t) const
{
    // some math here: http://www.anderswallin.net/2007/06/drop-cutter-part-13-cutter-vs-vertex/
    
    int result = 0;
    
    BOOST_FOREACH( const Point& p, t.p)
    {
        // distance in XY-plane from cl to p
        double q = cl.xyDistance(p);
        
        if (q<= diameter/2) { // p is inside the cutter
            // q^2 + h2^2 = r^2
            // h2 = sqrt( r^2 - q^2 )
            // h1 = r - h2
            // cutter_tip = p.z - h1
            double h1 = radius - sqrt( radius*radius - q*q );
            if (cl.liftZ(p.z - h1)) { // we need to lift the cutter
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

int BallCutter::facetDrop(Point &cl, CCPoint &cc, const Triangle &t) const
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


int BallCutter::edgeDrop(Point &cl, CCPoint &cc, const Triangle &t) const
{
    // Drop cutter at (p.x, p.y) against edges of Triangle t
    // strategy:
    // 1) calculate distance to infinite line
    // 2) calculate intersection points w. cutter
    // 3) pick the higher intersection point and test if it is in the edge
    
    int result = 0;
    
    for (int n=0;n<3;n++) { // loop through all three edges
    
        // 1) distance from point to line in xy plane
        int start=n;      // index of the start-point of the edge
        int end=(n+1)%3;  // index of the end-point of the edge
        //std::cout << "testing edge " << start << " to " << end << "\n";
        Point p1 = t.p[start];
        Point p2 = t.p[end];
        //std::cout << "Points " << p1 << " to " << p2 << "\n";
        double d = cl.xyDistanceToLine(p1, p2);
        if (d < 0.0)
            std::cout << "ballcutter.cpp: d<0 ERROR!\n";
            
        if (d<=radius) { // potential hit
        
            // the plane of the line will slice the spherical cutter at
            // a distance d from the center of the cutter
            // here the radius of the circular section is
            double s = sqrt( radius*radius - d*d );
                
            // the center-point of this circle, in the xy plane lies at
            Point sc = cl.xyClosestPoint( p1, p2 );           
            
            Point v = p2 - p1;
            Point start2sc_dir = sc - p1;
            start2sc_dir.xyNormalize();
            start2sc_dir.z=0;
            double dz = p2.z - p1.z;
            double p2u = v.dot(start2sc_dir); // xyNorm(); // u-coord of p2 in plane coordinates.
            // was: dot(start2sc_dir); ??  
             
            //v.normalize(); // 3D
            
            // in the vertical plane of the line:
            // (du,dz) points in the direction of the line
            // so (dz, -du) is a normal to the line
            Point tangent = Point(p2u,dz,0);
            tangent.xyNormalize();
            Point normal = Point (dz, -p2u, 0);
            normal.xyNormalize();
            if (normal.y < 0) { // flip normal so it points upward
                normal = -1*normal;
            }
            
            Point start2sc = sc - p1;
            double sc_u = start2sc.dot( start2sc_dir  ); // horiz distance from startpoint to sc
            
            double cc_u = sc_u - s * normal.x; // horiz dist of cc-point in plane-cordinates
            
            Point cc_tmp = p1 + (cc_u/p2u)*v;
            
            double cl_z = cc_tmp.z + s*normal.y - radius;
            
            // test if cc-point is in edge
            if ( cc_tmp.isInsidePoints( p1, p2 ) ) {
                if (cl.liftZ(cl_z)) {
                    cc = cc_tmp;
                    cc.type = EDGE;
                }
            
            }
            
        }// end if(potential hit)
        else {
            // edge is too far away from cutter. nothing to do.
        }
        
    } // end loop through all edges
        
    return result;
}




//********  CylCutter string output ********************** */
std::string BallCutter::str()
{
    std::ostringstream o;
    o << "BallCutter"<< id <<"(d=" << diameter << ", radius=" << radius << ")";
    return o.str();
}

std::ostream& operator<<(std::ostream &stream, BallCutter c)
{
  stream << "BallCutter"<< c.id <<"(d=" << c.diameter << ", radius=" << c.radius << ")";
  return stream;
}

