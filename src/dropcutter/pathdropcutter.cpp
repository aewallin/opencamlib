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


#include "millingcutter.hpp"
#include "clpoint.hpp"
#include "pathdropcutter.hpp"

namespace ocl
{

//********   ********************** */

PathDropCutter::PathDropCutter() {
    cutter = NULL;
    surf = NULL;
    path = NULL;
    minimumZ = 0.0;
    subOp.clear();
    subOp.push_back(std::make_unique<BatchDropCutter>());  // we delegate to BatchDropCutter, who does the heavy lifting
    nthreads = subOp[0]->getThreads();
    sampling = 0.1;
}

PathDropCutter::~PathDropCutter() {
    subOp.clear();
}

void PathDropCutter::setPath(const Path *p) {
    if (!p) throw std::invalid_argument("Path must not be null");
    path = p;
    // ((BatchDropCutter*)(subOp[0]))->clearCLPoints();
    subOp[0]->clearCLPoints();
}

void PathDropCutter::run() {
    uniform_sampling_run();

}

void PathDropCutter::uniform_sampling_run() {
    validateInputs(true);
    if (!path) throw std::logic_error("Set a path before running");
    clpoints.clear();
    subOp[0]->clearCLPoints();
    for (const Span* span : path->span_list) { // loop through the spans calling run() on each
        this->sample_span(span); // append points to bdc
    }
    subOp[0]->run();
    clpoints = subOp[0]->getCLPoints();
}

// this samples the Span and pushes the corresponding sampled points to bdc
void PathDropCutter::sample_span(const Span* span) {
    if (!span) throw std::invalid_argument("Path contains a null span");
    const double steps = span->length2d() / sampling + 1;
    if (!std::isfinite(steps) || steps < 1 ||
        steps >= std::numeric_limits<unsigned int>::max())
        throw std::invalid_argument("Path sampling requires too many points");
    unsigned int num_steps = static_cast<unsigned int>(steps);
    for(unsigned int i = 0; i<=num_steps; i++) {
        double fraction = (double)i / num_steps;
        Point ptmp = span->getPoint(fraction);
        CLPoint p(ptmp.x, ptmp.y, minimumZ);
        subOp[0]->appendPoint(p);
    }    
}

} // end namespace
// end file pathdropcutter.cpp
