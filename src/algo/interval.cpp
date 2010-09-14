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

#include "interval.h"

namespace ocl
{

Interval::Interval() {
    lower = 0.0;
    upper = 0.0;
    lower_cc = CCPoint();
    upper_cc = CCPoint();
    in_weave=false;
}

Interval::Interval(const double l, const double u) {
    assert( l <= u );
    lower = l;
    upper = u;
    in_weave=false;
}

void Interval::updateUpper(const double t, CCPoint& p) {
    if (upper_cc.type == NONE) {
        upper = t;
        lower = t;
        upper_cc = p;
        lower_cc = p;
    }
    if ( t > upper ) {
        upper = t;
        upper_cc = p;
    } 
}

void Interval::updateLower(const double t, CCPoint& p) {
    if (lower_cc.type == NONE) {
        lower = t;
        upper = t;
        lower_cc = p;
        upper_cc = p;
    }
    if ( t < lower ) {
        lower = t; 
        lower_cc = p;
    }
}

// return true if *this is completely non-overlapping, or outside of i.
bool Interval::outside(const Interval& i) const {
    if ( this->lower > i.upper )
        return true;
    else if ( this->upper < i.lower )
        return true;
    else
        return false;
}

// return true if *this is contained within i
bool Interval::inside(const Interval& i) const {
    if ( (this->lower > i.lower) && (this->upper < i.upper) )
        return true;
    else
        return false;
}

bool Interval::empty() const {
    if ( lower==0.0  && upper==0.0 )
        return true;
    else
        return false;
}

std::string Interval::str() const {
    std::ostringstream o;
    o << "I ["<< lower <<" , " << upper << " ]";
    return o.str();
}

} // end namespace
// end file interval.cpp
