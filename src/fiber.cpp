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
#include <boost/numeric/interval.hpp>
#include <boost/foreach.hpp>
#include <boost/python.hpp>

#include "fiber.h"
namespace bn = boost::numeric;

namespace ocl
{


//Interval

Interval::Interval()
{
    i = dinterval(0,0);
}

Interval::Interval(double l, double u)
{
    i = dinterval(l,u);
}

Interval::~Interval()
{
    return;
}

double Interval::upper() const {
    return i.upper();
}
double Interval::lower() const {
    return i.lower();
}


std::string Interval::str() const {
    std::ostringstream o;
    o << "I ["<< lower() <<" , " << upper() << " ]";
    return o.str();
}


/*********************** Fiber ****************************************/

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

void Fiber::addInt(double t1, double t2) {
    ints.push_back( dinterval(t1,t2) );
}

void Fiber::condense() {
    if ( ints.size() > 1) {
        for (unsigned int n=0; n< (ints.size()-1); ++n) {
            for (unsigned int m = (n+1); m<ints.size() ; ++m) {
                std::cout << "before: \n";
                printInts();
                std::cout << "testing " << n << " and " << m ; 
                if ( bn::subset( ints[m], ints[n]  ) ) { // if m contained in n, remove m
                    std::cout << " subset, deleting " << m << "\n";
                    ints.erase(ints.begin()+m);
                }
                else if ( bn::overlap( ints[n], ints[m] ) ) { // if there is overlap, replace with hull
                    std::cout << " overlap " << "\n";
                    bn::interval<double> h = bn::hull( ints[n] ,  ints[m] );
                    ints.erase(ints.begin()+n);
                    ints.erase(ints.begin()+m-1);
                    ints.insert(ints.begin()+n, h);
                } else {
                    std::cout << " none " << "\n";
                }
                std::cout << "after: \n";
                printInts();
            }
        }
    }
}


/// return t-value correspoinding to p
double Fiber::tval(Point& p) const {
    // fiber is  f = p1 + t * (p2-p1)
    // t = (f-p1).dot(p2-p1) / (p2-p1).dot(p2-p1)
    double t = (p-p1).dot(p2-p1) / (p2-p1).dot(p2-p1);
    return t;
}


/// return a point on the fiber
Point Fiber::point(double t) const {
    Point p = p1 + t*(p2-p1);
    return p;
}

boost::python::list Fiber::getInts() {
    boost::python::list l;
    BOOST_FOREACH( bn::interval<double> i, ints) {
        boost::python::list interval;
        interval.append( i.lower() );
        interval.append( i.upper() );
        l.append( interval );
    }
    return l;
}

void Fiber::printInts() {
    int n=0;
    BOOST_FOREACH( bn::interval<double> i, ints) {
        std::cout << n << ": [ " << i.lower() << " , " << i.upper() << " ]" << "\n";
        ++n;
    }
}



} // end namespace
// end file fiber.cpp
