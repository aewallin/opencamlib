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

#include "cutter.h"
#include "point.h"
#include "triangle.h"
#include "pfinish.h"

//********   ********************** */

ParallelFinish::ParallelFinish() {
    clpoints = new std::list<Point>();
    ccpoints = new std::list<CCPoint>();
}

void ParallelFinish::dropCutterSTL1(MillingCutter &cutter, STLSurf &s)
{
    // very simple drop-cutter
    std::cout << "ParallelFinish::dropCutterSTL1 " << clpoints->size() << " cl-points and " << s.tris.size() << " triangles...";
    BOOST_FOREACH(Point cl, *clpoints) {
        // test against all triangles in s
        CCPoint cc;
        BOOST_FOREACH( const Triangle& t, s.tris) {
            cutter.dropCutter(cl,cc,t);
        }
        ccpoints->push_back(cc);
    }
    std::cout << "done.\n";
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
            ytemp = miny + n*dy;
            clpoints->push_back( Point(xtemp, ytemp, base_z) );
        } 
    }
    std::cout << clpoints->size() << " cl-points initialized \n";
}




