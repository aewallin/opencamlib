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
#include "kdtree.h"
#include "pathfinish.h"

//********   ********************** */

PathDropCutterFinish::PathDropCutterFinish() {
}

PathDropCutterFinish::PathDropCutterFinish(const Path &p, const MillingCutter *c, const STLSurf *s) {
	cutter = c;
    surf = s;
    root = KDTree::build_kdtree( &(surf->tris), 20 );
	path = &p;
}

void PathDropCutterFinish::run() {
	// to do

	// test, just add some spans to the output path
	outputPath.append(Line(Point(0.0, 0.0, 0.0), Point(10.0, 0.0, 0.0)));
	outputPath.append(Arc(Point(10.0, 0.0, 0.0), Point(20.0, 10.0, 0.0), Point(10.0, 10.0, 0.0), true));

	// loop through the input path, splitting each input span into 0.1mm steps
	for(std::list<Span*>::const_iterator It = path->span_list.begin(); It != path->span_list.end(); It++)
	{
		const Span* span = *It;

		run(span);
	}
}

#define SPLIT_STEP 0.1
#define PATH_DROP_CUTTER_TOLERANCE 0.01

void PathDropCutterFinish::run(const Span* span)
{
	std::list<Point> point_list;

	unsigned int num_steps = (unsigned int)(span->length2d() / SPLIT_STEP + 1);
	for(unsigned int i = 0; i<=num_steps; i++)
	{
		double fraction = (double)i / num_steps;
		Point p = span->getPoint(fraction);
		point_list.push_back(p);
	}

	refinePointList(point_list);

	Point* prev_p = NULL;
	for(std::list<Point>::iterator It = point_list.begin(); It != point_list.end(); It++)
	{
		Point& p = *It;
		if(prev_p)
			outputPath.append(Line(*prev_p, p));
		prev_p = &p;
	}
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
		if(p->xyDistanceToLine(*refineStart, *refineEnd) > PATH_DROP_CUTTER_TOLERANCE)return false;
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
		}
	}

	if(refineMiddlePoints.size() > 0)
		new_points.push_back(*refineEnd);

	point_list = new_points;
}
