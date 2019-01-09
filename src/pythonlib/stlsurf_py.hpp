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
#ifndef STLSURF_PY_H
#define STLSURF_PY_H

#include <boost/python.hpp>
#include <boost/foreach.hpp>

#include "stlsurf.hpp"

namespace ocl
{
    
/// STLSurf python wrapper
class STLSurf_py : public STLSurf {
    public:
        /// default constructor
        STLSurf_py() : STLSurf() {};
        /// return list of all triangles to python
        boost::python::list getTriangles() const {
            boost::python::list tlist;
            BOOST_FOREACH(Triangle t, tris) {
                tlist.append(Triangle_py(t));
            }
            return tlist;
        };
        
        /// return bounds in a list to python
        boost::python::list getBounds() const {
            boost::python::list bounds;
            bounds.append( bb.minpt.x );
            bounds.append( bb.maxpt.x );
            bounds.append( bb.minpt.y );
            bounds.append( bb.maxpt.y );
            bounds.append( bb.minpt.z );
            bounds.append( bb.maxpt.z );
            return bounds;
        };
        
        /// string output
        std::string str() const {
            std::ostringstream o;
            o << *this;
            return o.str();
        };
};

} // end namespace
#endif
// end file stlsurf_py.h
