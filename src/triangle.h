/*  Copyright 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com)
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
#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "point.h"
#include <boost/foreach.hpp>
#include <boost/python.hpp>

namespace ocl
{

///
/// \brief a Triangle defined by its three vertices
///
class Triangle {
    public:
        /// default constructor
        Triangle();
        /// destructor
        ~Triangle();
        /// Create a triangle with the vertices p1, p2, and p3.
        Triangle(Point p1, Point p2, Point p3);   
        
        /// string repr     
        friend std::ostream &operator<<(std::ostream &stream, const Triangle t);
        /// string repr
        std::string str() const;
        
        /// Returns a list of the vertices to Python
        boost::python::list getPoints();        

        /// the three vertex Points of the Triangle
        Point p[3]; // vertices
        /// normal vector
        Point *n;    // surface normal
        /// id-number count (obsolete?)
        static int count;
        /// id-number (obsolete?)
        int id;
        /// bounding-box maximum x-coordinate
        double maxx;
        /// bounding-box maximum y-coordinate
        double maxy;
        /// bounding-box minimum x-coordinate
        double minx;
        /// bounding-box minimum y-coordinate
        double miny;
        
    private:
        /// set id-number
        void setId();
        /// calculate and set Triangle normal
        void calcNormal();
        /// calculate bounding-box
        void calcBB();
};

} // end namespace
#endif
// end file triangle.h
