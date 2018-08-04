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
#include "pointdropcutter.hpp"
#include "adaptivepathdropcutter.hpp"

namespace ocl
{

//********   ********************** */

AdaptivePathDropCutter::AdaptivePathDropCutter() {
    cutter = NULL;
    surf = NULL;
    path = NULL;
    minimumZ = 0.0;
    subOp.clear();
    subOp.push_back( new PointDropCutter() ); // we delegate to PointDropCutter, who does the heavy lifting
    sampling = 0.1;
    min_sampling = 0.01;
    cosLimit = 0.999;
}

AdaptivePathDropCutter::~AdaptivePathDropCutter() {
    //std::cout << " ~AdaptivePathDropCutter() \n";
    delete subOp[0];
}

void AdaptivePathDropCutter::run() {
    adaptive_sampling_run();
}

void AdaptivePathDropCutter::adaptive_sampling_run() {
    //std::cout << " apdc::adaptive_sampling_run()... ";
    
    clpoints.clear();
    BOOST_FOREACH( const Span* span, path->span_list ) {  // this loop could run in parallel, since spans don't depend on eachother
        CLPoint start = span->getPoint(0.0);
        CLPoint stop = span->getPoint(1.0);
        subOp[0]->run(start);
        subOp[0]->run(stop);
        clpoints.push_back(start);
        adaptive_sample( span, 0.0, 1.0, start, stop);
    }
    //std::cout << " DONE clpoints.size()=" << clpoints.size() << "\n";
}

void AdaptivePathDropCutter::adaptive_sample(const Span* span, double start_t, double stop_t, CLPoint start_cl, CLPoint stop_cl) {
    const double mid_t = start_t + (stop_t-start_t)/2.0; // mid point sample
    assert( mid_t > start_t );  assert( mid_t < stop_t );
    CLPoint mid_cl = span->getPoint(mid_t);
    //std::cout << " apdc sampling at " << mid_t << "\n";
    subOp[0]->run( mid_cl );
    double fw_step = (stop_cl-start_cl).xyNorm();
    if ( (fw_step > sampling) || // above minimum step-forward, need to sample more
          ( (!flat(start_cl,mid_cl,stop_cl)) && (fw_step > min_sampling) ) ) { // OR not flat, and not max sampling
        adaptive_sample( span, start_t, mid_t , start_cl, mid_cl  );
        adaptive_sample( span, mid_t  , stop_t, mid_cl  , stop_cl );
    } else {
        clpoints.push_back(stop_cl); 
    }
}

bool AdaptivePathDropCutter::flat(CLPoint& start_cl, CLPoint& mid_cl, CLPoint& stop_cl)  {
    CLPoint v1 = mid_cl-start_cl;
    CLPoint v2 = stop_cl-mid_cl;
    v1.normalize();
    v2.normalize();
    return (v1.dot(v2) > cosLimit);
}

} // end namespace

