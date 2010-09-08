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

#ifndef OCTREE_H
#define OCTREE_H

#include <iostream>
#include <list>

#include "point.h"
//#include "volume.h"
#include "triangle.h"
#include "bbox.h"
//#include "octnode.h"

namespace ocl
{

class Octnode;
class OCTVolume;

/// octree class which stores the root-node and allows operations on the tree
class Octree {
    public:
        Octree() { assert(0); };
        virtual ~Octree();
        /// create an octree with a root node with scale=root_scale, maximum
        /// tree-depth of max_depth and centered at centerp.
        Octree(double root_scale, unsigned int max_depth, Point& centerp);
        /// subtract vol from tree
        void diff_negative_root(const OCTVolume* vol);
        /// return the leaf-nodes
        void get_leaf_nodes(Octnode* current, std::vector<Octnode*>& nodelist) const;
        /// return all nodes in tree
        void get_all_nodes(Octnode* current, std::vector<Octnode*>& nodelist) const;
        /// run marching-cubes on the tree
        std::vector<Triangle> mc();
        /// generate the side-triangles
        std::vector<Triangle> side_triangles();
        
        /// initialize by recursively calling subdivide() on all nodes n times
        void init(const unsigned int n);
        /// return the maximum depth of the tree
        unsigned int get_max_depth() const;
        /// return the root scale
        double get_root_scale() const;
        /// return the leaf scale (the minimum resolution of the tree)
        double leaf_scale() const;
        /// string output
        std::string str() const;
        
    // python interface
        /// return python-list of leaf nodes
        boost::python::list py_get_leaf_nodes() const;
        /// return python-list of marching-cubes triangles
        boost::python::list py_mc_triangles(); 
        /// return python-list of side-trianges
        boost::python::list py_s_triangles(); 
        
    protected:
        /// recursively traverse the tree subtracting vol
        void diff_negative(Octnode* current, const OCTVolume* vol);
    // DATA
        /// the root scale
        double root_scale;
        /// the maximum tree-depth
        unsigned int max_depth;
        /// pointer to the root node
        Octnode* root;
};

} // end namespace
#endif
// end file octree2.h
