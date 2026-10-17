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

#ifndef OP_H
#define OP_H

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>

#include "point.hpp"
#include "fiber.hpp"
#include "kdtree.hpp"
#include "stlsurf.hpp"

namespace ocl
{

class STLSurf;
class Triangle;
class MillingCutter;

/// \brief base-class for low-level cam algorithms
///
/// base-class for cam algorithms
class Operation {
    public:
        Operation() : sampling(0), nCalls(0), bucketSize(1), cutter(nullptr),
                      surf(nullptr), root(nullptr), nthreads(1) {}
        Operation(const Operation&) = delete;
        Operation& operator=(const Operation&) = delete;
        virtual ~Operation() {
            //std::cout << "~Operation()\n";
        }
        /// set the STL-surface and build kd-tree
        virtual void setSTL(const STLSurf& s) {
            setSTLSnapshot(s, std::make_shared<const std::vector<Triangle>>(s.tris.begin(), s.tris.end()));
        }
        /// set the MillingCutter to use
        virtual void setCutter(const MillingCutter* c) {
            if (!c) throw std::invalid_argument("Cutter must not be null");
            cutter = c;
            for (const auto& op : subOp) {
                op->setCutter(cutter);
            }
        }
        /// Set the requested thread count (zero means one). WASM caps it to its pool.
        void setThreads(unsigned int n) {
            if (n > static_cast<unsigned int>(std::numeric_limits<int>::max()))
                throw std::invalid_argument("Thread count is too large");
            nthreads = n > 0 ? n : 1;
#ifdef OCL_WASM_THREAD_POOL_SIZE
            nthreads = std::min<unsigned int>(nthreads, OCL_WASM_THREAD_POOL_SIZE + 1);
#endif
            for (const auto& op : subOp) {
                op->setThreads(nthreads);
            }
        }
        /// Return the configured thread count.
        int  getThreads() const {return nthreads;}
        /// return the kd-tree bucket-size
        int getBucketSize() const {return bucketSize;}
        /// set the kd-tree bucket-size
        void setBucketSize(unsigned int s) {
            if (s == 0 || s > static_cast<unsigned int>(std::numeric_limits<int>::max()))
                throw std::invalid_argument("Bucket size must be a positive integer");
            bucketSize = s;
            for (const auto& op : subOp) {
                op->setBucketSize(bucketSize);
            }
        }
        /// return number of low-level calls
        int getCalls() const {return nCalls;}
        
        /// set the sampling interval for this Operation and all sub-operations
        virtual void setSampling(double s) {
            requirePositive(s, "Sampling must be finite and positive");
            sampling=s;
            for (const auto& op : subOp) {
                op->setSampling(sampling);
            }
        }
        /// return the sampling interval
        virtual double getSampling() {return sampling;}
        
        /// run the algorithm
        virtual void run() = 0;
        /// run algorithm on a single input CLPoint
        virtual void run(CLPoint&) {throw std::logic_error("Operation does not accept a point");}
        /// run push-cutter type algorithm on input Fiber
        virtual void run(Fiber&) {throw std::logic_error("Operation does not accept a fiber");}
        
        virtual void reset() {}
        
        /// return CL-points
        virtual std::vector<CLPoint> getCLPoints() {
            return {};
        }
        virtual void clearCLPoints() {}
        
        /// add an input CLPoint to this Operation
        virtual void appendPoint(CLPoint& p) {}
        /// used by batchpushcutter
        virtual void setXDirection() {}
        /// used by batchpushcutter
        virtual void setYDirection() {}
        /// add a fiber input to a push-cutter type operation
        virtual void appendFiber( Fiber& f ) {}
        /// return the result of a push-cutter type operation
        virtual std::vector<Fiber>* getFibers() const {return 0;}
        
    protected:
        virtual void setSTLSnapshot(const STLSurf& s, KDTree<Triangle>::Snapshot snapshot) {
            surf = &s;
            for (const auto& op : subOp) op->setSTLSnapshot(s, snapshot);
        }
        static void requireFinite(double value, const char* message) {
            if (!std::isfinite(value)) throw std::invalid_argument(message);
        }
        static void requirePositive(double value, const char* message) {
            if (!std::isfinite(value) || value <= 0) throw std::invalid_argument(message);
        }
        static void requireCosine(double value) {
            if (!std::isfinite(value) || value < -1 || value > 1)
                throw std::invalid_argument("Cosine limit must be between -1 and 1");
        }
        void validateInputs(bool needsSampling = false) const {
            if (!surf || !cutter) throw std::logic_error("Set surface and cutter before running");
            if (needsSampling) requirePositive(sampling, "Sampling must be finite and positive");
        }
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
        std::unique_ptr<KDTree<Triangle>> root;
        /// number of threads to use
        unsigned int nthreads;
        /// sub-operations, if any, of this operation
        std::vector<std::unique_ptr<Operation>> subOp;
};

} // end namespace

#endif // end operation.h
