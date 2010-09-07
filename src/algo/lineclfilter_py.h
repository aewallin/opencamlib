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

#ifndef LINE_CL_FILTER_PY_H
#define LINE_CL_FILTER_PY_H

#include <boost/python.hpp>

#include "lineclfilter.h"

namespace ocl
{  
    
class LineCLFilter_py : public LineCLFilter {
    public:
        LineCLFilter_py() : LineCLFilter() {};
        boost::python::list getCLPoints() {
            // return points to python
            boost::python::list plist;
            BOOST_FOREACH(CLPoint p, clpoints) {
                plist.append(p);
            }
            return plist;
        };
};

} // end namespace
#endif
// end file lineclfilter_py.h
