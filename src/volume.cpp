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

//#include <iostream>
//#include <stdio.h>
//#include <sstream>
//#include <math.h>
//#include <vector>
//#include <algorithm>

#include <cassert>
// uncomment to disable assert() calls
// #define NDEBUG

#include <boost/python.hpp>
//#include <list>

#include "point.h"
#include "triangle.h"
#include "millingcutter.h"
#include "numeric.h"
#include "octree.h"
#include "volume.h"

namespace ocl
{

//************* OCTVolume base-class **************/

/// return true if point p is inside the Bbox bb
bool OCTVolume::isInsideBB(Point& p) const{
    return bb.isInside(p);
}

/// return false if the Ocode is outside the Bbox
bool OCTVolume::isInsideBBo(Ocode& o) const
{
    // bb.maxpt has maximum xyz coords
    // bb.minpt has minimum xyz coords
    // o.corner(0) is max
    // o.corner(7) is min
    // so we compare these:
    Point o_maxpt = o.corner(0);
    Point o_minpt = o.corner(7);
    if ( bb.maxpt.x < o_minpt.x )
        return false;
    else if ( bb.minpt.x > o_maxpt.x )
        return false;
    else if ( bb.maxpt.y < o_minpt.y )
        return false;
    else if ( bb.minpt.y > o_maxpt.y )
        return false;
    else if ( bb.maxpt.z < o_minpt.z )
        return false;
    else if ( bb.minpt.z > o_maxpt.z )
        return false;
            
    return true;
    /* old bad way of doing this
    for (int n=0;n<9;n++) { // loop through the 9 points of the ocode
        Point p = o.corner(n);
        if (isInsideBB(p)) // this is too expensive. avoid.
            return true;
    }
    return false;
    */
}

//************* Sphere **************/

/// sphere at center
SphereOCTVolume::SphereOCTVolume()
{
    center = Point(2,0,0);
    radius = 3.0;
    calcBB();
}

/// set the bounding box values
void SphereOCTVolume::calcBB() {
    Point maxpt = Point(center.x + radius, center.y + radius, center.z + radius);
    Point minpt = Point(center.x - radius, center.y - radius, center.z - radius);
    bb.addPoint( maxpt );
    bb.addPoint( minpt );
}

bool SphereOCTVolume::isInside(Point& p) const
{
    if (!isInsideBB(p))
        return false;
        // build time with isInsideBB
        // 1.74 1.73 1.68 1.67
        // without isInsideBB
        // 1.66 1.79 1.79 1.94
    
    if ( (center-p).norm() <= radius ) 
        return true;
    else
        return false;
}


//************* Cube **************/

/// cube at center with side length side
CubeOCTVolume::CubeOCTVolume()
{
    center = Point(0,0,0);
    side = 1.234;
}

/// set bbox values
void CubeOCTVolume::calcBB()
{
    Point maxpt = Point(center.x + side/2, center.y + side/2, center.z + side/2);
    Point minpt = Point(center.x - side/2, center.y - side/2, center.z - side/2);
    bb.addPoint( maxpt );
    bb.addPoint( minpt );
}

bool CubeOCTVolume::isInside(Point& p) const
{
    bool x,y,z;
    x = ( (p.x >= (center.x-side/2)) && (p.x <= (center.x+side/2)) );
    y = ( (p.y >= (center.y-side/2)) && (p.y <= (center.y+side/2)) );
    z = ( (p.z >= (center.z-side/2)) && (p.z <= (center.z+side/2)) );
    if ( x && y && z )
        return true;
    else
        return false;
}


//************* Box *******************/

BoxOCTVolume::BoxOCTVolume()
{
    corner = Point(0,0,0); 
    v1 = Point(1,0,0); 
    v2 = Point(0,1,0);
    v3 = Point(0,0,1);
}

void BoxOCTVolume::calcBB() {
    bb.addPoint(corner);
    bb.addPoint(corner+v1);
    bb.addPoint(corner+v2);
    bb.addPoint(corner+v3);
}

bool BoxOCTVolume::isInside(Point& p) const
{
    // translate to origo
    Point pt = p - corner;
    
    // box is = a*v1 + b*v2 + c*v3
    // where a,b,c are in [0,1]
    
    // v1 radial
    // v2 along move
    // v3 axial(z-dir)
    
    Point v1xy = v1;
    v1xy.z = 0;
    
    Point v2xy = v2;
    v2xy.z = 0;
    
    // projection along each vector, in turn
    // this only works if the vectors are orthogonal
    double t1 = pt.dot(v1xy)/v1xy.dot(v1xy);
    if ( (t1 < 0.0) || (t1>1.0) )
        return false;
        
    double t2 = pt.dot(v2xy)/v2xy.dot(v2xy);
    if ( (t2 < 0.0) || (t2>1.0) )
        return false;
    
    // this ensures we are OK in the XY plane
    // now check the z-coordinate.
    double zmin = corner.z + t2*v2.z;
    if (p.z < zmin)
        return false;
    double zmax = corner.z + v3.z + t2*v2.z;
    if (p.z > zmax)
        return false;
        
    //t = pt.dot(v3)/v3.dot(v3);
    //if ( (t < 0.0) || (t>1.0) )
    //    return false;
    
    return true;
    
}

//************* Cylinder **************/

CylinderOCTVolume::CylinderOCTVolume()
{
    p1 = Point(0,0,0);
    p2 = Point(1,1,1);
    radius = 1.234;
}

bool CylinderOCTVolume::isInside(Point& p) const 
{
    // closest point on axis
    Point c = p.closestPoint(p1, p2);
    // line = p1 + t*(p2-p1)
    // t is in [0,1] for points on the line
    double t = (c.dot(p2-p1) - p1.dot( p2-p1)) / (p2-p1).dot(p2-p1);
    if ( (t>1.0) || (t < 0.0))
        return false;
        
    if ( (c-p).norm() <= radius)
        return true;
    else
        return false;
}

/// calculate the bounding-box 
void CylinderOCTVolume::calcBB()
{
    // this is an approximate bounding-box, not exact
    Point xmax1 = p1 + Point(radius,0,0);
    Point xmin1 = p1 - Point(radius,0,0);
    Point xmax2 = p2 + Point(radius,0,0);
    Point xmin2 = p2 - Point(radius,0,0);
    
    Point ymax1 = p1 + Point(0,radius,0);
    Point ymin1 = p1 - Point(0,radius,0);
    Point ymax2 = p2 + Point(0,radius,0);
    Point ymin2 = p2 - Point(0,radius,0);    
    
    Point zmax1 = p1 + Point(0,0,radius);
    Point zmin1 = p1 - Point(0,0,radius);
    Point zmax2 = p2 + Point(0,0,radius);
    Point zmin2 = p2 - Point(0,0,radius);  
    
    bb.addPoint( xmax1 );   
    bb.addPoint( xmax2 );
    bb.addPoint( xmin1 );
    bb.addPoint( xmin2 );

    bb.addPoint( ymax1 );   
    bb.addPoint( ymax2 );
    bb.addPoint( ymin1 );
    bb.addPoint( ymin2 );
    
    bb.addPoint( zmax1 );   
    bb.addPoint( zmax2 );
    bb.addPoint( zmin1 );
    bb.addPoint( zmin2 );
    
    
}
//************* EtubeOCTVolume *************/


EtubeOCTVolume::EtubeOCTVolume() 
{
    p1 = Point(0,0,0);
    p2 = Point(1,0,0);
    a = Point(0,0.1,0);
    b = Point(0,0,0.2);
}

EtubeOCTVolume::EtubeOCTVolume(Point& p1in, Point& p2in, Point& ain, Point& bin) 
{
    p1 = p1in;
    p2 = p2in;
    a = ain;
    b = bin;
}


bool EtubeOCTVolume::isInside(Point& p) const 
{
    
    // xy-plane check
    
    //Point v = p2-p1;
    //Point vxy = v;
    //vxy.z =0;
    
    // translate so (0,0) is at p1
    //Point pt = p1 - p;
    
    // restrict to points closer than a.norm() to tool-line   
    double xyd = p.xyDistanceToLine(p1,p2);
    if (xyd > a.norm() )
        return false;
        
    // coordinates along ellipse
    
    // center of ellipse
    Point close = p.closestPoint(p1, p2);
    
    Point ellvec = p-close;
    
    // is ellvec within ellipse?
    double ta = ellvec.dot(a)/a.dot(a);
    double tb = ellvec.dot(b)/b.dot(b);
    if ( (ta>1.0) || (ta<-1.0) )
        return false;
        
    if ( (tb>1.0) || (tb<-1.0) )
        return false;
        
    if ((ta*ta + tb*tb) > 1.0)
        return false;
    
    // projection along 
    // this only works if the vectors are orthogonal
    /*
    double t1 = pt.dot(v1xy)/v1xy.dot(v1xy);
    if ( (t1 < 0.0) || (t1>1.0) )
        return false;
        
    double t2 = pt.dot(v2xy)/v2xy.dot(v2xy);
    if ( (t2 < 0.0) || (t2>1.0) )
        return false;
    */
    
    // z-direction check
    double maxz = p1.z > p2.z ? p1.z : p2.z; 
    double minz = p1.z < p2.z ? p1.z : p2.z;   
    
    if (p.z < minz)
        return false;
        
    if (p.z > maxz)
        return false;
        
    
    //if (p.z 
    // figure out where we are and return true/false
    return true;
}


//************* CylCutterMove **************/


CylMoveOCTVolume::CylMoveOCTVolume(const CylCutter& cin, const Point& p1in, const Point& p2in)
{
    p1 = p1in;
    p2 = p2in;
    c = cin;
    
    Point cutter_zvec = Point(0,0,c.getLength());
    // start of move, extreme point(s)
    bb.addPoint( p1 + c.getRadius()*Point(1,0,0) );
    bb.addPoint( p1 + c.getRadius()*Point(-1,0,0) );
    bb.addPoint( p1 + c.getRadius()*Point(0,1,0) );
    bb.addPoint( p1 + c.getRadius()*Point(0,-1,0) );
    // start of move (top)
    bb.addPoint( p1 + c.getRadius()*Point(1,0,0)  + cutter_zvec );
    bb.addPoint( p1 + c.getRadius()*Point(-1,0,0) + cutter_zvec );
    bb.addPoint( p1 + c.getRadius()*Point(0,1,0)  + cutter_zvec );
    bb.addPoint( p1 + c.getRadius()*Point(0,-1,0) + cutter_zvec );
    // end of move (bottom)
    bb.addPoint( p2 + c.getRadius()*Point(1,0,0) );
    bb.addPoint( p2 + c.getRadius()*Point(-1,0,0) );
    bb.addPoint( p2 + c.getRadius()*Point(0,1,0) );
    bb.addPoint( p2 + c.getRadius()*Point(0,-1,0) );
    // end of move (top)
    bb.addPoint( p2 + c.getRadius()*Point(1,0,0)  + cutter_zvec );
    bb.addPoint( p2 + c.getRadius()*Point(-1,0,0) + cutter_zvec );
    bb.addPoint( p2 + c.getRadius()*Point(0,1,0)  + cutter_zvec );
    bb.addPoint( p2 + c.getRadius()*Point(0,-1,0) + cutter_zvec );
    
    
    // cylinder at start of move
    c1.p1 = p1;
    c1.p2 = p1+Point(0,0,c.getLength());
    c1.radius=c.getRadius();
    // std::cout << " startcyl at " << c1.p1 << " to " << c1.p2 << "radius="<< c1.radius << "\n";
    
    // cylinder at end of move
    c2.p1 = p2;
    c2.p2 = p2+Point(0,0,c.getLength());
    c2.radius=c.getRadius();
    // std::cout << " endcyl at " << c2.p1 << " to " << c2.p2 <<" radius=" << c2.radius << "\n";
    
    // for XY-plane moves, a box:
    Point v = p2-p1; // vector along move
    Point v2 = p2-p1; 
    box.v2 = p2-p1;
    
    v.z = 0;
    v.normalize();
    
    box.corner = p1 + c.getRadius()*v.xyPerp();
    box.v1 = -2*c.getRadius()*v.xyPerp();
    box.v2 = v2;
    box.v3 = Point(0,0,c.getLength());
    // std::cout << " box at corner=" << box.corner << "\n";
    // std::cout << "            v1=" << box.v1 << "\n";
    // std::cout << "            v2=" << box.v2 << "\n";
    // std::cout << "            v3=" << box.v3 << "\n";
    
    // the elliptic tube
    etube = EtubeOCTVolume();
    // set the parameters
    etube.p1 = p1;
    etube.p2 = p2;
    etube.a = c.getRadius()*v.xyPerp();
    //etube.b = Point(0,0,0); //fixme
    // angle of move
    double dz = p2.z-p1.z;
    double length = (p2-p1).norm();
        
    double sin = dz/length;
    //double cos = sqrt( 1.0-sin*sin);
    double baxis = fabs(c.getRadius()*sin);
    // std::cout << " Etube baxis length="<< baxis << "\n";
    // direction is cross product 
    Point bdir = (p2-p1).cross(etube.a);
    bdir.normalize();
    etube.b= baxis*bdir;
    // std::cout << " Etube a="<< etube.a << " b=" << etube.b << "\n";
    
}

bool CylMoveOCTVolume::isInside(Point& p) const 
{
    // CL follows line
    // line = p1 + t*(p2-p1)
    // top of cutter follows same line only c.length() higher
    if (c1.isInside(p)) 
        return true;
    
    if (c2.isInside(p))
        return true;
    
    // for XY-plane moves, a box:
    if (box.isInside(p))
        return true;
    
    // the Elliptic tube
    if (etube.isInside(p))
        return true;
        
    // the default is to return false
    return false;

}

} // end namespace
// end of file volume.cpp
