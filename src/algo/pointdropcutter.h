/*  $Id: $
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

#ifndef POINTDROPCUTTER_H
#define POINTDROPCUTTER_H

#include <iostream>
#include <string>
#include <vector>

#include "clpoint.h"
#include "millingcutter.h"
#include "kdtree3.h"

namespace ocl
{

class STLSurf;
class Triangle;

///
class PointDropCutter {
    public:
        PointDropCutter();
        virtual ~PointDropCutter() {delete root;};
        /// set the STL-surface and build kd-tree to enable optimized algorithm
        void setSTL(const STLSurf &s);
        /// set the MillingCutter to use
        void setCutter(MillingCutter* cutter);
        /// run drop-cutter on all clpoints
        void run(CLPoint& cl);
        
    // getters and setters
        /// return bucketSize
        int getBucketSize() const {return bucketSize;}
        /// set the bucketSize used when building a KDTree
        void setBucketSize(unsigned int s) {bucketSize = s;}
        /// return number of low-level calls made during run()
        int getCalls() const {return dcCalls;}
        /// set number of threads to use in OpenMP
        void setThreads(int n) {nthreads = n;}
        /// return number of OpenMP threads
        int  getThreads() const {return nthreads;}

    protected:
        void pointDropCutter1(CLPoint& clp);
    // DATA
        /// the MillingCutter used
        MillingCutter* cutter;
        /// root of kd-tree
        KDTree<Triangle>* root;
        /// the STLSurf which we test against.
        const STLSurf* surf;
        /// how many times DropCutter was called. Useful for optimization.
        int dcCalls;
        /// number of OpenMP threads to use
        unsigned int nthreads;
        /// when building the kd-tree, use this bucket-size
        unsigned int bucketSize;
};

} // end namespace

#endif
