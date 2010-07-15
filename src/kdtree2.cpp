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

#include "millingcutter.h"
#include "point.h"
#include "triangle.h"
#include "numeric.h"
#include "kdtree2.h"

namespace ocl
{

//#define DEBUG_KD



//********   KDNode ********************** */


KDNode2::KDNode2(int d, double cv, KDNode2 *parent, KDNode2 *hi_c, KDNode2 *lo_c,
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
KDNode2* KDNode2::build_kdtree(const std::list<Triangle> *tris, 
                               unsigned int bucketSize,   // defaults to 1
                               int level,                 // defualts to 0 == root
                               KDNode2 *parent)            // defaults to NULL    
{
    
    if (tris->size() == 0) { //this is a fatal error.
        std::cout << "kdtree2.cpp ERROR: build_kdtree() called with tris->size()==0 ! \n";
        assert(0);
        return 0;
    }
    
    // calculate spread in order to know how to cut
    Spread2* spr = KDNode2::spread(tris); // call to static method which returns new object
    // calculate cut value
    double cutvalue = spr->start + spr->val/2; // cut in the middle
    
    // if spr.val==0 (no need/possibility to subdivide anymore)
    // OR number of triangles is smaller than bucketSize 
    // then return a bucket/leaf node
    if ( (tris->size() <= bucketSize) || (spr->val == 0.0)) {
        KDNode2 *bucket;
        bucket = new KDNode2(spr->d, 0.0 , parent , NULL, NULL, tris, level);
        return bucket; // this is the leaf/end of the recursion-tree
    }
    
    // build lists of triangles for hi and lo child nodes
    std::list<Triangle> *lolist = new std::list<Triangle>();
    std::list<Triangle> *hilist = new std::list<Triangle>();
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
        std::cout << "kdtree2.cpp: hilist.size()==0!\n";
        assert(0);
    }
    if (lolist->size() == 0) {
        std::cout << "kdtree2.cpp: lolist.size()==0!\n";
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
    KDNode2 *node = new KDNode2(spr->d, cutvalue, parent, NULL,NULL,NULL, level);
    
    // recursion:                   list    bucketsize   level   parent
    node->hi = KDNode2::build_kdtree(hilist, bucketSize, level+1, node);
    node->lo = KDNode2::build_kdtree(lolist, bucketSize, level+1, node);    
    
    // return a new node
    return node;
}


/// compare two spreads, this is required for sorting
bool Spread2::spread_compare(Spread2 *x, Spread2 *y) {
    if (x->val > y->val)
        return true;
    else
        return false;
}

/// static variable for counting cuts
int KDNode2::cutcount=0;

/// find the maximum 'extent' of Triangle list tris along dimension d
Spread2* KDNode2::spread(const std::list<Triangle> *tris) {
    Bbox minvals;
    Bbox maxvals;
    if (tris->size() == 0) {
        std::cout << "kdtree2.cpp ERROR, spread() called with tris->size()==0 ! \n";
        assert( 0 );
        return NULL;
    } else {
        BOOST_FOREACH(Triangle t, *tris) {
            minvals.addPoint( t.bb.minpt );
            maxvals.addPoint( t.bb.maxpt );
        } 
        
        // calculate the spread along each dimension
        // put the spreads in a list
        std::vector<Spread2*> spreads;
        spreads.push_back( new Spread2(0, maxvals.maxpt.x - maxvals.minpt.x , maxvals.minpt.x ) );  // dim=0  is maxx
        spreads.push_back( new Spread2(1, minvals.maxpt.x - minvals.minpt.x , minvals.minpt.x ) );   // dim=1  is minx
        spreads.push_back( new Spread2(2, maxvals.maxpt.y - maxvals.minpt.y , maxvals.minpt.y ) );  // dim=2  is maxy
        spreads.push_back( new Spread2(3, minvals.maxpt.y - minvals.minpt.y , minvals.minpt.y ) );   // dim=3  is miny
        spreads.push_back( new Spread2(4, maxvals.maxpt.z - maxvals.minpt.z , maxvals.minpt.z ) );   // dim=4  is minz
        spreads.push_back( new Spread2(5, minvals.maxpt.z - minvals.minpt.z , minvals.minpt.z ) );   // dim=5  is maxz
        std::sort(spreads.begin(), spreads.end(), Spread2::spread_compare); // sort the list
        // priority-queue could also be used ??
        
        /*
        std::cout << "\n";
        std::cout <<"spreads for " << tris->size() << " triangles:\n";
        std::cout << " selecting " << (spreads[0])->d << " with s="<< (spreads[0])->val << "\n";
        char c;
        std::cin >> c;
        */
        
        cutcount++;
        if (cutcount == 4)
            cutcount = 0;
        
        // select each dim in turn
        // return spreads[ cutcount ];
        
        return spreads[ 0 ]; // select the biggest spread and return
    } // end tris->size != 0
} // end spread()

bool KDNode2::overlap(const KDNode2 *node, const CLPoint &cl, const MillingCutter &cutter)
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
void KDNode2::getTriangles( std::list<Triangle> *tris, KDNode2 *node)
{
    if (node->tris != NULL) { 
        // found a bucket node, add all triangles
        BOOST_FOREACH(Triangle t, *(node->tris)) {
            tris->push_back(t); 
        }
        return;
    }
    // not a bucket node, so search recursively high and low:
    KDNode2::getTriangles(tris, node->hi);
    KDNode2::getTriangles(tris, node->lo);
    return;
}

#define DEBUG_KD_SEARCH


/// search kd-tree starting at KDNode node for triangles.
/// find the ones which overlap (in the xy-plane)
/// with the MillingCutter cutter positioned at  Point cl
/// these triangles are added to the tris list.
void KDNode2::search_kdtree( std::list<Triangle>* tris,      // found triangles added to tris
                            const CLPoint &cl,              // cutter positioned at cl
                            const MillingCutter &cutter,    // cutter
                            KDNode2 *node)                   // start search here and recurse into tree
{
    // we found a bucket node, so add all triangles and return.
    if (node->tris != NULL) { 
        //std::cout << "bucket: cl=" << cl << "r=" << cutter.getRadius() 
        //          << " len(tris)=" << node->tris->size() << " KDNode:" << *node << "\n";

        //if ( KDNode::overlap(node,cl,cutter) ) {  // check if node overlaps
            #ifdef DEBUG_KD_SEARCH
                 //std::cout << " returning bucket node with len(tris)="<< (*(node->tris)).size() << "\n";
                 //char c;
                 //std::cin >> c;
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
    } // end bucket-node
    
    // not a bucket node, so recursevily seach hi/lo branches of KDNode
    
    #ifdef DEBUG_KD_SEARCH
        //std::cout << "dim=" << node->dim << " cv=" << node->cutval << "\n";
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
                KDNode2::search_kdtree(tris, cl, cutter, node->hi); //hi
            } 
            else { // else we need to search both child-nodes
                #ifdef DEBUG_KD_SEARCH
                    //std::cout << " dim=" << node->dim << " branch hi AND lo\n";
                #endif 
                KDNode2::search_kdtree(tris, cl, cutter, node->hi);
                KDNode2::search_kdtree(tris, cl, cutter, node->lo);
            }
            break;
        case 1: // cut along xminus
            if ( node->cutval > ( cl.x + cutter.getRadius() ) ) {
                #ifdef DEBUG_KD_SEARCH
                    //std::cout << " dim=" << node->dim << " branch lo\n";
                    //std::cout << "NO triangles with xmin < " << node-> cutval << "\n";
                #endif 
                KDNode2::search_kdtree(tris, cl, cutter, node->lo);
            }
            else {
                KDNode2::search_kdtree(tris, cl, cutter, node->hi);
                KDNode2::search_kdtree(tris, cl, cutter, node->lo);
            }
            break;
        case 2: // cut along yplus
            if ( node->cutval < ( cl.y - cutter.getRadius() ) ) {
                #ifdef DEBUG_KD_SEARCH
                    // std::cout << " dim=" << node->dim << " branch hi\n";
                    //std::cout << "NO triangles with ymax < " << node-> cutval << "\n";
                #endif 
                KDNode2::search_kdtree(tris, cl, cutter, node->hi);
            }
            else {
                #ifdef DEBUG_KD_SEARCH
                    // std::cout << " dim=" << node->dim << " branch hi AND lo\n";
                #endif 
                KDNode2::search_kdtree(tris, cl, cutter, node->hi);
                KDNode2::search_kdtree(tris, cl, cutter, node->lo);
            }        
            break;
        case 3: // cut along yminus
            if ( node->cutval > ( cl.y + cutter.getRadius() ) ) {
                #ifdef DEBUG_KD_SEARCH
                    //std::cout << " dim=" << node->dim << " branch lo\n";
                    //std::cout << "NO triangles with ymin > " << node-> cutval << "\n";
                #endif 
                KDNode2::search_kdtree(tris, cl, cutter, node->lo);
            }
            else {
                #ifdef DEBUG_KD_SEARCH
                    //std::cout << " dim=" << node->dim << " branch hi AND lo\n";
                #endif 
                KDNode2::search_kdtree(tris, cl, cutter, node->lo);
                KDNode2::search_kdtree(tris, cl, cutter, node->hi);
            }      
            break;
        default:
            std::cout << "kdtree.cpp ERROR!\n"; // error
            assert(0);
    } // end of switch(dim)
    
    // we get here after all the recursive calls above.
    return; 
    

} // end search_kdtree()

//*********** Spread ****************

Spread2::Spread2(int dim, double v, double s) {
    d = dim;
    val = v;
    start = s;
}



//********  string output ********************** */
std::string KDNode2::str() const {
    std::ostringstream o;
    o << *this;
    return o.str();
}

std::ostream& operator<<(std::ostream &stream, const KDNode2 root) {
    stream << "KDNode d:" << root.dim << " cv:" << root.cutval;    
    return stream;
}

} // end namespace
// end file kdtree2.cpp
