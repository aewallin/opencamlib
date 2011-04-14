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

#ifndef MARCHING_CUBES_H
#define MARCHING_CUBES_H

#include <iostream>
#include <list>

#include <boost/foreach.hpp>

#include "point.h"
#include "triangle.h"
#include "bbox.h"
#include "octnode.h"
// #include "octree.h"

#include "numeric.h"

namespace ocl
{

/// Marching-cubes isosurface extraction from distance field stored in Octree
/// see http://en.wikipedia.org/wiki/Marching_cubes
///
/// (Note: there is no cpp file, all code is here in the .h file)
class MarchingCubes {
    public:
        MarchingCubes() { 
        }
        virtual ~MarchingCubes() {
        }
        

        // run mc on one Octnode
        std::vector<Triangle> mc_node(Octnode* node);
        // generate the interpolated vertices required for triangle construction
        std::vector<Point> interpolated_vertices(Octnode* node, unsigned int edges) ;
        // use linear interpolation of the distance-field between vertices idx1 and idx2
        // to generate a new iso-surface point on the idx1-idx2 edge
        Point interpolate(Octnode* node, int idx1, int idx2);

        
    protected:
        // based on the funcion values (positive or negative) at the corners of the node,
        // calculate the edgeTableIndex
        unsigned int mc_edgeTableIndex(Octnode* node) ;
        
        std::vector<Triangle> triangles;
        
        /// Marching-Cubes edge table
        static const unsigned int edgeTable[256];
        /// Marching-Cubes triangle table
        static const int triTable[256][16]; 
};


} // end namespace
#endif
// end file marching_cubes.h
