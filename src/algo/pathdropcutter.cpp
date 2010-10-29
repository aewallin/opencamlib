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

void PathDropCutter::adaptive_sampling_run() {
    clpoints.clear();
    BOOST_FOREACH( const Span* span, path->span_list ) {
        adaptive_sample( span, 0.0, 1.0, 0.0, 0.0);
    }
}

void PathDropCutter::adaptive_sample(const Span* span, double start_t, double stop_t, double start_z, double stop_z) {
    double mid_t = (stop_t-start_t)/2.0; // mid point sample
    double mid_z = drop_cutter_height(span, mid_t);
    if (0) { // below sampling-limit, need to sample more
    
    } else if ( flat(start_z,mid_z,stop_z) ) {
        //line start_t, stop_t
    } else {
        // need to sample further
        adaptive_sample( span, start_t, mid_t , start_z, mid_z  );
        adaptive_sample( span, mid_t  , stop_t, mid_z  , stop_z );
    }
}

bool PathDropCutter::flat(double start_z, double mid_z, double stop_z) {
    double dz = fabs(mid_z-start_z)+fabs(mid_z-stop_z);
    return (dz > 1e-3);
}

double PathDropCutter::drop_cutter_height(const Span* span, double t) {
    // find triangles under cutter
    // drop against each
    // return height of point (or highest CL-point)
    CLPoint* p = new CLPoint( span->getPoint(t) );
    p->z = minimumZ;
    bdc->appendPoint( *p );
    bdc->run();
    
    return 0.0;
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
