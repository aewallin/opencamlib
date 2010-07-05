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
#include "stlsurf.h"

namespace ocl
{
    

//********   MillingCutter ********************** */
int MillingCutter::count = 0;

MillingCutter::MillingCutter()
{   
    setDiameter( 1.0 );
    setLength( 1.0 );
    setId();
}

void MillingCutter::setDiameter(double d)
{
    if ( d >= 0.0 ) {
        diameter=d;
        radius = d/2.0;
    } else {
        std::cout << "cutter.cpp: ERROR, MillingCutter.setDiameter(d) called with d<0 !!";
        diameter=1.0;
    }
}

void MillingCutter::setLength(double l)
{
    if ( l > 0.0 ) {
        length=l;
    } else {
        std::cout << "cutter.cpp: ERROR, MillingCutter.setLength(l) called with l<0 !!";
        length=1.0;
    }
}

void MillingCutter::setId()
{
        id = count;
        count++;
}

double MillingCutter::getDiameter() const
{
    return diameter;
}

double MillingCutter::getRadius() const
{
    return radius;
}

double MillingCutter::getLength() const
{
    return length;
}

MillingCutter* MillingCutter::offsetCutter(double d) const
{
    assert(0); // DON'T call me
    return  new CylCutter();
}

/// call vertex, facet, and edge drop methods
int MillingCutter::dropCutter(CLPoint &cl, const Triangle &t) const
{
    /* template-method, or "self-delegation", pattern */
    //CLPoint cl2 = CLPoint(cl.x,cl.y,cl.z); //FIXME FIXME FIXME
    vertexDrop(cl,t);
    /// \todo optimisation: if we are already above the triangle we don't need facet and edge
    facetDrop(cl,t); 
    edgeDrop(cl,t);

    return 0; // void would be better, return value not used for anything
}

// TESTING ONLY, don't use for real
int MillingCutter::dropCutterSTL(CLPoint &cl, const STLSurf &s) const
{
    /* template-method, or "self-delegation", pattern */
    //boost::progress_display show_progress( s.tris.size() );

    BOOST_FOREACH( const Triangle& t, s.tris) {
        dropCutter(cl,t);
    }

    return 0; // void
}

bool MillingCutter::overlaps(Point &cl, const Triangle &t) const
{
    double r = this->getRadius();
    if ( t.maxx < cl.x-r )
        return false;
    else if ( t.minx > cl.x+r )
        return false;
    else if ( t.maxy < cl.y-r )
        return false;
    else if ( t.miny > cl.y+r )
        return false;
    else
        return true;
}


} // end namespace
// end file cutter.cpp
