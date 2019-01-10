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
#ifndef BDC_PY_H
#define BDC_PY_H

#include <boost/python.hpp> 
#include <boost/foreach.hpp> 

#include "batchdropcutter.hpp"

namespace ocl
{

/// Python wrapper for BatchDropCutter
class BatchDropCutter_py : public BatchDropCutter {
    public:
        BatchDropCutter_py() : BatchDropCutter() {};
        /// return CL-points to Python
        boost::python::list getCLPoints_py() {
            boost::python::list plist;
            BOOST_FOREACH(CLPoint p, *clpoints) {
                plist.append(p);
            }
            return plist;
        };
        /// return triangles under cutter to Python. Not for CAM-algorithms, 
        /// more for visualization and demonstration.
        boost::python::list getTrianglesUnderCutter(CLPoint& cl, MillingCutter& cutter) {
            boost::python::list trilist;
            std::list<Triangle> *triangles_under_cutter = new std::list<Triangle>();
            triangles_under_cutter = root->search_cutter_overlap( &cutter , &cl);
            BOOST_FOREACH(Triangle t, *triangles_under_cutter) {
                trilist.append(t);
            }
            delete triangles_under_cutter;
            return trilist;
        };
};

} // end namespace

#endif
