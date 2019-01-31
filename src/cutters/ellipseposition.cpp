/*  $Id$
 * 
 *  Copyright (c) 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com).
 *  
 *  This file is part of OpenCAMlib 
 *  (see https://github.com/aewallin/opencamlib).
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <sstream>
#include <cmath>
// uncomment to disable assert() calls
// #define NDEBUG
#include <cassert>

#include "point.hpp"
#include "ellipseposition.hpp"
#include "ellipse.hpp"
#include "numeric.hpp"
namespace ocl
{

//********   EllipsePosition ********************** */
EllipsePosition::EllipsePosition() {
    diangle = 0.0;
    setD();
}

void EllipsePosition::setDiangle(double dia) {
    assert( !std::isnan(dia) );
    diangle = dia;
    setD();
}

void EllipsePosition::setD() {
    // set (s,t) to angle corresponding to diangle
    // see: http://www.freesteel.co.uk/wpblog/2009/06/encoding-2d-angles-without-trigonometry/
    // see: http://www.anderswallin.net/2010/07/radians-vs-diamondangle/
    // return P2( (a < 2 ? 1-a : a-3),
    //           (a < 3 ? ((a > 1) ? 2-a : a) : a-4)
    double d = diangle;
    assert( !std::isnan(d) );
    while ( d > 4.0 ) // make d a diangle in [0,4]
        d -= 4.0;
    while ( d < 0.0)
        d+=4.0;
        
    assert( d >= 0.0 && d <= 4.0 ); // now we should be in [0,4]
    Point p( (d < 2 ? 1-d : d-3) ,
             (d < 3 ? ((d > 1) ? 2-d : d) : d-4) );

    // now we have a vector pointing in the right direction
    // but it is not normalized
    p.normalize();
    s = p.x;
    t = p.y;
    assert( this->isValid() );
}

// check that s and t values are OK
bool EllipsePosition::isValid() const {
    if ( isZero_tol( square(s) + square(t) - 1.0 ) )
        return true;
    else {
        std::cout << " EllipsePosition=" << *this << "\n";
        std::cout << " square(s) + square(t) - 1.0 = " << square(s) + square(t) - 1.0 << " !!\n";
        return false;
    }
}

EllipsePosition& EllipsePosition::operator=(const EllipsePosition &pos)  {
    s = pos.s;
    t = pos.t;
    diangle = pos.diangle;
    return *this;
}

std::string EllipsePosition::str() const {
    std::ostringstream o;
    o << *this;
    return o.str();
}

std::ostream& operator<<(std::ostream &stream, EllipsePosition pos) {
    stream << "("<< pos.s <<" ," << pos.t << ")";
    return stream;
}

}//end namespace
//end file ellipseposition.cpp
