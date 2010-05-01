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
#include "stlsurf.h"
#include "cutter.h"
#include "numeric.h"

namespace ocl
{


CompoundCutter::CompoundCutter()
{
    radius = std::vector<double>();
    cutter = std::vector<MillingCutter*>();
}

void CompoundCutter::addCutter(MillingCutter& c, double r, double zoff)
{
    radius.push_back(r);
    cutter.push_back(&c);
    zoffset.push_back(zoff);
}

bool CompoundCutter::ccValid(int n, Point& cl, CCPoint& cc_tmp) const
{
    if (cc_tmp.type == NONE)
        return false;
        
    double d = cl.xyDistance(cc_tmp);
    double lolimit;
    double hilimit;
    if (n==0)
        lolimit = 0.0;
    else
        lolimit = radius[n-1];
    hilimit = radius[n];
    
    if (d<lolimit)
        return false;
    else if (d>hilimit)
        return false;
    else
        return true;
}

//********   drop-cutter methods ********************** */
int CompoundCutter::vertexDrop(Point &cl, CCPoint &cc, const Triangle &t) const
{
    int result = 0;
    Point cl_tmp = cl;
    for (unsigned int n=0; n<cutter.size(); ++n) { // loop through cutters
        CCPoint cc_tmp;
        if ( cutter[n]->vertexDrop(cl_tmp,cc_tmp,t) ) {
            if ( ccValid(n,cl_tmp,cc_tmp) ) { // cc-point is valid
                if (cl.liftZ(cl_tmp.z-zoffset[n])) { // we need to lift the cutter
                    cc = cc_tmp;
                    cc.type = VERTEX;
                    result = 1;
                }
            }
        }
    }
    return result;
}

//********   facet ********************** */
int CompoundCutter::facetDrop(Point &cl, CCPoint &cc, const Triangle &t) const
{
    int result = 0;
    Point cl_tmp = cl;
    for (unsigned int n=0; n<cutter.size(); ++n) { // loop through cutters
        CCPoint* cc_tmp = new CCPoint();
        if ( cutter[n]->facetDrop(cl_tmp, *cc_tmp,t) ) {
            if ( ccValid(n,cl_tmp, *cc_tmp) ) { // cc-point is valid
                if (cl.liftZ(cl_tmp.z-zoffset[n])) { // we need to lift the cutter
                    cc = *cc_tmp;
                    cc.type = FACET;
                    result = 1;
                }
            }
        }
    }
    return result;
}


//********   edge **************************************************** */
int CompoundCutter::edgeDrop(Point &cl, CCPoint &cc, const Triangle &t) const
{
    int result = 0;
    for (unsigned int n=0; n<cutter.size(); ++n) { // loop through cutters
        Point cl_tmp = cl;
        CCPoint* cc_tmp = new CCPoint();
        if ( cutter[n]->edgeDrop(cl_tmp,*cc_tmp,t) ) {
       
            if ( ccValid(n,cl,*cc_tmp) ) { // cc-point is valid
                if (cl.liftZ(cl_tmp.z-zoffset[n])) { // we need to lift the cutter
                    cc = *cc_tmp;
                    cc.type = EDGE;
                    result = 1;
                }
            }
        }
    }
    return result;
}



//********   actual compound-cutters ***************************************************** /

CylConeCutter::CylConeCutter(double diam1, double diam2, double angle)
{
    MillingCutter* p1 = new CylCutter(diam1);
    MillingCutter* p2 = new ConeCutter(diam2, angle);
    double cone_offset= - (diam1/2)/tan(angle);
    
    addCutter( *p1, diam1/2.0, 0.0 );
    addCutter( *p2, diam2/2.0, cone_offset );
}


} // end namespace
// end file compoundcutter.cpp
