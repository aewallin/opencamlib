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
#include "stlsurf.h"
#include "cutter.h"
#include "numeric.h"

namespace ocl
{
    

//********   constructors ********************** */
ConeCutter::ConeCutter()
{
    setDiameter(1.0);
    angle = 45;
    height = (diameter/2)/tan(angle);
}

ConeCutter::ConeCutter(const double d, const double a)
{
    setDiameter(d);
    angle = a;
    height = (diameter/2)/tan(angle);
}



//********   drop-cutter methods ********************** */
int ConeCutter::vertexDrop(Point &cl, CCPoint &cc, const Triangle &t) const
{
    int result = 0;
    BOOST_FOREACH( const Point& p, t.p)
    {
        double q = cl.xyDistance(p); // distance in XY-plane from cl to p
        if (q<= diameter/2) { // p is inside the cutter
            // h1 = q / tan(alfa)
            // cutter_tip = p.z - h1
            assert( tan(angle) > 0.0 ); // guard against division by zero
            double h1 =  q/tan(angle);
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


// we either hit the tip, when the slope of the plane is smaller than angle
// or when the slope is steep, the circular edge between the cone and the cylindrical shaft
int ConeCutter::facetDrop(Point &cl, CCPoint &cc, const Triangle &t) const
{
    int result = 0;
    // Drop cutter at (cl.x, cl.y) against facet of Triangle t

    Point normal; // facet surface normal
    
    if ( isZero_tol( t.n->z ) )  {// vertical surface
        return -1;  //can't drop against vertical surface
    } else if (t.n->z < 0) {  // normal is pointing down
        normal = -1* (*t.n); // flip normal
    } else {
        normal = *t.n;
    }   
    
    assert( isPositive( normal.z ) );
    
    if ( (isZero_tol(normal.x)) && (isZero_tol(normal.y)) ) { // horizontal plane
        // so any vertex is at the correct height
        Point cc_tmp;
        cc_tmp.x = cl.x;
        cc_tmp.y = cl.y;
        cc_tmp.z = t.p[0].z;
        if (cc_tmp.isInside(t)) { // cc-point is on the axis of the cutter       
            if ( cl.liftZ(cc_tmp.z) ) {
                cc = cc_tmp;
                cc.type = FACET_TIP;
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
        
    normal.xyNormalize(); // make length of normal == 1.0
    
    // cylindrical contact point case
    // find the xy-coordinates of the cc-point
    Point cyl_cc_tmp = cl - (diameter/2)*normal;
    cyl_cc_tmp.z = (1.0/c)*(-d-a*cyl_cc_tmp.x-b*cyl_cc_tmp.y);
    double cyl_cl_z = cyl_cc_tmp.z - height; // tip positioned here
    
    Point tip_cc_tmp = Point(cl.x,cl.y,0);
    tip_cc_tmp.z = (1.0/c)*(-d-a*tip_cc_tmp.x-b*tip_cc_tmp.y);
    double tip_cl_z = tip_cc_tmp.z;
          
    if (tip_cc_tmp.isInside(t)) { // TIP case     
        if ( cl.liftZ(tip_cl_z) ) {
            cc = tip_cc_tmp;
            cc.type = FACET_TIP;
            result = 1;
        }
    } if (cyl_cc_tmp.isInside(t))  { // CYLINDER case
        if ( cl.liftZ(cyl_cl_z) ) {
            cc = cyl_cc_tmp;
            cc.type = FACET_CYL;
            result = 1; 
        }
    } else {
        return 0;
    }
    return result; // we never get here (?)
}

// cone sliced with vertical plane results in a hyperbola as the intersection curve
// find point where hyperbola and line slopes match? (or use radius-vector approach as in ball cutter?) 
int ConeCutter::edgeDrop(Point &cl, CCPoint &cc, const Triangle &t) const
{
    // Drop cutter at (p.x, p.y) against edges of Triangle t
    // strategy:
    // 1) calculate distance to infinite line
    // 2) calculate intersection points w. cutter
    
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
        
            //std::cout << "Points " << p1 << " to " << p2 << "\n";
            double d = cl.xyDistanceToLine(p1, p2);
            assert( d >= 0.0 );
                
            if (d<=(diameter/2)) { // potential hit
            
                // http://en.wikipedia.org/wiki/Hyperbola
                // hyperbola defined by
                // x^2 / a^2 - y^2 / b^2 = 1
                // or in parametric form
                // ( a*cosh(u), b*sinh(u) )
                // where
                // x = c * cos(theta) * cosh(u)
                // y = c * sin(theta) * sinh(u)
                // hyperbola has focus at c on the x-axis
                // theta= angle of asymptotes with line of foci (?)
                // this parabola lies on its side and opens towards the +x axis
                // at u=0 the x-coordinate is a, since
                // cosh(0) = 1.0
                
                double a=1.0;
                
                
                // the plane of the line slices the cone at
                // a distance d from the center of the cutter
                // here the width of the parabola should be
                double s = sqrt( square((diameter/2)) - square(d) );
                    
                // the center-point of this circle, in the xy plane lies at
                Point sc = cl.xyClosestPoint( p1, p2 );   
                

                
                Point v = p2 - p1;
                Point start2sc_dir = sc - p1;
                start2sc_dir.xyNormalize();
                start2sc_dir.z=0;
                double dz = p2.z - p1.z;
                double p2u = v.dot(start2sc_dir); // u-coord of p2 in plane coordinates.
                
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
                assert( isPositive(normal.y) );
                
                Point start2sc = sc - p1;
                double sc_u = start2sc.dot( start2sc_dir  ); // horiz distance from startpoint to sc
                
                double cc_u = sc_u - s * normal.x; // horiz dist of cc-point in plane-cordinates
                
                Point cc_tmp = p1 + (cc_u/p2u)*v;
                
                double cl_z = cc_tmp.z + s*normal.y - (diameter/2);
                
                // test if cc-point is in edge
                if ( cc_tmp.isInsidePoints( p1, p2 ) ) {
                    if (cl.liftZ(cl_z)) {
                        cc = cc_tmp;
                        cc.type = EDGE;
                        result = 1;
                    }
                
                }
                
            }// end if(potential hit)
            else {
                // edge is too far away from cutter. nothing to do.
            }
        }// end if(vertical edge)
        
    } // end loop through all edges
        
    return result;
}


//******** string output ********************** */
std::string ConeCutter::str()
{
    std::ostringstream o;
    o << *this;
    return o.str();
}

std::ostream& operator<<(std::ostream &stream, ConeCutter c)
{
  stream << "ConeCutter (d=" << c.diameter << ", angle=" << c.angle << ", height=" << c.height << ")";
  return stream;
}

} // end namespace
// end file conecutter.cpp
