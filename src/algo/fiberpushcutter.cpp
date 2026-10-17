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


#ifdef _OPENMP  
    #include <omp.h>
#endif

#include "millingcutter.hpp"
#include "point.hpp"
#include "triangle.hpp"
#include "fiberpushcutter.hpp"

namespace ocl
{

//********   ********************** */

FiberPushCutter::FiberPushCutter() : x_direction(false), y_direction(false) {
    nCalls = 0;
    cutter = NULL;
    bucketSize = 1;
    root = std::make_unique<KDTree<Triangle>>();
}

FiberPushCutter::~FiberPushCutter() {
}

void FiberPushCutter::setSTL(const STLSurf &s) {
    Operation::setSTL(s);
}

void FiberPushCutter::setSTLSnapshot(const STLSurf& s, KDTree<Triangle>::Snapshot snapshot) {
    // std::cout << "BPC::setSTL() Building kd-tree... bucketSize=" << bucketSize << "..";
    root->setBucketSize( bucketSize );
    if (x_direction)
        root->setYZDimensions(); 
    else if (y_direction)
        root->setXZDimensions();
    else {
        throw std::logic_error("Set fiber direction before setting the surface");
    }
    // std::cout << "BPC::setSTL() root->build()";
    root->buildSnapshot(std::move(snapshot));
    surf = &s;
    // std::cout << " done.\n";
}

void FiberPushCutter::pushCutter1(Fiber& f) {
    nCalls = 0;
    for (const Triangle& t : surf->tris) {// test against all triangles in s
        Interval i;
        cutter->pushCutter(f,i,t);
        f.addInterval(i);
        ++nCalls;
    }
}

unsigned int FiberPushCutter::pushCutter2(Fiber& f) const {
    unsigned int calls = 0;
    CLPoint cl;
    if ( x_direction ) {
        cl.x=0;
        cl.y=f.p1.y;
        cl.z=f.p1.z;
    } else if (y_direction ) {
        cl.x=f.p1.x;
        cl.y=0;
        cl.z=f.p1.z;
    }
    root->visit_cutter_overlap(cutter, &cl, [&](const Triangle& t) {
        Interval i;
        cutter->pushCutter(f,i,t);
        f.addInterval(i);
        ++calls;
    });
    return calls;
}

}// end namespace
// end file fiberpushcutter.cpp
