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
#ifndef INTERVAL_H
#define INTERVAL_H

#include <vector>

//#include "point.h"
#include "ccpoint.h"
//#include "numeric.h"
#include "weave_typedef.h"

namespace ocl
{

/// interval for use by fiber and weave
class Interval {
    public:
        Interval();
        /// create and interval [l,u]  (is this ever called??)
        Interval(const double l, const double u);
        virtual ~Interval() {};
        
        /// update upper with t, and corresponding cc-point p
        void updateUpper(const double t, CCPoint& p);
        /// update lower with t, and corresponding cc-point p
        void updateLower(const double t, CCPoint& p);
        /// return true if Interval i is outside *this
        bool outside(const Interval& i) const;
        /// return true if Interval i is inside *this
        bool inside(const Interval& i) const;
        /// return true if the interval is empty
        bool empty() const;
        /// string repr
        std::string str() const;
        
        /// cutter contact points at upper and lower are stored in upper_cc and lower_cc
        CCPoint upper_cc;
        /// cutter contact point correspoinding to lower
        CCPoint lower_cc;
        /// the upper t-value 
        double upper; 
        /// the lower t-value
        double lower;
        
        /// flag for use by Weave::build()
        bool in_weave; 
        /// intersections with other intervals are stored in this set of
        /// VertexPairs of type std::set<VertexDescriptor, double>
        std::set< VertexPair, VertexPairCompare > intersections; 
};


} // end namespace
#endif
// end file interval.h
