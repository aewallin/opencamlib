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

#include "point.h"
#include "clpoint.h"
#include "millingcutter.h"

namespace ocl
{

class STLSurf;
class KDNode;
class KDNode2;
class Triangle;

///
/// \brief Drop cutter interface
class BatchDropCutter {
    public:
        BatchDropCutter();
        virtual ~BatchDropCutter() {};
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
        /// version 5 of the algorithm
        void dropCutter5();
        
    // DATA
        /// the MillingCutter used
        MillingCutter *cutter;
        /// pointer to list of CL-points on which to run drop-cutter.
        std::vector<CLPoint>* clpoints;
        /// root of the kd-tree
        KDNode *root;
        /// root of KDNode2 kd-tree
        KDNode2 *root2;
        /// the STLSurf which we test against.
        STLSurf *surf;
        /// how many times DropCutter was called. Useful for optimization.
        int dcCalls;
        /// number of OpenMP threads to use
        unsigned int nthreads;
        /// when building the kd-tree, use this bucket-size
        unsigned int bucketSize;
};

} // end namespace

#endif
