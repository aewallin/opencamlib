/*  $Id$
 * 
 *  Copyright (c) 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com).
 *  
 *  This file is part of OpenCAMlib 
 *  (see https://github.com/aewallin/opencamlib).
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef KDNODE_H
#define KDNODE_H

#include <iostream>
#include <sstream>
#include <string>

#include <list>

namespace ocl
{
    
/// \brief K-D tree node. http://en.wikipedia.org/wiki/Kd-tree
///
/// A k-d tree is used for searching for triangles overlapping with the cutter.
///
template < class BBObj > 
class KDNode {
    public:
        /// Create a node which partitions(cuts) along dimension d, at 
        /// cut value cv, with child-nodes hi_c and lo_c.
        /// If this is a bucket-node containing triangles, 
        /// they are in the list tris
        /// depth indicates the depth of the node in the tree
        KDNode(int d, double cv,  KDNode<BBObj> *parentNode,                        // parent node
                                  KDNode<BBObj> *hi_child,                        // hi-child
                                  KDNode<BBObj> *lo_child,                        // lo-child
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
                for (BBObj bo : *tlist) {
                    tris->push_back(bo);
                }
            }
        }
        virtual ~KDNode() {
            // std::cout << " ~KDNode3()\n";
            if (hi)
                delete hi;
            if (lo)
                delete lo;
            if (tris)
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
        KDNode* parent; 
        /// Child-node hi.
        KDNode* hi; 
        /// Child-node lo.
        KDNode* lo; 
        /// A list of triangles, if this is a bucket-node (NULL for internal nodes)
        std::list< BBObj >* tris;
        /// flag to indicate leaf in the tree. Leafs or bucket-nodes contain triangles in the list tris.
        bool isLeaf;
};


} // end namespace
#endif
// end file kdnode.h
