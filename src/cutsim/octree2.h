/*  $Id:  $
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

namespace ocl
{

/// octree node
class Octnode {
    public:
        Octnode(Octnode* parent, Point& centerp, double nodescale, unsigned int nodedepth);
        virtual ~Octnode() {};
        
        void subdivide(); // create children
        Point childcenter(int n); // return position of child centerpoint
        void setvertices(); // set vertices[]
        
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, const Octnode &o);
        /// string repr
        std::string str() const;
        
        /// pointer to children
        Octnode* child[8];
        Octnode* parent;
        Point* vertex[8]; // eight corners of this node
        double f[8]; // value of implicit function at vertex
        bool surface[6]; // flag for surface triangles
        Point* center; // the centerpoint of this node
        unsigned int depth; // depth of node
        double scale; // distance from center to vertices
        
        /// the direction to the vertices, from the center 
        static Point direction[8];
};

} // end namespace
#endif
// end file octree2.h
