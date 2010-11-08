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

#ifndef OP_H
#define OP_H

#include <iostream>
#include <string>
#include <vector>

#include "point.h"
#include "fiber.h"
#include "kdtree3.h"

namespace ocl
{

class STLSurf;
class Triangle;
class MillingCutter;

///
/// base-class for cam algorithms
class Operation {
    public:
        Operation() {
            
        }
        virtual ~Operation() {}
        /// set the STL-surface and build kd-tree
        virtual void setSTL(const STLSurf& s) {
            surf = &s;
            BOOST_FOREACH(Operation* op, subOp) {
                op->setSTL(s);
            }
        }
        /// set the MillingCutter to use
        virtual void setCutter(const MillingCutter* c) {
            cutter = c;
            BOOST_FOREACH(Operation* op, subOp) {
                op->setCutter(cutter);
            }
        }
        /// set number of OpenMP threads. Defaults to OpenMP::omp_get_num_procs()
        void setThreads(unsigned int n) {nthreads = n;}
        /// return number of OpenMP threads
        int  getThreads() const {return nthreads;}
        /// return the kd-tree bucket-size
        int getBucketSize() const {return bucketSize;}
        /// set the kd-tree bucket-size
        void setBucketSize(unsigned int s) {bucketSize = s;}
        /// return number of low-level calls
        int getCalls() const {return nCalls;}
        
        /// set the sampling interval for this Operation and all sub-operations
        virtual void setSampling(double s) {sampling=s;}
        /// return the sampling interval
        virtual double getSampling() {return sampling;}
        
        /// run the algorithm
        virtual void run() = 0;
        /// run algorithm on a single input CLPoint
        virtual void run(CLPoint& cl) {assert(0);}
        /// return CL-points
        virtual std::vector<CLPoint> getCLPoints() {
            std::vector<CLPoint>* clv = new std::vector<CLPoint>(); 
            return *clv;
        }
        /// add an input CLPoint to this Operation
        virtual void appendPoint(CLPoint& p) {}
        
    protected:
        /// sampling interval
        double sampling;
        /// how many low-level calls were made
        int nCalls;
        /// size of bucket-node in KD-tree
        unsigned int bucketSize;
        /// the MillingCutter used
        const MillingCutter* cutter;
        /// the STLSurf which we test against.
        const STLSurf* surf;
        /// root of a kd-tree
        KDTree<Triangle>* root;
        /// number of threads to use
        unsigned int nthreads;
        /// sub-operations, if any, of this operation
        std::vector<Operation*> subOp;
};

} // end namespace

#endif // end operation.h
