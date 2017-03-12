/*  
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
#ifndef INTERVAL_HPP
#define INTERVAL_HPP

#include <vector>
//#include <pair>

#include "ccpoint.hpp"
//#include "weave_typedef.hpp"
#include "halfedgediagram.hpp"

namespace ocl {

class Fiber;

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
        /// call both updateUpper() and updateLower() with the given (t,p) pair
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
        
        
        CCPoint upper_cc; ///< cutter contact points at upper and lower are stored in upper_cc and lower_cc
        CCPoint lower_cc; ///< cutter contact point corresponding to lower
        double upper;  ///< the upper t-value 
        double lower; ///< the lower t-value
        bool in_weave;  ///< flag for use by Weave::build()
        std::set<std::vector<Fiber>::iterator> intersections_fibers; ///< fibers
        
        /// intersections with other intervals are stored in this set of
        /// VertexPairs of type std::pair<VertexDescriptor, double>
        
        typedef boost::adjacency_list_traits<boost::listS, 
                                     boost::listS, 
                                     boost::bidirectionalS, 
                                     boost::listS >::vertex_descriptor WeaveVertex;
        typedef std::pair< WeaveVertex, double > VertexPair;

        /// compare based on pair.second, the coordinate of the intersection
        struct VertexPairCompare {
            /// comparison operator
            bool operator() (const VertexPair& lhs, const VertexPair& rhs) const
            { return lhs.second < rhs.second ;}
        };

        /// intersections stored in this set (for rapid finding of neighbors etc)
        typedef std::set< VertexPair, VertexPairCompare > VertexIntersectionSet;
        
        // this is the same type as ocl::weave::VertexPairIterator, but redefined here anywhere
        typedef VertexIntersectionSet::iterator VertexPairIterator;    

        
        VertexIntersectionSet intersections2;
};

} // end namespace
#endif
// end file interval.hpp
