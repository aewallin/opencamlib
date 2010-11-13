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

#ifndef KDNODE3_H
#define KDNODE3_H

#include <iostream>
#include <list>

namespace ocl
{
    
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
        KDNode3(int d, double cv, KDNode3<BBObj> *parentNode,                        // parent node
                                  KDNode3<BBObj> *hi_child,                        // hi-child
                                  KDNode3<BBObj> *lo_child,                        // lo-child
                                  const std::list< BBObj >* tlist,     // list of tris, if bucket
                                  int nodeDepth)                           // depth of node
                                  {
            dim = d;
            cutval = cv;
            parent = parentNode;
            hi = hi_child;
            lo = lo_child;
            tris = new std::list<BBObj>();
            depth = nodeDepth;
            isLeaf = false;
            if (tlist) {
                isLeaf = true;
                BOOST_FOREACH(BBObj bo, *tlist) {
                    tris->push_back(bo);
                }
            }
        }
        virtual ~KDNode3() {
            delete hi;
            delete lo;
            delete tris;
        }
        /// string repr
        std::string str() const {
            std::ostringstream o;
            o << "KDNode d:" << dim << " cv:" << cutval; 
            return o.str();
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
        KDNode3* parent; 
        /// Child-node hi.
        KDNode3* hi; 
        /// Child-node lo.
        KDNode3* lo; 
        /// A list of triangles, if this is a bucket-node (NULL for internal nodes)
        std::list< BBObj >* tris;
        bool isLeaf;
};


} // end namespace
#endif
// end file kdnode3.h
