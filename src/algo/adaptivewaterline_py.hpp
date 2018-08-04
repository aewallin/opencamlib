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
#ifndef ADAPTIVEWATERLINE_PY_H
#define ADAPTIVEWATERLINE_PY_H

#include "adaptivewaterline.hpp"
#include "fiber_py.hpp"

namespace ocl
{

/// \brief python wrapper for AdaptiveWaterline
class AdaptiveWaterline_py : public AdaptiveWaterline {
    public:
        AdaptiveWaterline_py() : AdaptiveWaterline() {}
        ~AdaptiveWaterline_py() {
            std::cout << "~AdaptiveWaterline_py()\n";
        }
        
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
        }
        /// return a list of xfibers to python
        boost::python::list getXFibers() const {
            boost::python::list flist;
            BOOST_FOREACH( Fiber f, xfibers ) {
                if (!f.empty()) {
                    Fiber_py f2(f);
                    flist.append(f2);
                }
            }
            return flist;
        }
        /// return a list of yfibers to python
        boost::python::list getYFibers() const {
            boost::python::list flist;
            BOOST_FOREACH( Fiber f, yfibers ) {
                if (!f.empty()){
                    Fiber_py f2(f);
                    flist.append(f2);
                }
            }
            return flist;
        }
};

} // end namespace

#endif
