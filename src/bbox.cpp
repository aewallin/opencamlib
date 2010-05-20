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
#include <vector>
#include <algorithm>
#include <boost/python.hpp>
#include <list>
#include <cassert>

// uncomment to disable assert() calls
// #define NDEBUG

#include "bbox.h"
#include "point.h"

namespace ocl
{


//************* Bounding-Box **************/

Bbox::Bbox()
{
    maxx=0;
    minx=0;
    maxy=0;
    miny=0;
    maxz=0;
    minz=0;
}

bool Bbox::isInside(Point& p) const
{
    if (p.x > maxx)
        return false;
    else if (p.x < minx)
        return false;
    else if (p.y > maxy)
        return false;
    else if (p.y < miny)
        return false;
    else if (p.z > maxz)
        return false;
    else if (p.z < minz)
        return false;
    else
        return true;
}

void Bbox::addPoint(Point &p)
{
    if (p.x > maxx)
        maxx = p.x;
    if (p.x < minx)
        minx = p.x;
    
    if (p.y > maxy)
        maxy = p.y;
    if (p.y < miny)
        miny = p.y;
    
    if (p.z > maxz)
        maxz = p.z;
    if (p.z < minz)
        minz = p.z;
        
}


} // end namespace
// end of file volume.cpp
