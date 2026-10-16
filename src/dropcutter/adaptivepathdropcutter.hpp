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

#ifndef ADAPTIVEPATHDROPCUTTER_H
#define ADAPTIVEPATHDROPCUTTER_H

#include <iostream>
#include <string>
#include <list>

#include <boost/foreach.hpp>

#include "pathdropcutter.hpp"
#include "pointdropcutter.hpp"
#include "path.hpp"
#include "clpoint.hpp"

namespace ocl
{
    
class MillingCutter;
class STLSurf;
class Triangle;


///
/// \brief path drop cutter finish Path generation
class AdaptivePathDropCutter : public Operation {
    public:
        /// construct an empty PathDropCutter object
        AdaptivePathDropCutter();
        virtual ~AdaptivePathDropCutter();
    
        /// run drop-cutter on the whole Path
        virtual void run();
        /// set the minimum sapling interval
        void setMinSampling(double s) {
            assert( s > 0.0 );
            //std::cout << " apdc::setMinSampling = " << s << "\n";
            min_sampling=s;
        }
        /// set the cosine limit for the flat() predicate
        void setCosLimit(double lim) {cosLimit=lim;}
        void setZ(const double z) {
            minimumZ = z;
        }
        double getZ() const {
            return minimumZ;
        }
        void setPath(const Path* p) {
            path = p;
            
            subOp[0]->clearCLPoints();
        }
        std::vector<CLPoint> getCLPoints() { return clpoints; }
        std::vector<CLPoint> getPoints() { return clpoints; } // strange naming

    protected:
        /// run adaptive sample on the given Span between t-values of start_t and stop_t
        void adaptive_sample(const Span* span, double start_t, double stop_t, CLPoint start_cl, CLPoint stop_cl);
        /// flatness predicate for adaptive sampling
        bool flat(CLPoint& start_cl, CLPoint& mid_cl, CLPoint& stop_cl);
        /// run adaptive sampling
        void adaptive_sampling_run();
    // DATA
        /// the smallest sampling interval used when adaptively subdividing
        double min_sampling;
        /// the limit for dot-product used in flat()
        double cosLimit;
        const Path* path;
        double minimumZ;
        std::vector<CLPoint> clpoints;
};

} // end namespace
#endif
// end file pathdropcutter.h
