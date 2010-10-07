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
#ifndef TRIANGLE_PY_H
#define TRIANGLE_PY_H

#include <sstream>

#include <boost/python.hpp>
#include <boost/foreach.hpp>

#include "triangle.h"

namespace ocl
{

///
/// \brief python wrapper for Triangle
///
class Triangle_py : public Triangle {
    public:
        Triangle_py() : Triangle() {};
        Triangle_py( const Point& p0, 
                     const Point& p1,
                     const Point& p2) : Triangle(p0,p1,p2) {};
        Triangle_py( const Triangle_py& t) : Triangle(t) {};
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
