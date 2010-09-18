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

#include "millingcutter.h"
#include "clpoint.h"
//#include "triangle.h"
//#include "kdtree.h"
#include "pathdropcutter.h"

namespace ocl
{

//********   ********************** */

PathDropCutter::PathDropCutter() {
    cutter = NULL;
    surf = NULL;
    // root = NULL;
    path = NULL;
    minimumZ = 0.0;
    bdc = new BatchDropCutter(); // we delegate to BatchDropCutter, who does the heavy lifting
}

PathDropCutter::~PathDropCutter() {
    delete bdc;
}

// we don't want many ways of doing the same thing. allways set the STLSurf using setSTL
/*
PathDropCutter::PathDropCutter(const STLSurf *s) {
    cutter = NULL;
    surf = s;
    root = KDNode::build_kdtree( &(surf->tris) );
    path = NULL;
    minimumZ = 0.0;
}*/

void PathDropCutter::setSTL(const STLSurf& s) {
    //std::cout << " Waterline::setSTL()\n";
    //bdc->setXYDirection();
    //bpc_y->setYDirection();
    //std::cout << " Waterline::setSTL() DIRECTIONS DONE\n";
    bdc->setSTL( s );
    //bpc_y->setSTL( s );
    surf = &s;
    //std::cout << " Waterline::setSTL() DONE\n";
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
    {   // loop through the spans calling run() on each
        const Span* span = *It;
        this->run(span); // append points to bdc
    }
    
    bdc->run(); // run the actual drop-cutter on all points
    clpoints = bdc->getCLPoints();
}

/// \todo these should be member of the class, so they can be set by the user
/// to something else than the defaults, if desired.
#define SPLIT_STEP 0.1

void PathDropCutter::run(const Span* span)
{
    //std::list<CLPoint> point_list;
    
    unsigned int num_steps = (unsigned int)(span->length2d() / SPLIT_STEP + 1);
    for(unsigned int i = 0; i<=num_steps; i++)
    {
        double fraction = (double)i / num_steps;
        
        Point ptmp = span->getPoint(fraction);
        CLPoint* p = new CLPoint(ptmp.x, ptmp.y, ptmp.z);
        p->z = minimumZ;
        bdc->appendPoint( *p );
        
        // find triangles under cutter
        //std::list<Triangle> triangles_under_cutter;
        //KDNode::search_kdtree( &triangles_under_cutter, p, *cutter, root);
    
        //CCPoint cc;
        
        // this should be implemented using the lower-level OpenMP-enabled batchdropcutter
        //BOOST_FOREACH( const Triangle& t, triangles_under_cutter) {
        //    cutter->dropCutter(p,t);
        //}
        //point_list.push_back(p);
        //cc_point_list.push_back(cc);
    }

    //for(std::list<CLPoint>::iterator It = point_list.begin(); It != point_list.end(); It++)
    //    clpoints.push_back(*It);
    
}

} // end namespace
// end file pathdropcutter.cpp
