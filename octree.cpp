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
double Ocode::scale = 2;
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

int Ocode::degree()
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
    for (int n=0;n<8;n++) {// loop through all corners, and center
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

/// string repr
std::ostream& operator<<(std::ostream &stream, const Ocode &o)
{
    stream << "OCode: " ; 
    for (int n = 0; n<Ocode::depth; n++)
        stream << (int)o.code[n] << " center=" << o.point();
    
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

void LinOCT::delete_at(int idx) {
    if ( valid_index(idx) )
        clist.erase( clist.begin()+idx) ;
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

// build LinOCT from input OCTVolume
void LinOCT::build(OCTVolume* vol, int min_expand)
{
    // expand min_expand times to sample densely enough
    for (int m=0; m<min_expand ; m++) {
        for (int n=0; n<size() ; n++) {
            if ( clist[n].expandable() ) {
                expand_at(n); // expand the node
                n=n+7; // jump forward, since we have inserted new nodes
            }
        }
    }
    
    // loop until no expandable nodes remain
    // flagging black nodes as "done", and 
    // deleting white nodes at each step
    for (int n=0; n<size() ; n++) { // loop through all remaining nodes
        //std::cout << n <<"\n";
        if (  clist[n].isWhite( vol ) ) {// white nodes can be deleted
            //std::cout << "FOUND WHITE node "<< clist[n] <<"\n";
            clist[n].color = 1;
            //delete_at(n);
        }
        
    }
    

    
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

/// string repr
std::string LinOCT::str()
{
    std::ostringstream o;
    o << *this;
    return o.str();
}




//********   OCTNode ********************** */


double  OCTNode::max_scale = 10.0;
int     OCTNode::max_depth = 5;
int     OCTNode::min_depth = 3;

double OCTNode::get_max_scale()
{
    return max_scale;
}


OCTNode::OCTNode()
{
    level = 0;
    scale = max_scale/pow(2.0, level);
    
    center = Point(0,0,0);
    type = WHITE;
    parent = NULL;
    child = std::vector<OCTNode*>(8);
}

OCTNode::OCTNode(int in_level, Point& in_center, OCType in_type, OCTNode* in_parent) 
{
    center = in_center;
    type = in_type;
    parent = in_parent;
    level = in_level;
    scale = max_scale/pow(2.0, level);
    child = std::vector<OCTNode*>(8);
    return;
}

/// return a node-point    
Point OCTNode::nodePoint(int id)
{
    return center + (scale/2)*nodeDir(id);
}

/// return center-point of child
Point OCTNode::childCenter(int id)
{
    if ( id > 0 )
        return center + (scale/4)*nodeDir(id);
    else {
        assert(0);
        return Point(0,0,0);
    }
}
        
/// return direction to node-point
Point OCTNode::nodeDir(int id)
{
    switch(id)
    {
        case 0:
            return Point(0,0,0);
            break;
        case 1:
            return Point(1,1,1);
            break;
        case 2:
            return Point(-1,1,1);
            break;
        case 3:
            return Point(1,-1,1);
            break;
        case 4:
            return Point(1,1,-1);
            break;
        case 5:
            return Point(1,-1,-1);
            break;
        case 6:
            return Point(-1,1,-1);
            break;
        case 7:
            return Point(-1,-1,1);
            break;
        case 8:
            return Point(-1,-1,-1);
            break;
        default:
            std::cout << "octree.cpp nodeDir() called with invalid id!!\n";
            assert(0);
            break;
    }
    assert(0);
    return Point(0,0,0);
}
        



OCTNode* OCTNode::build_octree(OCTVolume* vol, 
                               int in_level, 
                               Point& in_center,
                               OCTNode* in_parent)
{
    #ifdef DEBUG_BUILD_OCT
        std::cout << "build_octree()\n";
    #endif
    //std::vector<OCTNode*> children(8);
    
    OCTNode* node = new OCTNode( in_level, in_center, GREY, in_parent);
    //node->center = in_center;
    //node->level = in_level;
    //node->parent = in_parent;
    
    // test all node-points: inside or outside volume?
    std::vector<bool> flag(9);
    int sum=0;
    for (int n=0 ; n<9 ; n++) {
        Point p = node->nodePoint(n);
        flag[n]= vol->isInside( p );
        if (flag[n] == true)
            sum += 1;
        else 
            sum -= 1;
    }
    #ifdef DEBUG_BUILD_OCT
        std::cout << "level=" << node->level << " sum=" << sum << "\n";
    #endif
    if ( node->level > min_depth ) {
        if ( sum == 9 ) { // all points are inside
            node->type = WHITE;
            return node;
        }
        else if ( sum == -9 ) { // all points are outside
            #ifdef DEBUG_BUILD_OCT
                std::cout << " BLACK\n";
            #endif
            node->type = BLACK;
            #ifdef DEBUG_BUILD_OCT
                std::cout << " set type= BLACK\n";
            #endif
            return node;
        } 
    }
    
    // check if we reached the max allowed depth
    if (node->level == max_depth) {
        #ifdef DEBUG_BUILD_OCT
            std::cout << " max level reached\n";
        #endif
        return node;
    }
    
    // if we get here, we need to subdivide
    node->type = GREY;
    Point c_center;
    for (int n=1; n<9; n++) {
        c_center = node->childCenter(n);
        #ifdef DEBUG_BUILD_OCT
            std::cout << " build child n="<< n << " at center=" << c_center << "\n";
        #endif
        node->child[n-1] = OCTNode::build_octree( vol, node->level+1, c_center, node );
    }
    
    #ifdef DEBUG_BUILD_OCT
        std::cout << " build_octre() done.\n";
    #endif
    return node;
}

void OCTNode::getNodes( std::list<OCTNode> *nodelist, OCTNode *node)
{
    if (node)  
        nodelist->push_back(*node);
        
    if ( node->child[0] ) {
        for (int n=0; n<8 ; n++)
            OCTNode::getNodes( nodelist, node->child[n] );
    }
    return;
}


int OCTNode::prune_all(OCTNode* root)
{
    std::list<OCTNode> *nodes = new std::list<OCTNode>();
    OCTNode::getNodes( nodes, root);
    double current_n = nodes->size();
    double new_n = 0;
    int n=0;
    while (new_n < current_n) {
        current_n = nodes->size();
        OCTNode::prune(root);
        nodes->clear();
        OCTNode::getNodes(nodes,root);
        new_n = nodes->size();
        n=n+1;
    }
    return n;
}

void OCTNode::prune(OCTNode* root) {
    
    
    if (root->child[0]) {
        int sum=0;
        for (int n=0; n<8; n++) {
            if ( root->child[n]->type == WHITE )
                sum +=1;
            else if ( root->child[n]->type == BLACK )
                sum -=1;
        }
        
        if (sum == 8) { // all white
            for (int n=0; n<8; n++) {
                root->child[n] = NULL; /// \todo FIXME this is probably a memory leak, need to delete child OCTNodes also
            }
            root->type=WHITE;
            return;
        }
        if (sum == -8) { // all black
            for (int n=0; n<8; n++) {
                root->child[n] = NULL; /// \todo FIXME this is probably a memory leak, need to delete child OCTNodes also
            }
            root->type=BLACK;
            return;
        }
        
        // else go prune children
        for (int n=0; n<8; n++) {
            OCTNode::prune( root->child[n] );
        }
    }
    
    return;
}


void OCTNode::balance( OCTNode* root1, OCTNode* root2)
{
    if (!root2->child[0]) {
        return; // nothing to do if we are at leaf of tree2
    } 
    else 
    { // root2 has children
        if (!root1->child[0]) {
            // create children for root1 if none exist
            for (int n=0;n<8;n++) {
                Point c_center = root1->childCenter(n+1);
                root1->child[n] = new OCTNode( (root1->level)+1, c_center, root1->type, root1);
            }
            // balance the new children
            for (int n=0;n<8;n++) {
                OCTNode::balance( root1->child[n], root2->child[n] );
            }
        }
        else { // root1 also has children
            // call balance() on children
            for (int n=0;n<8;n++) {
                OCTNode::balance( root1->child[n], root2->child[n] );
            }
        }
    }
    return;
}

void OCTNode::diff( OCTNode* root1, OCTNode* root2)
{
    // root1 = root1 - root2
    
    if (!root2->child[0]) {
        // leaf of root2
        if (root2->type == WHITE) {
            root1->type = BLACK;
            for (int n=0; n<8; n++) {
                root1->child[n] = NULL; /// \todo FIXME this is probably a memory leak, need to delete child OCTNodes also
            }
        }
    }
    else {
        if (root2->type == GREY) {
            for (int n=0; n<8; n++) {
                    OCTNode::diff( root1->child[n], root2->child[n] );
            }
        }
    }
    return;
    
}



//********  OCTNode string output ********************** */
std::string OCTNode::str()
{
    std::ostringstream o;
    o << "OCTNode l="<< level << " center=" << center << " scale=" << scale << " type="<< type;
    return o.str();
}

std::ostream& operator<<(std::ostream &stream, const OCTNode root)
{
    stream << "OCTNode";    
    return stream;
}

//************* Volumes **************/

SphereOCTVolume::SphereOCTVolume()
{
    center = Point(2,0,0);
    radius = 3.0;
}

bool SphereOCTVolume::isInside(Point& p) const
{
    std::cout << "dist to point=" << (center-p).norm() <<"\n";
    if ( (center-p).norm() < radius )
        return true;
    else
        return false;
}

//*********** Cube colume **********/
CubeOCTVolume::CubeOCTVolume()
{
    center = Point(0,0,0);
    side = 1.234;
}

bool CubeOCTVolume::isInside(Point& p) const
{
    bool x,y,z;
    x = ( (p.x > (center.x-side/2)) && (p.x < (center.x+side/2)) );
    y = ( (p.y > (center.y-side/2)) && (p.y < (center.y+side/2)) );
    z = ( (p.z > (center.z-side/2)) && (p.z < (center.z+side/2)) );
    if ( x && y && z )
        return true;
    else
        return false;
}



//********* OCTest***************/
// mostly testing/debugging class for now

OCTest::OCTest()
{
    volume = new SphereOCTVolume();
    //volume = new CubeOCTVolume();
    root = 0;
}

void OCTest::build_octree()
{
    Point c = Point(0,0,0);
    root = OCTNode::build_octree(volume , 0 , c , NULL); 
}



double OCTest::get_max_depth() 
{
    return OCTNode::max_depth;
}

void OCTest::set_max_depth(int d) 
{
    OCTNode::max_depth = d;
}

void OCTest::setVol(OCTVolume& in_volume)
{
    volume = &in_volume;
}



void OCTest::prune() 
{
    OCTNode::prune(root);
}

int OCTest::prune_all() 
{
    return OCTNode::prune_all(root);
}

void OCTest::balance(OCTest& other)
{
    OCTNode::balance(root, other.root);
    return;
}

void OCTest::diff(OCTest& other)
{
    OCTNode::diff(root, other.root);
    return;
}

boost::python::list OCTest::get_all_nodes()
{
    std::list<OCTNode> *nodes = new std::list<OCTNode>();
    OCTNode::getNodes( nodes, root);
    
    boost::python::list nodelist;
    BOOST_FOREACH( OCTNode n, *nodes) {
        nodelist.append(n);
    }
    return nodelist;
}

boost::python::list OCTest::get_white_nodes()
{
    std::list<OCTNode> *nodes = new std::list<OCTNode>();
    OCTNode::getNodes( nodes, root);
    
    boost::python::list nodelist;
    BOOST_FOREACH( OCTNode n, *nodes) {
        if (n.type == WHITE) 
            nodelist.append(n);
    }
    return nodelist;
}

boost::python::list OCTest::get_black_nodes()
{
    std::list<OCTNode> *nodes = new std::list<OCTNode>();
    OCTNode::getNodes( nodes, root);
    
    boost::python::list nodelist;
    BOOST_FOREACH( OCTNode n, *nodes) {
        if (n.type == BLACK) 
            nodelist.append(n);
    }
    return nodelist;
}

// end of file octree.cpp
