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
#include <boost/progress.hpp>


#ifndef WIN32
    #include <omp.h>
#endif

#include "millingcutter.h"
#include "point.h"
#include "triangle.h"
#include "waterline.h"

#include "weave.h"
#include "batchpushcutter.h"
#include "kdtree2.h"

namespace ocl
{

//********   ********************** */

Waterline::Waterline() {
    bpc = new BatchPushCutter();
}
Waterline::~Waterline() {
    delete bpc;
}

void Waterline::setSTL(const STLSurf& s) {
    bpc->setSTL( s );
    surface = &s;
}
void Waterline::setCutter(const MillingCutter& c) {
    bpc->setCutter( &c );
    cutter = &c;
}
void Waterline::setTolerance(const double tol) {
    tolerance = tol;
}
void Waterline::setZ(const double z) {
    zh = z;
}
        
void Waterline::run() {
    this->init_fibers();
    // run the actual push-cutter
    bpc->pushCutter3();
    // run the weave-analysis to generate the loops
    // std::vector<Fiber> fibers = *(bpc->fibers);
    std::cout << "Weave..." << std::flush;
    Weave w;
    int n=0;
    BOOST_FOREACH( Fiber f, *(bpc->fibers) ) {
        w.addFiber(f);
        ++n;
    }
    std::cout << "build()..." << std::flush;
    w.build(); // build weave from fibers
    //w.printGraph();
    std::cout << "split()..." << std::flush;
    std::vector<Weave> subweaves = w.split_components(); // split into components
    //std::cout << " weave split into " << subweaves.size() << " components\n";
    std::cout << "traverse()..." << std::flush;
    std::vector< std::vector<Point> > subweave_loops;
    BOOST_FOREACH( Weave sw, subweaves ) {
        sw.face_traverse();
        subweave_loops = sw.getLoops();
        BOOST_FOREACH( std::vector<Point> loop, subweave_loops ) {
            this->loops.push_back( loop );
        }
    }
    std::cout << "done.\n" << std::flush;
}

void Waterline::init_fibers() {
    //std::cout << " cutter->getRadius()=" << cutter->getRadius() << "\n";
    double minx = surface->bb.minpt.x - 2*cutter->getRadius();
    double maxx = surface->bb.maxpt.x + 2*cutter->getRadius();
    double miny = surface->bb.minpt.y - 2*cutter->getRadius();
    double maxy = surface->bb.maxpt.y + 2*cutter->getRadius();
    int Nx = (int)( (maxx-minx)/tolerance );
    int Ny = (int)( (maxy-miny)/tolerance );
    //std::cout << minx << "  " <<  maxx << "  " << miny << "  " << maxy <<  "\n";
    //std::cout << " tolerance= " << tolerance << " Nx=" << Nx << " Ny=" << Ny << "\n";
    std::vector<double> xvals = generate_range(minx,maxx,Nx);
    std::vector<double> yvals = generate_range(miny,maxy,Ny);
    BOOST_FOREACH( double y, yvals ) {
        Point p1 = Point( minx, y, zh );
        Point p2 = Point( maxx, y, zh );
        Fiber f = Fiber( p1 , p2 );
        bpc->appendFiber( f );
    }
    BOOST_FOREACH( double x, xvals ) {
        Point p1 = Point( x, miny,  zh );
        Point p2 = Point( x, maxy,  zh );
        Fiber f = Fiber( p1 , p2 );
        bpc->appendFiber( f );
    }
}        

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

boost::python::list Waterline::py_getLoops() const {
    boost::python::list loop_list;
    BOOST_FOREACH( std::vector<Point> loop, this->loops ) {
        boost::python::list point_list;
        BOOST_FOREACH( Point p, loop ) {
            point_list.append( p );
        }
        loop_list.append(point_list);
    }
    return loop_list;
}



}// end namespace

// end file batchpushcutter.cpp
