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

#ifndef BBOX_H
#define BBOX_H

//#include <iostream>
//#include <list>
//#include <vector>

#include "point.h"

namespace ocl
{
    
class Triangle;

/// axis-aligned bounding-box
class Bbox {
    public:
        /// default constructor
        Bbox();
        virtual ~Bbox();
        
        Point maxpt; /// the maximum point
        Point minpt; /// the minimum point
               
        /// return true if Point p is inside this Bbox
        bool isInside(Point& p) const;
        
        /// Add a Point to the Bbox.
        /// This enlarges the Bbox so that p is contained within it.
        void addPoint(const Point& p);
        
        /// Add each vertex of a Triangle to the Bbox.
        /// This enlarges the Bbox so that the Triangle is contained within it.
        void addTriangle(const Triangle& t);
};


} // end namespace
#endif
// end file bbox.h
