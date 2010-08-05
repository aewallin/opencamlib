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

#include <list>

#include <boost/foreach.hpp>

#include "point.h"
#include "triangle.h"
#include "stlsurf.h"

namespace ocl
{

STLSurf::STLSurf() {
    
}



STLSurf::~STLSurf() {
    //delete tris;
}


void STLSurf::addTriangle(const Triangle &t) {
    tris.push_back(t);
    bb.addTriangle(t);
    return;
}

unsigned int STLSurf::size() const {
    return tris.size();
}

boost::python::list STLSurf::getTriangles() const
{
    boost::python::list tlist;
    BOOST_FOREACH(Triangle t, tris) {
        tlist.append(t);
    }
    return tlist;
}

boost::python::list STLSurf::getBounds() const
{
    boost::python::list bounds;
    bounds.append( bb.minpt.x );
    bounds.append( bb.maxpt.x );
    bounds.append( bb.minpt.y );
    bounds.append( bb.maxpt.y );
    bounds.append( bb.minpt.z );
    bounds.append( bb.maxpt.z );
    return bounds;
}

std::string STLSurf::str() const {
    std::ostringstream o;
    o << *this;
    return o.str();
}

std::ostream &operator<<(std::ostream &stream, const STLSurf s)
{
  stream << "STLSurf(N="<< s.tris.size() <<")";
  return stream;
}

} // end namespace
// end file stlsurf.cpp
