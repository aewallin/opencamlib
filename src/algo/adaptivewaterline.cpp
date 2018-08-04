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

#include <algorithm>
#include <vector>


#include <boost/foreach.hpp> 

#ifdef _OPENMP
    #include <omp.h>
#endif

#include "millingcutter.hpp"
#include "point.hpp"
#include "triangle.hpp"
#include "line.hpp"
#include "path.hpp"

#include "adaptivewaterline.hpp"
#include "weave.hpp"
#include "fiberpushcutter.hpp"


namespace ocl
{

//********   ********************** */

AdaptiveWaterline::AdaptiveWaterline() {
    subOp.clear();
    subOp.push_back( new FiberPushCutter() );
    subOp.push_back( new FiberPushCutter() );
    subOp[0]->setXDirection();
    subOp[1]->setYDirection();
    nthreads=1;
#ifdef _OPENMP
    nthreads = omp_get_num_procs(); 
    //omp_set_dynamic(0);
    omp_set_nested(1);
#endif
    sampling = 1.0;
    min_sampling = 0.1;
    cosLimit = 0.999;
}

AdaptiveWaterline::~AdaptiveWaterline() {
    std::cout << "~AdaptiveWaterline(): subOp.size()= " << subOp.size() <<"\n";
    subOp.clear();
}

void AdaptiveWaterline::run() {
    adaptive_sampling_run();
    weave_process(); // in base-class Waterline
}

void AdaptiveWaterline::run2() {
    adaptive_sampling_run();
    weave_process2(); // in base-class Waterline
}

void AdaptiveWaterline::adaptive_sampling_run() {
    minx = surf->bb.minpt.x - 2*cutter->getRadius();
    maxx = surf->bb.maxpt.x + 2*cutter->getRadius();
    miny = surf->bb.minpt.y - 2*cutter->getRadius();
    maxy = surf->bb.maxpt.y + 2*cutter->getRadius();
    Line* line = new Line( Point(minx,miny,zh) , Point(maxx,maxy,zh) );
    Span* linespan = new LineSpan(*line);
    
#ifdef _WIN32 // OpenMP task not supported with the version 2 of VS2013 OpenMP
	#pragma omp parallel sections
	{
		#pragma omp section // Replace OMP Task by Parallel sections
		{ // first child
#else
#pragma omp parallel
	{
#pragma omp single nowait
		{ // initial root task
#pragma omp task
			{ // first child task
#endif // _WIN32
				xfibers.clear();
				Point xstart_p1 = Point(minx, linespan->getPoint(0.0).y, zh);
				Point xstart_p2 = Point(maxx, linespan->getPoint(0.0).y, zh);
				Point xstop_p1 = Point(minx, linespan->getPoint(1.0).y, zh);
				Point xstop_p2 = Point(maxx, linespan->getPoint(1.0).y, zh);
				Fiber xstart_f = Fiber(xstart_p1, xstart_p2);
				Fiber xstop_f = Fiber(xstop_p1, xstop_p2);
				subOp[0]->run(xstart_f);
				subOp[0]->run(xstop_f);
				xfibers.push_back(xstart_f);
				std::cout << " XFiber adaptive sample \n";
				xfiber_adaptive_sample(linespan, 0.0, 1.0, xstart_f, xstop_f);
#ifdef _WIN32 // OpenMP task not supported with the version 2 of VS2013 OpenMP
		}
		#pragma omp section
		{ // second child
#else
		}
# pragma omp task
			{ // second child task
#endif // _WIN32
				yfibers.clear();
				Point ystart_p1 = Point(linespan->getPoint(0.0).x, miny, zh);
				Point ystart_p2 = Point(linespan->getPoint(0.0).x, maxy, zh);
				Point ystop_p1 = Point(linespan->getPoint(1.0).x, miny, zh);
				Point ystop_p2 = Point(linespan->getPoint(1.0).x, maxy, zh);
				Fiber ystart_f = Fiber(ystart_p1, ystart_p2);
				Fiber ystop_f = Fiber(ystop_p1, ystop_p2);
				subOp[1]->run(ystart_f);
				subOp[1]->run(ystop_f);
				yfibers.push_back(ystart_f);
				std::cout << " YFiber adaptive sample \n";
				yfiber_adaptive_sample(linespan, 0.0, 1.0, ystart_f, ystop_f);
#ifdef _WIN32 // OpenMP task not supported with the version 2 of VS2013 OpenMP
		}
	} // end omp parallel
#else
            }
        }
    } // end omp parallel
#endif // _WIN32

    delete line;
    delete linespan;
    
}


void AdaptiveWaterline::xfiber_adaptive_sample(const Span* span, double start_t, double stop_t, Fiber start_f, Fiber stop_f) {
    const double mid_t = start_t + (stop_t-start_t)/2.0; // mid point sample
    assert( mid_t > start_t );  assert( mid_t < stop_t );
    //std::cout << "xfiber sample= ( " << start_t << " , " << stop_t << " ) \n";
    Point mid_p1 = Point( minx, span->getPoint( mid_t ).y,  zh );
    Point mid_p2 = Point( maxx, span->getPoint( mid_t ).y,  zh );
    Fiber mid_f = Fiber( mid_p1, mid_p2 );
    subOp[0]->run( mid_f );
    double fw_step = fabs( start_f.p1.y - stop_f.p1.y ) ;
    if ( fw_step > sampling ) { // above minimum step-forward, need to sample more
        xfiber_adaptive_sample( span, start_t, mid_t , start_f, mid_f  );
        xfiber_adaptive_sample( span, mid_t  , stop_t, mid_f  , stop_f );
    } else if ( !flat(start_f,mid_f,stop_f)   ) {
        if (fw_step > min_sampling) { // not a flat segment, and we have not reached maximum sampling
            xfiber_adaptive_sample( span, start_t, mid_t , start_f, mid_f  );
            xfiber_adaptive_sample( span, mid_t  , stop_t, mid_f  , stop_f );
        }
    } else {
        xfibers.push_back(stop_f);
    } 
}

void AdaptiveWaterline::yfiber_adaptive_sample(const Span* span, double start_t, double stop_t, Fiber start_f, Fiber stop_f) {
    const double mid_t = start_t + (stop_t-start_t)/2.0; // mid point sample
    assert( mid_t > start_t );  assert( mid_t < stop_t );
    //std::cout << "yfiber sample= ( " << start_t << " , " << stop_t << " ) \n";
    Point mid_p1 = Point( span->getPoint( mid_t ).x, miny,  zh );
    Point mid_p2 = Point( span->getPoint( mid_t ).x, maxy, zh );
    Fiber mid_f = Fiber( mid_p1, mid_p2 );
    subOp[1]->run( mid_f );
    double fw_step = fabs( start_f.p1.x - stop_f.p1.x ) ;
    if ( fw_step > sampling ) { // above minimum step-forward, need to sample more
        yfiber_adaptive_sample( span, start_t, mid_t , start_f, mid_f  );
        yfiber_adaptive_sample( span, mid_t  , stop_t, mid_f  , stop_f );
    } else if ( !flat(start_f,mid_f,stop_f)   ) {
        if (fw_step > min_sampling) { // not a flat segment, and we have not reached maximum sampling
            yfiber_adaptive_sample( span, start_t, mid_t , start_f, mid_f  );
            yfiber_adaptive_sample( span, mid_t  , stop_t, mid_f  , stop_f );
        }
    } else {
        yfibers.push_back(stop_f); 
    }
}

// flat predicate to determine when we subdivide
bool AdaptiveWaterline::flat( Fiber& start, Fiber& mid, Fiber& stop ) const {
    if ( start.size() != stop.size() ) // start, mid, and stop need to have same size()
        return false;
    else if ( start.size() != mid.size() )
        return false;
    else if ( mid.size() != stop.size() )
        return false;
    else {
        if (!start.empty() ) { // all now have same size
            assert( start.size() == stop.size() && start.size() == mid.size() );
            for (unsigned int n=0;n<start.size();++n) {
                // now check for angles between cl-points (NOTE: cl-points might not belong to same loop?)
                // however, errors here only lead to dense sampling which is harmless (but slow)
                if ( (!flat( start.upperCLPoint(n) , mid.upperCLPoint(n) , stop.upperCLPoint(n) )) )
                    return false;
                else if (!flat( start.lowerCLPoint(n) , mid.lowerCLPoint(n) , stop.lowerCLPoint(n) )) 
                    return false;
            }
        }
        return true;
    } 
}


bool AdaptiveWaterline::flat(Point start_cl, Point mid_cl, Point stop_cl)  const {
    Point v1 = mid_cl-start_cl;
    Point v2 = stop_cl-mid_cl;
    v1.normalize();
    v2.normalize();
    double dotprod =  v1.dot(v2) ;
    return (dotprod>cosLimit);
}

}// end namespace
// end file adaptivewaterline.cpp
