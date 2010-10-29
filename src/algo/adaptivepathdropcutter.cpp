/*  $Id:  $
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
#include "pointdropcutter.h"
#include "adaptivepathdropcutter.h"

namespace ocl
{

//********   ********************** */

AdaptivePathDropCutter::AdaptivePathDropCutter() {
    cutter = NULL;
    surf = NULL;
    path = NULL;
    minimumZ = 0.0;
    pdc = new PointDropCutter(); // we delegate to BatchDropCutter, who does the heavy lifting
    sampling = 0.1;
}

AdaptivePathDropCutter::~AdaptivePathDropCutter() {
    delete pdc;
}

void AdaptivePathDropCutter::setSTL(const STLSurf& s) {
    pdc->setSTL( s );
    surf = &s;
}

void AdaptivePathDropCutter::setCutter(MillingCutter *c) {
    pdc->setCutter( c );
    cutter = c;
}

void AdaptivePathDropCutter::run() {
    adaptive_sampling_run();
}

void AdaptivePathDropCutter::adaptive_sampling_run() {
    clpoints.clear();
    BOOST_FOREACH( const Span* span, path->span_list ) {
        CLPoint start = span->getPoint(0.0);
        CLPoint stop = span->getPoint(1.0);
        pdc->run(start);
        pdc->run(stop);
        clpoints.push_back(start);
        std::cout << " starting on new span.\n";
        adaptive_sample( span, 0.0, 1.0, start, stop);
    }
}

void AdaptivePathDropCutter::adaptive_sample(const Span* span, double start_t, double stop_t, CLPoint start_cl, CLPoint stop_cl) {
    std::cout << " (start, stop) = ( "<< start_t << " , " << stop_t << " )\n" << std::flush;
    
    double mid_t = (stop_t-start_t)/2.0; // mid point sample
    CLPoint mid_cl = span->getPoint(mid_t);
    pdc->run( mid_cl );
    start_cl = span->getPoint(start_t);
    pdc->run( start_cl );
    stop_cl = span->getPoint(stop_t);
    pdc->run( stop_cl );
    //std::cout << " sta=" << start_cl << "\n";
    //std::cout << " mid=" << mid_cl << "\n";
    //std::cout << " stop=" << stop_cl << "\n";
    //std::cout << " pdc->run() done \n";
    //std::cout << " (stop_cl-start_cl).xyNorm() = " << (stop_cl-start_cl).xyNorm() << "\n";
    //char c;
    if ( (stop_cl-start_cl).xyNorm() > sampling ) { // above minimum step-forward, need to sample more
        std::cout << " forward-step= " << (stop_cl-start_cl).xyNorm() << " subdividing.\n";
        //std::cin >> c;
        std::cout << " start - mid call() \n";
        adaptive_sample( span, start_t, mid_t , start_cl, mid_cl  );
        std::cout << " mid - stop call() \n";
        //adaptive_sample( span, mid_t  , stop_t, mid_cl  , stop_cl );
    } else {
        std::cout << " adding point " << stop_cl << "...";
        //clpoints.push_back(stop_cl); 
        std::cout << " done. \n";
        //std::cout << " t=" << start_t << " sta=" << start_cl << "\n";
        //std::cout << " t=" << mid_t << " mid=" << mid_cl << "\n";
        //std::cout << " t=" << stop_t << " stop=" << stop_cl << "\n";
    
        //std::cin >> c;
        //CLPoint* p = new CLPoint(stop_cl);
        
    } 
    /*
    else if ( flat(start_cl,mid_cl,stop_cl) ) { // sufficient sampling, and flat, so store values:
        //clpoints.push_back(start_cl);
        clpoints.push_back(stop_cl); // line start_t, stop_t
    } else {
        // sufficient sampling, but not flat enough.
        // need to sample further.
        adaptive_sample( span, start_t, mid_t , start_cl, mid_cl  );
        adaptive_sample( span, mid_t  , stop_t, mid_cl  , stop_cl );
    }*/
    return;
}

bool AdaptivePathDropCutter::flat(CLPoint& start_cl, CLPoint& mid_cl, CLPoint& stop_cl)  {
    CLPoint v1 = mid_cl-start_cl;
    CLPoint v2 = stop_cl-mid_cl;
    v1.normalize();
    v2.normalize();
    double dotprod = v1.dot(v2);
    //double dz = fabs(mid_z-start_z)+fabs(mid_z-stop_z);
    //return (dz > 1e-3);
    return (dotprod>0.2);
}




} // end namespace

