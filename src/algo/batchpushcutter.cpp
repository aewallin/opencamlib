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
#include <boost/progress.hpp>

#ifdef _OPENMP  
    #include <omp.h>
#endif

#include "millingcutter.hpp"
#include "point.hpp"
#include "triangle.hpp"
#include "batchpushcutter.hpp"

namespace ocl
{

//********   ********************** */

BatchPushCutter::BatchPushCutter() {
    fibers = new std::vector<Fiber>();
    nCalls = 0;
    nthreads = 1;
#ifdef _OPENMP
    nthreads = omp_get_num_procs(); // figure out how many cores we have
#endif
    cutter = NULL;
    bucketSize = 1;
    root = new KDTree<Triangle>();
}

BatchPushCutter::~BatchPushCutter() {
    delete fibers;
    delete root;
}

void BatchPushCutter::setSTL(const STLSurf &s) {
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
    std::cout << "BPC::setSTL() root->build()...";
    root->build(s.tris);
    std::cout << "done.\n";
}

void BatchPushCutter::appendFiber(Fiber& f) {
    fibers->push_back(f);
}

void BatchPushCutter::reset() {
    fibers->clear();
}

/// very simple batch push-cutter
/// each fiber is tested against all triangles of surface
void BatchPushCutter::pushCutter1() {
    std::cout << "BatchPushCutter1 with " << fibers->size() << 
              " fibers and " << surf->tris.size() << " triangles..." << std::endl;
    nCalls = 0;
    boost::progress_display show_progress( fibers->size() );
    BOOST_FOREACH(Fiber& f, *fibers) {
        BOOST_FOREACH( const Triangle& t, surf->tris) {// test against all triangles in s
            Interval i;
            cutter->pushCutter(f,i,t);
            f.addInterval(i);
            ++nCalls;
        }
        ++show_progress;
    }
    std::cout << "BatchPushCutter done." << std::endl;
    return;
}

/// push-cutter which uses KDNode2 kd-tree search to find triangles 
/// overlapping with the cutter.
void BatchPushCutter::pushCutter2() {
    std::cout << "BatchPushCutter2 with " << fibers->size() << 
              " fibers and " << surf->tris.size() << " triangles..." << std::endl;
    nCalls = 0;
    std::list<Triangle>* overlap_triangles;
    boost::progress_display show_progress( fibers->size() );
    BOOST_FOREACH(Fiber& f, *fibers) {
        CLPoint cl;
        if (x_direction) {
            cl.x = 0;
            cl.y = f.p1.y;
            cl.z = f.p1.z;
        } else if (y_direction) {
            cl.x = f.p1.x;
            cl.y = 0;
            cl.z = f.p1.z;
        } else {
            assert(0);
        }
        overlap_triangles = root->search_cutter_overlap(cutter, &cl);
        assert( overlap_triangles->size() <= surf->size() ); // can't possibly find more triangles than in the STLSurf 
        BOOST_FOREACH( const Triangle& t, *overlap_triangles) {
            //if ( bb->overlaps( t.bb ) ) {
                Interval i;
                cutter->pushCutter(f,i,t);
                f.addInterval(i);
                ++nCalls;
            //}
        }
        delete( overlap_triangles );
        ++show_progress;
    }
    std::cout << "BatchPushCutter2 done." << std::endl;
    return;
}

/// use kd-tree search to find overlapping triangles
/// use OpenMP for multi-threading
void BatchPushCutter::pushCutter3() {
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
#ifdef _WIN32 // OpenMP version 2 of VS2013 OpenMP need signed loop variable
	int n; // loop variable
#else
    unsigned int n; // loop variable
#endif
    unsigned int calls=0;
    
    #pragma omp parallel for schedule(dynamic) shared(calls, fiberr) private(n,i,tris,it,it_end)
    //#pragma omp parallel for shared( calls, fiberr) private(n,i,tris,it,it_end)
    for (n=0; n<Nmax; ++n) { // loop through all fibers
#ifdef _OPENMP
        if ( n== 0 ) { // first iteration
            if (omp_get_thread_num() == 0 ) 
                std::cout << "Number of OpenMP threads = "<< omp_get_num_threads() << "\n";
        }
#endif  
        CLPoint cl; // cl-point on the fiber
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
        for ( it=tris->begin() ; it!=it_end ; ++it) { // loop through the found overlapping triangles
            //if ( bb->overlaps( it->bb ) ) {
                // todo: optimization where method-calls are skipped if triangle bbox already in the fiber
                i = new Interval();
                cutter->pushCutter(fiberr[n],*i,*it);  
                fiberr[n].addInterval(*i); 
                ++calls;
                delete i;
            //}
        }
        delete( tris );
        ++show_progress;
    } // OpenMP parallel region ends here
    
    this->nCalls = calls;
    std::cout << "\nBatchPushCutter3 done." << std::endl;
    return;
}

}// end namespace
// end file batchpushcutter.cpp
