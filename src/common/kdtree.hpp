/*  
 *  Copyright 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com)
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

#ifndef KDTREE_H
#define KDTREE_H

#include <iostream>
#include <list>

#include <boost/foreach.hpp>

#include "kdnode.hpp"
#include "bbox.hpp"
#include "millingcutter.hpp"
#include "clpoint.hpp"
#include "numeric.hpp"

namespace ocl
{
    
class Point;
class CLPoint;
class Triangle;
class MillingCutter;

/// \brief KDTree spread, a measure of how spread-out a list of triangles are.
///
/// simple struct-like class for storing the "spread" or maximum 
/// extent of a list of triangles. Used by the kd-tree algorithm.
class Spread {
    public:
        /// constructor
        Spread(int dim, double v, double s) {
            d = dim;
            val = v;
            start = s;
        };
        /// dimension
        int d;
        /// spread-value
        double val;
        /// minimum or start value
        double start;
        /// comparison of Spread objects. Used for finding the largest spread
        /// along which the next partition/cut is made.
        static bool spread_compare(Spread *x, Spread *y) {
            if (x->val > y->val)
                return true;
            else
                return false;
        };
};

/// a kd-tree for storing triangles and fast searching for triangles
/// that overlap the cutter
template <class BBObj>
class KDTree {
    public:
        KDTree() {};
        virtual ~KDTree() {
            // std::cout << " ~KDTree()\n";
            delete root;
        }
        /// set the bucket-size 
        void setBucketSize(int b){
            //std::cout << "KDTree::setBucketSize = " << b << "\n"; 
            bucketSize = b;
        }
        /// set the search dimension to the XY-plane
        void setXYDimensions(){
            //std::cout << "KDTree::setXYDimensions()\n"; 
            dimensions.clear();
            dimensions.push_back(0); // x
            dimensions.push_back(1); // x
            dimensions.push_back(2); // y
            dimensions.push_back(3); // y
        } // for drop-cutter search in XY plane
        /// set search-plane to YZ
        void setYZDimensions(){ // for X-fibers
            //std::cout << "KDTree::setYZDimensions()\n"; 
            dimensions.clear();
            dimensions.push_back(2); // y
            dimensions.push_back(3); // y
            dimensions.push_back(4); // z
            dimensions.push_back(5); // z
        } // for X-fibers
        /// set search plane to XZ
        void setXZDimensions(){ // for Y-fibers
            //std::cout << "KDTree::setXZDimensions()\n";
            dimensions.clear();
            dimensions.push_back(0); // x
            dimensions.push_back(1); // x
            dimensions.push_back(4); // z
            dimensions.push_back(5); // z
        } // for Y-fibers
        /// build the kd-tree based on a list of input objects
        void build(const std::list<BBObj>& list){
            //std::cout << "KDTree::build() list.size()= " << list.size() << " \n";
            root = build_node( &list, 0, NULL ); 
        }
        /// search for overlap with input Bbox bb, return found objects
        std::list<BBObj>* search( const Bbox& bb ){
            assert( !dimensions.empty() );
            std::list<BBObj>* tris = new std::list<BBObj>();
            this->search_node( tris, bb, root );
            return tris;
        }
        /// search for overlap with a MillingCutter c positioned at cl, return found objects
        std::list<BBObj>* search_cutter_overlap(const MillingCutter* c, CLPoint* cl ){
            double r = c->getRadius();
            // build a bounding-box at the current CL
            Bbox bb( cl->x-r, cl->x+r, cl->y-r, cl->y+r, cl->z, cl->z+c->getLength() );    
            return this->search( bb );
        }
        /// string repr
        std::string str() const;
        
    protected:
        /// build and return a KDNode containing list *tris at depth dep.
        KDNode<BBObj>* build_node(     const std::list<BBObj> *tris,  // triangles 
                                        int dep,                       // depth of node
                                        KDNode<BBObj> *par)   {       // parent node
            //std::cout << "KDNode::build_node list.size()=" << tris->size() << "\n";
            
            if (tris->size() == 0 ) { //this is a fatal error.
                std::cout << "ERROR: KDTree::build_node() called with tris->size()==0 ! \n";
                assert(0);
                return 0;
            }
            Spread* spr = calc_spread(tris); // calculate spread in order to know how to cut
            double cutvalue = spr->start + spr->val/2; // cut in the middle
            //std::cout << " cutvalue= " << cutvalue << "\n";
            if ( (tris->size() <= bucketSize) ||  isZero_tol( spr->val ) ) {  // then return a bucket/leaf node
                //std::cout << "KDNode::build_node BUCKET list.size()=" << tris->size() << "\n";
                KDNode<BBObj> *bucket;   //  dim   cutv   parent   hi    lo   triangles depth
                bucket = new KDNode<BBObj>(spr->d, cutvalue , par , NULL, NULL, tris, dep);
                assert( bucket->isLeaf );
                delete spr;
                return bucket; // this is the leaf/end of the recursion-tree
            }
            // build lists of triangles for hi and lo child nodes
            std::list<BBObj>* lolist = new std::list<BBObj>();
            std::list<BBObj>* hilist = new std::list<BBObj>();
            BOOST_FOREACH(BBObj t, *tris) { // loop through each triangle and put it in either lolist or hilist
                if (t.bb[spr->d] > cutvalue) 
                    hilist->push_back(t);
                else
                    lolist->push_back(t);
            } 
            
            /*
            if (hilist->empty() || lolist->empty()) {// an error ??
                std::cout << "kdtree: hilist.size()==0! or lolist.size()==0! \n";
                std::cout << "kdtree: tris->size()= " << tris->size()<< "\n";
                std::cout << "kdtree: hilist.size()= " << hilist->size()<< "\n";
                std::cout << "kdtree: lolist.size()= " << lolist->size()<< "\n";
                BOOST_FOREACH(BBObj t, *tris) {
                    std::cout << t << "\n";
                    std::cout << t.bb << "\n";
                }
                std::cout << "kdtree: spr->d= " << spr->d << "\n";
                std::cout << "kdtree: cutvalue= " << cutvalue << "\n";
                assert(0);
            }*/
            
            
            // create the current node  dim     value    parent  hi   lo   trilist  depth
            KDNode<BBObj> *node = new KDNode<BBObj>(spr->d, cutvalue, par, NULL,NULL,NULL, dep);
            // create the child-nodes through recursion
            //                    list    depth   parent
            if (!hilist->empty())
                node->hi = build_node(hilist, dep+1, node); 
            //else
                //std::cout << "hilist empty!\n";
                
            if (!lolist->empty()) {
                node->lo = build_node(lolist, dep+1, node); 
            } else {
                //std::cout << "lolist empty!\n";
            }
             
            lolist->clear();
            hilist->clear();
            delete spr;
            delete lolist;
            delete hilist;
            
            return node; // return a new node
        };
        
        /// calculate the spread of list *tris                        
        Spread* calc_spread(const std::list<BBObj> *tris) {
            std::vector<double> maxval( 6 );
            std::vector<double> minval( 6 );
            if ( tris->empty() ) {
                std::cout << " ERROR, KDTree::calc_spread() called with tris->size()==0 ! \n";
                assert( 0 );
                return NULL;
            } else {
                // find out the maximum spread
                //std::cout << "calc_spread()...\n";
                bool first=true;
                BOOST_FOREACH(BBObj t, *tris) { // check each triangle
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
                std::vector<Spread*> spreads;// calculate the spread along each dimension
                for (unsigned int m=0;m<dimensions.size();++m) {   // dim,  spread, start
                    spreads.push_back( new Spread(dimensions[m] , 
                                       maxval[dimensions[m]]-minval[dimensions[m]], 
                                       minval[dimensions[m]] ) );  
                }// priority-queue could also be used ??  
                assert( !spreads.empty() );
                //std::cout << " spreads.size()=" << spreads.size() << "\n";
                std::sort(spreads.begin(), spreads.end(), Spread::spread_compare); // sort the list
                Spread* s= new Spread(*spreads[0]); // this is the one we want to return
                while(!spreads.empty()) delete spreads.back(), spreads.pop_back(); // delete the others
                //std::cout << "calc_spread() done\n";
                return s; // select the biggest spread and return
            } // end tris->size != 0
        } // end spread();
        
        
        /// search kd-tree starting at *node, looking for overlap with bb, and placing
        /// found objects in *tris
        void search_node( std::list<BBObj> *tris, const Bbox& bb, KDNode<BBObj> *node) {
            if (node->isLeaf ) { // we found a bucket node, so add all triangles and return.
            
                BOOST_FOREACH( BBObj t, *(node->tris) ) {
                        tris->push_back(t); 
                } 
                //std::cout << " search_node Leaf bucket tris-size() = " << tris->size() << "\n";
                return; // end recursion
            } else if ( (node->dim % 2) == 0) { // cutting along a min-direction: 0, 2, 4
                // not a bucket node, so recursevily search hi/lo branches of KDNode
                unsigned int maxdim = node->dim+1;
                if ( node->cutval > bb[maxdim] ) { // search only lo
                    search_node(tris, bb, node->lo );
                } else { // need to search both child nodes
                    if (node->hi)
                        search_node(tris, bb, node->hi );
                    if (node->lo)
                        search_node(tris, bb, node->lo );
                }
            } else { // cutting along a max-dimension: 1,3,5
                unsigned int mindim = node->dim-1;
                if ( node->cutval < bb[mindim] ) { // search only hi
                    search_node(tris, bb, node->hi);
                } else { // need to search both child nodes
                    if (node->hi)
                        search_node(tris, bb, node->hi);
                    if (node->lo)
                        search_node(tris, bb, node->lo);
                }
            }
            return; // Done. We get here after all the recursive calls above.
        } // end search_kdtree();
    // DATA
        /// bucket size of tree
        unsigned int bucketSize;
        /// pointer to root KDNode
        KDNode<BBObj>* root;
        /// the dimensions in this kd-tree
        std::vector<int> dimensions;
};

} // end ocl namespace
#endif
// end file kdtree.hpp
