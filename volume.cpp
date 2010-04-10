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

// this is mostly a translation to c++ of the earlier c# code
// http://code.google.com/p/monocam/source/browse/trunk/Project2/monocam_console/monocam_console/kdtree.cs

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

//************* Volumes **************/

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





// end of file octree.cpp
