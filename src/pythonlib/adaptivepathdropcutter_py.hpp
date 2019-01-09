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
#ifndef ADAPTIVEPATHDROPCUTTER_PY_H
#define ADAPTIVEPATHDROPCUTTER_PY_H

#include <boost/python.hpp>

#include "adaptivepathdropcutter.hpp"

namespace ocl
{
    
/// Python wrapper for PathDropCutter
class AdaptivePathDropCutter_py : public AdaptivePathDropCutter {
    public:
        AdaptivePathDropCutter_py() : AdaptivePathDropCutter() {}
        virtual ~AdaptivePathDropCutter_py()  {}
        /// return a list of CL-points to python
        boost::python::list getCLPoints_py() {
            //std::cout << " apdc_py::getCLPoints_py()...";
            boost::python::list plist;
            BOOST_FOREACH(CLPoint p, clpoints) {
                plist.append(p);
            }
            //std::cout << " DONE.\n";
            return plist;
        }
};

} // end namespace
#endif
// end file adaptivepathdropcutter_py.h
