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


#include "parallel.hpp"

#include "point.hpp"
#include "triangle.hpp"
#include "pointdropcutter.hpp"


namespace ocl
{

//********   ********************** */

PointDropCutter::PointDropCutter() {
    nCalls = 0;
    nthreads = detail::default_threads();
    cutter = NULL;
    bucketSize = 1;
    root = std::make_unique<KDTree<Triangle>>();
}

void PointDropCutter::setSTL(const STLSurf &s) {
    Operation::setSTL(s);
}

void PointDropCutter::setSTLSnapshot(const STLSurf& s, KDTree<Triangle>::Snapshot snapshot) {
    //std::cout << "PointDropCutter::setSTL()\n";
    root->setXYDimensions(); // we search for triangles in the XY plane, don't care about Z-coordinate
    root->setBucketSize( bucketSize );
    root->buildSnapshot(std::move(snapshot));
    surf = &s;
}

void PointDropCutter::run(CLPoint& clp) {
    //std::cout << "PointDropCutter::run() clp= " << clp << " dropped to ";
    pointDropCutter1(clp);
    //std::cout  << clp << " nCalls = " << nCalls <<"\n ";
}

// Each query owns its CLPoint; the tree is read-only during traversal.
void PointDropCutter::pointDropCutter1(CLPoint& clp) {
    nCalls = dropCutter(clp);
}

int PointDropCutter::dropCutter(CLPoint& clp) const {
    validateInputs();
    int calls = 0;
    root->visit_cutter_overlap(cutter, &clp, [&](const Triangle& t) {
        if (cutter->overlaps(clp,t) && clp.below(t)) {
            cutter->dropCutter(clp,t);
            ++calls;
        }
    });
    return calls;
}

}// end namespace
// end file pointdropcutter.cpp
