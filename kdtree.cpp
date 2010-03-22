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
#include <boost/progress.hpp>
#include <vector>
#include <algorithm>

#include "cutter.h"
#include "point.h"
#include "triangle.h"
#include "cutter.h"
#include "numeric.h"

#include "kdtree.h"



//********   KDNode ********************** */


KDNode::KDNode(int d, double cv, KDNode *hi_c, KDNode *lo_c,
            const std::list<Triangle> *tlist, int lev) 
{
    dim = d;
    cutval = cv;
    hi = hi_c;
    lo = lo_c;
    tris = tlist;
    level = lev;
}

//** KDTree


int KDTree::level = 0;
/// given a list of triangles, build and return the root node of a kd-tree with the triangles
KDNode* KDTree::build_kdtree(const std::list<Triangle> *tris, unsigned int bucketSize) 
{
    
    if (tris->size() == 0) {
        std::cout << "kdtree.cpp ERROR: build_kdtree called with tris->size()==0 ! \n";
        return 0;
    }
    
    // calculate spread in order to know how to cut
    Spread* spr = KDTree::spread(tris);
    // calculate cut value
    double cutvalue = spr->start + spr->val/2;
    
    // if triangles contained within sufficiently small rectangle (FIXME?)
    // OR number of triangles is smaller than bucketSize 
    // then return a bucket node
    if ( (tris->size() <= bucketSize) || (spr->val == 0.0)) {
        //std::cout << "Bucket with len(tris)=" << tris->size() << "\n";
        KDNode *bucket;
        if ( (spr->d == 0) || (spr->d == 2) )
            bucket = new KDNode(spr->d, spr->start+spr->val, NULL, NULL, tris, level);
        else
            bucket = new KDNode(spr->d, spr->start, NULL, NULL, tris, level);
        
        return bucket;
    }
    
   
    
    // build lists of triangles for hi and lo child nodes
    std::list<Triangle> *lolist = new std::list<Triangle>();
    std::list<Triangle> *hilist = new std::list<Triangle>();
    lolist->clear();
    hilist->clear();
    
    BOOST_FOREACH(Triangle t, *tris) {
        
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
    
    if (hilist->size() == 0) // an error ??
        std::cout << "hilist.size()==0!\n";
    if (lolist->size() == 0)
        std::cout << "lolist.size()==0!\n";
    
    /*
    std::cout << "hilist.size()=" << hilist->size() << "\n";
    std::cout << "lolist.size()=" << lolist->size() << "\n";
    std::cout << "sum: " << hilist->size()+lolist->size() << "== " << tris->size() << " ? \n";
    char c;
    std::cin >> c;
    */
    
    KDNode *node = new KDNode(0,0,NULL,NULL,NULL, ++level);
    node->dim = spr->d;
    node->cutval = cutvalue;
    
    // recursion:
    node->hi = KDTree::build_kdtree(hilist, bucketSize);
    node->lo = KDTree::build_kdtree(lolist, bucketSize);
    
     
    // return a new node
    return node;
}


bool Spread::spread_compare(Spread *x, Spread *y) {
    if (x->val > y->val)
        return true;
    else
        return false;
}

/// find the maximum 'extent' of triangles in list tris along dimension d
Spread* KDTree::spread(const std::list<Triangle> *tris) {
    double max_xplus=0, min_xplus=0, max_xminus=0, min_xminus=0;
    double max_yplus = 0, min_yplus = 0, max_yminus = 0, min_yminus = 0;
    double spr_xplus = 0, spr_xminus = 0, spr_yplus = 0, spr_yminus = 0;
    
    if (tris->size() == 0) {
        std::cout << "kdtree.cpp ERROR, spread() called with tris->size()==0 ! \n";
        return new Spread(0,0.0,0.0); // probably an error if we get here?
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
        //std::cout << "spreads:" << spr_xplus << ", " << spr_xminus << ", " << spr_yplus << ", " << spr_yminus << "\n";
        //char c;
        //std::cin >> c;
        
        // put the spreads in a list
        std::vector<Spread*> spreads;
        // = new std::vector<Spread*>();
        spreads.push_back( new Spread(0, spr_xplus, min_xplus)   );
        //std::cout <<"0: spread=" << spr_xplus << "\n";
        spreads.push_back( new Spread(1, spr_xminus, min_xminus) );
        //std::cout <<"1: spread=" << spr_xminus << "\n";
        spreads.push_back( new Spread(2, spr_yplus, min_yplus)   );
        //std::cout <<"2: spread=" << spr_yplus << "\n";
        spreads.push_back( new Spread(3, spr_yminus, min_yminus) );
        //std::cout <<"3: spread=" << spr_yminus << "\n";
        // sort the list
        std::sort(spreads.begin(), spreads.end(), Spread::spread_compare);
        //std::cout << " selecting " << (spreads[0])->d << " with s="<< (spreads[0])->val << "\n";
        //char c;
        //std::cin >> c;
        // select the biggest spread and return
        return spreads[ 0 ];
        
    } // end tris->size != 0

} // end spread()

bool KDTree::overlap(const KDNode *node, const Point &cl, const MillingCutter &cutter)
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
    } // end of switch(dim)

    return false;
}

/// search kd-tree starting at KDNode node for triangles.
/// find the ones which overlap (in the xy-plane)
/// with the MillingCutter cutter positioned at  Point cl
/// these triangles are added to the tris list.
void KDTree::search_kdtree(std::list<Triangle> *tris,
                            Point &cl, 
                            const MillingCutter &cutter, 
                            KDNode *node)
{
    // we found a bucket node, so add all triangles and return.
    if (node->tris != NULL) { 
        //std::cout << "bucket: cl=" << cl << "r=" << cutter.getRadius() 
        //          << " len(tris)=" << node->tris->size() << " KDNode:" << *node << "\n";
        if ( KDTree::overlap(node,cl,cutter) ) {
            BOOST_FOREACH(Triangle t, *(node->tris)) {
                //std::cout << t << "\n";
                //double r = cutter.getRadius();
                //std::cout << "T: " << t.minx << "\t" << t.maxx << "\t" << t.miny << "\t" << t.maxy << "\n";
                //std::cout << "C: " << cl.x-r << "\t" << cl.x+r << "\t" << cl.y-r << "\t" << cl.y+r << "\n";
                //std::cout << "overlap?:" << KDTree::overlap(node,cl,cutter) << "\n";
                if ( cutter.overlaps(cl,t) ) {
                    tris->push_back(t); 
                }
            }

        }
        //char c;
        //std::cin >> c;
        return;
    }
    
    // not a bucket node, so recursevily seach hi/lo branches of KDNode
    
    //std::cout << "cl=" << cl << "r=" << cutter.getRadius() 
    //              << " Internal KDNode:" << *node << "\n";
    switch(node->dim) { // ugly, solve with polymorphism?
        case 0: // cut along xplus
            // if one child node is not overlapping, search only the other
            if ( node->cutval <= ( cl.x - cutter.getRadius() ) ) {
                KDTree::search_kdtree(tris, cl, cutter, node->hi); //hi
            } 
            else { // else we need to search both child-nodes
                KDTree::search_kdtree(tris, cl, cutter, node->hi);
                KDTree::search_kdtree(tris, cl, cutter, node->lo);
            }
            break;
        case 1: // cut along xminus
            if ( node->cutval >= ( cl.x + cutter.getRadius() ) ) {
                KDTree::search_kdtree(tris, cl, cutter, node->lo);
            }
            else {
                KDTree::search_kdtree(tris, cl, cutter, node->hi);
                KDTree::search_kdtree(tris, cl, cutter, node->lo);
            }
            break;
        case 2: // cut along yplus
            if ( node->cutval <= ( cl.y - cutter.getRadius() ) ) {
                KDTree::search_kdtree(tris, cl, cutter, node->hi);
            }
            else {
                KDTree::search_kdtree(tris, cl, cutter, node->hi);
                KDTree::search_kdtree(tris, cl, cutter, node->lo);
            }        
            break;
        case 3: // cut along yminus
            if ( node->cutval >= ( cl.y + cutter.getRadius() ) ) {
                KDTree::search_kdtree(tris, cl, cutter, node->lo);
            }
            else {
                KDTree::search_kdtree(tris, cl, cutter, node->hi);
                KDTree::search_kdtree(tris, cl, cutter, node->lo);
            }      
            break;
    } // end of switch(dim)
    
   
    return; // do we ever get here?
    

} // end search_kdtree()

//*********** Spread ****************

Spread::Spread(int dim, double v, double s)
{
    d = dim;
    val = v;
    start = s;
}

void KDTree::str(KDNode *root)
{
    //static int level=0;
    std::cout << "L: " << root->level << " d:" << root->dim << " v:" << root->cutval <<" ";
    if (root->tris)
        std::cout << "tris.size()=" << root->tris->size() << "\n";
    else
        std::cout << "INT \n";    
        
    if (root->hi) {
        KDTree::str(root->hi);
        KDTree::str(root->lo);
    }
}


//********  string output ********************** */
std::string KDNode::str()
{
    std::ostringstream o;
    o << "KDNode\n";
    return o.str();
}

std::ostream& operator<<(std::ostream &stream, const KDNode root)
{
    stream << "d:" << root.dim << " cv:" << root.cutval;    
    return stream;
}

