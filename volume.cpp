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
#include <vector>
#include <algorithm>
#include <boost/python.hpp>
#include <list>

// uncomment to disable assert() calls
// #define NDEBUG
#include <cassert>

#include "cutter.h"
#include "point.h"
#include "triangle.h"
#include "cutter.h"
#include "numeric.h"
#include "octree.h"
#include "volume.h"

//************* Sphere **************/

/// sphere at center
SphereOCTVolume::SphereOCTVolume()
{
    center = Point(2,0,0);
    radius = 3.0;
}


bool SphereOCTVolume::isInside(Point& p) const
{
    
    if ( (center-p).norm() <= radius ) {
        //std::cout << "dist to point=" << (center-p).norm() <<"\n";
        return true;
    }
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

bool BoxOCTVolume::isInside(Point& p) const
{
    // translate to origo
    Point pt = p - corner;
    
    // projection along each vector, in turn
    double t = pt.dot(v1)/v1.dot(v1);
    if ( (t < 0.0) || (t>1.0) )
        return false;
        
    t = pt.dot(v2)/v2.dot(v2);
    if ( (t < 0.0) || (t>1.0) )
        return false;
        
    t = pt.dot(v3)/v3.dot(v3);
    if ( (t < 0.0) || (t>1.0) )
        return false;
    
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


//************* CylCutterMove **************/


CylMoveOCTVolume::CylMoveOCTVolume(const CylCutter& cin, const Point& p1in, const Point& p2in)
{
    p1 = p1in;
    p2 = p2in;
    c = cin;
    
    // cylinder at start of move
    c1.p1 = p1;
    c1.p2 = p1+Point(0,0,c.getLength());
    c1.radius=c.getRadius();
    
    // cylinder at end of move
    c2.p1 = p2;
    c2.p2 = p2+Point(0,0,c.getLength());
    c2.radius=c.getRadius();
    
    // for XY-plane moves, a box:
    Point v = p2-p1; // vector along move
    box.v2 = p2-p1;
    v.normalize();
    
    box.corner = c.getRadius()*v.xyPerp();
    box.v1 = -2*c.getRadius()*v.xyPerp();
    box.v2 = v;
    box.v3 = Point(0,0,c.getLength());
    
}

bool CylMoveOCTVolume::isInside(Point& p) const 
{
    // CL follows line
    // line = p1 + t*(p2-p1)
    // top of cutter follows same line only c.length() higher
    //CylinderOCTVolume c1;
    //CylinderOCTVolume c2;
    
    // cylinder at start of move
    //c1.p1 = p1;
    //c1.p2 = p1+Point(0,0,c.getLength());

    if (c1.isInside(p)) 
        return true;
    
    if (c2.isInside(p))
        return true;
    
    // for XY-plane moves, a box:
    //BoxOCTVolume box;

    
    //if (b.isInside(p))
    //    return true;
    
        
    // Elliptic tube...
    
    
    // the default is to return false
    return false;

}

// end of file volume.cpp
