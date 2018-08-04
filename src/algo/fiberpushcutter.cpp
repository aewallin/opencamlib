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
#include <boost/progress.hpp>

#ifdef _OPENMP  
    #include <omp.h>
#endif

#include "millingcutter.hpp"
#include "point.hpp"
#include "triangle.hpp"
#include "fiberpushcutter.hpp"

namespace ocl
{

//********   ********************** */

FiberPushCutter::FiberPushCutter() {
    nCalls = 0;
    nthreads = 1;
#ifdef _OPENMP
    nthreads = omp_get_num_procs(); // figure out how many cores we have
#endif
    cutter = NULL;
    bucketSize = 1;
    root = new KDTree<Triangle>();
}

FiberPushCutter::~FiberPushCutter() {
    delete root;
}

void FiberPushCutter::setSTL(const STLSurf &s) {
    surf = &s;
    std::cout << "BPC::setSTL() Building kd-tree... bucketSize=" << bucketSize << "..";
    root->setBucketSize( bucketSize );
    if (x_direction)
        root->setYZDimensions(); 
    else if (y_direction)
        root->setXZDimensions();
    else {
        std::cout << " ERROR: setXDirection() or setYDirection() must be called before setSTL() \n";
        assert(0);
    }
    std::cout << "BPC::setSTL() root->build()";
    root->build(s.tris);
    std::cout << " done.\n";
}

void FiberPushCutter::pushCutter1(Fiber& f) {
    nCalls = 0;
    BOOST_FOREACH( const Triangle& t, surf->tris) {// test against all triangles in s
        Interval i;
        cutter->pushCutter(f,i,t);
        f.addInterval(i);
        ++nCalls;
    }
}

void FiberPushCutter::pushCutter2(Fiber& f) {
    std::list<Triangle>::iterator it,it_end;    // for looping over found triangles
    Interval* i;
    std::list<Triangle>* tris;
    CLPoint cl;
    if ( x_direction ) {
        cl.x=0;
        cl.y=f.p1.y;
        cl.z=f.p1.z;
    } else if (y_direction ) {
        cl.x=f.p1.x;
        cl.y=0;
        cl.z=f.p1.z;
    }
    tris = root->search_cutter_overlap(cutter, &cl);
    it_end = tris->end();
    for ( it=tris->begin() ; it!=it_end ; ++it) {
		i = new Interval();
		cutter->pushCutter(f,*i,*it);
		f.addInterval(*i); 
		++nCalls;
		delete i;
    }
    delete( tris );
}

}// end namespace
// end file fiberpushcutter.cpp
