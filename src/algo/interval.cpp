/*  $Id$
 * 
 *  Copyright (c) 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com).
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

#include <iostream>
#include <sstream>
#include <string>

#include "interval.hpp"

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

void Interval::update(const double t, CCPoint& p) {
    this->updateUpper(t,p);
    this->updateLower(t,p);
}

bool Interval::update_ifCCinEdgeAndTrue( double t_cl, CCPoint& cc_tmp, const Point& p1, const Point& p2, bool condition) {
    //if ( cc_tmp.isInsidePoints(p1,p2)  && condition ) {
    if ( cc_tmp.isInside(p1,p2)  && condition ) {
        update( t_cl  , cc_tmp ); // both updateUpper(t,p) and updateLower(t,p)
        return true;  
    } else {
        return false;
    }
}

void Interval::updateUpper(const double t, CCPoint& p) {
    if (upper_cc.type == NONE) {
        upper = t;
        lower = t;
        CCPoint* tmp = new CCPoint(p);
        upper_cc = *tmp;
        lower_cc = *tmp;
        delete tmp;
    }
    if ( t > upper ) {
        upper = t;
        CCPoint* tmp = new CCPoint(p);
        upper_cc = *tmp;
        delete tmp;
    } 
}

void Interval::updateLower(const double t, CCPoint& p) {
    if (lower_cc.type == NONE) {
        lower = t;
        upper = t;
        CCPoint* tmp = new CCPoint(p);
        lower_cc = *tmp;
        upper_cc = *tmp;
        delete tmp;
    }
    if ( t < lower ) {
        lower = t; 
        CCPoint* tmp = new CCPoint(p);
        lower_cc = *tmp;
        delete tmp;
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
