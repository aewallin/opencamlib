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
#include "kdtree3.h"

namespace ocl
{

//*********** KDTREE ***************************************************



void KDTree::setXYDimensions() {
    std::cout << "KDTree::setXYDimensions()\n"; 
    dimensions.clear();
    dimensions.push_back(0); // x
    dimensions.push_back(1); // x
    dimensions.push_back(2); // y
    dimensions.push_back(3); // y
}

void KDTree::setYZDimensions() { // for X-fibers
    std::cout << "KDTree::setYZDimensions()\n"; 
    dimensions.clear();
    dimensions.push_back(2); // y
    dimensions.push_back(3); // y
    dimensions.push_back(4); // z
    dimensions.push_back(5); // z
}

void KDTree::setXZDimensions() { // for Y-fibers
    std::cout << "KDTree::setXZDimensions()\n";
    dimensions.clear();
    dimensions.push_back(0); // x
    dimensions.push_back(1); // x
    dimensions.push_back(4); // z
    dimensions.push_back(5); // z
}

void KDTree::build() {
    //std::cout << "KDTree::build() tris.size()=" << surf->tris.size() 
    //          << " bucketSize=" << bucketSize << "\n";
    assert( !dimensions.empty() );
    root = build_node( &surf->tris, 0, NULL ); // build the root node (and recursively everything below)
}

// return triangles which overlap with Bbox bb
std::list<Triangle>* KDTree::search( const Bbox& bb ) {
    assert( !dimensions.empty() );
    std::list<Triangle>* tris = new std::list<Triangle>();
    search_node( tris, bb, root );
    return tris;
}

std::list<Triangle>* KDTree::search_cutter_overlap(const MillingCutter* c, CLPoint* cl ) {
    double r = c->getRadius();
    Bbox bb( cl->x- r, // build a bounding-box at the current CL
             cl->x+ r, 
             cl->y- r, 
             cl->y+ r,
             cl->z, 
             cl->z + c->getLength() );    
    //std::cout << " KDTree::search_cutter_overlap() \n";
    return search( bb );
}

void KDTree::search_node( std::list<Triangle>* tris,    // found triangles added to tris
                            const Bbox& bb,             // bbox for search
                            KDNode3<Triangle> *node)              // start search here and recurse into tree
{
    if (node->tris != NULL) { // we found a bucket node, so add all triangles and return.
            BOOST_FOREACH( Triangle t, *(node->tris) ) {
                    tris->push_back(t); 
            } 
        return; // end recursion
    } else if ( (node->dim % 2) == 0) { // cutting along a min-direction: 0, 2, 4
        // not a bucket node, so recursevily seach hi/lo branches of KDNode
        unsigned int maxdim = node->dim+1;
        if ( node->cutval > bb[maxdim] ) { // search only lo
            search_node(tris, bb, node->lo );
        } else { // need to search both child nodes
            search_node(tris, bb, node->hi );
            search_node(tris, bb, node->lo );
        }
    } else { // cutting along a max-dimension: 1,3,5
        unsigned int mindim = node->dim-1;
        if ( node->cutval < bb[mindim] ) { // search only hi
            search_node(tris, bb, node->hi);
        } else { // need to search both child nodes
            search_node(tris, bb, node->hi);
            search_node(tris, bb, node->lo);
        }
    }
    return; // Done. We get here after all the recursive calls above.
} // end search_kdtree()

// build the kd-tree 
KDNode3<Triangle>* KDTree::build_node(    const std::list<Triangle> *tris,      // triangles 
                                int dep,                        // depth of node
                                KDNode3<Triangle> *par)                   // parent-node
{
    if (tris->size() == 0 ) { //this is a fatal error.
        std::cout << "ERROR: KDTree::build_node() called with tris->size()==0 ! \n";
        assert(0);
        return 0;
    }
    // calculate spread in order to know how to cut
    Spread3* spr = calc_spread(tris); // call to static method which returns new object
    double cutvalue = spr->start + spr->val/2; // cut in the middle
    // if spr.val==0 (no need/possibility to subdivide anymore)
    // OR number of triangles is smaller than bucketSize 
    // then return a bucket/leaf node
    if ( (tris->size() <= bucketSize) || (spr->val == 0.0)) {
        KDNode3<Triangle> *bucket;   //  dim   cutv   parent   hi    lo   triangles depth
        bucket = new KDNode3<Triangle>(spr->d, 0.0 , par , NULL, NULL, tris, dep);
        return bucket; // this is the leaf/end of the recursion-tree
    }
    // build lists of triangles for hi and lo child nodes
    std::list<Triangle>* lolist = new std::list<Triangle>();
    std::list<Triangle>* hilist = new std::list<Triangle>();
    BOOST_FOREACH(Triangle t, *tris) { // loop through each triangle and put it in either lolist or hilist
        if (t.bb[spr->d] > cutvalue) 
            hilist->push_back(t);
        else
            lolist->push_back(t);
    } 
    if (hilist->empty() ) {// an error ??
        std::cout << "kdtree: hilist.size()==0!\n";
        assert(0);
    }
    if (lolist->empty() ) {
        std::cout << "kdtree: lolist.size()==0!\n";
        assert(0);
    }
    // cereate the current node  dim     value    parent  hi   lo   trilist  depth
    KDNode3<Triangle> *node = new KDNode3<Triangle>(spr->d, cutvalue, par, NULL,NULL,NULL, dep);
    // create the child-nodes through recursion
    //                    list    depth   parent
    node->hi = build_node(hilist, dep+1, node); // FIXME? KDTree has direct access to hi and lo (bad?)
    node->lo = build_node(lolist, dep+1, node);    
    return node; // return a new node
}

/// find the maximum 'extent' of Triangle list tris along dimension d
Spread3* KDTree::calc_spread(const std::list<Triangle> *tris) {
    std::vector<double> maxval( 6 );
    std::vector<double> minval( 6 );
    assert( !dimensions.empty() );
    //std::vector<Bbox> values( dimensions.size() );
    if ( tris->empty() ) {
        std::cout << " ERROR, KDTree::calc_spread() called with tris->size()==0 ! \n";
        assert( 0 );
        return NULL;
    } else {
        // find out the maximum spread
        bool first=true;
        BOOST_FOREACH(Triangle t, *tris) { // check each triangle
            for (unsigned int m=0;m<dimensions.size();++m) {
                
                // dimensions[m] is the dimensions we want to update
                // t.bb[ dimensions[m] ]   is the update value
                
                if (first) {
                    maxval[ dimensions[m] ] = t.bb[ dimensions[m] ];
                    minval[ dimensions[m] ] = t.bb[ dimensions[m] ];
                    if (m==(dimensions.size()-1) )
                        first=false;
                } else {
                    if (maxval[ dimensions[m] ] < t.bb[ dimensions[m] ] )
                        maxval[ dimensions[m] ] = t.bb[ dimensions[m] ];
                    if (minval[ dimensions[m] ] > t.bb[ dimensions[m] ])
                        minval[ dimensions[m] ] = t.bb[ dimensions[m] ];
                }
            }
        } 
        // calculate the spread along each dimension
        // put the spreads in a list
        std::vector<Spread3*> spreads;
        for (unsigned int m=0;m<dimensions.size();++m) {   // dim,  spread, start
            spreads.push_back( new Spread3(dimensions[m] , maxval[dimensions[m]]-minval[dimensions[m]], minval[dimensions[m]] ) );  
        }
        std::sort(spreads.begin(), spreads.end(), Spread3::spread_compare); // sort the list
        // priority-queue could also be used ??
        /*
        std::cout << "\n";
        std::cout <<"spreads for " << tris->size() << " triangles:\n";
        std::cout << " selecting " << (spreads[0])->d << " with s="<< (spreads[0])->val << "\n";
        char c;
        std::cin >> c;
        */
        //cutcount++;
        //if (cutcount == 6)
        //    cutcount = 0;
        // return spreads[ cutcount ]; // select each dim in turn
        
        return spreads[ 0 ]; // select the biggest spread and return
    } // end tris->size != 0
} // end spread()


} // end namespace
// end file kdtree3.cpp
