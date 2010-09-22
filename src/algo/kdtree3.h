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
#include "kdnode3.h"

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
        Spread3(int dim, double v, double s) {
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
        static bool spread_compare(Spread3 *x, Spread3 *y) {
            if (x->val > y->val)
                return true;
            else
                return false;
        };
};


// template this on the object stored in the tree, could be triangles or spans
class KDTree {
    public:
        KDTree() {};
        virtual ~KDTree() {};
        void setSTL(const STLSurf &s){surf = &s;};
        void setBucketSize(int b){bucketSize = b;};
        void setXYDimensions(); // for drop-cutter search in XY plane
        void setYZDimensions(); // for X-fibers
        void setXZDimensions(); // for Y-fibers
        void build();
        std::list<Triangle>* search( const Bbox& bb );
        std::list<Triangle>* search_cutter_overlap(const MillingCutter* c, CLPoint* cl );
        /// string repr
        std::string str() const;
        
    protected:
        KDNode3<Triangle>* build_node(    const std::list<Triangle> *tris,  // triangles 
                                int dep,                                    // depth of node
                                KDNode3<Triangle> *parent);                 // parent-node
                                
        Spread3* calc_spread(const std::list<Triangle> *tris);
        void search_node(   std::list<Triangle> *tris, 
                            const Bbox& bb, 
                            KDNode3<Triangle> *root);
    // DATA
        unsigned int bucketSize;
        const STLSurf* surf; // needed as state? or only during build?
        KDNode3<Triangle>* root;
        /// the dimensions in this kd-tree
        std::vector<int> dimensions;
};

} // end namespace
#endif
// end file kdtree3.h
