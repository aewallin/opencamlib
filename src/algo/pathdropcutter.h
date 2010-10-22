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
        /// construct an empty PathDropCutter object
        PathDropCutter();
        virtual ~PathDropCutter();
        /// set the STLSurf surface for this operation
        void setSTL(const STLSurf& s);
        /// set the MillingCutter for this operation
        void setCutter(MillingCutter* cutter);
        /// set the Path to follow and sample
        void setPath(const Path* path);
        /// set the minimum z-value, or "floor" for drop-cutter
        void setZ(const double z) {minimumZ = z;};
        /// return Z
        double getZ() const {return minimumZ;};
        /// set the sampling-distance for the Path
        void setSampling(double s) {sampling=s;};
        /// run drop-cutter on the whole Path
        void run();
        
    protected:
        /// run drop-cutter on Span
        void sample_span(const Span* span);
        /// the path to follow
        const Path* path;
        /// the cutter used for this operation
        MillingCutter* cutter;
        /// the surface for this operation
        const STLSurf* surf;
        /// how closely to sample points from a Path, default is sampling=0.1
        double sampling;
        /// the BatchDropCutter object that runs drop-cutter on the sampled poitns
        BatchDropCutter* bdc;
        /// the lowest z height, used when no triangles are touched, default is minimumZ = 0.0
        double minimumZ;
        /// list of CL-points
        std::vector<CLPoint> clpoints;
};

} // end namespace
#endif
// end file pathdropcutter.h
