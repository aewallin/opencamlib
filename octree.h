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

#ifndef OCTREE_H
#define OCTREE_H

#include <iostream>
#include <list>
#include <vector>

class Point;
class Triangle;
class MillingCutter;

/// type of cc-point
enum OCType {BLACK, GREY, WHITE};

/// \brief A node in a octree. http://en.wikipedia.org/wiki/Octree
///
class OCTNode {
    public:
        /// empty default constructor
        OCTNode();
        
        /// Create a node 
        OCTNode(int level, Point &center, OCType type, KDNode *parent,
                                std::vector<OCTNode*> childvec);
                                
        /// string repr
        std::string str();
        /// string repr
        friend std::ostream &operator<<(std::ostream &stream, const OCTNode node);
        
        
        /// level of node in tree 
        int level;
        /// center Point of node
        Point center;
        /// type of node
        OCType type;
        /// pointer to parent-node
        OCTNode *parent;
        /// pointers to the eight Child-nodes
        std::vector<OCTNode*> child;
            
        static Point maxextent;
        static Point minextent;
        
        /* static functions to build and search KD-trees) */
        /// build a kd-tree from a list of triangles. return root of tree.
        static OCTNode* build_octree();
};



#endif
