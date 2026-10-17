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

#include <list>
#include <cmath>
#include <algorithm>
#include <stdexcept>


#include "stlsurf.hpp"

namespace ocl
{

void STLSurf::addTriangle(const Triangle &t) {
    
    for (const auto& p : t.p)
        if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z))
            throw std::invalid_argument("Triangle coordinates must be finite");
    Point a = t.p[1] - t.p[0], b = t.p[2] - t.p[0];
    const double scale = std::max({std::abs(a.x), std::abs(a.y), std::abs(a.z),
                                   std::abs(b.x), std::abs(b.y), std::abs(b.z)});
    if (!std::isfinite(scale) || scale == 0)
        throw std::invalid_argument("Triangle edges must have finite, nonzero length");
    a = Point(a.x/scale, a.y/scale, a.z/scale);
    b = Point(b.x/scale, b.y/scale, b.z/scale);
    if (a.cross(b).norm() == 0)
        throw std::invalid_argument("Triangle vertices must not be collinear");

    tris.push_back(t);
    bb.addTriangle(t);
    return;
}

void STLSurf::rotate(double xr, double yr, double zr) {
    //std::cout << " before " << t << "\n";
    bb.clear();
    for (Triangle& t : tris) {
        //std::cout << " before " << t << "\n";
        t.rotate(xr,yr,zr);
        //std::cout << " after " << t << "\n";
        //char c;
        //std::cin >> c;
        bb.addTriangle(t);
    } 
}

unsigned int STLSurf::size() const {
    return static_cast<unsigned int>(tris.size());
}

std::ostream &operator<<(std::ostream &stream, const STLSurf& s) {
  stream << "STLSurf(N="<< s.tris.size() <<")";
  return stream;
}

} // end namespace
// end file stlsurf.cpp
