/*  $Id: $
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

//#include "point.h"
#include "clpoint.h"
//#include "triangle.h"
//#include "numeric.h"

namespace ocl
{
    
/* ********************************************** CLPoint *************/


CLPoint::CLPoint() 
    : Point() {
    cc = CCPoint();
}

CLPoint::CLPoint(double x, double y, double z) 
    : Point(x,y,z) {
    cc = CCPoint();
}

CLPoint::CLPoint(double x, double y, double z, CCPoint& ccp) 
    : Point(x,y,z) {
    cc = ccp;
}

int CLPoint::liftZ(double zin, CCPoint& ccp)
{
    if (zin>z) {
        z=zin;
        cc=ccp;
        return 1;
    } else {
        return 0;
    }
}


std::string CLPoint::str() const
{
    std::ostringstream o;
    o << "CL(" << x << ", " << y << ", " << z << ") " << cc ;
    return o.str();
}



} // end namespace
// end file point.cpp
