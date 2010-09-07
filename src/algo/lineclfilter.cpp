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

#include "lineclfilter.h"
#include "point.h"
#include "clpoint.h"

namespace ocl
{

LineCLFilter::LineCLFilter() {
    clpoints.clear();
}

void LineCLFilter::addCLPoint(const CLPoint& p) {
    clpoints.push_back(p);
} 
void LineCLFilter::setTolerance(const double tolerance) {
    tol = tolerance;
}

void LineCLFilter::run() {
    typedef std::list<CLPoint>::iterator cl_itr;
    int n = clpoints.size();
    if (n<2)
        return; // can't filter lists of length 0, 1, or 2
    
    cl_itr p0 = clpoints.begin();
    cl_itr p1 = clpoints.begin();
    p1++;
    cl_itr p2 = p1;
    p2++;
    for(  ; p2 != clpoints.end(); ) {
        Point p = p1->closestPoint(*p0, *p2);
        if ( (p- *p1).norm() < tol )  { // p1 is to be removed
            p1 = clpoints.erase(p1);
            p2++;
        } else {
            p0++;
            p1++;
            p2++;
        }
    }
    return;
}

} // end namespace
// end file lineclfilter.cpp
