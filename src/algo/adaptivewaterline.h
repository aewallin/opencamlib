/*  $Id:  $
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

#ifndef ADAPTIVEWATERLINE_H
#define ADAPTIVEWATERLINE_H

#include <iostream>
#include <string>
#include <vector>

#include <boost/python.hpp>
#include <boost/foreach.hpp>


#include "waterline.h"
#include "fiber_py.h"

namespace ocl
{

class STLSurf;
class KDNode2;
class Triangle;
class MillingCutter;
class Span;

/// \brief a Waterline toolpath follows the shape of the model at a constant z-height in the xy-plane

class AdaptiveWaterline : public Waterline {
    public:
        /// create an empty Waterline object
        AdaptiveWaterline(); 
        virtual ~AdaptiveWaterline();
        void setMinSampling(double s) {min_sampling=s;}
        /// run the Waterline algorithm. setSTL, setCutter, setSampling, and setZ must
        /// be called before a call to run()
        void run();
        
    protected:
        void adaptive_sampling_run();
        void xfiber_adaptive_sample(const Span* span, double start_t, double stop_t, Fiber start_f, Fiber stop_f);
        void yfiber_adaptive_sample(const Span* span, double start_t, double stop_t, Fiber start_f, Fiber stop_f);
        bool flat( Fiber& start, Fiber& mid, Fiber& stop );
        void weave_process();

    // DATA
        std::vector<Fiber> xfibers;
        std::vector<Fiber> yfibers;
        double minx;
        double maxx;
        double miny;
        double maxy;
        double min_sampling;
};

class AdaptiveWaterline_py : public AdaptiveWaterline {
    public:
        AdaptiveWaterline_py() : AdaptiveWaterline() {};
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
        boost::python::list getXFibers() const {
            boost::python::list flist;
            BOOST_FOREACH( Fiber f, xfibers ) {
                if (!f.empty()) {
                    Fiber_py f2(f);
                    flist.append(f2);
                }
            }
            return flist;
        };
        boost::python::list getYFibers() const {
            boost::python::list flist;
            BOOST_FOREACH( Fiber f, yfibers ) {
                if (!f.empty()){
                    Fiber_py f2(f);
                    flist.append(f2);
                }
            }
            return flist;
        };
};

} // end namespace

#endif
