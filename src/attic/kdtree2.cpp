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
    
    if (tris->empty() ) { //this is a fatal error.
        std::cout << "kdtree2.cpp ERROR: build_kdtree() called with tris->size()==0 ! \n";
        assert(0);
        return 0;
    }
    
    // calculate spread in order to know how to cut
    Spread2* spr = KDNode2::spread(tris); // call to static method which returns new object
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
        if (t.bb[spr->d] > cutvalue) 
            hilist->push_back(t);
        else
            lolist->push_back(t);
    } 
    
    if (hilist->empty() ) {// an error ??
        std::cout << "kdtree2.cpp: hilist.size()==0!\n";
        assert(0);
    }
    if (lolist->empty() ) {
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
    
    // cereate the current node  dim     value    parent  hi   lo   trilist  level
    KDNode2 *node = new KDNode2(spr->d, cutvalue, parent, NULL,NULL,NULL, level);
    
    // create the child-nodes:
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
    //Bbox minvals;
    //Bbox maxvals;
    std::vector<double> maxval(6);
    std::vector<double> minval(6);
    if ( tris->empty() ) {
        std::cout << "kdtree2.cpp ERROR, spread() called with tris->size()==0 ! \n";
        assert( 0 );
        return NULL;
    } else {
        bool first=true;
        BOOST_FOREACH(Triangle t, *tris) {
            for (unsigned int m=0;m<6;++m) {
                if (first) {
                    maxval[m] = t.bb[m];
                    minval[m] = t.bb[m];
                    if (m==5)
                        first=false;
                } else {
                    if (maxval[m] < t.bb[m] )
                        maxval[m] = t.bb[m];
                    if (minval[m] > t.bb[m])
                        minval[m] = t.bb[m];
                }
            }
        } 
        
        // calculate the spread along each dimension
        // put the spreads in a list
        std::vector<Spread2*> spreads;
        for (unsigned int m=0;m<6;++m) {   // dim,  spread, start
            spreads.push_back( new Spread2(m , maxval[m]-minval[m], minval[m] ) );  
        }
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
        if (cutcount == 6)
            cutcount = 0;
        // return spreads[ cutcount ]; // select each dim in turn
        
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
                            const Bbox& bb,                  // bbox for search
                            KDNode2 *node,                   // start search here and recurse into tree
                            const unsigned int plane)        // the search plane, defaults to 0 = all.
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
                // THIS IS PROBABLY WRONG, should not have to do an explicit overlap check here
                // if ( cutter.overlaps(cl,t) ) { // explicit cutter-overlap check
                    tris->push_back(t); 
                //}
            } // loop through triangles
        //} // node overlap check
        return;
    } // end bucket-node
    
    // search in the KD-tree only if the plane variable says so.
    // plane=0  means search all dimensions            dims=[ 0 1 2 3 4 5 ]
    // plane=1 search in the yz plane (perp to x-dir)  dims=[ _ _ 2 3 4 5 ]
    // plane=2 search in the xz plane (perp to y-dir)  dims=[ 0 1 _ _ 4 5 ]
    // plane=3 search in the xy plane (perp to z-dir)  dims=[ 0 1 2 3 _ _ ]
    
    // not a bucket node, so recursively search hi/lo branches of KDNode
    if ( (node->dim % 2) == 0) { // cutting along a min-direction: 0, 2, 4
        unsigned int maxdim = node->dim+1;
        if ( (KDNode2::plane_valid(node->dim,plane)) && (node->cutval > bb[maxdim]) ) { // search only lo
            KDNode2::search_kdtree(tris, bb, node->lo, plane);
        } else { // need to search both child nodes
            KDNode2::search_kdtree(tris, bb, node->hi, plane);
            KDNode2::search_kdtree(tris, bb, node->lo, plane);
        }
    } else { // cutting along a max-dimension: 1,3,5
        unsigned int mindim = node->dim-1;
        if ( (KDNode2::plane_valid(node->dim,plane)) && (node->cutval < bb[mindim]) ) { // search only hi
            KDNode2::search_kdtree(tris, bb, node->hi, plane);
        } else { // need to search both child nodes
            KDNode2::search_kdtree(tris, bb, node->hi, plane);
            KDNode2::search_kdtree(tris, bb, node->lo, plane);
        }
    }
    return; // Done. We get here after all the recursive calls above.
} // end search_kdtree()

bool KDNode2::plane_valid(const unsigned int dim,const unsigned int plane) {
    // search in the KD-tree only if the plane variable says so.
    // plane=0  means search all dimensions            dims=[ 0 1 2 3 4 5 ]
    // plane=1 search in the yz plane (perp to x-dir)  dims=[ _ _ 2 3 4 5 ]
    // plane=2 search in the xz plane (perp to y-dir)  dims=[ 0 1 _ _ 4 5 ]
    // plane=3 search in the xy plane (perp to z-dir)  dims=[ 0 1 2 3 _ _ ]
    switch (dim) {
        case 0: 
            return (plane!=1); 
            break;
        case 1: 
            return (plane!=1); 
            break;
        case 2: 
            return (plane!=2); 
            break;
        case 3: 
            return (plane!=2); 
            break;
        case 4: 
            return (plane!=3); 
            break;
        case 5: 
            return (plane!=3); 
            break;
        default:
            assert(0);
            return false;
            break;    
    }
}

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
