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

#include <sstream>
#include <cmath>
// uncomment to disable assert() calls
// #define NDEBUG
#include <cassert>

#include "point.h"
#include "epos.h"
#include "oellipse.h"
#include "numeric.h"
namespace ocl
{

//********   Epos ********************** */
Epos::Epos() {
    setD();
}

void Epos::setD() {
    // set (s,t) to angle corresponding to diangle
    // see: http://www.freesteel.co.uk/wpblog/2009/06/encoding-2d-angles-without-trigonometry/
    // return P2( (a < 2 ? 1-a : a-3),
    //           (a < 3 ? ((a > 1) ? 2-a : a) : a-4)
    double d = diangle;
    while ( d > 4.0 ) // make d a diangle in [0,4]
        d -= 4.0;
    while ( d < 0.0)
        d+=4.0;
    assert( d >= 0.0 && d <= 4.0 ); // now we should be in [0,4]
    // std::cout << "Epos::setD(): " << diangle << " mod 4 = " << d << "\n";
    Point p;
    if ( d < 2.0 ) // we are in the y>0 region
        p.x = 1-d; // in quadrant1 and quadrant2
    else
        p.x = d-3; // in q3 and q4
        
    if ( d < 3.0 ) {
        if ( d > 1.0 ) 
            p.y = 2-d; // d=[1,3] in q2 and q3
        else
            p.y = d;  // d=[0,1] in q1
    } else {
        p.y = d - 4; // d=[3,4] in q4
    }
    // now we have a vector pointing in the right direction
    // but it is not normalized
    p.normalize();
    s = p.x;
    t = p.y;
    assert( this->isValid() );
}

// check that s and t values are OK
bool Epos::isValid() const {
    if ( isZero_tol( square(s) + square(t) - 1.0 ) )
        return true;
    else
        return false;
}

Epos& Epos::operator=(const Epos &pos)  {
    s = pos.s;
    t = pos.t;
    diangle = pos.diangle;
    return *this;
}

std::string Epos::str() const {
    std::ostringstream o;
    o << *this;
    return o.str();
}

std::ostream& operator<<(std::ostream &stream, Epos pos) {
    stream << "("<< pos.s <<" ," << pos.t << ")";
    return stream;
}


}//end namespace
//end file epos.cpp
