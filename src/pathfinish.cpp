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
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <math.h>
#include <boost/progress.hpp>

#include "cutter.h"
#include "point.h"
#include "triangle.h"
#include "kdtree.h"
#include "pathfinish.h"

namespace ocl
{

//********   ********************** */

PathDropCutterFinish::PathDropCutterFinish() {
	cutter = NULL;
    surf = NULL;
    root = NULL;
	path = NULL;
	minimumZ = 0.0;
}

PathDropCutterFinish::PathDropCutterFinish(const STLSurf *s) {
	cutter = NULL;
    surf = s;
    root = KDNode::build_kdtree( &(surf->tris) );
	path = NULL;
	minimumZ = 0.0;
}

void PathDropCutterFinish::setCutter(const MillingCutter *c) {
	cutter = c;
}

void PathDropCutterFinish::setPath(const Path *p) {
	path = p;
}

void PathDropCutterFinish::run() {
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
#define PATH_DROP_CUTTER_TOLERANCE 0.01

void PathDropCutterFinish::run(const Span* span)
{
	std::list<Point> point_list;
    std::list<CCPoint> cc_point_list;
    
	unsigned int num_steps = (unsigned int)(span->length2d() / SPLIT_STEP + 1);
	for(unsigned int i = 0; i<=num_steps; i++)
	{
		double fraction = (double)i / num_steps;
		Point p = span->getPoint(fraction);

        // find triangles under cutter
        std::list<Triangle> triangles_under_cutter;
        KDNode::search_kdtree( &triangles_under_cutter, p, *cutter, root);
        
        CCPoint cc;
		p.z = minimumZ;
        // this should be implemented using the lower-level OpenMP-enabled batchdropcutter
        BOOST_FOREACH( const Triangle& t, triangles_under_cutter) {
            cutter->dropCutter(p,cc,t);
        }
        point_list.push_back(p);
        cc_point_list.push_back(cc);
	}

    // this requires a better implementation which keeps the cc-points intact also
	// refinePointList(point_list);

	for(std::list<Point>::iterator It = point_list.begin(); It != point_list.end(); It++)
		clpoints.push_back(*It);
    
	for(std::list<CCPoint>::iterator It = cc_point_list.begin(); It != cc_point_list.end(); It++)
		ccpoints.push_back(*It);    
    
}

static Point* refineStart = NULL;
static Point* refineEnd = NULL;
static std::list<Point*> refineMiddlePoints;
static bool middlePointsFit()
{
	if(refineMiddlePoints.size() == 0)return true;
	for(std::list<Point*>::iterator It = refineMiddlePoints.begin(); It != refineMiddlePoints.end(); It++)
	{
		Point* p = *It;
		Line l(*refineStart, *refineEnd);
		Point pnear = l.Near(*p);
		double dist = (pnear - *p).norm();
		if(dist > PATH_DROP_CUTTER_TOLERANCE)return false;
	}
	return true;
}

void PathDropCutterFinish::refinePointList(std::list<Point> &point_list)
{
	std::list<Point> new_points;
	refineMiddlePoints.clear();

	if(point_list.size() < 3)return;

	std::list<Point>::iterator It = point_list.begin();
	refineStart = &(*It);
	new_points.push_back(*It);
	It++;

	for(; It != point_list.end(); It++)
	{
		refineEnd = &(*It);
		if(middlePointsFit())
			refineMiddlePoints.push_back(refineEnd);
		else
		{
			Point* ok_point = refineMiddlePoints.back();
			new_points.push_back(*ok_point);
			refineStart = ok_point;
			refineMiddlePoints.clear();
		}
	}

	if(refineMiddlePoints.size() > 0)
		new_points.push_back(*refineEnd);

	point_list = new_points;
}

boost::python::list PathDropCutterFinish::getCLPoints()
{
    boost::python::list plist;
    BOOST_FOREACH(Point p, clpoints) {
        plist.append(p);
    }
    return plist;
}

boost::python::list PathDropCutterFinish::getCCPoints()
{
    boost::python::list plist;
    BOOST_FOREACH(CCPoint p, ccpoints) {
        plist.append(p);
    }
    return plist;
}



} // end namespace
// end file pathfinish.cpp
