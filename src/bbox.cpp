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


#include <vector>
#include <list>
#include <cassert>
// uncomment to disable assert() calls
// #define NDEBUG

#include <boost/python.hpp>

#include "bbox.h"
#include "point.h"
#include "triangle.h"

namespace ocl
{


//************* Bounding-Box **************/

Bbox::Bbox() {
    minpt = Point(0,0,0);
    maxpt = Point(0,0,0);
    initialized = false;
}

Bbox::~Bbox() {
    return;
}

bool Bbox::isInside(Point& p) const {
    assert( initialized );
    if (p.x > maxpt.x)
        return false;
    else if (p.x < minpt.x)
        return false;
    else if (p.y > maxpt.y)
        return false;
    else if (p.y < minpt.y)
        return false;
    else if (p.z > maxpt.z)
        return false;
    else if (p.z < minpt.z)
        return false;
    else
        return true;
}

void Bbox::addPoint(const Point &p) {
    if (!initialized) {
        maxpt = p;
        minpt = p;
        initialized = true;
    } else {
        if (p.x > maxpt.x)
            maxpt.x = p.x;
        if (p.x < minpt.x)
            minpt.x = p.x;
        
        if (p.y > maxpt.y)
            maxpt.y = p.y;
        if (p.y < minpt.y)
            minpt.y = p.y;
        
        if (p.z > maxpt.z)
            maxpt.z = p.z;
        if (p.z < minpt.z)
            minpt.z = p.z;
    }
        
}

/// add each vertex of the Triangle      
void Bbox::addTriangle(const Triangle &t) {   
    addPoint( t.p[0] );
    addPoint( t.p[1] );
    addPoint( t.p[2] );
    return;
}

/// return the bounding box values as a vector:
///  0    1    2    3    4    5
/// [minx maxx miny maxy minz maxz]
double Bbox::operator[](const unsigned int idx) const{
    switch(idx) {
        case 0:
            return minpt.x;
            break;
        case 1:
            return maxpt.x;
            break;
        case 2:
            return minpt.y;
            break;
        case 3:
            return maxpt.y;
            break;
        case 4:
            return minpt.z;
            break;
        case 5:
            return maxpt.z;
            break;    
        default:
            assert(0);
            break;
    }
    assert(0);
    return -1;
}


} // end namespace
// end of file volume.cpp
