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

#include <boost/foreach.hpp>

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
    subOp.push_back( new BatchDropCutter() );  // we delegate to BatchDropCutter, who does the heavy lifting
    sampling = 0.1;
}

PathDropCutter::~PathDropCutter() {
    delete subOp[0];
    subOp.clear();
}

void PathDropCutter::setPath(const Path *p) {
    path = p;
    // ((BatchDropCutter*)(subOp[0]))->clearCLPoints();
    subOp[0]->clearCLPoints();
}

void PathDropCutter::run() {
    uniform_sampling_run();

}

void PathDropCutter::uniform_sampling_run() {
    clpoints.clear();
    BOOST_FOREACH( const Span* span, path->span_list ) { // loop through the spans calling run() on each
        this->sample_span(span); // append points to bdc
    }
    subOp[0]->run();
    clpoints = subOp[0]->getCLPoints();
}

// this samples the Span and pushes the corresponding sampled points to bdc
void PathDropCutter::sample_span(const Span* span) {
    assert( sampling > 0.0 );
    unsigned int num_steps = (unsigned int)(span->length2d() / sampling + 1);
    for(unsigned int i = 0; i<=num_steps; i++) {
        double fraction = (double)i / num_steps;
        Point ptmp = span->getPoint(fraction);
        CLPoint* p = new CLPoint(ptmp.x, ptmp.y, ptmp.z);
        p->z = minimumZ;
        subOp[0]->appendPoint( *p );
        delete p;
    }    
}

} // end namespace
// end file pathdropcutter.cpp
