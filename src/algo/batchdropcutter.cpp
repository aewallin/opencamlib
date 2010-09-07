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

#ifndef WIN32  // this should really not be a check for Windows, but a check for OpenMP
    #include <omp.h>
#endif

#include "millingcutter.h"

#include "point.h"
#include "triangle.h"
#include "kdtree.h"
#include "batchdropcutter.h"
#include "kdtree2.h"

namespace ocl
{

//********   ********************** */

BatchDropCutter::BatchDropCutter() {
    clpoints = new std::vector<CLPoint>();
    dcCalls = 0;
#ifndef WIN32
    nthreads = omp_get_num_procs(); // figure out how many cores we have
#endif
    cutter = NULL;
    bucketSize = 1;
}

void BatchDropCutter::setSTL(STLSurf &s) {
    surf = &s;
    std::cout << "Building kd-tree... bucketSize=" << bucketSize << "..";
    root = KDNode::build_kdtree( &(surf->tris), bucketSize );
    root2 = KDNode2::build_kdtree( &(surf->tris), bucketSize );
    std::cout << " done.\n";
}

void BatchDropCutter::setCutter(MillingCutter *c) {
    cutter = c;
}

void BatchDropCutter::setThreads(int n) {
    nthreads = n;
}

void BatchDropCutter::appendPoint(CLPoint& p) {
    clpoints->push_back(p);
}

// drop cutter against all triangles in surface
void BatchDropCutter::dropCutter1() {
    std::cout << "dropCutterSTL1 " << clpoints->size() << 
              " cl-points and " << surf->tris.size() << " triangles...";
    dcCalls = 0;
    BOOST_FOREACH(CLPoint &cl, *clpoints) {
        BOOST_FOREACH( const Triangle& t, surf->tris) {// test against all triangles in s
            cutter->dropCutter(cl,t);
            ++dcCalls;
        }
    }
    std::cout << "done.\n";
    return;
}

// first search for triangles under the cutter
// then only drop cutter against found triangles
void BatchDropCutter::dropCutter2() {
    std::cout << "dropCutterSTL2 " << clpoints->size() << 
            " cl-points and " << surf->tris.size() << " triangles.\n";
    std::cout.flush();
    dcCalls = 0;
    std::list<Triangle> *triangles_under_cutter = new std::list<Triangle>();
    BOOST_FOREACH(CLPoint &cl, *clpoints) { //loop through each CL-point
        triangles_under_cutter->clear();
        KDNode::search_kdtree( triangles_under_cutter, cl, *cutter, root); // find triangles under cutter
        BOOST_FOREACH( const Triangle& t, *triangles_under_cutter) {
            cutter->dropCutter(cl,t);
            ++dcCalls;
        }
    }
    std::cout << "done. " << dcCalls << " dropCutter() calls.\n";
    std::cout.flush();
    return;
}

// compared to dropCutter2, add an additional explicit overlap-test before testing triangle
void BatchDropCutter::dropCutter3() {
    std::cout << "dropCutterSTL3 " << clpoints->size() << 
            " cl-points and " << surf->tris.size() << " triangles.\n";
    dcCalls = 0;
    std::list<Triangle> *triangles_under_cutter = new std::list<Triangle>();
    BOOST_FOREACH(CLPoint &cl, *clpoints) { //loop through each CL-point
        triangles_under_cutter->clear();
        KDNode::search_kdtree( triangles_under_cutter, cl, *cutter, root);// find triangles under cutter
        BOOST_FOREACH( const Triangle& t, *triangles_under_cutter) {
            if (cutter->overlaps(cl,t)) {
                cutter->dropCutter(cl,t);
                ++dcCalls;
            }
        }
    }
    std::cout << "done. " << dcCalls << " dropCutter() calls.\n";
    return;
}

// use OpenMP to share work between threads
void BatchDropCutter::dropCutter4() {
    std::cout << "dropCutterSTL4 " << clpoints->size() << 
            " cl-points and " << surf->tris.size() << " triangles.\n";
    boost::progress_display show_progress( clpoints->size() );
    dcCalls = 0;
    int calls=0;
    long int ntris = 0;
    std::list<Triangle>* tris;
    unsigned int n;
    unsigned int Nmax = clpoints->size();
    std::vector<CLPoint>& clref = *clpoints; 
    MillingCutter& cutref = *cutter;
    int nloop=0;
    unsigned int ntriangles = surf->tris.size();
#ifndef WIN32
    omp_set_num_threads(nthreads); // the constructor sets number of threads right
                                   // or the user can explicitly specify something else
#endif
    std::list<Triangle>::iterator it;
    #pragma omp parallel for shared( nloop, ntris, calls, clref, cutref) private(n,tris,it)
        for (n=0;n< Nmax ;n++) { // PARALLEL OpenMP loop!
#ifndef WIN32
            if ( n== 0 ) { // first iteration
                if (omp_get_thread_num() == 0 ) 
                    std::cout << "Number of OpenMP threads = "<< omp_get_num_threads() << "\n";// print out how many threads we are using
            }
#endif
            nloop++;
            tris=new std::list<Triangle>();
            KDNode::search_kdtree( tris, clref[n], cutref, root); // tris will contain overlapping triangles
            assert( tris->size() <= ntriangles ); // can't possibly find more triangles than in the STLSurf 
            
            for( it=tris->begin(); it!=tris->end() ; ++it) { // loop over found triangles  
                if ( cutref.overlaps(clref[n],*it) ) { // cutter overlap triangle? check
                    if (clref[n].below(*it)) {
                        cutref.vertexDrop( clref[n],*it);
                        ++calls;
                    }
                }
            }
            
            for( it=tris->begin(); it!=tris->end() ; ++it) { // loop over found triangles  
                if ( cutref.overlaps(clref[n],*it) ) { // cutter overlap triangle? check
                    if (clref[n].below(*it))
                        cutref.facetDrop( clref[n],*it);
                    //++calls;
                }
            }
            
            for( it=tris->begin(); it!=tris->end() ; ++it) { // loop over found triangles  
                if ( cutref.overlaps(clref[n],*it) ) { // cutter overlap triangle? check
                    if (clref[n].below(*it))
                        cutref.edgeDrop( clref[n],*it);
                    //++calls;
                }
            }
            
            ntris += tris->size();
            delete( tris );
            ++show_progress;
        } // end OpenMP PARALLEL for

    dcCalls = calls;
    std::cout << " " << dcCalls << " dropCutter() calls.\n";
    return;
}


// use OpenMP to share work between threads
// use the new KDNode2 class
// FIXME: as of r395 this is slightly slower than dropCutter4 above.
void BatchDropCutter::dropCutter5() {
    std::cout << "dropCutterSTL5 " << clpoints->size() << 
            " cl-points and " << surf->tris.size() << " triangles.\n";
    boost::progress_display show_progress( clpoints->size() );
    dcCalls = 0;
    int calls=0;
    long int ntris = 0;
    std::list<Triangle>* tris;
    Bbox* bb;
    unsigned int n;
    unsigned int Nmax = clpoints->size();
    std::vector<CLPoint>& clref = *clpoints; 
    MillingCutter& cutref = *cutter;
    int nloop=0;
    unsigned int ntriangles = surf->tris.size();
    double r = cutref.getRadius();
#ifndef WIN32
    omp_set_num_threads(nthreads); // the constructor sets number of threads right
                                   // or the user can explicitly specify something else
#endif
    std::list<Triangle>::iterator it;
    #pragma omp parallel for shared( nloop, ntris, calls, clref, cutref) private(n,tris,bb,it)
        for (n=0;n<Nmax;++n) { // PARALLEL OpenMP loop!
#ifndef WIN32
            if ( n== 0 ) { // first iteration
                if (omp_get_thread_num() == 0 ) 
                    std::cout << "Number of OpenMP threads = "<< omp_get_num_threads() << "\n";
            }
#endif
            nloop++;
            tris=new std::list<Triangle>();
            bb = new Bbox( clref[n].x-r, // build a bounding-box at the current CL
                           clref[n].x+r, 
                           clref[n].y-r, 
                           clref[n].y+r,
                           0,
                           0); 
            KDNode2::search_kdtree( tris, *bb, root2, 3); // tris will contain overlapping triangles
            assert( tris->size() <= ntriangles ); // can't possibly find more triangles than in the STLSurf 
            for( it=tris->begin(); it!=tris->end() ; ++it) { // loop over found triangles  
                if ( cutref.overlaps(clref[n],*it) ) { // cutter overlap triangle? check
                    cutref.dropCutter( clref[n],*it);
                    ++calls;
                }
            }
            ntris += tris->size();
            delete( tris );
            delete( bb );
            ++show_progress;
        } // end OpenMP PARALLEL for
    dcCalls = calls;
    std::cout << " " << dcCalls << " dropCutter() calls.\n";
    return;
}

}// end namespace

// end file batchdropcutter.cpp
