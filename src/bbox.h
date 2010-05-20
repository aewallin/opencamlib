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

#include <iostream>
#include <list>
#include <vector>

namespace ocl
{
    
class Point;

/// bounding-box
class Bbox {
    public:
        /// default constructor
        Bbox();
        
        /// maximum x-coordinate
        double maxx;
        /// minimum x-coordinate
        double minx;
        
        /// maximum y-coordinate
        double maxy;
        /// minimum y-coordinate
        double miny;
        
        /// maximum z-coordinate
        double maxz;
        /// minimum z-coordinate
        double minz;
        
        /// return true if Point p is inside this Bbox
        bool isInside(Point& p) const;
        
        /// add a Point to the Bbox
        void addPoint(Point& p);
};


} // end namespace
#endif
// end file bbox.h
