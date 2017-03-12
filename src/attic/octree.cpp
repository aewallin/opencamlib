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
#include "octree.h"
#include "ocode.h"

//#define DEBUG_BUILD_OCT

namespace ocl
{




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
       // if ( itr->expandable()) {  // was: clist[idx].expandable()
            std::list<Ocode> newnodes = itr->expand(); // the new nodes  clist[idx]                
            BOOST_FOREACH( Ocode o, newnodes) {
                clist.insert(itr, o);
            }

            // delete old node
            std::list<Ocode>::iterator temp;
            temp = itr;
            itr--; // jump out of the way from erase()
            clist.erase(temp);
        /*} else {
            std::cout << "LinOCT::expand_at() cannot expand " << *itr << "!\n";
        }*/
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
    
    Ocode o = Ocode(); // create an onode, initially all "8"
    o.init();
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
        // std::cout << " LinOCT:init() m=" << m << " N=" << size() << "\n";
    }
    return;
}

/// build LinOCT octree from input volume OCTVolume
void LinOCT::build(OCTVolume* vol)
{   // loop through the whole list:
    // - deleting white nodes 
    // - expanding grey nodes if possible
    // - skipping over black nodes (only these remain when done) 
    // std::cout << size() << " nodes before build()\n";
    std::list<Ocode>::iterator it;
    std::list<Ocode>::iterator temp;
    it = clist.begin();
    int calc_calls = 0;
    while ( it != clist.end()  ) { 
        if (  ! (vol->isInsideBBo( *it )) ) { // nodes outside bounding-box can be deleted
            temp = it;
            if ( it != clist.begin() ) 
                it--; // jump back out of the way from erase()
            else
                it++;
            clist.erase(temp);
        }
        else {  // this ocode contains the bounding-box
            it->calcScore( vol ); // expensive call...
            ++calc_calls;
            if ( (it->score == 9) ) { // black node
                it++; // node is black, so leave it in the list, and move forward
            } else if ( (it->score == 0) && (it->deg > 5) ) { 
                temp = it;
                if ( it != clist.begin() ) 
                    it--; // jump out of the way from erase()
                else
                    it++;
                clist.erase(temp); // white node, delete.
            }
            else { // grey node, expand if possible, otherwise delete
                if ( it->expandable() ) {
                    temp = it;
                    bool first=false; 
                    if (it == clist.begin()) {
                        first = true;
                    } else {
                        temp--;
                    }
                    expand_at(it); // iterator moves to last expanded node.
                    if (first) // so need to reset iterator to first expanded node
                        it = clist.begin();
                    else
                        it = temp;    
                } else { // grey non-expandable nodes are removed
                    temp = it;
                    if ( it != clist.begin() ) 
                        it--; // jump out of the way from erase()
                    else
                        it++;
                    clist.erase(temp);
                }  
            }
        }
    }
    std::cout << " LinOCT::build() " << calc_calls << " calcScore() calls \n";
    // std::cout << size() << " nodes after build()\n";  
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
                // std::cout << "o2=" << *itr2 << "number=" << itr2->number() <<  "\n";
                // std::cout << "o1=" << *itr1 << "number=" << itr1->number() << "\n";
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

/// return list of triangles to python
/// the triangles correspond to every node/cube of the octree
boost::python::list LinOCT::get_triangles()
{    
    boost::python::list tlist;
    BOOST_FOREACH( Ocode o, clist) {
        std::vector<Point> p(8);
        for (int m=0;m<8;++m)
            p[m]=o.corner(m);
        // these 12 triangles cover the 6 sides of the cube
        tlist.append(Triangle(p[0],p[1],p[2])); 
        tlist.append(Triangle(p[1],p[2],p[6])); 
        tlist.append(Triangle(p[3],p[4],p[5])); 
        tlist.append(Triangle(p[4],p[5],p[7])); 
        tlist.append(Triangle(p[0],p[2],p[3])); 
        tlist.append(Triangle(p[3],p[4],p[2]));
        tlist.append(Triangle(p[1],p[5],p[6])); 
        tlist.append(Triangle(p[6],p[7],p[5]));
        tlist.append(Triangle(p[2],p[4],p[6])); 
        tlist.append(Triangle(p[6],p[7],p[4]));
        tlist.append(Triangle(p[0],p[1],p[3])); 
        tlist.append(Triangle(p[3],p[5],p[1]));
    }
    return tlist;
}


boost::python::list LinOCT::get_nodes() {    
    boost::python::list nodelist;
    BOOST_FOREACH( Ocode o, clist) {
            nodelist.append(o);
    }
    return nodelist;
}


/// string repr
std::ostream& operator<<(std::ostream &stream, const LinOCT &l) {
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
std::string LinOCT::str() const {
    std::ostringstream o;
    o << *this;
    return o.str();
}

} // end namespace
// end of file octree.cpp
