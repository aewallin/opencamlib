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


#ifndef BULL_CUTTER_H
#define BULL_CUTTER_H

#include <iostream>
#include <string>
#include <vector>

#include <boost/python.hpp>
#include "millingcutter.h"

namespace ocl
{

/* ********************************************************************
 *  TORUS       bullcutter.cpp
 * ********************************************************************/
/// \brief Bull-nose or Toroidal MillingCutter (filleted endmill)
///
/// defined by the cutter diameter and by the corner radius
///
class BullCutter : public MillingCutter {
    public:
        /// Create bull-cutter with default diameter and corner radius.
        BullCutter();
        /// Create bull-cutter with diamter d and corner radius r.
        BullCutter(const double d, const double r);

        
        MillingCutter* offsetCutter(const double d) const;
        
        /// drop cutter
        //int facetDrop(CLPoint &cl, const Triangle &t) const;
        int edgeDrop(CLPoint &cl, const Triangle &t) const;
        
        // pushCutter methods   
        // bool vertexPush(const Fiber& f, Interval& i, const Triangle& t) const;
        bool facetPush(const Fiber& f, Interval& i, const Triangle& t) const;
        bool edgePush(const Fiber& f, Interval& i, const Triangle& t) const;
        
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, BullCutter c);
        /// string repr
        std::string str() const;
        
    protected:
        double height(const double r) const;
        double width(const double h) const; 
        double xy_normal_length() const;
        double normal_length() const;
        double center_height() const;
        
        /// set radius of cutter
        void setRadius();  
        /// radius of cylindrical part of cutter
        double radius1;
        /// tube radius of toroid
        double radius2;
};

} // end namespace
#endif
