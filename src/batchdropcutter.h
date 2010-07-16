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

#ifndef BDC_H
#define BDC_H

#include <iostream>
#include <string>
#include <vector>
#include <boost/python.hpp>
#include "point.h"

namespace ocl
{

class STLSurf;
class KDNode;
class KDNode2;
class Triangle;
class MillingCutter;

///
/// \brief Drop cutter interface
class BatchDropCutter {
    public:
        BatchDropCutter();
        
        /// set the STL-surface and build kd-tree to enable optimized algorithm
        void setSTL(STLSurf &s);
        /// set the MillingCutter to use
        void setCutter(MillingCutter *cutter);
        /// set number of threads to use in OpenMP
        void setThreads(int n);
        /// append to list of CL-points to evaluate
        void appendPoint(CLPoint& p);
        
        /// unoptimized drop-cutter,  tests against all triangles of surface
        void dropCutter1();
        /// better, kd-tree optimized version      
        void dropCutter2();
        /// kd-tree and explicit overlap test      
        void dropCutter3();
        /// use OpenMP for multi-threading     
        void dropCutter4();
        void dropCutter5();
        // Python interface
        /// return CL-points to Python
        boost::python::list getCLPoints();

        /// return triangles under cutter to Python. Not for CAM-algorithms, more for visualization and demonstration.
        boost::python::list getTrianglesUnderCutter(CLPoint &cl, MillingCutter &cutter);
        
        // DATA
        /// the MillingCutter used
        MillingCutter *cutter;
        
        /// pointer to list of CL-points on which to run drop-cutter.
        std::vector<CLPoint>* clpoints;
        /// root of the kd-tree
        KDNode *root;
        KDNode2 *root2;
        
        /// the STLSurf which we test against.
        STLSurf *surf;
        
        /// how many times DropCutter was called. Useful for optimization.
        int dcCalls;

        /// number of threads to use
        unsigned int nthreads;
        unsigned int bucketSize;
};

}

#endif
