/*  $Id$
 * 
 *  Copyright (c) 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com).
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

#include "batchpushcutter.hpp"
#include "parallel.hpp"
#include "millingcutter.hpp"
#include "point.hpp"
#include "triangle.hpp"

namespace ocl
{

//********   ********************** */

BatchPushCutter::BatchPushCutter() : x_direction(false), y_direction(false) {
    fibers = std::make_unique<std::vector<Fiber>>();
    nCalls = 0;
    nthreads = detail::default_threads();
    cutter = NULL;
    bucketSize = 1;
    root = std::make_unique<KDTree<Triangle>>();
}

BatchPushCutter::~BatchPushCutter() {
}

void BatchPushCutter::setSTL(const STLSurf &s) {
    Operation::setSTL(s);
}

void BatchPushCutter::setSTLSnapshot(const STLSurf& s, KDTree<Triangle>::Snapshot snapshot) {
    // std::cout << "BPC::setSTL() Building kd-tree... bucketSize=" << bucketSize << "..";
    root->setBucketSize( bucketSize );
    if (x_direction)
        root->setYZDimensions(); // we search for triangles in the XY plane, don't care about Z-coordinate
    else if (y_direction)
        root->setXZDimensions();
    else {
        throw std::logic_error("Set fiber direction before setting the surface");
    }
    // std::cout << "BPC::setSTL() root->build()...";
    root->buildSnapshot(std::move(snapshot));
    surf = &s;
    // std::cout << "done.\n";
}

void BatchPushCutter::appendFiber(Fiber& f) {
    fibers->push_back(f);
}

void BatchPushCutter::reset() {
    fibers->clear();
}

/// very simple batch push-cutter
/// each fiber is tested against all triangles of surface
void BatchPushCutter::pushCutter1() {
    // std::cout << "BatchPushCutter1 with " << fibers->size() << 
    //           " fibers and " << surf->tris.size() << " triangles..." << std::endl;
    nCalls = 0;
    for (Fiber& f : *fibers) {
        for (const Triangle& t : surf->tris) { // test against all triangles in s
            Interval i;
            cutter->pushCutter(f,i,t);
            f.addInterval(i);
            ++nCalls;
        }
    }
    // std::cout << "BatchPushCutter done." << std::endl;
    return;
}

/// push-cutter which uses KDNode2 kd-tree search to find triangles 
/// overlapping with the cutter.
void BatchPushCutter::pushCutter2() {
    // std::cout << "BatchPushCutter2 with " << fibers->size() << 
    //           " fibers and " << surf->tris.size() << " triangles..." << std::endl;
    nCalls = 0;
    for (Fiber& f : *fibers) {
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
        root->visit_cutter_overlap(cutter, &cl, [&](const Triangle& t) {
            Interval i;
            cutter->pushCutter(f,i,t);
            f.addInterval(i);
            ++nCalls;
        });
    }
    // std::cout << "BatchPushCutter2 done." << std::endl;
    return;
}

/// use kd-tree search to find overlapping triangles
/// use OpenMP for multi-threading
void BatchPushCutter::pushCutter3() {
    std::vector<Fiber>& fiberr = *fibers;
    nCalls = detail::parallel_sum(fiberr.size(), nthreads, [&](std::size_t n) {
        unsigned int calls = 0;
        CLPoint cl; // cl-point on the fiber
        if ( x_direction ) {
            cl.x=0;
            cl.y=fiberr[n].p1.y;
            cl.z=fiberr[n].p1.z;
        } else if (y_direction ) {
            cl.x=fiberr[n].p1.x;
            cl.y=0;
            cl.z=fiberr[n].p1.z;
        }
        root->visit_cutter_overlap(cutter, &cl, [&](const Triangle& t) {
            Interval i;
            cutter->pushCutter(fiberr[n],i,t);
            fiberr[n].addInterval(i);
            ++calls;
        });
        return calls;
    });
    // std::cout << "\nBatchPushCutter3 done." << std::endl;
    return;
}

std::vector<CLPoint> BatchPushCutter::getCLPoints() {
    std::vector<CLPoint> clPoints;
    clPoints.reserve(2 * fibers->size());

    for (const auto& f : *fibers) {
        for (const auto& i : f.ints) {
            if (i.empty())
                continue;

            // Get lower point.
            const Point lower = f.point(i.lower);
            CLPoint p1(lower.x, lower.y, lower.z);
            *p1.cc.load() = i.lower_cc;
            clPoints.emplace_back(p1);

            // Get upper point.
            const Point upper = f.point(i.upper);
            CLPoint p2(upper.x, upper.y, upper.z);
            *p2.cc.load() = i.upper_cc;
            clPoints.emplace_back(p2);
        }
    }
    return clPoints;
}

std::list<Triangle> BatchPushCutter::getOverlapTriangles(Fiber& f) {
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
        assert(false);
    }

    std::list<Triangle>* overlapTriangles = root->search_cutter_overlap(cutter, &cl);
    std::list<Triangle> ret = *overlapTriangles;
    delete overlapTriangles;
    return ret;
}

}// end namespace
// end file batchpushcutter.cpp
