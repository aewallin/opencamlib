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

#include <boost/foreach.hpp>

#include "fiber.hpp"

namespace ocl
{

Fiber::Fiber(const Point &p1in, const Point &p2in) {
    p1=p1in;
    p2=p2in;
    calcDir();
}

void Fiber::calcDir() {
    dir = p2 - p1;
    assert( dir.z == 0.0 );
    dir.normalize();
}

bool Fiber::contains(Interval& i) const {
    BOOST_FOREACH( Interval fi, ints) {
        if ( i.inside( fi ) )
            return true;
    }
    return false;
}

bool Fiber::missing(Interval& i) const {
    bool result = true;
    BOOST_FOREACH( Interval fi, ints) {
        if ( !i.outside( fi ) ) // all existing ints must be non-overlapping
            result = false; 
    }
    return result;
}

void Fiber::addInterval(Interval& i) {
    if (i.empty())
        return; // do nothing.
    
    if (ints.empty()) { // empty fiber case
        ints.push_back(i); 
        return;
    } else if ( this->contains(i)  ) { // if fiber already contains i  
        return; // do nothing
    } else if ( this->missing(i) ) { // if fiber doesn't contain i 
        ints.push_back(i);
        return;
    } else {
        // this is the messier general case with partial overlap
        std::vector<Interval>::iterator itr;
        itr = ints.begin();
        std::vector<Interval> overlaps;
        while (itr!=ints.end()) { // loop through all intervals
            if ( ! (itr->outside( i )) ) {
                overlaps.push_back(*itr); // add overlaps here
                itr = ints.erase(itr); // erase overlaps from ints
            } else {
                ++itr;
            }
        }
        overlaps.push_back(i);
        // now build a new interval from i and the overlaps
        Interval sumint;        
        BOOST_FOREACH(Interval intr, overlaps) {
            sumint.updateLower( intr.lower, intr.lower_cc );
            sumint.updateUpper( intr.upper, intr.upper_cc );
        }
        ints.push_back(sumint); // add the sum-interval to ints
        return;
    }
}

double Fiber::tval(Point& p) const {
    // fiber is  f = p1 + t * (p2-p1)
    // t = (f-p1).dot(p2-p1) / (p2-p1).dot(p2-p1)
    return  (p-p1).dot(p2-p1) / (p2-p1).dot(p2-p1);
}

Point Fiber::point(double t) const {
    Point p = p1 + t*(p2-p1);
    return p;
}

void Fiber::printInts() const {
    int n=0;
    BOOST_FOREACH( Interval i, ints) {
        std::cout << n << ": [ " << i.lower << " , " << i.upper << " ]" << "\n";
        ++n;
    }
}

std::ostream& operator<<(std::ostream &stream, const Fiber& f) {
  stream << " fiber dir=" << f.dir << " and " << f.ints.size() << " intervals\n"; 
  stream << " fiber.p1=" << f.p1 << " fiber.p2 " << f.p2 ; 
  return stream;
}


} // end namespace
// end file fiber.cpp
