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
double Ocode::scale = 10;
Point  Ocode::center = Point(0,0,0);

Ocode::Ocode() {
    code.resize(depth);   
    code.assign(depth, 8);
}

Ocode::Ocode(const Ocode &o) {
    code.resize(depth); 
    for (int n=0;n<depth;n++) 
        this->code[n] = o.code[n];
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
    //this->color = rhs.color;
    
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

void Ocode::set_scale(double s)
{
    Ocode::scale = s;
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


std::list<Ocode> Ocode::expand()
{
    std::list<Ocode> list;
    int deg = degree();
    if ( expandable() ) {
        for (int n=0;n<8;n++) {
            Ocode child;
            child = *this; // child is copy of parent
            child.code[deg] = n; // but at position deg, cycle through 0-7
            list.push_back(child);
        }
    } else {
        std::cout << "can't expand this node! \n";
    }
    return list;
}

/// fill Ocode with invalid information
void Ocode::null()
{
    for (int n=0;n<depth;n++) {
        code[n]=9; // an invalid Ocode
    }
    return;
}

bool Ocode::isNull()
{
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


bool Ocode::operator==(const Ocode &o){
    for (int n=0 ; n < depth ; n++ ) {
        if ( !( this->code[n] == o.code[n] ) )
            return false;
    }
    return true;
}

unsigned long Ocode::number() const {
    unsigned long n=0;
    for (int m=0;m<depth;m++) {
        n += pow(10,depth-m)*code[m];
    }
    assert( n > 0 );
    return n;
}

bool Ocode::operator<(const Ocode& o1) const{
    return ( this->number() < o1.number() ) ;
}

/// string repr
std::ostream& operator<<(std::ostream &stream, const Ocode &o)
{
    //stream << "OCode: " ; 
    
    for (int n = 0; n<Ocode::depth; n++)
        stream << (int)o.code[n];
        
    //stream << " center=" << o.point() ;
    
    return stream;
}

/// string repr
std::string Ocode::str()
{
    std::ostringstream o;
    o << *this;
    return o.str();
}
//**************** end of Ocode ********************/





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

void LinOCT::delete_at(std::list<Ocode>::iterator& it) { 
    clist.erase( it );
}


void LinOCT::expand_at(std::list<Ocode>::iterator& itr) {
    // note: there was a valid-index check here
    // consider checking that itr is valid?
        if ( itr->expandable()) {  // was: clist[idx].expandable()
            std::list<Ocode> newnodes = itr->expand(); // the new nodes  clist[idx]                
            BOOST_FOREACH( Ocode o, newnodes) {
                clist.insert(itr, o);
            }

            // delete old node
            std::list<Ocode>::iterator temp;
            temp = itr;
            itr--; // jump out of the way from erase()
            clist.erase(temp);
        } else {
            std::cout << "LinOCT::expandcannot expand " << *itr << "!\n";
        }
    return;
}

/// initialize octree and expand min_expans times
void LinOCT::init(int min_expand) 
{
    // assume the list is empty.
    if (size() > 0) {
        std::cout << "cannot call LinOCT::init() on non-empty tree! \n";
        assert(0);
    }
    
    Ocode o = Ocode(); // create an onode, initally all "8"
    append(o);
    
    for (int m=0; m<min_expand ; m++) { // go through the list min_expand times
        
        std::list<Ocode>::iterator itr;
        std::list<Ocode>::iterator current_end;
        itr = clist.begin();
        current_end = clist.end();        
        for(int n=0; n<size() ; n++) {
            if ( itr->expandable() ) { // if expandable
                //std::cout << " init() expanding " << *itr << "\n";
                expand_at(itr); // expand the node
                //std::cout << " after expand itr= " << *itr << "\n";
                n=n+7; // jump forward, since we have inserted new nodes
                
                itr++; // after expand(), itr points to the last expanded node
                       // so jump forward to expand next node
                if ( itr == clist.end() )
                    itr--; // unless at the end of list
            }
        }
        std::cout << " LinOCT:init() m=" << m << " N=" << size() << "\n";
        
    }
    return;
}

/// build LinOCT octree from input volume OCTVolume
void LinOCT::build(OCTVolume* vol)
{
    // loop through the whole list:
    // - deleting white nodes 
    // - expanding grey nodes if possible
    // - skipping over black nodes (only these remain when done) 
    std::cout << size() << " nodes before build()\n";
    std::list<Ocode>::iterator it;
    std::list<Ocode>::iterator temp;
    it = clist.begin();

    while ( it != clist.end()  ) { 
        //std::cout << n << "build()  it: " << *it << "\n";
        
        if (  ! (vol->isInsideBBo( *it )) ) { // nodes outside bounding-box can be deleted
            temp = it;
            if ( it != clist.begin() ) 
                it--; // jump out of the way from erase()
            else
                it++;
            //std::cout << "outside BBox-deleting!\n";
            clist.erase(temp);
        }
        else if (  it->isWhite( vol ) ) { // white nodes can be deleted
            //std::cout << n << "before delete idx" << clist[n] << "\n";
            //std::cout << n << "before delete it:" << *it << "\n";
            //std::cout << "white node, deleting\n";
            temp = it;
            if ( it != clist.begin() ) 
                it--; // jump out of the way from erase()
            else
                it++;
            clist.erase(temp);
        }
        else if ( it->isGrey( vol ) ) {
            // grey nodes should be expanded, if possible
            if ( it->expandable() ) {
                //std::cout << "expand_at( " << n << " )\n";
                //printList();
                //std::cout << n << " before expand it: " << *it << "\n";
                temp = it;
                bool first=false; 
                if (it == clist.begin()) {
                    first = true;
                } else {
                    temp--;
                }
                //std::cout << n << " before expand temp: " << *temp << "\n";
                expand_at(it); // iterator moves to last expanded node.
                
                // so need to reset iterator to first expanded node
                if (first)
                    it = clist.begin();
                else
                    it = temp;
                
                // storing the iterator in temp (code above)
                // saves us from this jumping (but doesn't save much running-time)
                // jump back seven steps
                //for (int j=0;j<7;j++)
                //    it--;
                //it--;
                
                // for debugging, print the whole list             
                //printList();
   
                
                                
            }
            else {
                // grey non-expandable nodes are removed
                //std::cout << "grey non-exp delete_at()" << n << "\n";
                temp = it;
                if ( it != clist.begin() ) 
                    it--; // jump out of the way from erase()
                else
                    it++;
                clist.erase(temp);
            }
        } 
        
        else {
            // node is black, so leave it in the list
            // and move forward in the list
            it++;
        }
    }
    std::cout << size() << " nodes after build()\n";  
}

// NOTE: condense() seems to run very slowly 
// >60s run time on length=138000 list.
void LinOCT::condense() {
    // NOTE: list needs to be sorted before we come here.  
    // NOTE: consider using std::list<> unique() to remove duplicates
    int n=0;
    int n_duplicates=0;
    int n_contained=0;
    int n_collapse=0;
    std::list<Ocode>::iterator itr;
    std::list<Ocode>::iterator next;
    itr = clist.begin();
    
    while ( n < (size()-1) ) {
        next=itr;
        next++;
        if ( (*itr) == (*next) ) { //( clist[n] == clist[n+1] ) { // remove duplicates
        
            // FIXME delete_at(n); 
            n_duplicates++;
            
            // deleting a duplicate creates an opportunity to collapse
            // so need to jump back by 7 steps to check for collapse
            int jump=7;
            if (n<8)
                jump=n;
            n-=jump; 
        }
        else if ( itr->containedIn( *next ) ) {
            // remove nodes contained in the following node
            // FIXME delete_at(n);
            if (n>0)
                n--; // jump back to check for more contained nodes 
            n_contained++;
        }
        // condense nodes if all eight sub-quadrants are present
        else if ( can_collapse_at(n) ) { // can collapse the octet
            //std::cout << "collapsing at " << n << "\n";
            int deg = itr->degree();
            

            // construct parent node of sub-octants
            Ocode o; // parent node, all digits default to "8"
            for (int m=0;m<(deg-1); m++) {
                o.code[m] =  itr->code[m]; // match code up to deg-1
            }
            
            //std::cout << "before collapse at " << n <<" code:" << clist[n] << "\n";            
            // add parent 
            
            // FIXME append_at(o, n); 
            
            //std::cout << "parent insert at " << n <<" code:" << clist[n] << "\n"; 
            n++; // jump forward and delete the redundant sub-octants
            
            for (int m=0;m<8;m++) {
                //std::cout << " deleting at " << n<< " : " << clist[n] << "\n";
                // FIXME delete_at(n); 
            }
                
            n--; // jump back to the new parent            
            int jump=7;
            if (n<8)
                jump=n;
            n-=jump; 
                    // jump backward and see if the collapse has created 
                     // an opportunity for more collapsing
                     // collapsable nodes can be as far back as 7 steps
            n_collapse++;
        }        
        else {
            n++; // move forward in list
            itr++;
        }
    }
    if ( (n_duplicates>0) || (n_contained>0) || (n_collapse>0)) {
        std::cout << "n_duplicates="<<n_duplicates<<"\n";
        std::cout << "n_contained="<<n_contained<<"\n";
        std::cout << "n_collapse="<<n_collapse<<"\n";
    } else {
        std::cout << "condense(): nothing to do!\n";
    }
    return;
}

/// return true if eight consequtive 
/// nodes beginning at idx can be collapsed
bool LinOCT::can_collapse_at(int idx) {
    std::list<Ocode>::iterator it;  
    it=clist.begin();
    for (int n=0;n<idx;n++)
        it++;
    
    if ( (size()-idx) < 8 ) // at least 8 nodes must remain
        return false;
        
    int deg = it->degree();
    // check for consequtive numbers 0-7 at position deg
    Ocode o;
    //std::cout << " checking "<< idx << " to " << idx+7 << " deg=" << deg << "\n"; 
    for (int n=0; n < 8 ; n++) {
        o = *it; // clist[idx+n];
        //std::cout << "n=" << n << " Ocode= "<< o <<" code=" << (int)o.code[deg-1] << "\n";
        
        if ( (o.code[deg-1] != n) || (o.degree() != deg) ) {// code must match 0-7
            //std::cout << " no match\n";
            return false;
        }
        it++;
    }
    return true;
}

/// remove o from this
void LinOCT::diff( LinOCT& o ) 
{
    std::list<Ocode>::iterator itr1;  
    std::list<Ocode>::iterator temp;  
    std::list<Ocode>::iterator itr2; 
    std::vector<Ocode> Q12;
    
    itr1=clist.begin();
    itr2=o.clist.begin(); 
    Ocode Hold12;
    Hold12.null();
    
    while ( (itr1 != clist.end() ) && ( itr2 != o.clist.end() )   ) {
        if  ( *itr1 == *itr2 ) {
            // remove from 1
            temp = itr1;
            itr1++;
            clist.erase(temp);
            itr2++;
        }
        else if ( itr1->containedIn( *itr2 ) ) {
            temp = itr1;
            itr1++;
            clist.erase(temp);
        }
        else if ( itr2->containedIn( *itr1 ) ) { // case 2
            expand_at(itr1);
            // need to jump back 7 steps
            for (int m=0;m<7;m++)
                itr1--;
        }
        else if ( *itr1 < *itr2 ) {  // case 3
            itr1++;
        }
        else { // case 4:  o2 < o1
            itr2++;
        }
    } // end while-loop
    
    return;
}


/// compute difference, i.e.
/// remove nodes in other from this
LinOCT LinOCT::operation(int type, LinOCT& o)
{
    // traverse through both lists
    int idx1 = 0;
    int idx2 = 0;
    std::list<Ocode>::iterator itr1;  
    std::list<Ocode>::iterator itr2; 
    itr1=clist.begin();
    itr2=o.clist.begin();
    
    
    std::vector<Ocode> intersection;
    std::vector<Ocode> sum; // a.k.a. union
    std::vector<Ocode> diff12;
    std::vector<Ocode> diff21;
    std::list<Ocode> Q21;
    std::list<Ocode> Q12;
    Ocode Hold21;
    Ocode Hold12;
    Hold21.null();
    Hold12.null();
    
    while ( (idx1<size()) && (idx2<o.size())   ) {  
        
        // case 0
        if  ( *itr1 == *itr2 ) { //(clist[idx1] == o.clist[idx2]) { // identical nodes
            intersection.push_back( *itr1 ); //clist[idx1] );
            sum.push_back( *itr1 ); //clist[idx1] );
            idx1++;
            idx2++;  
            itr1++;
            itr2++;
        }
        
        else if ( itr1->containedIn( *itr2 ) ) { //clist[idx1].containedIn( o.clist[idx2]  )  ) {  // case 1
            intersection.push_back( *itr1 ); //clist[idx1] ); // idx1 contained is in both o1 and o2
            if ( Hold21.isNull() )
                Hold21 = *itr2; // o.clist[idx2];   // remember this node for later processing
            Q21.push_back( *itr1 ); //clist[idx1] ); // these need to be removed from Hold21 later
            idx1++; 
            itr1++;
        }
        
        else if ( itr2->containedIn( *itr1 ) ) { //o.clist[idx2].containedIn( clist[idx1] ) ) { // case 2
            intersection.push_back( *itr2 ); //o.clist[idx2] ); // o2[idx2] is in both o1 and o2
            if ( Hold12.isNull() )
                Hold12 = *itr1; //clist[idx1];        // store for later processing
            Q12.push_back( *itr2 ); //o.clist[idx2] );  // remove these later from Hold12
            idx2++;
            itr2++;
        }
        
        
        else if ( *itr1 < *itr2 ) { // clist[idx1] < o.clist[idx2] ) { // case 3
            // add o1 element to union
            sum.push_back( *itr1 ); //clist[idx1] );
            
            // process the difference queues, if any
            if ( Hold12 == *itr1 ) { //clist[idx1] )  { //compute difference o1-o2  Hold12 == clist[idx1]
                do_diff( Hold12, Q12, diff12 ); // function for calculating difference
                Hold12.null();
            }
            else
                diff12.push_back( *itr1 ); //clist[idx1] );  // no matching node in o2, so o1 belongs to diff
            idx1++;
            itr1++;
        }
        else { // case 4:  o2 < o1
            if ( !( *itr2 < *itr1 ) ) { //o.clist[idx2] < clist[idx1]) ) {
                std::cout << " case 4 o2 < o1 not true!\n";
                std::cout << "o2=" << *itr2 << "number=" << itr2->number() <<  "\n";
                std::cout << "o1=" << *itr1 << "number=" << itr1->number() << "\n";
                assert(0);
            }
                
            // add o2 element to union
            sum.push_back( *itr2 ); //o.clist[idx2] );
            
            if ( Hold21 == *itr2 ) { //o.clist[idx2] ) { // Hold21 == o.clist[idx2]
                do_diff( Hold21, Q21, diff21);
                Hold21.null();
            }
            else
                diff21.push_back( *itr2 ); //o.clist[idx2] ); // o2 belongs to diff21
            idx2++;
            itr2++;
        }
        
    } // end of while-loop through elements
    
    // now process remaining elements, i.e. case where o1 is longer than o2 or vice versa
    if (idx1 < size()) {// process rest of o1
        int idx3 = idx1;
        std::list<Ocode>::iterator itr3;
        itr3 = itr1;
        if ( Hold12 == *itr1 ) { //clist[idx1] ) {
            do_diff( Hold12, Q12, diff12);
            Hold12.null();
            idx3++;
            itr3++;
        }
        //for (int i=idx3; i<size(); i++)
        for ( ; itr3 != clist.end() ; itr3++ )
            diff12.push_back( *itr3 );
            //diff12.push_back( clist[i] ); // o1 elements not in o2 are in diff12
            
        //union calc here
        //for (int i=idx1; i<size();i++)
        for ( ; itr1 != clist.end(); itr1++)
            sum.push_back( *itr1 );
            
    }
    else { // process rest of o2
        int idx3=idx2;
        std::list<Ocode>::iterator itr3;
        itr3 = itr2;
        
        if (Hold21 == *itr2 ) {
            do_diff(Hold21, Q21, diff21);
            Hold21.null();
            idx3++;
            itr3++;
        }
        for (; itr3 != o.clist.end() ; itr3++) {
            diff21.push_back( *itr3 ); // o2 elements to diff21
        }
        
        // union calc here
        for (; itr2 != o.clist.end(); itr2++)
            sum.push_back( *itr2 );
        
    }
    
    /*
    std::cout << " diff12= " << diff12.size() << "\n";
    std::cout << " diff21= " << diff21.size() << "\n";
    std::cout << " inters= " << intersection.size() << "\n";
    */
    
    LinOCT result;
    
    if (type==1) {
        BOOST_FOREACH( Ocode o, diff12 ) {
            result.append(o);
        }
    }
    else if (type==2) {
        BOOST_FOREACH( Ocode o, diff21 ) {
            result.append(o);
        }
    } else if (type==3) {
        BOOST_FOREACH( Ocode o, intersection) {
            result.append(o);
        }
    } else if (type==4) {
        BOOST_FOREACH( Ocode o, sum) {
            result.append(o);
        }
        result.condense();
    }
        
    
    return result;
}


// computes difference H - Q
// where H is a node that is expanded
// and Q is a queue of nodes to be subtracted from H
// the result is appended to D
void LinOCT::do_diff(Ocode& H, std::list<Ocode>& Q, std::vector<Ocode>& D) 
{
    // H - an expandable node
    // Q - queue of nodes contained in H
    // D - difference queue for H-Q results
    
    // Q2 contains expanded node
    std::list<Ocode> Q2;
    
    if ( !H.expandable()) {
        /*
        std::cout << " do_diff node H not expandable...\n";
        std::cout << " H=" << H << "\n";
        std::cout << " Q=\n";
        BOOST_FOREACH( Ocode o, Q) {
            std::cout << o << "\n";
        }*/
        Q2.push_back(H);
    } else {
        Q2 = H.expand(); 
    }
    
    /*
    std::cout << " H.expand() on H=" << H <<" :\n";
    BOOST_FOREACH( Ocode o, Q2) {
        std::cout << o << "\n";
    }*/
    
    while ( !Q2.empty() ) { // go through the expanded nodes
    
        if ( !Q.empty() ) {
            Ocode n = Q.back();
            Ocode n2 = Q2.back();
            if ( n == n2 ) {// matching elements
                Q2.pop_back(); //erase(Q2.begin());  // nothing to put into diff
                Q.pop_back(); // erase(Q.begin());
            } else if ( n.containedIn( n2 ) ) {// need to expand further
                // expand n2 and add to front of Q2
                std::list<Ocode> subocts = n2.expand();
                Q2.pop_back(); // delete parent
                BOOST_FOREACH( Ocode o, subocts) {
                    Q2.push_back(o); // insert new children
                }
            } else {
                // no match in Q, so push node to diff
                D.push_back( n2 );
                Q2.pop_back();
            }
        }
        else { // Q is empty
            D.push_back( Q2.back() ); // no match in Q, so push node to diff
            Q2.pop_back();
        }
        
    }// end while
    //Q.clear();
    
    //std::cout << " after do_diff Q.size() = " << Q.size() << "\n";
    return;
}

/// add nodes of two trees together into one
void LinOCT::sum(LinOCT& other) {
    // join the two lists, sort, and condense
    BOOST_FOREACH( Ocode o, other.clist ) {
        append( o );
    }
    sort();
    condense();
    return;
}

/// sort list of ocodes
void LinOCT::sort() {
    clist.sort();
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
    return stream;
}

/// print the whole list
void LinOCT::printList() {
    BOOST_FOREACH( Ocode o, clist ) {
        std::cout << " " << o << "\n";
    }
}

/// string repr
std::string LinOCT::str()
{
    std::ostringstream o;
    o << *this;
    return o.str();
}

// end of file octree.cpp
