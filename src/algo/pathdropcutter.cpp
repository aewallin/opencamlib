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

#include <boost/foreach.hpp>

#include "millingcutter.h"
#include "clpoint.h"
#include "pathdropcutter.h"

namespace ocl
{

//********   ********************** */

PathDropCutter::PathDropCutter() {
    cutter = NULL;
    surf = NULL;
    path = NULL;
    minimumZ = 0.0;
    bdc = new BatchDropCutter(); // we delegate to BatchDropCutter, who does the heavy lifting
    sampling = 0.1;
}

PathDropCutter::~PathDropCutter() {
    delete bdc;
}

void PathDropCutter::setSTL(const STLSurf& s) {
    bdc->setSTL( s );
    surf = &s;
}

void PathDropCutter::setCutter(MillingCutter *c) {
    bdc->setCutter( c );
    cutter = c;
}

void PathDropCutter::setPath(const Path *p) {
    path = p;
}

void PathDropCutter::run() {
    uniform_sampling_run();

}

void PathDropCutter::uniform_sampling_run() {
    clpoints.clear();
    BOOST_FOREACH( const Span* span, path->span_list ) { // loop through the spans calling run() on each
        this->sample_span(span); // append points to bdc
    }
    bdc->run(); // run the actual drop-cutter on all points
    clpoints = bdc->getCLPoints();
}

// this samples the Span and pushes the corresponding sampled points to bdc
void PathDropCutter::sample_span(const Span* span)
{
    assert( sampling > 0.0 );
    unsigned int num_steps = (unsigned int)(span->length2d() / sampling + 1);
    for(unsigned int i = 0; i<=num_steps; i++) {
        double fraction = (double)i / num_steps;
        Point ptmp = span->getPoint(fraction);
        CLPoint* p = new CLPoint(ptmp.x, ptmp.y, ptmp.z);
        p->z = minimumZ;
        bdc->appendPoint( *p );
    }    
}

} // end namespace
// end file pathdropcutter.cpp
