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

#ifndef BPC_PY_H
#define BPC_PY_H

#include <boost/python.hpp>

#include "batchpushcutter.hpp"

#include "fiber_py.hpp"

namespace ocl
{
/// \brief python wrapper for batchpushcutter
class BatchPushCutter_py : public BatchPushCutter {
    public:
        BatchPushCutter_py() : BatchPushCutter() {}
        /// return CL-points to Python
        boost::python::list getCLPoints() const {
            boost::python::list plist;
            BOOST_FOREACH(Fiber f, *fibers) {
                BOOST_FOREACH( Interval i, f.ints ) {
                    if ( !i.empty() ) {
                        Point tmp = f.point(i.lower);
                        CLPoint p1 = CLPoint( tmp.x, tmp.y, tmp.z );
                        p1.cc = new CCPoint(i.lower_cc);
                        tmp = f.point(i.upper);
                        CLPoint p2 = CLPoint( tmp.x, tmp.y, tmp.z );
                        p2.cc = new CCPoint(i.upper_cc);
                        plist.append(p1);
                        plist.append(p2);
                    }
                }
               
            }
            return plist;
        };
        /// return triangles under cutter to Python. Not for CAM-algorithms, 
        /// more for visualization and demonstration.
        boost::python::list getOverlapTriangles(Fiber& f) {
            boost::python::list trilist;
            std::list<Triangle> *overlap_triangles = new std::list<Triangle>();
            //int plane = 3; // XY-plane
            //Bbox bb; //FIXME
            //KDNode2::search_kdtree( overlap_triangles, bb,  root, plane);
            CLPoint cl;
            if (x_direction) {
                cl.x = 0;
                cl.y = f.p1.y;
                cl.z = f.p1.z;
            } else if (y_direction) {
                cl.x = f.p1.x;
                cl.y = 0;
                cl.z = f.p1.z;
            } else {
                assert(0);
            }
            overlap_triangles = root->search_cutter_overlap(cutter, &cl);
            
            BOOST_FOREACH(Triangle t, *overlap_triangles) {
                trilist.append(t);
            }
            delete overlap_triangles;
            return trilist;
        };
        /// return list of Fibers to python
        boost::python::list getFibers_py() const {
            boost::python::list flist;
            BOOST_FOREACH(Fiber f, *fibers) {
                flist.append( Fiber_py(f) );
            }
            return flist;
        };

};

} // end namespace

#endif
