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
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <math.h>
#include <boost/progress.hpp>

#include "cutter.h"
#include "point.h"
#include "triangle.h"
#include "kdtree.h"
#include "pfinish.h"

//********   ********************** */

ParallelFinish::ParallelFinish() {
    clpoints = new std::list<Point>();
    ccpoints = new std::list<CCPoint>();
}


// drop cutter against all triangles in surface
void ParallelFinish::dropCutterSTL1(MillingCutter &cutter)
{
    // very simple drop-cutter
    std::cout << "ParallelFinish::dropCutterSTL1 " << clpoints->size() << " cl-points and " << surf->tris.size() << " triangles...";
    std::cout.flush();
    dcCalls = 0;
    BOOST_FOREACH(Point &cl, *clpoints) {
        // test against all triangles in s
        CCPoint cc;
        BOOST_FOREACH( const Triangle& t, surf->tris) {
            cutter.dropCutter(cl,cc,t);
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
void ParallelFinish::dropCutterSTL2(MillingCutter &cutter)
{
    std::cout << "ParallelFinish::dropCutterSTL2 " << clpoints->size() << " cl-points and " << surf->tris.size() << " triangles.\n";
    std::cout.flush();
    dcCalls = 0;
    BOOST_FOREACH(Point &cl, *clpoints) { //loop through each CL-point
    
        // find triangles under cutter
        std::list<Triangle> *triangles_under_cutter = new std::list<Triangle>();
        KDNode::search_kdtree( triangles_under_cutter, cl, cutter, root);
        
        //std::cout << "found " << triangles_under_cutter->size() << " triangles at cl=" << cl << "\n";
        //char c;
        //std::cin >> c; 
        
        
        CCPoint cc;
        BOOST_FOREACH( const Triangle& t, *triangles_under_cutter) {
            cutter.dropCutter(cl,cc,t);
            ++dcCalls;
        }
        triangles_under_cutter = NULL;
        ccpoints->push_back(cc);
    }
    std::cout << "done.\n";
    std::cout.flush();
    return;
}

void ParallelFinish::initSTLSurf(STLSurf &s, int bucketSize)
{
    surf = &s;
    std::cout << "Building kd-tree...";
    std::cout.flush();
    root = KDNode::build_kdtree( &(surf->tris), bucketSize );
    std::cout << " done.\n";
    //KDTree::str(root);
}

void ParallelFinish::initCLpoints(double minx,double dx, double maxx, 
                       double miny, double dy,double maxy, double base_z)
{
    // initialize grid of cl-points
    std::cout << "initCLPoints in region (" << minx << ", " << miny << ") to (" << maxx << ", " << maxy << ") \n";
    int Nmax = int( ((maxx-minx)/dx) + 1 ); // number of x-rows
    int Mmax = int( ((maxy-miny)/dy) + 1 ); // number of y-columns
    double xtemp, ytemp;
    
    for (int n=0; n<Nmax ; ++n) {
        for (int m=0; m<Mmax ; ++m) {
            xtemp = minx + n*dx;
            ytemp = miny + m*dy;
            clpoints->push_back( Point(xtemp, ytemp, base_z) );
        } 
    }
    std::cout << clpoints->size() << " cl-points initialized \n";
}

boost::python::list ParallelFinish::getTrianglesUnderCutter(Point &cl, MillingCutter &cutter)
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


boost::python::list ParallelFinish::getCLPoints()
{
    boost::python::list plist;
    BOOST_FOREACH(Point p, *clpoints) {
        plist.append(p);
    }
    return plist;
}

boost::python::list ParallelFinish::getCCPoints()
{
    boost::python::list plist;
    BOOST_FOREACH(CCPoint p, *ccpoints) {
        plist.append(p);
    }
    return plist;
}

