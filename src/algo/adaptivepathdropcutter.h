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

#ifndef ADAPTIVEPATHDROPCUTTER_H
#define ADAPTIVEPATHDROPCUTTER_H

#include <boost/python.hpp>
#include <boost/foreach.hpp>

#include <iostream>
#include <string>
#include <list>

#include "pathdropcutter.h"
#include "pointdropcutter.h"
#include "path.h"
#include "clpoint.h"

namespace ocl
{
    
class MillingCutter;
class STLSurf;
class Triangle;
class KDNode;

///
/// \brief path drop cutter finish Path generation
class AdaptivePathDropCutter : public PathDropCutter{
    public:
        /// construct an empty PathDropCutter object
        AdaptivePathDropCutter();
        virtual ~AdaptivePathDropCutter();
        /// set the STLSurf surface for this operation
        virtual void setSTL(const STLSurf& s);
        void setCutter(MillingCutter* cutter);        
        /// run drop-cutter on the whole Path
        virtual void run();
        void setMinSampling(double s) {min_sampling=s;}
        
    protected:
        void adaptive_sample(const Span* span, double start_t, double stop_t, CLPoint start_cl, CLPoint stop_cl);
        /// flatness predicate for adaptive sampling
        bool flat(CLPoint& start_cl, CLPoint& mid_cl, CLPoint& stop_cl);
        void adaptive_sampling_run();
        PointDropCutter* pdc;
        double min_sampling;
};

/// Python wrapper for PathDropCutter
class AdaptivePathDropCutter_py : public AdaptivePathDropCutter {
    public:
        AdaptivePathDropCutter_py() : AdaptivePathDropCutter() {};
        /// return a list of CL-points to python
        boost::python::list getCLPoints() {
            boost::python::list plist;
            BOOST_FOREACH(CLPoint p, clpoints) {
                plist.append(p);
            }
            return plist;
        };
};

} // end namespace
#endif
// end file pathdropcutter.h
