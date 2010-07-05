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
//#include <iostream>
//#include <stdio.h>
//#include <sstream>
//#include <math.h>
//#include <boost/progress.hpp>
#include <boost/foreach.hpp>
#ifndef WIN32
#include <omp.h>
#endif

#include "millingcutter.h"
#include "point.h"
#include "triangle.h"
#include "kdtree.h"
#include "batchdropcutter.h"

namespace ocl
{

//********   ********************** */

BatchDropCutter::BatchDropCutter() {
    clpoints = new std::vector<CLPoint>();
    //ccpoints = new std::vector<CCPoint>();
    dcCalls = 0;
#ifndef WIN32
    nthreads = omp_get_num_procs(); // figure out how many cores we have
#endif
    cutter = new CylCutter(1.0);
}

void BatchDropCutter::setSTL(STLSurf &s, int bucketSize)
{
    surf = &s;
    std::cout << "Building kd-tree...";
    std::cout.flush();
    root = KDNode::build_kdtree( &(surf->tris), bucketSize );
    std::cout << " done.\n";
}

void BatchDropCutter::setCutter(MillingCutter *c)
{
    cutter = c;
}

void BatchDropCutter::setThreads(int n)
{
    nthreads = n;
}

void BatchDropCutter::appendPoint(CLPoint& p)
{
    clpoints->push_back(p);
}

// drop cutter against all triangles in surface
void BatchDropCutter::dropCutter1()
{
    // very simple drop-cutter
    std::cout << "dropCutterSTL1 " << clpoints->size() << 
              " cl-points and " << surf->tris.size() << " triangles...";
    std::cout.flush();
    dcCalls = 0;
    BOOST_FOREACH(CLPoint &cl, *clpoints) {
        // test against all triangles in s
        BOOST_FOREACH( const Triangle& t, surf->tris) {
            cutter->dropCutter(cl,t);
            ++dcCalls;
        }
    }
    std::cout << "done.\n";
    std::cout.flush();
    return;
}


// first search for triangles under the cutter
// then only drop cutter against found triangles
void BatchDropCutter::dropCutter2()
{
    std::cout << "dropCutterSTL2 " << clpoints->size() << 
            " cl-points and " << surf->tris.size() << " triangles.\n";
    std::cout.flush();
    dcCalls = 0;
    std::list<Triangle> *triangles_under_cutter = new std::list<Triangle>();
    //CCPoint* cc;
    BOOST_FOREACH(CLPoint &cl, *clpoints) { //loop through each CL-point
    
        // find triangles under cutter
        triangles_under_cutter->clear();
        KDNode::search_kdtree( triangles_under_cutter, cl, *cutter, root);
        
        //cc = new CCPoint();
        BOOST_FOREACH( const Triangle& t, *triangles_under_cutter) {
            cutter->dropCutter(cl,t);
            //cutter->vertexDrop(cl,*cc,t);
            //cutter->facetDrop(cl,*cc,t);
            //cutter->edgeDrop(cl,*cc,t);
            ++dcCalls;
        }
        
        //ccpoints->push_back(*cc);
    }
    std::cout << "done. " << dcCalls << " dropCutter() calls.\n";
    std::cout.flush();
    return;
}

// compared to dropCutter2, add an additional explicit overlap-test before testing triangle
void BatchDropCutter::dropCutter3()
{
    std::cout << "dropCutterSTL3 " << clpoints->size() << 
            " cl-points and " << surf->tris.size() << " triangles.\n";
    std::cout.flush();
    dcCalls = 0;
    std::list<Triangle> *triangles_under_cutter = new std::list<Triangle>();
    
    BOOST_FOREACH(CLPoint &cl, *clpoints) { //loop through each CL-point
        // find triangles under cutter
        triangles_under_cutter->clear();
        KDNode::search_kdtree( triangles_under_cutter, cl, *cutter, root);
        
        //CCPoint cc;
        BOOST_FOREACH( const Triangle& t, *triangles_under_cutter) {
            if (cutter->overlaps(cl,t)) {
                cutter->dropCutter(cl,t);
                ++dcCalls;
            }
        }
        
        //ccpoints->push_back(cc);
    }
    std::cout << "done. " << dcCalls << " dropCutter() calls.\n";
    std::cout.flush();
    return;
}

// use OpenMP to share work between threads
void BatchDropCutter::dropCutter4()
{
    std::cout << "dropCutterSTL4 " << clpoints->size() << 
            " cl-points and " << surf->tris.size() << " triangles.\n";
    std::cout.flush();
    dcCalls = 0;
    int calls=0;
    std::list<Triangle> *tris;
    
    unsigned int n;
    unsigned int Nmax = clpoints->size();
    
    Triangle* t;
    //CCPoint cc;
    std::vector<CLPoint>& clref = *clpoints; 
    
    //ccpoints->resize( clpoints->size() ); // preallocate room in cc-vector
    //std::vector<CCPoint>& ccref = *ccpoints; 
    
    MillingCutter& cutref = *cutter;

#ifndef WIN32
    omp_set_num_threads(nthreads);
#endif
    std::list<Triangle>::iterator it;
    #pragma omp parallel for shared( calls, clref, cutref) private(n,t,tris,it)
        for (n=0;n< Nmax ;n++) {
#ifndef WIN32
            if ( n== 0 ) {
                // on first iteration, print out how many threads we are using
                if (omp_get_thread_num() == 0 ) {   
                    std::cout << "Number of threads = "<< omp_get_num_threads() << "\n";
                }
            }
#endif
            
            // reset variables before each run
            t= 0;
            tris=new std::list<Triangle>();
            // cc.type = NONE;
            
            KDNode::search_kdtree( tris, clref[n], cutref, root);
            
            for( it=tris->begin(); it!=tris->end() ; it++) { // loop over found triangles  
                if ( cutref.overlaps(clref[n],*it) ) { //overlap check
                    cutref.dropCutter( clref[n],*it);
                    ++calls;
                }
            }
            delete( tris );
            
            //ccref[n]=cc;
        }
    // end OpenMP PARALLEL for
        
    dcCalls = calls;
        
    std::cout << "done. " << dcCalls << " dropCutter() calls.\n";
    std::cout.flush();
    return;
}


// used only for testing, not actual work
boost::python::list BatchDropCutter::getTrianglesUnderCutter(Point &cl, MillingCutter &cutter)
{
    boost::python::list trilist;
    std::list<Triangle> *triangles_under_cutter = new std::list<Triangle>();
    KDNode::search_kdtree( triangles_under_cutter, cl, cutter, root);
    BOOST_FOREACH(Triangle t, *triangles_under_cutter)
    {
        trilist.append(t);
    }
    delete triangles_under_cutter;
    return trilist;
}

// return CL points to python
boost::python::list BatchDropCutter::getCLPoints()
{
    boost::python::list plist;
    BOOST_FOREACH(CLPoint p, *clpoints) {
        plist.append(p);
    }
    return plist;
}




}// end namespace

// end file batchdropcutter.cpp
