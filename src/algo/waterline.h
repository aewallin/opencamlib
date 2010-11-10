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

#include "point.h"
#include "fiber.h"
#include "batchpushcutter.h"
#include "operation.h"

namespace ocl
{

class STLSurf;
class KDNode2;
class Triangle;
class MillingCutter;


/// The Waterline object is used for generating waterline or z-slice toolpaths
/// from an STL-model. Waterline calls BatchPushCutter to do most of the work.
class Waterline : public Operation {
    public:
        /// create an empty Waterline object
        Waterline(); 
        virtual ~Waterline();
        
        /// Set the z-coordinate for the waterline we generate
        void setZ(const double z) {zh = z;};
        /// run the Waterline algorithm. setSTL, setCutter, setSampling, and setZ must
        /// be called before a call to run()
        void run();
        
    protected:
        /// initialization
        void init_fibers();
        /// x and y-coordinates for fiber generation
        std::vector<double> generate_range( double start, double end, int N) const;
        
    // DATA
        /// the z-height for this Waterline
        double zh;
        /// the results of this operation, a list of loops
        std::vector< std::vector<Point> >  loops; // change to CLPoint!!
};



} // end namespace

#endif
