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

#ifndef ADAPTIVEWATERLINE_H
#define ADAPTIVEWATERLINE_H

#include <iostream>
#include <string>
#include <vector>

#include <boost/foreach.hpp>

#include "waterline.h"
#include "fiber.h"

namespace ocl
{

class STLSurf;
class KDNode2;
class Triangle;
class MillingCutter;
class Span;

/// \brief a Waterline toolpath follows the shape of the model at a constant z-height in the xy-plane

class AdaptiveWaterline : public Waterline {
    public:
        /// create an empty Waterline object
        AdaptiveWaterline(); 
        virtual ~AdaptiveWaterline();
        /// set the minimum sampling interval
        void setMinSampling(double s) {min_sampling=s;}
        /// set the cosine limit for the flat() predicate
        void setCosLimit(double lim) {cosLimit=lim;}
        
        /// run the Waterline algorithm. setSTL, setCutter, setSampling, and setZ must
        /// be called before a call to run()
        void run();
        
    protected:
        /// adaptive waterline algorithm
        void adaptive_sampling_run();
        /// x-direction adaptive sampling
        void xfiber_adaptive_sample(const Span* span, double start_t, double stop_t, Fiber start_f, Fiber stop_f);
        /// y-direction adaptive sampling
        void yfiber_adaptive_sample(const Span* span, double start_t, double stop_t, Fiber start_f, Fiber stop_f);
        /// flatness predicate for fibers. Checks Fiber.size() and then calls flat() on cl-points
        bool flat( Fiber& start, Fiber& mid, Fiber& stop ) const;
        /// flatness predicate for cl-points. checks for angle metween start-mid-stop
        bool flat(Point start_cl, Point mid_cl, Point stop_cl) const;
        /// build weave and run face_traverse to get loops
        void weave_process();

    // DATA
        /// x-fibers for this operation
        std::vector<Fiber> xfibers;
        /// y-fibers for this operation
        std::vector<Fiber> yfibers;
        /// minimum x-coordinate
        double minx;
        /// maximum x-coordinate
        double maxx;
        /// minimum y-coordinate
        double miny;
        /// maximum y-coordinate
        double maxy;
        /// the minimum sampling interval when subdividing
        double min_sampling;
        /// the cosine limit value for cl-point flat(). In the constructor, cosLimit = 0.999 by default.
        double cosLimit;
};

} // end namespace

#endif
