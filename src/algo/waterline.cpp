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

#include <boost/foreach.hpp> 

#ifdef _OPENMP
    #include <omp.h>
#endif

#include "millingcutter.hpp"
#include "point.hpp"
#include "triangle.hpp"
#include "waterline.hpp"
#include "batchpushcutter.hpp"
// #include "weave.hpp"
#include "simple_weave.hpp"
#include "smart_weave.hpp"

namespace ocl
{

//********   ********************** */

Waterline::Waterline() {
    subOp.clear();
    subOp.push_back( new BatchPushCutter() );
    subOp.push_back( new BatchPushCutter() );
    subOp[0]->setXDirection();
    subOp[1]->setYDirection();
    nthreads=1;
#ifdef _OPENMP
    nthreads = omp_get_num_procs(); 
    //omp_set_dynamic(0);
    omp_set_nested(1);
#endif

}

Waterline::~Waterline() {
    // std::cout << "~Waterline(): subOp.size()= " << subOp.size() <<"\n";
    delete subOp[1];
    delete subOp[0];
    subOp.clear();
}


// run the batchpuschutter sub-operations to get x- and y-fibers
// pass the fibers to weave, and process the weave to get waterline-loops
void Waterline::run2() {
    init_fibers();
    subOp[0]->run(); // these two are independent, so could/should run in parallel
    subOp[1]->run();
    
    xfibers = *( subOp[0]->getFibers() );
    yfibers = *( subOp[1]->getFibers() );
    
    weave_process2();
}

void Waterline::run() {
    init_fibers();
    subOp[0]->run(); // these two are independent, so could/should run in parallel
    subOp[1]->run();
    
    xfibers = *( subOp[0]->getFibers() );
    yfibers = *( subOp[1]->getFibers() );
    
    weave_process();
}


void Waterline::reset() {
    xfibers.clear();
    yfibers.clear();
    subOp[0]->reset();
    subOp[1]->reset();
}

void Waterline::weave_process() {
    // std::cout << "Weave...\n" << std::flush;
    weave::SimpleWeave weave;
    BOOST_FOREACH( Fiber f, xfibers ) {
        weave.addFiber(f);
    }
    BOOST_FOREACH( Fiber f, yfibers ) {
        weave.addFiber(f);
    }
   
    //std::cout << "Weave::build()..." << std::flush;
    weave.build(); 
    // std::cout << "done.\n";
    
    // std::cout << "Weave::face traverse()...";
    weave.face_traverse();
    // std::cout << "done.\n";

    // std::cout << "Weave::get_loops()...";
    loops = weave.getLoops();
    // std::cout << "done.\n";   
}

void Waterline::weave_process2() {
    // std::cout << "Weave...\n" << std::flush;
    weave::SmartWeave weave;
    BOOST_FOREACH( Fiber f, xfibers ) {
        weave.addFiber(f);
    }
    BOOST_FOREACH( Fiber f, yfibers ) {
        weave.addFiber(f);
    }
   
    //std::cout << "Weave::build2()..." << std::flush;
    weave.build(); 
    // std::cout << "done.\n";
    
    // std::cout << "Weave::face traverse()...";
    weave.face_traverse();
    // std::cout << "done.\n";

    // std::cout << "Weave::get_loops()...";
    loops = weave.getLoops();
    // std::cout << "done.\n";   
}

void Waterline::init_fibers() {
    // std::cout << " Waterline::init_fibers()\n";
    double minx = surf->bb.minpt.x - 2*cutter->getRadius();
    double maxx = surf->bb.maxpt.x + 2*cutter->getRadius();
    double miny = surf->bb.minpt.y - 2*cutter->getRadius();
    double maxy = surf->bb.maxpt.y + 2*cutter->getRadius();
    int Nx = (int)( (maxx-minx)/sampling );
    int Ny = (int)( (maxy-miny)/sampling );
    std::vector<double> xvals = generate_range(minx,maxx,Nx);
    std::vector<double> yvals = generate_range(miny,maxy,Ny);
    BOOST_FOREACH( double y, yvals ) {
        Point p1 = Point( minx, y, zh );
        Point p2 = Point( maxx, y, zh );
        Fiber f = Fiber( p1 , p2 );
        subOp[0]->appendFiber( f );
    }
    BOOST_FOREACH( double x, xvals ) {
        Point p1 = Point( x, miny,  zh );
        Point p2 = Point( x, maxy,  zh );
        Fiber f = Fiber( p1 , p2 );
        subOp[1]->appendFiber( f );
    }
}

// return a double-vector [ start , ... , end ] with N elements
// for generating fibers.
std::vector<double> Waterline::generate_range( double start, double end, int N) const {
    std::vector<double> output;
    double d = (end-start)/ (double)N;
    double v = start;
    for (int n=0; n< (N+1); ++n) {
        output.push_back( v );
        v=v+d;
    }
    return output;
}

}// end namespace
// end file waterline.cpp
