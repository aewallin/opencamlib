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

// this is mostly a translation to c++ of the earlier c# code
// http://code.google.com/p/monocam/source/browse/trunk/Project2/monocam_console/monocam_console/kdtree.cs

//#include <iostream>
//#include <stdio.h>
//#include <sstream>
//#include <math.h>
//#include <vector>
//#include <algorithm>

#include <boost/foreach.hpp>

#include "millingcutter.h"
#include "point.h"
#include "triangle.h"
#include "numeric.h"
#include "kdtree.h"

namespace ocl
{

//#define DEBUG_KD

//#define DEBUG_KD_SEARCH

//********   KDNode ********************** */


KDNode::KDNode(int d, double cv, KDNode *parent, KDNode *hi_c, KDNode *lo_c,
            const std::list<Triangle> *tlist, int lev) 
{
    dim = d;
    cutval = cv;
    up = parent;
    hi = hi_c;
    lo = lo_c;
    tris = tlist;
    level = lev;
}



/// given a list of triangles, build and return the root node of a kd-tree with the triangles
KDNode* KDNode::build_kdtree(const std::list<Triangle> *tris, 
                             unsigned int bucketSize,   // defaults to 1
                             int level,                 // defualts to 0 == root
                             KDNode *parent)            // defaults to NULL    
{
    
    if (tris->size() == 0) { //this is a fatal error.
        std::cout << "kdtree.cpp ERROR: build_kdtree() called with tris->size()==0 ! \n";
        assert(0);
        return 0;
    }
    
    // calculate spread in order to know how to cut
    Spread* spr = KDNode::spread(tris);
    // calculate cut value
    double cutvalue = spr->start + spr->val/2; // cut in the middle
    
    // if spr.val==0 (no need/possibility to subdivide anymore)
    // OR number of triangles is smaller than bucketSize 
    // then return a bucket node
    if ( (tris->size() <= bucketSize) || (spr->val == 0.0)) {
        //std::cout << "Bucket with len(tris)=" << tris->size() << "\n";
        KDNode *bucket;
        if ( (spr->d == 0) || (spr->d == 2) )  {// maxx or maxy cut
            // does the node spread value make a difference here??
            bucket = new KDNode(spr->d, spr->start+spr->val, parent , NULL, NULL, tris, level);
            //bucket = new KDNode(spr->d, 0.0 , parent , NULL, NULL, tris, level);
        }
        else {// the min cut case:
            // does the node spread value make a difference here??
             bucket = new KDNode(spr->d, spr->start, parent , NULL, NULL, tris, level);
            //bucket = new KDNode(spr->d, 0.0 , parent , NULL, NULL, tris, level);
        }
        return bucket;
    }
    
   
    
    // build lists of triangles for hi and lo child nodes
    std::list<Triangle> *lolist = new std::list<Triangle>();
    std::list<Triangle> *hilist = new std::list<Triangle>();
    
    // these are probably not required
    // lolist->clear();
    // hilist->clear();
    
    BOOST_FOREACH(Triangle t, *tris) { // loop through each triangle and put it in either lolist or hilist
        #ifdef DEBUG_KD
            std::cout << "adding tri=" << t;
            std::cout << " dim=" << spr->d << "\n";
        #endif
        if (spr->d == 0) { // cut along maxx
            if (t.maxx > cutvalue) {
                hilist->push_back(t);
            }
            else {
                lolist->push_back(t);
            }
        } else if (spr->d == 1) { // cut along minx
            if (t.minx > cutvalue) {
                hilist->push_back(t);
            }
            else {
                lolist->push_back(t);           
            }
        } else if (spr->d == 2) { // cut along maxy
            if (t.maxy > cutvalue) {
                hilist->push_back(t);
            }
            else {
                lolist->push_back(t);           
            }
        } else if (spr->d == 3) { // cut along miny
            if (t.miny > cutvalue) {
                hilist->push_back(t);
            }
            else {
                lolist->push_back(t);           
            }
        }
            
    } // end loop through each triangle
    
    if (hilist->size() == 0) {// an error ??
        std::cout << "kdtree.cpp: hilist.size()==0!\n";
        assert(0);
    }
    if (lolist->size() == 0) {
        std::cout << "kdtree.cpp: lolist.size()==0!\n";
        assert(0);
    }
    
    /*
    std::cout << "hilist.size()=" << hilist->size() << "\n";
    std::cout << "lolist.size()=" << lolist->size() << "\n";
    std::cout << "sum: " << hilist->size()+lolist->size() << "== " << tris->size() << " ? \n";
    char c;
    std::cin >> c;
    */
    
    //                         dim     value    parent  hi   lo   trilist  level
    KDNode *node = new KDNode(spr->d, cutvalue, parent, NULL,NULL,NULL, level);
    
    // recursion:                   list    bucketsize   level   parent
    node->hi = KDNode::build_kdtree(hilist, bucketSize, level+1, node);
    node->lo = KDNode::build_kdtree(lolist, bucketSize, level+1, node);    
     
    // return a new node
    return node;
}


bool Spread::spread_compare(Spread *x, Spread *y) {
    if (x->val > y->val)
        return true;
    else
        return false;
}

int KDNode::cutcount=0;

/// find the maximum 'extent' of Triangle list tris along dimension d
Spread* KDNode::spread(const std::list<Triangle> *tris) {
    double max_xplus=0, min_xplus=0, max_xminus=0, min_xminus=0;
    double max_yplus = 0, min_yplus = 0, max_yminus = 0, min_yminus = 0;
    double spr_xplus = 0, spr_xminus = 0, spr_yplus = 0, spr_yminus = 0;
    
    if (tris->size() == 0) {
        std::cout << "kdtree.cpp ERROR, spread() called with tris->size()==0 ! \n";
        assert( 0 );
        return NULL;
    }
    else {
        int n=1;
        BOOST_FOREACH(Triangle t, *tris) {
            if (n==1) {
                // initialize things on the first run
                max_xplus  = t.maxx;
                min_xplus  = t.maxx;
                max_xminus = t.minx;
                min_xminus = t.minx;

                max_yplus  = t.maxy;
                min_yplus  = t.maxy;
                max_yminus = t.miny;
                min_yminus = t.miny;
            } // end initialize
            else { // FIXME madness really, but for only 4 dimensions this works...
            
                // compute spread in xplus
                if (t.maxx > max_xplus)
                    max_xplus = t.maxx;
                if (t.maxx < min_xplus)
                    min_xplus = t.maxx;
                    
                // compute spread in xminus
                if (t.minx > max_xminus)
                    max_xminus = t.minx;
                if (t.minx < min_xminus)
                    min_xminus = t.minx;
                
                // compute spread in yplus
                if (t.maxy > max_yplus)
                    max_yplus = t.maxy;
                if (t.maxy < min_yplus)
                    min_yplus = t.maxy;
                    
                // compute spread in yminus
                if (t.miny > max_yminus)
                    max_yminus = t.miny;
                if (t.miny < min_yminus)
                    min_yminus = t.miny;
            }
            ++n;
        } // end FOREACH triangle in tris
        
        // calculate the spread along each dimension
        spr_xplus = max_xplus - min_xplus;
        spr_xminus = max_xminus - min_xminus;
        spr_yplus = max_yplus - min_yplus;
        spr_yminus = max_yminus - min_yminus;
        
        // spreads are distances, they should be zero or positive.
        assert(  spr_xplus  >= 0.0 );
        assert(  spr_xminus >= 0.0 );
        assert(  spr_yplus  >= 0.0 );
        assert(  spr_yminus >= 0.0 );
      
        // put the spreads in a list
        std::vector<Spread*> spreads;

        spreads.push_back( new Spread(0, spr_xplus, min_xplus)   );  // dim=0  is maxx
        //std::cout <<"0: spread=" << spr_xplus << "\n";
        spreads.push_back( new Spread(1, spr_xminus, min_xminus) );  // dim=1  is minx
        //std::cout <<"1: spread=" << spr_xminus << "\n";
        spreads.push_back( new Spread(2, spr_yplus, min_yplus)   );  // dim=2  is maxy
        //std::cout <<"2: spread=" << spr_yplus << "\n";
        spreads.push_back( new Spread(3, spr_yminus, min_yminus) );  // dim=3  is miny
        //std::cout <<"3: spread=" << spr_yminus << "\n";
        // sort the list
        std::sort(spreads.begin(), spreads.end(), Spread::spread_compare);
        //std::cout << " selecting " << (spreads[0])->d << " with s="<< (spreads[0])->val << "\n";
        //char c;
        //std::cin >> c;
        
      
        
        cutcount++;
        if (cutcount ==4)
            cutcount = 0;
        
        // select each dim in turn
        //return spreads[ cutcount ];
        
        // select the biggest spread and return
        return spreads[ 0 ];
        
    } // end tris->size != 0

} // end spread()

bool KDNode::overlap(const KDNode *node, const Point &cl, const MillingCutter &cutter)
{
    switch(node->dim) { 
        case 0: // cut along xplus
            if ( node->cutval <= cl.x - cutter.getRadius() )
                return false;
            else 
                return true;
            break;
        case 1: // cut along xminus
            if ( node->cutval >= cl.x + cutter.getRadius() )
                return false;
            else 
                return true;
            break;
        case 2: // cut along yplus
            if ( node->cutval <= cl.y - cutter.getRadius() )
                return false;
            else 
                return true;
            break;
        case 3: // cut along yminus
            if ( node->cutval >= cl.y + cutter.getRadius() )
                return false;
            else 
                return true;
            break;
        default:
            assert(0);
    } // end of switch(dim)

    return false;
}


/// returns all triangles under KDNode node in the tree
void KDNode::getTriangles( std::list<Triangle> *tris, KDNode *node)
{
    if (node->tris != NULL) { 
        // found a bucket node, add all triangles
        BOOST_FOREACH(Triangle t, *(node->tris)) {
            tris->push_back(t); 
        }
        return;
    }
    // not a bucket node, so search recursively high and low:
    KDNode::getTriangles(tris, node->hi);
    KDNode::getTriangles(tris, node->lo);
    return;
}

/// search kd-tree starting at KDNode node for triangles.
/// find the ones which overlap (in the xy-plane)
/// with the MillingCutter cutter positioned at  Point cl
/// these triangles are added to the tris list.
void KDNode::search_kdtree(std::list<Triangle> *tris,
                            const Point &cl, 
                            const MillingCutter &cutter, 
                            KDNode *node)
{
    // we found a bucket node, so add all triangles and return.
    if (node->tris != NULL) { 
        //std::cout << "bucket: cl=" << cl << "r=" << cutter.getRadius() 
        //          << " len(tris)=" << node->tris->size() << " KDNode:" << *node << "\n";

        //if ( KDNode::overlap(node,cl,cutter) ) {  // check if node overlaps
            #ifdef DEBUG_KD_SEARCH
                // std::cout << " returning bucket node with len(tris)="<< (*(node->tris)).size() << "\n";
            #endif
            BOOST_FOREACH( Triangle t, *(node->tris) ) {
                //std::cout << t << "\n";
                //double r = cutter.getRadius();
                //std::cout << "T: " << t.minx << "\t" << t.maxx << "\t" << t.miny << "\t" << t.maxy << "\n";
                //std::cout << "C: " << cl.x-r << "\t" << cl.x+r << "\t" << cl.y-r << "\t" << cl.y+r << "\n";
                //std::cout << "overlap?:" << KDTree::overlap(node,cl,cutter) << "\n";
                
                // THIS IS PROBABLY WRONG, should not have to do an explicit overlap check here
                // if ( cutter.overlaps(cl,t) ) { // explicit cutter-overlap check
                    tris->push_back(t); 
                //}
                
                
            } // loop through triangles

        //} // node overlap check

        return;
    }
    
    // not a bucket node, so recursevily seach hi/lo branches of KDNode
    
    #ifdef DEBUG_KD_SEARCH
        std::cout << "dim=" << node->dim << " cv=" << node->cutval << "\n";
    #endif 
    //std::cout << "cl=" << cl << "r=" << cutter.getRadius() 
    //              << " Internal KDNode:" << *node << "\n";
    switch(node->dim) { // ugly, solve with polymorphism?
        case 0: // cut along xplus
            // if one child node is not overlapping, search only the other
            if ( node->cutval < ( cl.x - cutter.getRadius() ) ) {
                #ifdef DEBUG_KD_SEARCH
                    //std::cout << " dim=" << node->dim << " branch hi\n";
                    //std::cout << "NO triangles with xmax < " << node-> cutval << "\n"; 
                #endif 
                KDNode::search_kdtree(tris, cl, cutter, node->hi); //hi
            } 
            else { // else we need to search both child-nodes
                #ifdef DEBUG_KD_SEARCH
                    //std::cout << " dim=" << node->dim << " branch hi AND lo\n";
                #endif 
                KDNode::search_kdtree(tris, cl, cutter, node->hi);
                KDNode::search_kdtree(tris, cl, cutter, node->lo);
            }
            break;
        case 1: // cut along xminus
            if ( node->cutval > ( cl.x + cutter.getRadius() ) ) {
                #ifdef DEBUG_KD_SEARCH
                    //std::cout << " dim=" << node->dim << " branch lo\n";
                    //std::cout << "NO triangles with xmin < " << node-> cutval << "\n";
                #endif 
                KDNode::search_kdtree(tris, cl, cutter, node->lo);
            }
            else {
                KDNode::search_kdtree(tris, cl, cutter, node->hi);
                KDNode::search_kdtree(tris, cl, cutter, node->lo);
            }
            break;
        case 2: // cut along yplus
            if ( node->cutval < ( cl.y - cutter.getRadius() ) ) {
                #ifdef DEBUG_KD_SEARCH
                    // std::cout << " dim=" << node->dim << " branch hi\n";
                    //std::cout << "NO triangles with ymax < " << node-> cutval << "\n";
                #endif 
                KDNode::search_kdtree(tris, cl, cutter, node->hi);
            }
            else {
                #ifdef DEBUG_KD_SEARCH
                    // std::cout << " dim=" << node->dim << " branch hi AND lo\n";
                #endif 
                KDNode::search_kdtree(tris, cl, cutter, node->hi);
                KDNode::search_kdtree(tris, cl, cutter, node->lo);
            }        
            break;
        case 3: // cut along yminus
            if ( node->cutval > ( cl.y + cutter.getRadius() ) ) {
                #ifdef DEBUG_KD_SEARCH
                    //std::cout << " dim=" << node->dim << " branch lo\n";
                    //std::cout << "NO triangles with ymin > " << node-> cutval << "\n";
                #endif 
                KDNode::search_kdtree(tris, cl, cutter, node->lo);
            }
            else {
                #ifdef DEBUG_KD_SEARCH
                    //std::cout << " dim=" << node->dim << " branch hi AND lo\n";
                #endif 
                KDNode::search_kdtree(tris, cl, cutter, node->lo);
                KDNode::search_kdtree(tris, cl, cutter, node->hi);
            }      
            break;
        default:
            // error
            std::cout << "kdtree.cpp ERROR!\n";
            assert(0);
    } // end of switch(dim)
    
    // we get here after all the recursive calls above.
    return; 
    

} // end search_kdtree()

//*********** Spread ****************

Spread::Spread(int dim, double v, double s)
{
    d = dim;
    val = v;
    start = s;
}



//********  string output ********************** */
std::string KDNode::str() const
{
    std::ostringstream o;
    o << *this;
    return o.str();
}

std::ostream& operator<<(std::ostream &stream, const KDNode root)
{
    stream << "KDNode d:" << root.dim << " cv:" << root.cutval;    
    return stream;
}

} // end namespace
// end file kdtree.cpp
