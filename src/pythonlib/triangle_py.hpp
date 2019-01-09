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
#ifndef TRIANGLE_PY_H
#define TRIANGLE_PY_H

#include <sstream>

#include <boost/python.hpp>
#include <boost/foreach.hpp>

#include "triangle.hpp"

namespace ocl
{

///
/// \brief python wrapper for Triangle
///
class Triangle_py : public Triangle {
    public:
        /// default constructor
        Triangle_py() : Triangle() {};
        /// construct from three points
        Triangle_py( const Point& p0, 
                     const Point& p1,
                     const Point& p2) : Triangle(p0,p1,p2) {};
        /// copy constructor
        Triangle_py( const Triangle_py& t) : Triangle(t) {};
        /// cast-down constructor
        Triangle_py( const Triangle& t) : Triangle(t) {};
                     
        /// string repr
        std::string str() const {
            std::ostringstream o;
            o << *this;
            return o.str();
        };
        
        /// Returns a list of the vertices to Python
        boost::python::list getPoints() const {
            boost::python::list plist;
            BOOST_FOREACH(Point vertex, p) {
                plist.append(vertex);
            }
            return plist;
        };
        
};

} // end namespace
#endif
// end file triangle_py.h
