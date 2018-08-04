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
#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <vector>

#include "point.hpp"
#include "bbox.hpp"

namespace ocl
{

///
/// \brief a Triangle defined by its three vertices
///
class Triangle {
    public:
        /// default constructor
        Triangle();
        /// copy constructor
        Triangle(const Triangle &t);
        
        /// destructor
        virtual ~Triangle() {}
        /// Create a triangle with the vertices p1, p2, and p3.
        Triangle(Point p1, Point p2, Point p3);   
        
        /// return true if Triangle is sliced by a z-plane at z=zcut
        /// modify p1 and p2 so that they are intesections of the triangle edges
        /// and the plane. These vertices are used by CylCutter::edgePush()
        bool zslice_verts(Point& p1, Point& p2, double zcut) const;
        
        /// rotate triangle xrot radians around X-axis, yrot radians around Y-axis
        /// and zrot radians around Z-axis
        void rotate(double xrot, double yrot, double zrot);
        
        /// Triangle string repr     
        friend std::ostream &operator<<(std::ostream &stream, const Triangle t);
        
        /// the three vertex Points of the Triangle
        Point p[3]; // vertices
        /// normal vector
        Point n;    
        /// return normal vector with positive z-coordinate 
        Point upNormal() const;
        /// bounding-box 
        Bbox bb;


        
    protected:
        /// calculate and set Triangle normal
        void calcNormal();
        /// update bounding-box
        void calcBB();
};

} // end namespace
#endif
// end file triangle.h
