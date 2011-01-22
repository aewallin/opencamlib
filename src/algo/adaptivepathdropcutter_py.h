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

#ifndef ADAPTIVEPATHDROPCUTTER_PY_H
#define ADAPTIVEPATHDROPCUTTER_PY_H

#include <boost/python.hpp>

#include "adaptivepathdropcutter.h"

namespace ocl
{
    
/// Python wrapper for PathDropCutter
class AdaptivePathDropCutter_py : public AdaptivePathDropCutter {
    public:
        AdaptivePathDropCutter_py() : AdaptivePathDropCutter() {};
        /// return a list of CL-points to python
        boost::python::list getCLPoints_py() {
            boost::python::list plist;
            BOOST_FOREACH(CLPoint p, clpoints) {
                plist.append(p);
            }
            return plist;
        };
};

} // end namespace
#endif
// end file adaptivepathdropcutter_py.h
