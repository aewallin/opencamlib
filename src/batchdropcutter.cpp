/*  Copyright 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com)
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
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <math.h>
#include <boost/progress.hpp>
#include <omp.h>

#include "cutter.h"
#include "point.h"
#include "triangle.h"
#include "kdtree.h"
#include "batchdropcutter.h"

//********   ********************** */

BatchDropCutter::BatchDropCutter() {
    clpoints = new std::vector<Point>();
    ccpoints = new std::vector<CCPoint>();
    dcCalls = 0;
    nthreads = 1;
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

void BatchDropCutter::appendPoint(Point& p)
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
    BOOST_FOREACH(Point &cl, *clpoints) {
        // test against all triangles in s
        CCPoint cc;
        BOOST_FOREACH( const Triangle& t, surf->tris) {
            cutter->dropCutter(cl,cc,t);
            ++dcCalls;
        }
        ccpoints->push_back(cc);
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
    
    BOOST_FOREACH(Point &cl, *clpoints) { //loop through each CL-point
    
        // find triangles under cutter
        triangles_under_cutter->clear();
        KDNode::search_kdtree( triangles_under_cutter, cl, *cutter, root);
        
        CCPoint cc;
        BOOST_FOREACH( const Triangle& t, *triangles_under_cutter) {
            cutter->dropCutter(cl,cc,t);
            ++dcCalls;
        }
        
        ccpoints->push_back(cc);
    }
    std::cout << "done. " << dcCalls << " dropCutter() calls.\n";
    std::cout.flush();
    return;
}

void BatchDropCutter::dropCutter3()
{
    std::cout << "dropCutterSTL3 " << clpoints->size() << 
            " cl-points and " << surf->tris.size() << " triangles.\n";
    std::cout.flush();
    dcCalls = 0;
    std::list<Triangle> *triangles_under_cutter = new std::list<Triangle>();
    
    BOOST_FOREACH(Point &cl, *clpoints) { //loop through each CL-point
    
        // find triangles under cutter
        triangles_under_cutter->clear();
        KDNode::search_kdtree( triangles_under_cutter, cl, *cutter, root);
        
        CCPoint cc;
        BOOST_FOREACH( const Triangle& t, *triangles_under_cutter) {
            if (cutter->overlaps(cl,t)) {
                cutter->dropCutter(cl,cc,t);
                ++dcCalls;
            }
        }
        
        ccpoints->push_back(cc);
    }
    std::cout << "done. " << dcCalls << " dropCutter() calls.\n";
    std::cout.flush();
    return;
}

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
    CCPoint cc;
    std::vector<Point>& clref = *clpoints; 
    MillingCutter& cutref = *cutter;
    //KDNode* root2 = root;
    std::cout << "threads: " << omp_get_num_threads()<<"\n";
    omp_set_num_threads(nthreads);
    
    #pragma omp parallel for shared( calls, clref, cutref ) private(n,t,tris)
        for (n=0;n< Nmax ;n++) {
            int tid = omp_get_thread_num();
            if (tid == 0 && n == 0)
            {
                int nthreads = omp_get_num_threads();
                std::cout << "Number of threads = "<< nthreads << "\n";
            }


            t= 0;
            tris=new std::list<Triangle>();
            
        //BOOST_FOREACH(Point &cl, *clpoints) { //loop through each CL-point
            //cl = (*clpoints)[n];
            // find triangles under cutter
            
            
            
            KDNode::search_kdtree( tris, clref[n], cutref, root);
            std::list<Triangle>::iterator it;
            //it = tris->begin();
            
            //BOOST_FOREACH( *t, *tris) {
            for( it=tris->begin(); it!=tris->end() ; it++) { // loop over found triangles
                
                if ( cutref.overlaps(clref[n],*it) ) {
                    cutref.dropCutter( clref[n],cc,*it);
                    //++dcCalls;
                    //t->str();
                    ++calls;
                }
                
            }
            tris->clear();
            
            //ccpoints->push_back(cc);
        }
    // end PARALLEL for
        
    dcCalls = calls;
        
        
    std::cout << "done. " << dcCalls << " dropCutter() calls.\n";
    std::cout.flush();
    return;
}



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


boost::python::list BatchDropCutter::getCLPoints()
{
    boost::python::list plist;
    BOOST_FOREACH(Point p, *clpoints) {
        plist.append(p);
    }
    return plist;
}

boost::python::list BatchDropCutter::getCCPoints()
{
    boost::python::list plist;
    BOOST_FOREACH(CCPoint p, *ccpoints) {
        plist.append(p);
    }
    return plist;
}

