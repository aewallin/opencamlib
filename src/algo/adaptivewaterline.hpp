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

#ifndef ADAPTIVEWATERLINE_HPP
#define ADAPTIVEWATERLINE_HPP

#include <iostream>
#include <string>
#include <vector>

#include <boost/foreach.hpp>

#include "waterline.hpp"
#include "fiber.hpp"

namespace ocl
{

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
        void run2();
        
        virtual void setSampling(double s) {
            sampling=s;
            min_sampling = sampling/10.0; // default to this when setMinSampling is not called
        }
        
        
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

    // DATA

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
