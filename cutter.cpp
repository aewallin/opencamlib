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
#include "cutter.h"
#include "point.h"
#include "triangle.h"

//********   MillingCutter ********************** */
int MillingCutter::count = 0;

MillingCutter::MillingCutter()
{   
    setId();
}

void MillingCutter::setDiameter(double d)
{
    if ( d >= 0.0 ) {
        diameter=d;
    } else {
        std::cout << "cutter.cpp: ERROR, MillingCutter.setDiameter(d) called with d<0 !!";
        diameter=1.0;
    }
}

void MillingCutter::setLength(double l)
{
    if ( l > 0.0 ) {
        length=l;
    } else {
        std::cout << "cutter.cpp: ERROR, MillingCutter.setLength(l) called with l<0 !!";
        length=1.0;
    }
}

void MillingCutter::setId()
{
		id = count;
		count++;
}

double MillingCutter::getDiameter()
{
    return diameter;
}

double MillingCutter::getLength()
{
    return length;
}




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
Point CylCutter::vertexDrop(Point &cl, const Triangle &t)
{
    /// loop through each vertex p of Triangle t
	/// drop down cutter at (cl.x, cl.y) against Point p
    Point cc;
    std::cout << "vertexDrop triangle=" << t << "\n";
    std::cout << "vertexDrop normal=" << t.n << "\n";
    
    BOOST_FOREACH( Point p, t.p)
    {
        // distance in XY-plane from cl to p
        double q = cl.xyDistance(p);
        std::cout << "xyDistance(cl, vertex) is: " << q << "\n";
        if (q<= diameter/2) { // p is inside the cutter
            std::cout << "inside case\n";
            if (p.z > cl.z) { // we need to lift the cutter
                cl.liftZ(p.z);
                cc = p;
                std::cout << "cl=" << cl << " cc=" << cc << "\n";
            }
        } else {
            std::cout << "outside case\n";
            std::cout << "cl=" << cl << " cc=" << cc << "\n";
        }
    }
    return cc;
}

Point CylCutter::facetDrop(Point &cl, const Triangle &t)
{
    // Drop cutter at (cl.x, cl.y) against facet of Triangle t
    Point cc;
    std::cout << "facetDrop triangle=" << t << "\n";
    std::cout << "facetDrop normal=" << t.n << "\n";
    Point normal; // facet surface normal
    
    if (t.n.z == 0)  {// vertical surface
        std::cout << "facetDrop vertical case. bye.\n";
        return cc;  //can't drop against vertical surface
    } else if (t.n.z < 0) {  // normal is pointing down
        normal = -1*t.n; // flip normal
        std::cout << "facetDrop flip normal\n";
    } else {
        std::cout << "facetDrop normal case\n";
        normal = t.n;
    }
    std::cout << "facetDrop normal="<<normal<<"\n";
    // define plane containing facet
    // a*x + b*y + c*z + d = 0, so
    // d = -a*x - b*y - c*z, where
    // (a,b,c) = surface normal
	double a = normal.x;
	double b = normal.y;
	double c = normal.z;
	double d = - normal.x * t.p[0].x - normal.y * t.p[0].y - normal.z * t.p[0].z;
    std::cout << "facetDrop plane d="<<d<<"\n";

    
    normal.xyNormalize(); // make length of normal in xy plane == 1.0
    
    // the contact point with the plane is on the periphery
    // of the cutter, a length diameter/2 from cl in the direction of -n
    cc = cl - (diameter/2)*normal;
    if (cc.isInside(t)) { // NOTE: cc.z is ignored in isInside()
        cc.z = (1.0/c)*(-d-a*cc.x-b*cc.y); // NOTE: potential for divide-by-zero (?!)
        cl.liftZ(cc.z);
    }        
    return cc;
}

// FIXME: place this somewhere better
double sign(double x) {
    if (x<0.0)
        return -1;
    else
        return 1;
}

Point CylCutter::edgeDrop(Point &cl, const Triangle &t)
{
    // Drop cutter at (p.x, p.y) against edges of Triangle t
    // strategy:
    // 1) calculate distance to infinite line
    // 2) calculate intersection points w. cutter
    // 3) pick the higher intersection point and test if it is in the edge
    Point cc;
  
    for (int n=0;n<3;n++) { // loop through all three edges
    
        // 1) distance from point to line
        int start=n;
        int end=(n+1)%3;
        double d = cl.xyDistanceToLine(t.p[start],t.p[end]);
        if (d<=diameter/2) { // potential hit
        
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
            
            if (discr < 0) {
                std::cout << "cutter.cpp ERROR: CylCutter::edgeTest discr<0 !!\n";
                return cc;
            } else if (discr == 0.0) {// tangent line
                cc.x = D*dy / pow(dr,2) + cl.x; // translate back to cl
                cc.y = -D*dx / pow(dr,2) + cl.y;
                // 3) check if cc is in edge
                // determine height of point. must be on line, so:
            
            } else { // discr > 0, two intersection points
                Point cc1;
                Point cc2;
                cc1.x= (D*dy  + sign(dy)*dx*sqrt(discr)) / pow(dr,2);
                cc1.y= (-D*dx + fabs(dy)*sqrt(discr)   ) / pow(dr,2);
                cc2.x= (D*dy  - sign(dy)*dx*sqrt(discr)) / pow(dr,2);
                cc2.y= (-D*dx - fabs(dy)*sqrt(discr)   ) / pow(dr,2);
                // 3) check if in edge
                
            }
        }
    }
        
    
    return cc;
}




//********  CylCutter other  methods ********************** */
std::string CylCutter::str()
{
	std::ostringstream o;
	o << "CylCutter"<< id <<"(d=" << diameter << ")";
	return o.str();
}

std::ostream& operator<<(std::ostream &stream, CylCutter c)
{
  stream << "CylCutter"<< c.id <<"(d=" << c.diameter << ")";
  return stream;
}
