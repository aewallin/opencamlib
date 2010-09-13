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

#include "point.h"
#include "triangle.h"
#include "stlsurf.h"

namespace ocl
{

void STLSurf::addTriangle(const Triangle &t) {
    tris.push_back(t);
    bb.addTriangle(t);
    return;
}

unsigned int STLSurf::size() const {
    return tris.size();
}

std::ostream &operator<<(std::ostream &stream, const STLSurf s) {
  stream << "STLSurf(N="<< s.tris.size() <<")";
  return stream;
}

} // end namespace
// end file stlsurf.cpp
