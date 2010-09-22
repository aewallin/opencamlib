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

#ifndef KDTREE3_H
#define KDTREE3_H

#include <iostream>
#include <list>
#include "clpoint.h"

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
class Spread3 {
    public:
        /// constructor
        Spread3(int dim, double v, double s);
        /// dimension
        int d;
        /// spread-value
        double val;
        /// minimum or start value
        double start;
        /// comparison of Spread objects. Used for finding the largest spread
        /// along which the next partition/cut is made.
        static bool spread_compare(Spread3 *x, Spread3 *y);
};

/// \brief K-D tree node. http://en.wikipedia.org/wiki/Kd-tree
///
/// A k-d tree is used for searching for triangles overlapping with the cutter.
///
template < class BBObj > 
class KDNode3 {
    public:
        /// Create a node which partitions(cuts) along dimension d, at 
        /// cut value cv, with child-nodes hi_c and lo_c.
        /// If this is a bucket-node containing triangles, 
        /// they are in the list tris
        /// depth indicates the depth of the node in the tree
        
        //template < class BBObj>
        KDNode3(int d, double cv, KDNode3<BBObj> *par,                        // parent node
                                  KDNode3<BBObj> *hi_c,                        // hi-child
                                  KDNode3<BBObj> *lo_c,                        // lo-child
                                  const std::list< BBObj > *tlist,     // list of tris, if bucket
                                  int dep)                           // depth of node
                                  {
            dim = d;
            cutval = cv;
            parent = par;
            hi = hi_c;
            lo = lo_c;
            tris = tlist;
            depth = dep;
        }
    // DATA
        /// level of node in tree 
        int depth;
        /// dimension of cut
        int dim;
        /// Cut value.
        /// Child node hi contains only triangles with a higher value than this.
        /// Child node lo contains triangles with lower values.
        double cutval;
        /// parent-node
        KDNode3 *parent; 
        /// Child-node hi.
        KDNode3 *hi; 
        /// Child-node lo.
        KDNode3 *lo; 
        /// A list of triangles, if this is a bucket-node (NULL for internal nodes)
        const std::list< BBObj > *tris;

        /// string repr
        //template < class BBObj >
        std::string str() const {
            std::ostringstream o;
            o << "KDNode d:" << dim << " cv:" << cutval; 
            return o.str();
        }
        // string repr
        //friend std::ostream &operator<<(std::ostream &stream, const KDNode3 node);
        
};

// template this on the object stored in the tree, could be triangles or spans
class KDTree {
    public:
        KDTree() {};
        virtual ~KDTree() {};
        void setSTL(const STLSurf &s);
        void setBucketSize(int b);
        void setXYDimensions(); // for drop-cutter search in XY plane
        void setYZDimensions(); // for X-fibers
        void setXZDimensions(); // for Y-fibers
        void build();
        std::list<Triangle>* search( const Bbox& bb );
        std::list<Triangle>* search_cutter_overlap(const MillingCutter* c, CLPoint* cl );

        /// string repr
        std::string str() const;
        
    protected:
        KDNode3<Triangle>* build_node(    const std::list<Triangle> *tris,// triangles 
                                int dep,                        // depth of node
                                KDNode3<Triangle> *parent);               // parent-node
        Spread3* calc_spread(const std::list<Triangle> *tris);
        void search_node(   std::list<Triangle> *tris, 
                            const Bbox& bb, 
                            KDNode3<Triangle> *root);
        unsigned int bucketSize;
        
        const STLSurf* surf; // needed as state? or only during build?
        
        KDNode3<Triangle>* root;
        /// the dimensions in this kd-tree
        std::vector<int> dimensions;
};

} // end namespace
#endif
// end file kdtree3.h
