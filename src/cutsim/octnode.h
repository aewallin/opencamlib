/*  $Id$
 * 
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

#ifndef OCTNODE_H
#define OCTNODE_H

#include <iostream>
#include <list>

#include "point.h"
#include "volume.h"
#include "triangle.h"
#include "bbox.h"

namespace ocl
{

/// Octnode represents a node in the octree.
///
/// each node in the octree is a cube with side length scale
/// the distance field at each corner vertex is stored.
class Octnode {
    public:
        Octnode(){};
        /// create suboctant idx of parent with scale nodescale and depth nodedepth
        Octnode(Octnode* parent, unsigned int idx, double nodescale, unsigned int nodedepth);
        virtual ~Octnode();
        
        /// create all eight children of this node
        void subdivide(); // create children
        /// evaluate the vol.dist() function for this node
        void evaluate(const OCTVolume* vol);

    // DATA
        /// pointers to child nodes
        std::vector<Octnode*> child;
        /// pointer to parent node
        Octnode* parent;
        /// number of children
        unsigned int childcount;
        /// The eight corners of this node
        std::vector<Point*> vertex; 
        /// value of implicit function at vertex
        std::vector<double> f; 
        /// flag set true if this node is outside
        bool outside;
        /// flag for inside node
        bool inside;
        /// bool-array to indicate surface status of the faces
        std::vector<bool> surface; // flag for surface triangles FIXME!
        /// the center point of this node
        Point* center; // the centerpoint of this node
        /// the tree-dept of this node
        unsigned int depth; // depth of node
        /// the index of this node [0,7]
        unsigned int idx; // index of node
        /// the scale of this node, i.e. distance from center out to corner vertices
        double scale; // distance from center to vertices
        /// flag for checking if evaluate() has run
        bool evaluated;
        /// bounding-box corresponding to this node
        Bbox bb;
        /// vector for storing marching-cubes triangles
        std::vector<Triangle> mc_tris;
        /// flag for telling if mc-triangles have been calculated and are valid
        bool mc_tris_valid;
        
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, const Octnode &o);
        /// string repr
        std::string str() const;

    protected:
        /// print out surfaces
        void print_surfaces();
        /// inherit the surface-property from a parent 
        void set_surfaces();
        
        /// interpolate a point between vertex idx1 and idx2. used by marching-cubes
        Point interpolate(int idx1, int idx2);
        /// return center of child with index n
        Point* childcenter(int n); // return position of child centerpoint
        
    // STATIC
        /// the direction to the vertices, from the center 
        static Point direction[8];
        
};

} // end namespace
#endif
// end file octnode.h
