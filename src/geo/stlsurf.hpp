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
#ifndef STLSURF_H
#define STLSURF_H

#include <list>

#include "triangle.hpp"
#include "bbox.hpp"

namespace ocl
{
    
class Point;

/// \brief STL surface, essentially an unordered list of Triangle objects
///
/// STL surfaces consist of triangles. There is by definition no structure
/// or order among the triangles, i.e. they can be positioned or connected in arbitrary ways.
class STLSurf {
    public:
        /// Create an empty STL-surface
        STLSurf() {};
        /// destructor
        virtual ~STLSurf() {};
        /// add Triangle t to this surface
        void addTriangle(const Triangle& t);
        /// return number of triangles in surface
        unsigned int size() const;
        /// call Triangle::rotate on all triangles
        void rotate(double xr,double yr, double zr);
        /// list of Triangles in this surface
        std::list<Triangle> tris; 
        /// bounding-box
        Bbox bb;
        /// STLSurf string repr
        friend std::ostream &operator<<(std::ostream& stream, const STLSurf s);
};

} // end namespace
#endif
// end file stlsurf.h
