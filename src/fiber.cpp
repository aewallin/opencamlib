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

#include <boost/foreach.hpp>
#include <boost/python.hpp>

#include "fiber.h"


namespace ocl
{

Interval::Interval() {
    lower = 0.0;
    upper = 0.0;
    lower_cc = CCPoint();
    upper_cc = CCPoint();
    in_weave=false;
}

Interval::Interval(double l, double u) {
    assert( l <= u );
    lower = l;
    upper = u;
    in_weave=false;
}

Interval::~Interval() {
    return;
}

void Interval::updateUpper(double t, CCPoint& p) {
    if (upper_cc.type == NONE) {
        //std::cout << " I: updateUpper() NONE up=lo=" << t << "\n";
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

void Interval::updateLower(double t, CCPoint& p) {
    if (lower_cc.type == NONE) {
        //std::cout << " I: updateLower() NONE up=lo=" << t << "\n";
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

/// return true if *this is completely non-overlapping, or outside of i.
bool Interval::outside(const Interval& i) const {
    if ( this->lower > i.upper )
        return true;
    else if ( this->upper < i.lower )
        return true;
    else
        return false;
}

/// return true if *this is contained within i
bool Interval::inside(const Interval& i) const {
    if ( (this->lower > i.lower) && (this->upper < i.upper) )
        return true;
    else
        return false;
}

bool Interval::empty() const {
    if ( isZero_tol(lower)  && isZero_tol(upper) )
        return true;
    else
        return false;
}

/// return a point on the interval at given t-value
Point Interval::point(double t) const {
    Point p = p1 + t*(p2-p1);
    return p;
}

std::string Interval::str() const {
    std::ostringstream o;
    o << "I ["<< lower <<" , " << upper << " ]";
    return o.str();
}

/* ********************** Fiber ***************************************/
/* ********************** Fiber ***************************************/
/* ********************** Fiber ***************************************/


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

/// return true if some Interval in this Fiber contains i
bool Fiber::contains(Interval& i) const {
    BOOST_FOREACH( Interval fi, ints) {
        if ( i.inside( fi ) )
            return true;
    }
    return false;
}

/// return true if Fiber is completely missing Invterval i
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
    
    i.p1=p1;
    i.p2=p2;
    
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



/// return t-value correspoinding to p
double Fiber::tval(Point& p) const {
    // fiber is  f = p1 + t * (p2-p1)
    // t = (f-p1).dot(p2-p1) / (p2-p1).dot(p2-p1)
    return  (p-p1).dot(p2-p1) / (p2-p1).dot(p2-p1);
}

/// return a point on the fiber at given t-value
Point Fiber::point(double t) const {
    Point p = p1 + t*(p2-p1);
    return p;
}

/// return intervals as a list to python
boost::python::list Fiber::getInts() const {
    boost::python::list l;
    BOOST_FOREACH( Interval i, ints) {
        l.append( i );
    }
    return l;
}

/// print out all intervals
void Fiber::printInts() const {
    int n=0;
    BOOST_FOREACH( Interval i, ints) {
        std::cout << n << ": [ " << i.lower << " , " << i.upper << " ]" << "\n";
        ++n;
    }
}

/// string repr of Fiber
std::ostream& operator<<(std::ostream &stream, const Fiber& f) {
  stream << " fiber dir=" << f.dir << " and " << f.ints.size() << " intervals "; 
  return stream;
}


} // end namespace
// end file fiber.cpp
