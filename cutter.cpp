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

//********   MillingCutter ********************** */
int MillingCutter::count = 0;

MillingCutter::MillingCutter()
{   
    setId();
}

void MillingCutter::setDiameter(double d)
{
    if ( d >= 0.0 ) {
        diameter=d;
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

double MillingCutter::getDiameter()
{
    return diameter;
}

double MillingCutter::getRadius()
{
    return diameter/2;
}

double MillingCutter::getLength()
{
    return length;
}


int MillingCutter::dropCutter(Point &cl, CCPoint &cc, const Triangle &t)
{
    /* template-method, or "self-delegation" pattern */
    
    vertexDrop(cl,cc,t);
    facetDrop(cl,cc,t); /// \todo optimisation: if we are already above the triangle we don't need facet and edge
    edgeDrop(cl,cc,t);

    return 0; // void
}

int MillingCutter::dropCutterSTL(Point &cl, CCPoint &cc, const STLSurf &s)
{
    /* template-method, or "self-delegation" pattern */
    //boost::progress_display show_progress( s.tris.size() );

    BOOST_FOREACH( const Triangle& t, s.tris) {
        dropCutter(cl,cc,t);
        //++show_progress;
    }

    return 0; // void
}

bool MillingCutter::overlaps(Point &cl, Triangle &t)
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




