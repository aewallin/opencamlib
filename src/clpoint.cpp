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

#include <string>
#include <iostream>
#include <sstream>

#include "clpoint.h"

namespace ocl
{
    
/* ********************************************** CLPoint *************/


CLPoint::CLPoint() 
    : Point() {
    // delete cc;
    cc = new CCPoint();
}

CLPoint::CLPoint(double x, double y, double z) 
    : Point(x,y,z) {
    // delete cc;
    cc = new CCPoint();
}

CLPoint::CLPoint(double x, double y, double z, CCPoint& ccp) 
    : Point(x,y,z) {
    // delete cc;
    cc = new CCPoint( ccp );
}


CLPoint::CLPoint(const CLPoint& cl) 
    : Point(cl.x,cl.y,cl.z) {
    // delete cc;
    cc = new CCPoint( *cl.cc );
}



CLPoint::~CLPoint() {
   delete cc;
}

bool CLPoint::below(const Triangle& t) const {
    if (z < t.bb.maxpt.z )
        return true;
    else
        return false;
}

bool CLPoint::liftZ(const double zin)
{
    if (zin>z) {
        z=zin;
        return true;
    } else {
        return false;
    }
}

int CLPoint::liftZ(double zin, CCPoint& ccp)
{
    if (zin>z) {
        z=zin;
        cc=&ccp;
        return 1;
    } else {
        return 0;
    }
}

CLPoint& CLPoint::operator=(const CLPoint &clp) {
    // FIXME check for self-assignment
    x=clp.x;
    y=clp.y;
    z=clp.z;
    cc= clp.cc;
    return *this;
}

const CLPoint CLPoint::operator+(const CLPoint &p) const {
    return CLPoint(this->x + p.x, this->y + p.y, this->z + p.z);
}

const CLPoint CLPoint::operator+(const Point &p) const {
    return CLPoint(this->x + p.x, this->y + p.y, this->z + p.z);
}

CCPoint CLPoint::getCC() {
    return *cc;
}

std::string CLPoint::str() const
{
    std::ostringstream o;
    o << "CL(" << x << ", " << y << ", " << z << ") " << cc ;
    return o.str();
}



} // end namespace
// end file point.cpp
