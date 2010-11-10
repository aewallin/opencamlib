/*  $Id: $
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

#ifdef _OPENMP  
    #include <omp.h>
#endif

#include "millingcutter.h"
#include "point.h"
#include "triangle.h"
#include "fiberpushcutter.h"
#include "kdtree3.h"

namespace ocl
{

//********   ********************** */

FiberPushCutter::FiberPushCutter() {
    //fibers = new std::vector<Fiber>();
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
    //delete fibers;
    delete root;
}

void FiberPushCutter::setSTL(const STLSurf &s) {
    surf = &s;
    std::cout << "BPC::setSTL() Building kd-tree... bucketSize=" << bucketSize << "..";
    root->setBucketSize( bucketSize );
    if (x_direction)
        root->setYZDimensions(); // we search for triangles in the XY plane, don't care about Z-coordinate
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

//void BatchPushCutter::appendFiber(Fiber& f) {
//    fibers->push_back(f);
//}

/// very simple batch push-cutter
/// each fiber is tested against all triangles of surface
void FiberPushCutter::pushCutter1(Fiber& f) {
    //std::cout << "FiberPushCutter1 with " << surf->tris.size() << " triangles..." << std::endl;
    nCalls = 0;
    //boost::progress_display show_progress( fibers->size() );
    
    BOOST_FOREACH( const Triangle& t, surf->tris) {// test against all triangles in s
        Interval i;
        cutter->vertexPush(f,i,t);
        cutter->facetPush(f,i,t);
        cutter->edgePush(f,i,t);
        f.addInterval(i);
        ++nCalls;
    }
        //++show_progress;
    
    //std::cout << "BatchPushCutter done." << std::endl;
    return;
}


/*
void FiberPushCutter::pushCutter2(Fiber& f) {
    std::cout << "BatchPushCutter3 with " << fibers->size() << 
              " fibers and " << surf->tris.size() << " triangles." << std::endl;
    std::cout << " cutter = " << cutter->str() << "\n";
    nCalls = 0;
    boost::progress_display show_progress( fibers->size() );
#ifdef _OPENMP
    omp_set_num_threads(nthreads);
    //omp_set_nested(1);
#endif
    unsigned int Nmax = fibers->size();         // the number of fibers to process
    std::list<Triangle>::iterator it,it_end;    // for looping over found triabgles
    Interval* i;
    std::list<Triangle>* tris;
    std::vector<Fiber>& fiberr = *fibers;
    unsigned int n; // loop variable
    unsigned int calls=0;
    
    #pragma omp parallel for schedule(dynamic) shared(calls, fiberr) private(n,i,tris,it,it_end)
    //#pragma omp parallel for shared( calls, fiberr) private(n,i,tris,it,it_end)
    for (n=0; n<Nmax; ++n) {
#ifdef _OPENMP
        if ( n== 0 ) { // first iteration
            if (omp_get_thread_num() == 0 ) 
                std::cout << "Number of OpenMP threads = "<< omp_get_num_threads() << "\n";
        }
#endif  
        tris = new std::list<Triangle>();
        CLPoint cl;
        if ( x_direction ) {
            cl.x=0;
            cl.y=fiberr[n].p1.y;
            cl.z=fiberr[n].p1.z;
        } else if (y_direction ) {
            cl.x=fiberr[n].p1.x;
            cl.y=0;
            cl.z=fiberr[n].p1.z;
        }
        tris = root->search_cutter_overlap(cutter, &cl);
        it_end = tris->end();
        for ( it=tris->begin() ; it!=it_end ; ++it) {
            //if ( bb->overlaps( it->bb ) ) {
                // todo: optimization where method-calls are skipped if triangle bbox already in the fiber
                i = new Interval();
                cutter->vertexPush(fiberr[n],*i,*it);  
                cutter->facetPush(fiberr[n],*i,*it);  
                cutter->edgePush(fiberr[n],*i,*it);  
                fiberr[n].addInterval(*i); 
                ++calls;
            //}
        }
        delete( tris );
        ++show_progress;
    } // OpenMP parallel region ends here
    
    this->nCalls = calls;
    std::cout << "\nBatchPushCutter3 done." << std::endl;
    return;
}*/

}// end namespace
// end file fiberpushcutter.cpp
