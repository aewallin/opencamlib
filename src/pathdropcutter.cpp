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

#include <boost/foreach.hpp>

#include "millingcutter.h"
#include "point.h"
#include "triangle.h"
#include "kdtree.h"
#include "pathdropcutter.h"

namespace ocl
{

//********   ********************** */

PathDropCutter::PathDropCutter() {
    cutter = NULL;
    surf = NULL;
    root = NULL;
    path = NULL;
    minimumZ = 0.0;
}

PathDropCutter::PathDropCutter(const STLSurf *s) {
    cutter = NULL;
    surf = s;
    root = KDNode::build_kdtree( &(surf->tris) );
    path = NULL;
    minimumZ = 0.0;
}

void PathDropCutter::setCutter(const MillingCutter *c) {
    cutter = c;
}

void PathDropCutter::setPath(const Path *p) {
    path = p;
}

void PathDropCutter::run() {
    clpoints.clear();
    // loop through the input path, splitting each input span into 0.1mm steps
    /// \todo FIXME: can we replace with BOOST_FOREACH ?
    /// \todo FIXME: 0.1mm resolution is arbitrary, allow user to change/set this
    for(std::list<Span*>::const_iterator It = path->span_list.begin(); It != path->span_list.end(); It++)
    {
        const Span* span = *It;
        run(span);
    }
}

/// \todo these should be static members of the class, so they can be set by the user
/// to something else than the defaults, if desired.
#define SPLIT_STEP 0.1
// #define PATH_DROP_CUTTER_TOLERANCE 0.01

void PathDropCutter::run(const Span* span)
{
    std::list<CLPoint> point_list;
    //std::list<CCPoint> cc_point_list;
    
    unsigned int num_steps = (unsigned int)(span->length2d() / SPLIT_STEP + 1);
    for(unsigned int i = 0; i<=num_steps; i++)
    {
        double fraction = (double)i / num_steps;
        
        Point ptmp = span->getPoint(fraction);
        CLPoint p = CLPoint(ptmp.x, ptmp.y, ptmp.z);

        // find triangles under cutter
        std::list<Triangle> triangles_under_cutter;
        KDNode::search_kdtree( &triangles_under_cutter, p, *cutter, root);
    
        //CCPoint cc;
        p.z = minimumZ;
        // this should be implemented using the lower-level OpenMP-enabled batchdropcutter
        BOOST_FOREACH( const Triangle& t, triangles_under_cutter) {
            cutter->dropCutter(p,t);
        }
        point_list.push_back(p);
        //cc_point_list.push_back(cc);
    }

    for(std::list<CLPoint>::iterator It = point_list.begin(); It != point_list.end(); It++)
        clpoints.push_back(*It);
    
}






/// return a python list of CLPoints
boost::python::list PathDropCutter::getCLPoints() {
    boost::python::list plist;
    BOOST_FOREACH(CLPoint p, clpoints) {
        plist.append(p);
    }
    return plist;
}




} // end namespace
// end file pathfinish.cpp
