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

#ifndef PATHDROPCUTTER_H
#define PATHDROPCUTTER_H

#include <iostream>
#include <string>
#include <list>

#include "batchdropcutter.hpp"
#include "path.hpp"
#include "operation.hpp"

namespace ocl
{
    
class MillingCutter;
class STLSurf;
class Triangle;
//class KDNode;

///
/// \brief path drop cutter finish Path generation
class PathDropCutter : public Operation {
    public:
        /// construct an empty PathDropCutter object
        PathDropCutter();
        virtual ~PathDropCutter();
        /// set the Path to follow and sample
        void setPath(const Path* path);
        /// set the minimum z-value, or "floor" for drop-cutter
        void setZ(const double z) {
            minimumZ = z;
        }
        /// return Z
        double getZ() const {
            return minimumZ;
        }
        std::vector<CLPoint> getPoints() const
        {
            return clpoints;
        }
        /// run drop-cutter on the whole Path
        virtual void run();

        std::vector<CLPoint> getCLPoints() { return clpoints; }

    protected:
        /// the path to follow
        const Path* path;
        /// the lowest z height, used when no triangles are touched, default is minimumZ = 0.0
        double minimumZ;
        /// list of CL-points
        std::vector<CLPoint> clpoints;
    private:
        /// the algorithm
        void uniform_sampling_run();
        /// sample the span unfirormly with tolerance sampling
        void sample_span(const Span* span);
};

} // end namespace
#endif
// end file pathdropcutter.h
