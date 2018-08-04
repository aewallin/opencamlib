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

#include <list>
// uncomment to disable assert() calls
// #define NDEBUG
#include <cassert>

#include <boost/python.hpp>
#include <boost/foreach.hpp>

#include "point.h"
#include "triangle.h"
#include "millingcutter.h"
#include "numeric.h"
#include "ocode.h"
#include "octree.h"

//#define DEBUG_BUILD_OCT

namespace ocl
{


//********** Ocode ******************/
int    Ocode::depth = 5;
double Ocode::scale = 10;
Point  Ocode::center = Point(0,0,0);

Ocode::Ocode() {
    code.resize(depth);   
    code.assign(depth, 8); // fill with all "8"
    crn = std::vector<Point>(9);
}

Ocode::Ocode(const Ocode &o) {
    code.resize(depth); 
    for (int n=0;n<depth;n++) 
        this->code[n] = o.code[n];
    crn = std::vector<Point>(9);
}

void Ocode::init() {
    // this precomputes node properties
    deg = degree();
    cen = point();
    for (int n=0;n<9;n++) {
        crn[n] = corner(n);
    }
    
}

void Ocode::set_depth(int d) {
    Ocode::depth = d;
    return;
}

int Ocode::get_depth() {
    return Ocode::depth;
}

char& Ocode::operator[](const int &idx) {
    return this->code[idx];
}

/// assignment
Ocode& Ocode::operator=(const Ocode &rhs) {
    if (this == &rhs)      // Same object?
      return *this; 
    for (int n=0;n<depth;n++) {
        this->code[n] = rhs.code[n]; // copy code
    }
    return *this;
}

/// return center-point corresponding to code
Point Ocode::point() const {
    Point p = center; // global center, a static variable!
    for (int n=0; n<depth ; n++) { // navigate to correct point
        p += ( scale/pow(2.0,n) )*dir( code[n] );  // note: code[n]==8  goes nowhere
    }
    return p;
}

/// return eight different corner points of cube
Point Ocode::corner(int idx) const {
    Point p;
    p = point();       // navigate to center of cell
    int n= degree();
    p += ( scale*2 /pow(2.0,n) )*dir( idx ); // from the center go out to the corner
    return p;
}

int Ocode::degree() const {
    int n=0;
    while ( (code[n] != 8) && (n<depth) )  // figure out degree of this code
        n++;
    return n;
}

void Ocode::set_scale(double s) {
    Ocode::scale = s;
}

double Ocode::get_scale() {
    return scale /pow(2.0,degree()-2 );
}

bool Ocode::expandable() {
    for (int n=0;n<depth;n++) {
        if (code[n]==8) // scan for an 8
            return true;
    }
    return false; // no 8 found
}

void Ocode::calcScore(OCTVolume* vol) {
    init(); //why??
    int sum=0;             // use 8/9 pts?   why9??
    for (int n=0;n<9;n++) {// loop through all corners, and center
        Point p = crn[n]; //crn[n]; // use precomputed corner
        if (vol->isInside( p ) ) 
            sum +=1;
    }
    score = sum;
}

/// return true if node is white, i.e. contains no points inside volume
bool Ocode::isWhite(OCTVolume* vol) {
    bool bbflag = false;
    for (int n=0;n<9;n++) {// loop through all corners, and center
        Point p = corner(n);
        if (vol->isInside( p ) ) {
            return false;
        }
        if (vol->isInsideBB( p ))
            bbflag = true;
    }
    if ( bbflag ) { // node is inside bounding-box, so do supersampling
        /// \todo FIXME test more points in the node here
    }
    return true; // get here only if all points outside volume
}

/// return true if node is grey, i.e. contains both
/// points inside and outside the volume
bool Ocode::isGrey(OCTVolume* vol) {
    //std::cout << "testing node" << *this<<"\n";
    int sum=0;
    for (int n=0;n<9;n++) {// loop through all corners, and center
        Point p = corner(n);
        if (vol->isInside( p ) ) 
            sum +=1;
        else
            sum -=1;
    }
    if ( (sum>-9) && (sum<9) ) // not white and not black
        return true; 
    else
        return false;
}

bool Ocode::containedIn(const Ocode& o) const {
    if ( o.degree() < this->degree() ) { // degree of o must be smaller than degree of this
        for (int m=0; m<o.degree() ; m++) { // and the code matches up to o.degree
            if ( o.code[m] != this->code[m] )
                return false;
        }
        return true; // then this is contained in o
    }
    return false;
}


std::list<Ocode> Ocode::expand()
{
    std::list<Ocode> list;
    int deg = degree();
    if ( expandable() ) {
        for (int n=0;n<8;n++) {
            Ocode child;
            child = *this; // child is copy of parent
            child.code[deg] = n; // but at position deg, cycle through 0-7
            child.init();
            list.push_back(child);
        }
    } else {
        std::cout << "can't expand this node! \n";
    }
    return list;
}

/// fill Ocode with invalid information
void Ocode::null() {
    for (int n=0;n<depth;n++) {
        code[n]=9; // an invalid Ocode
    }
    return;
}

bool Ocode::isNull() {
    for (int n=0;n<depth;n++) {
        if ( code[n] != 9 ) 
            return false;
    }
    return true;
}

/// return direction of octant
Point Ocode::dir(int idx) const {
    double r = 1.0;
    switch(idx)
    {
        case 0:
            return Point(r,r,r);
            break;
        case 1:
            return Point(-r,r,r);
            break;
        case 2:
            return Point(r,-r,r);
            break;
        case 3:
            return Point(r,r,-r);
            break;
        case 4:
            return Point(r,-r,-r);
            break;
        case 5:
            return Point(-r,r,-r);
            break;
        case 6:
            return Point(-r,-r,r);
            break;
        case 7:
            return Point(-r,-r,-r);
            break;
        case 8:
            return Point(0,0,0);
            break;
        default:
            std::cout << "octree.cpp Ocode:dir() called with invalid id!!\n";
            assert(0);
            break;
    }
    assert(0);
    return Point(0,0,0);
}

/// test each digit of the Ocode for equality
bool Ocode::operator==(const Ocode &o){
    for (int n=0 ; n < depth ; n++ ) {
        if ( !( this->code[n] == o.code[n] ) )
            return false;
    }
    return true;
}


/*
 * /// return corresponding number
unsigned long Ocode::number() const {
    unsigned long n=0;
    for (int m=0;m<depth;m++) {
        n += (unsigned long)(pow(10.0,depth-m)*code[m]);
    }
    assert( n > 0 );
    return n;
}*/

bool Ocode::operator<(const Ocode& o) const {
    for (int m=0;m<depth;++m) {
        if ( this->code[m] < o.code[m] ) 
            return true;
        else if ( this->code[m] > o.code[m] )
            return false;
    }
    return false; // we come here if codes equal
    
    // the old way to do this is below
    // this is problematic because number() is very large with deep trees
    // return ( this->number() < o.number() ) ;
}

/// string repr
std::ostream& operator<<(std::ostream &stream, const Ocode &o)
{   
    for (int n = 0; n<Ocode::depth; n++)
        stream << (int)o.code[n];
    return stream;
}

/// string repr
std::string Ocode::str() {
    std::ostringstream o;
    o << *this;
    return o.str();
}




} // end namespace
// end of file ocode.cpp
