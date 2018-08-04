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

#include "point.hpp"
#include "triangle.hpp"
#include "pointdropcutter.hpp"


namespace ocl
{

//********   ********************** */

PointDropCutter::PointDropCutter() {
    nCalls = 0;
#ifdef _OPENMP
    nthreads = omp_get_num_procs(); // figure out how many cores we have
#endif
    cutter = NULL;
    bucketSize = 1;
    root = new KDTree<Triangle>();
}

void PointDropCutter::setSTL(const STLSurf &s) {
    //std::cout << "PointDropCutter::setSTL()\n";
    surf = &s;
    root->setXYDimensions(); // we search for triangles in the XY plane, don't care about Z-coordinate
    root->setBucketSize( bucketSize );
    root->build(s.tris);
}

void PointDropCutter::run(CLPoint& clp) {
    //std::cout << "PointDropCutter::run() clp= " << clp << " dropped to ";
    pointDropCutter1(clp);
    //std::cout  << clp << " nCalls = " << nCalls <<"\n ";
}

// use OpenMP to share work between threads
void PointDropCutter::pointDropCutter1(CLPoint& clp) {
    nCalls = 0;
    int calls=0;
    std::list<Triangle>* tris;
    //tris=new std::list<Triangle>();
    tris = root->search_cutter_overlap( cutter, &clp );
    std::list<Triangle>::iterator it;
    for( it=tris->begin(); it!=tris->end() ; ++it) { // loop over found triangles  
        if ( cutter->overlaps(clp,*it) ) { // cutter overlap triangle? check
            if (clp.below(*it)) {
                cutter->dropCutter(clp,*it);
                ++calls;
            }
        }
    }
    delete( tris );
    nCalls = calls;
    return;
}

}// end namespace
// end file pointdropcutter.cpp
