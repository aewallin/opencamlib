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


#ifndef CONE_CUTTER_H
#define CONE_CUTTER_H

#include <iostream>
#include <string>
#include <vector>

#include <boost/python.hpp>
#include "millingcutter.h"

namespace ocl
{

class Triangle;
class STLSurf;

/* ********************************************************************
 *  CONE        conecutter.cpp
 * ********************************************************************/
/// \brief Conical MillingCutter 
///
/// cone defined by diameter and the cone half-angle(in radians). sharp tip. 
/// 60 degrees or 90 degrees are common
class ConeCutter : public MillingCutter {
    public:
        /// default constructor
        ConeCutter();
        /// create a ConeCutter with specified maximum diameter and cone-angle
        /// for a 90-degree cone specify the half-angle  angle= pi/4
        ConeCutter(const double d, const double angle);
        
        MillingCutter* offsetCutter(const double d) const;
        // drop-cutter
        int vertexDrop(CLPoint &cl, const Triangle &t) const;
        int facetDrop(CLPoint &cl, const Triangle &t) const;
        int edgeDrop(CLPoint &cl, const Triangle &t) const;
        
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, ConeCutter c);
        std::string str() const;
        
    protected:
        /// the half-angle of the cone, in radians
        double angle;
        /// the height of the cone
        double height;
};

} // end namespace
#endif
