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

#ifdef _OPENMP
    #include <omp.h>
#endif

#include "millingcutter.h"
#include "point.h"
#include "triangle.h"
#include "line.h"
#include "path.h"

#include "adaptivewaterline.h"

#include "weave.h"
#include "fiberpushcutter.h"


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

}

AdaptiveWaterline::~AdaptiveWaterline() {
    delete subOp[1];
    delete subOp[0];
}

void AdaptiveWaterline::run() {
}

void AdaptiveWaterline::adaptive_sampling_run() {
    
    minx = surf->bb.minpt.x - 2*cutter->getRadius();
    maxx = surf->bb.maxpt.x + 2*cutter->getRadius();

    miny = surf->bb.minpt.y - 2*cutter->getRadius();
    maxy = surf->bb.maxpt.y + 2*cutter->getRadius();
    
    Line* line = new Line( Point(minx,miny,zh) , Point(maxx,maxy,zh) );
    Span* linespan = new LineSpan(*line);
    
    xfibers.clear();
    Point xstart_p1 = Point( minx, linespan->getPoint(0.0).y,  zh );
    Point xstart_p2 = Point( maxx, linespan->getPoint(0.0).y,  zh );
    
    Point xstop_p1 = Point( minx, linespan->getPoint(1.0).y,  zh );
    Point xstop_p2 = Point( maxx, linespan->getPoint(1.0).y,  zh );
    
    Fiber start_f = Fiber( xstart_p1, xstart_p2 ) ;
    Fiber stop_f = Fiber( xstop_p1, xstop_p2 ); 
    subOp[0]->run(start_f);
    subOp[0]->run(stop_f);
    xfibers.push_back(start_f);
    adaptive_sample( linespan, 0.0, 1.0, start_f, stop_f);
    
    
}


void AdaptiveWaterline::adaptive_sample(const Span* span, double start_t, double stop_t, Fiber start_f, Fiber stop_f) {
    const double mid_t = start_t + (stop_t-start_t)/2.0; // mid point sample
    assert( mid_t > start_t );  assert( mid_t < stop_t );
    
    Point mid_p1 = Point( minx, span->getPoint( mid_t ).y,  zh );
    Point mid_p2 = Point( maxx, span->getPoint( mid_t ).y,  zh );
    //CLPoint mid_cl = span->getPoint(mid_t);
    Fiber mid_f = Fiber( mid_p1, mid_p2 );
    subOp[0]->run( mid_f );
    double fw_step = fabs( start_f.p1.y - stop_f.p1.y ) ;
    if ( fw_step > sampling ) { // above minimum step-forward, need to sample more
        adaptive_sample( span, start_t, mid_t , start_f, mid_f  );
        adaptive_sample( span, mid_t  , stop_t, mid_f  , stop_f );
    } else if ( !flat(start_f,mid_f,stop_f)   ) {
        if (fw_step > min_sampling) { // not a a flat segment, and we have not reached maximum sampling
            adaptive_sample( span, start_t, mid_t , start_f, mid_f  );
            adaptive_sample( span, mid_t  , stop_t, mid_f  , stop_f );
        }
    } 
    xfibers.push_back(stop_f); // finally add the last fiber (?)
    return;
}

bool AdaptiveWaterline::flat( Fiber& start, Fiber& mid, Fiber& stop ) {
    return true;
}

/*
void AdaptiveWaterline::weave_process() {
    std::cout << "Weave..." << std::flush;
    Weave w;
    BOOST_FOREACH( Fiber f, *( subOp[0]->getFibers() ) ) {
        w.addFiber(f);
    }
    BOOST_FOREACH( Fiber f, *( subOp[1]->getFibers() ) ) {
        w.addFiber(f);
    }
    std::cout << "build()..." << std::flush;
    w.build(); // build weave from fibers
    std::cout << "split()..." << std::flush;
    std::vector<Weave> subweaves = w.split_components(); // split into components
    std::cout << "traverse()..." << std::flush;
    std::vector< std::vector<Point> > subweave_loops;
    BOOST_FOREACH( Weave sw, subweaves ) {
        sw.face_traverse(); // traverse to find loops
        subweave_loops = sw.getLoops();
        BOOST_FOREACH( std::vector<Point> loop, subweave_loops ) {
            this->loops.push_back( loop );
        }
    }
    std::cout << "done.\n" << std::flush;
} */     


}// end namespace
// end file adaptivewaterline.cpp
