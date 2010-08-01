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

#ifndef BPC_H
#define BPC_H

#include <iostream>
#include <string>
#include <vector>
#include <boost/python.hpp>
#include "point.h"
#include "fiber.h"

namespace ocl
{

class STLSurf;
class KDNode2;
class Triangle;
class MillingCutter;

///
/// \brief Drop cutter interface
class BatchPushCutter {
    public:
        BatchPushCutter();
        
        /// set the STL-surface and build kd-tree
        void setSTL(STLSurf &s);
        /// set the MillingCutter to use
        void setCutter(MillingCutter *cutter);
        /// set number of OpenMP threads. Defaults to OpenMP::omp_get_num_procs()
        void setThreads(unsigned int n);
        /// append to list of Fibers to evaluate
        void appendFiber(Fiber& f);
        
        /// run push-cutter
        void pushCutter1();
        void pushCutter2();
        void pushCutter3();
        
        // Python interface
        /// return CL-points to Python
        boost::python::list getCLPoints() const;
        /// return triangles under cutter to Python. Not for CAM-algorithms, more for visualization and demonstration.
        boost::python::list getOverlapTriangles(Fiber& f, MillingCutter& cutter);
        /// return list of Fibers to python
        boost::python::list getFibers() const;
        
        // DATA
        /// how many low-level calls were made
        int nCalls;
        /// number of threads to use
        unsigned int nthreads;
        /// size of bucket-node in KD-tree
        unsigned int bucketSize;
    private:
        /// the MillingCutter used
        MillingCutter *cutter;
        /// pointer to list of Fibers
        std::vector<Fiber>* fibers;
        /// root of the kd-tree
        KDNode2 *root;
        /// the STLSurf which we test against.
        STLSurf *surf;

};

} // end namespace

#endif
