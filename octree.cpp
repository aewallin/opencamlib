/*  Copyright 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com)
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

// this is mostly a translation to c++ of the earlier c# code
// http://code.google.com/p/monocam/source/browse/trunk/Project2/monocam_console/monocam_console/kdtree.cs

#include <iostream>
#include <stdio.h>
#include <sstream>
#include <math.h>
#include <vector>
#include <algorithm>
#include <boost/python.hpp>
#include <list>

// uncomment to disable assert() calls
// #define NDEBUG
#include <cassert>

#include "cutter.h"
#include "point.h"
#include "triangle.h"
#include "cutter.h"
#include "numeric.h"
#include "octree.h"

//#define DEBUG_BUILD_OCT



//********** Ocode ******************/
int    Ocode::depth = 5;
double Ocode::scale = 5;
Point  Ocode::center = Point(0,0,0);

Ocode::Ocode() {
    code.resize(depth);   
    code.assign(depth, 8);
    color = 0;
    //std::cout << "created :" << *this << "\n"; 
}

Ocode::Ocode(const Ocode &o) {
    code.resize(depth); 
    for (int n=0;n<depth;n++) 
        this->code[n] = o.code[n];
    color = o.color;  
    //std::cout << "copy-constructor created :" << *this; 
}

void Ocode::set_depth(int d)
{
    Ocode::depth = d;
    return;
}

int Ocode::get_depth()
{
    return Ocode::depth;
}

char Ocode::operator[](const int &idx) {
    return this->code[idx];
}

/// assignment
Ocode& Ocode::operator=(const Ocode &rhs) {
    if (this == &rhs)      // Same object?
      return *this; 
    // copy code
    for (int n=0;n<depth;n++) {
        this->code[n] = rhs.code[n];
    }
    this->color = rhs.color;
    
    return *this;
}

/// return center-point corresponding to code
Point Ocode::point() const {
    Point p;
    p = center;
    // navigate to correct point
    for (int n=0; n<depth ; n++) {
        // note: code[n]==8  goes nowhere
        p += ( scale/pow(2,n) )*dir( code[n] );
    }
    return p;
}

/// return eight different corner points of cube
Point Ocode::corner(int idx) {
    Point p;
    p = point(); // navigate to center of cell

    int n= degree();
    // from the center go out to the corner
    p += ( scale*2 /pow(2,n) )*dir( idx );
    return p;
}

int Ocode::degree() const
{
    int n=0;
    while ( (code[n] != 8) && (n<depth) )  // figure out degree of this code
        n++;
    return n;
}

double Ocode::get_scale()
{
    return scale /pow(2,degree()-2 );
}

bool Ocode::expandable()
{
    // scan for an 8
    for (int n=0;n<depth;n++) {
        if (code[n]==8)
            return true;
    }
    return false;
}

bool Ocode::isWhite(OCTVolume* vol) {
    //std::cout << "testing node" << *this<<"\n";
    for (int n=0;n<9;n++) {// loop through all corners, and center
        Point p = corner(n);
        //std::cout << " corner" << p;
        if (vol->isInside( p ) ) {
            //std::cout << "is inside!\n";
            return false;
        }
        //std::cout << "is outside!\n";
    }
    //std::cout << "returnung true!\n";
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
    // degree of o must be smaller than degree of this
    if ( o.degree() < this->degree() ) {
        // and the code matches up to o.degree
        for (int m=0; m<o.degree() ; m++) {
            if ( o.code[m] != this->code[m] )
                return false;
        }
        // then this is contained in o
        return true;
    }
    return false;
}


std::vector<Ocode> Ocode::expand()
{
    std::vector<Ocode> list;
    int deg = degree();
    if ( expandable() ) {
        for (int n=0;n<8;n++) {
            Ocode child;
            child = *this;
            child.code[deg] = n;
            list.push_back(child);
        }
    } else {
        std::cout << "can't expand this node! \n";
    }
    return list;
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


bool Ocode::operator==(const Ocode &o){
    for (int n=0 ; n < depth ; n++ ) {
        if ( !( this->code[n] == o.code[n] ) )
            return false;
    }
    return true;
}

int Ocode::number() const {
    int n=0;
    for (int m=0;m<depth;m++) {
        n += pow(10,depth-m)*code[m];
    }
    return n;
}

bool Ocode::operator<(const Ocode& o1) const{
    return ( this->number() < o1.number() ) ;
}

/// string repr
std::ostream& operator<<(std::ostream &stream, const Ocode &o)
{
    stream << "OCode: " ; 
    
    for (int n = 0; n<Ocode::depth; n++)
        stream << (int)o.code[n];
        
    stream << " center=" << o.point() << " c=" << o.color;
    
    return stream;
}

/// string repr
std::string Ocode::str()
{
    std::ostringstream o;
    o << *this;
    return o.str();
}



//**************** LinOCT ********************/

LinOCT::LinOCT() {
    clist.clear();
}

int LinOCT::size() const {
    return clist.size();
}

void LinOCT::append(Ocode& code) {
    clist.push_back( code );
}

void LinOCT::append_at(Ocode& code, int idx) {
    if ( valid_index(idx) ) {
        clist.insert( clist.begin()+idx, code );
    }
}

void LinOCT::delete_at(int idx) {
    //std::cout << " DELETING " << clist[idx] << "\n";
    if ( valid_index(idx) )
        clist.erase( clist.begin()+idx) ;
    //std::cout << " after=" << size() << "\n";
    
    return;
}

bool LinOCT::valid_index(int idx) {
    if ( (idx >= 0) && (idx < size()) ) 
        return true;
    else {
        std::cout << "index out of range!\n";
        return false;
    }
}

// expand node at index
void LinOCT::expand_at(int idx) {
    if ( valid_index(idx) ) {
        if (clist[idx].expandable()) {
            std::vector<Ocode> newnodes = clist[idx].expand();
            std::vector<Ocode>::iterator it;
            
            
            // delete old node
            delete_at(idx);
            
            // insert new nodes
            int n = 0;
            BOOST_FOREACH( Ocode o, newnodes) {
                it = clist.begin() + idx + n;
                clist.insert(it, o);
                n++;
            }
        }
        
    } else {
        std::cout << "LinOCT::expand() index out of range!\n";
    }
    return;
}

/// initialize octree and expand min_expans times
void LinOCT::init(int min_expand) 
{
    Ocode o = Ocode(); // create an onode, initally all "8"
    append(o);
    
    for (int m=0; m<min_expand ; m++) {
        for (int n=0; n<size() ; n++) {
            if ( clist[n].expandable() ) {
                expand_at(n); // expand the node
                n=n+7; // jump forward, since we have inserted new nodes
            }
        }
    }
    return;
}

// build LinOCT from input OCTVolume
void LinOCT::build(OCTVolume* vol)
{
    
    // loop until no expandable nodes remain
    // deleting white nodes at each step
    // expanding grey nodes if possible
    // and skipping over black nodes (only these remain when done) 
    
    std::cout << size() << " nodes to process\n";
    int n=0;
    while ( n < size() ) { // go through all nodes
        if (  clist[n].isWhite( vol ) ) {
            // white nodes can be deleted
            clist[n].color = 1;
            delete_at(n);
        }
        else if ( clist[n].isGrey( vol ) ) {
            // grey nodes should be expanded, if possible
            if ( clist[n].expandable() ) {
                //std::cout << " expanding\n";
                expand_at(n);
            }
            else {
                // grey non-expandable nodes are removed
                delete_at(n); 
            }
        }
        else {
            // node is black, so leave it in the list
            n++; // move forward in the list
        }
    }
    std::cout << size() << " nodes after process\n";
    //std::cout << *this;
    
}

void LinOCT::condense() {
    // NOTE: list needs to be sorted before we come here.
    
    // remove duplicates
    int n=0;
    while ( n < (size()-1) ) {
        if ( clist[n] == clist[n+1] )
            delete_at(n);
        else
            n++;
    }
    
    // remove nodes which are contained in the following node
    n=0;
    while ( n < (size()-1) ) {
        if ( clist[n].containedIn( clist[n+1] ) ) {
            delete_at(n);
        }
        else
            n++;
    }
    
    // condense nodes if all eight sub-quadrants are present
    n=0;
    while ( n < (size()-7) ) {// at least 8 nodes must remain
        
        if ( can_collapse_at(n) ) { // can collapse the octet
            //std::cout << "collapsing at " << n << "\n";
            int deg = clist[n].degree();
            

            // construct parent node of sub-octants
            Ocode o; // parent node, all digits default to "8"
            for (int m=0;m<(deg-1); m++) {
                o.code[m] =  clist[n].code[m]; // match code up to deg-1
            }
            
            //std::cout << "before collapse at " << n <<" code:" << clist[n] << "\n";            
            // add parent 
            append_at(o, n); 
            //std::cout << "parent insert at " << n <<" code:" << clist[n] << "\n"; 
            n++; // jump forward and delete the redundant sub-octants
            for (int m=0;m<8;m++) {
                //std::cout << " deleting at " << n<< " : " << clist[n] << "\n";
                delete_at(n); 
            }
                
            n--; // jump back to the new parent            
            int jump=7;
            if (n<8)
                jump=n;
            n-=jump; 
                    // jump backward and see if the collapse has created 
                     // an opportunity for more collapsing
                     // collapsable nodes can be as far back as 7 steps
        }        
        else
            n++;
    }
    
    return;
}

bool LinOCT::can_collapse_at(int idx) {
    int deg = clist[idx].degree();
    // check for consequtive numbers 0-7 at position deg
    Ocode o;
    //std::cout << " checking "<< idx << " to " << idx+7 << " deg=" << deg << "\n"; 
    for (int n=0; n < 8 ; n++) {
        o = clist[idx+n];
        //std::cout << "n=" << n << " Ocode= "<< o <<" code=" << (int)o.code[deg-1] << "\n";
        
        if ( (o.code[deg-1] != n) || (o.degree() != deg) ) {// code must match 0-7
            //std::cout << " no match\n";
            return false;
        }
    }
    //std::cout << " Match!!\n";
    return true;
}



/// add nodes of LinOCT other to this
void LinOCT::sum(LinOCT& other) {
    BOOST_FOREACH( Ocode o, other.clist ) {
        append( o );
    }
    // TODO: condense!
    return;
}

/// sort list of ocodes
void LinOCT::sort() {
    
    std::sort( clist.begin(), clist.end() );
}

boost::python::list LinOCT::get_nodes()
{    
    boost::python::list nodelist;
    BOOST_FOREACH( Ocode o, clist) {
            nodelist.append(o);
    }
    return nodelist;
}


/// string repr
std::ostream& operator<<(std::ostream &stream, const LinOCT &l)
{
    stream << "LinOCT: N="<< l.size() ; 
    /*
    BOOST_FOREACH( Ocode o, l.clist ) {
        stream << " " << o << "\n";
    }
    */
    
    return stream;
}

/// string repr
std::string LinOCT::str()
{
    std::ostringstream o;
    o << *this;
    return o.str();
}






//************* Volumes **************/

/// sphere at center
SphereOCTVolume::SphereOCTVolume()
{
    center = Point(2,0,0);
    radius = 3.0;
}


bool SphereOCTVolume::isInside(Point& p) const
{
    
    if ( (center-p).norm() <= radius ) {
        //std::cout << "dist to point=" << (center-p).norm() <<"\n";
        return true;
    }
    else
        return false;
}

/// cube at center with side length side
CubeOCTVolume::CubeOCTVolume()
{
    center = Point(0,0,0);
    side = 1.234;
}

bool CubeOCTVolume::isInside(Point& p) const
{
    bool x,y,z;
    x = ( (p.x >= (center.x-side/2)) && (p.x <= (center.x+side/2)) );
    y = ( (p.y >= (center.y-side/2)) && (p.y <= (center.y+side/2)) );
    z = ( (p.z >= (center.z-side/2)) && (p.z <= (center.z+side/2)) );
    if ( x && y && z )
        return true;
    else
        return false;
}





// end of file octree.cpp
