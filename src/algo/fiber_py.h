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
#ifndef FIBER_PY_H
#define FIBER_PY_H

#include <boost/foreach.hpp>
#include <boost/python.hpp>

#include "fiber.h"

namespace ocl
{

/// python wrapper for Fiber
class Fiber_py : public Fiber {
    public:
        Fiber_py() : Fiber () {};
        /// construct p1-p2 fiber
        Fiber_py(const Point &p1, const Point &p2) : Fiber(p1, p2) {};
        Fiber_py(const Fiber& f) : Fiber(f) {};
        /// return a list of intervals to python
        boost::python::list getInts() const {
            boost::python::list l;
            BOOST_FOREACH( Interval i, ints) {
                l.append( i );
            }
            return l;
        };
};

} // end namespace
#endif
// end file fiber_py.h
