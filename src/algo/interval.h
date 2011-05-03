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
#ifndef INTERVAL_H
#define INTERVAL_H

#include <vector>

#include "ccpoint.h"
#include "weave_typedef.h"
#include "weave2_typedef.h"

namespace ocl
{

/// interval for use by fiber and weave
/// a parameter interval [upper, lower]
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
        /// call bot updateUpper() and updateLower() with the given (t,p) pair
        void update(const double t, CCPoint& p);
        /// update interval with t_cl and cc_tmp if cc_tmp is in the p1-p2 edge and condition==true
        bool update_ifCCinEdgeAndTrue( double t_cl, CCPoint& cc_tmp, const Point& p1, const Point& p2, bool condition);
        
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
        /// VertexPairs of type std::pair<VertexDescriptor, double>
        VertexIntersectionSet intersections; 
        weave2::VertexIntersectionSet intersections2;
};

} // end namespace
#endif
// end file interval.h
