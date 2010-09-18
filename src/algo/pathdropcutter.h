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

#ifndef PATHDROPCUTTER_H
#define PATHDROPCUTTER_H

#include <iostream>
#include <string>
#include <list>

#include "batchdropcutter.h"
#include "path.h"

namespace ocl
{
    
class MillingCutter;
class STLSurf;
class Triangle;
class KDNode;

///
/// \brief path drop cutter finish Path generation
class PathDropCutter {
    public:
        /// constructor
        PathDropCutter();
        virtual ~PathDropCutter();
        // create a PathDropCutter operation with surface surf.
        //PathDropCutter(const STLSurf *surf); // consider replacing with setSTL method
        void setSTL(const STLSurf& s);
            
        /// run drop-cutter on the whole Path
        void run();

        /// set the cutter
        void setCutter(const MillingCutter* cutter);
        /// set the path
        void setPath(const Path* path);
        void setZ(const double z) {minimumZ = z;};
        double getZ() const {return minimumZ;};
        
    protected:
        /// run drop-cutter on Span
        void run(const Span* span);
        
        /// the path to follow
        const Path* path;
        /// the cutter used for this operation
        const MillingCutter* cutter;
        /// the surface for this operation
        const STLSurf* surf;
        // /// root of a kd-tree that holds the triangles of the surface
        // KDNode *root;
        BatchDropCutter* bdc;
        /// the lowest z height, used when no triangles are touched
        double minimumZ;
        /// list of CL-points
        std::vector<CLPoint> clpoints;
};

} // end namespace
#endif
// end file pathdropcutter.h
