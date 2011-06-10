/*  $Id$
 * 
 *  Copyright 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com)
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
#include "waterline.h"

#include "weave.h"
#include "batchpushcutter.h"

#include "weave2.h"

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
    std::cout << "~Waterline()\n";
    std::cout << "~Waterline(): subOp.size()= " << subOp.size() <<"\n";
    subOp.clear();
    /*BOOST_FOREACH( Operation* op, subOp) {
        if ( op) {
            std::cout << " deleting " << op << "\n";
            delete op;
            op = 0;
        }
    }*/
    
   // delete subOp[0];
}

void Waterline::run_old() {
    this->init_fibers(); // create fibers and push them to bpc_x and bpc_y
    // bpc_x->setThreads(nthreads);
    // bpc_y->setThreads(nthreads);

    // run the X-direction and Y-direction in parallel using OpenMP tasks
    // see: http://wikis.sun.com/display/openmp/Using+the+Tasking+Feature
    // see: http://docs.sun.com/source/819-0501/2_nested.html
    //omp_set_num_threads(8);
    //omp_set_nested(1);
    // #pragma omp parallel 
    //{
    //    #pragma omp single nowait
    //    {
    //        #pragma omp task
            { subOp[0]->run(); }
    //        #pragma omp task
            { subOp[1]->run(); }
    //    }
    //}
    
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
    std::vector<Weave> subweaves = w.split_components(); // split into connected components
    std::cout << "traverse()..." << std::flush;
    std::vector< std::vector<Point> > subweave_loops;
    BOOST_FOREACH( Weave sw, subweaves ) { // go through all connected components
        sw.face_traverse(); // traverse to find loops
        subweave_loops = sw.getLoops();
        BOOST_FOREACH( std::vector<Point> loop, subweave_loops ) {
            this->loops.push_back( loop );
        }
    }
    std::cout << "done.\n" << std::flush;
}

// this will become the new faster version of the algorithm which uses Weave2
void Waterline::run() {
    this->init_fibers();
    subOp[0]->run();
    subOp[1]->run();
    
    weave2_process();
}

void Waterline::weave2_process() {
    std::cout << "Weave2..." << std::flush;
    weave2::Weave w;
    BOOST_FOREACH( Fiber f, *( subOp[0]->getFibers() ) ) {
        w.addFiber(f);
    }
    BOOST_FOREACH( Fiber f, *( subOp[1]->getFibers() ) ) {
        w.addFiber(f);
    }
   
    std::cout << "build()..." << std::flush;
    w.build(); // build weave from fibers
    std::cout << "done.\n";
    std::cout << "face traverse()\n";
    w.face_traverse();
    std::cout << "DONE face traverse()\n";
    std::cout << "get_loops()\n";
    std::vector< std::vector<Point> > weave_loops = w.getLoops();
    BOOST_FOREACH( std::vector<Point> loop, weave_loops ) {
        this->loops.push_back( loop );
    }
    std::cout << "DONE get_loops()\n";   
}

void Waterline::init_fibers() {
    std::cout << " Waterline::init_fibers()\n";
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
