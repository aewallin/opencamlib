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

#ifndef WATERLINE_H
#define WATERLINE_H

#include <iostream>
#include <string>
#include <vector>
#include <boost/python.hpp>
#include "point.h"
#include "fiber.h"
#include "batchpushcutter.h"

namespace ocl
{

class STLSurf;
class KDNode2;
class Triangle;
class MillingCutter;

///
/// \brief Drop cutter interface
class Waterline {
    public:
        Waterline(); // dummy constructor, don't use.
        virtual ~Waterline();
        void setSTL(const STLSurf& s);
        void setCutter(const MillingCutter& c);
        void setTolerance(const double tol);
        void setZ(const double z);
        
        void run();
        
        boost::python::list py_getLoops() const;
        
    private:
        void init_fibers();
        std::vector<double> generate_range( double start, double end, int N) const;
        // DATA
        const MillingCutter* cutter;
        const STLSurf* surface;
        BatchPushCutter* bpc;
        double zh;
        double tolerance;
        std::vector< std::vector<Point> >  loops;
};

} // end namespace

#endif
