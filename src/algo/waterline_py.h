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

#ifndef WATERLINE_PY_H
#define WATERLINE_PY_H

#include <boost/python.hpp>
#include <boost/foreach.hpp>

#include "waterline.h"

namespace ocl
{

/// Python wrapper for Waterline
class Waterline_py : public Waterline {
    public:
        Waterline_py() : Waterline() {};
        /// return loop as a list of lists to python
        boost::python::list py_getLoops() const {
            boost::python::list loop_list;
            BOOST_FOREACH( std::vector<Point> loop, this->loops ) {
                boost::python::list point_list;
                BOOST_FOREACH( Point p, loop ) {
                    point_list.append( p );
                }
                loop_list.append(point_list);
            }
            return loop_list;
        };
};

} // end namespace

#endif
