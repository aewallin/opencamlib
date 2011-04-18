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
#include <set>

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
        void setValid() {
            isosurface_valid = true;
            // try to propagate valid up the tree:
            if (parent) {
                for (int m=0;m<8;++m) {
                    if ( parent->hasChild(m) ) {
                        if ( ! (parent->child[m]->valid() ) ) {
                            return;
                        }
                    }
                }
                // if we get here, there was a parent and all children are valid
                parent->setValid();
            }
        }
        void setInValid() { 
            isosurface_valid = false;
            if ( parent && !parent->valid() ) // update parent status also
                parent->setInValid();
        }
        bool valid() const {
            return isosurface_valid;
        }
        bool surface() const { // surface nodes are neither inside nor outside
            return ( !inside && !outside );
        }
        bool hasChild(int n) {
            return (this->child[n] != NULL);
        }
        bool isLeaf() {return childcount==0;}
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
    
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, const Octnode &o);
        /// string repr
        std::string str() const;
        
        void addIndex(unsigned int id) { 
            std::set<unsigned int>::iterator found = vertexSet.find( id );
            assert( found == vertexSet.end() ); // we should not have id
            
            vertexSet.insert(id); 
        }
        void swapIndex(unsigned int oldId, unsigned int newId) {
            std::set<unsigned int>::iterator found = vertexSet.find(oldId);
            assert( found != vertexSet.end() ); // we must have oldId
            
            vertexSet.erase(oldId);
            vertexSet.insert(newId);
        }
        void removeIndex(unsigned int id) {
            std::set<unsigned int>::iterator found = vertexSet.find( id );
            assert( found != vertexSet.end() ); // we must have id
            
            vertexSet.erase(id);
        }
        

        // the vertex indices that this node produces
        std::set<unsigned int> vertexSet;

        

        
    protected:   
        
        /// return center of child with index n
        Point* childcenter(int n); // return position of child centerpoint
// DATA
        /// flag for telling isosurface extraction is valid for this node
        /// if false, the node needs updating.
        bool isosurface_valid;
// STATIC
        /// the direction to the vertices, from the center 
        static Point direction[8];
        
};

} // end namespace
#endif
// end file octnode.h
