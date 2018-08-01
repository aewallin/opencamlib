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

#include "lineclfilter.hpp"
#include "point.hpp"
#include "clpoint.hpp"

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

    std::list<CLPoint> new_list;
    
    cl_itr p0 = clpoints.begin();
    cl_itr p1 = clpoints.begin();
    p1++;
    cl_itr p2 = p1;
    p2++;
    cl_itr p_last_good = p1;

    new_list.push_back(*p0);

    bool even_number = true;

    for(  ; p2 != clpoints.end(); ) {
        Point p = p1->closestPoint(*p0, *p2);
        if((p - *p1).norm() < tol) {
            p_last_good = p2;
            p2++;
            if(even_number)p1++;
            even_number = !even_number;
        }else {
            new_list.push_back(*p_last_good);
            p0 = p_last_good;
            p1 = p2;
            p_last_good = p1;
            p2++;
        }
    }
    new_list.push_back(clpoints.back());
    clpoints = new_list;
    return;
}

} // end namespace
// end file lineclfilter.cpp
