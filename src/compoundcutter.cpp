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

#include "millingcutter.h"
#include "point.h"
#include "triangle.h"
#include "stlsurf.h"
#include "numeric.h"

namespace ocl
{


CompoundCutter::CompoundCutter()
{
    radiusvec = std::vector<double>();
    cutter = std::vector<MillingCutter*>();
}

void CompoundCutter::addCutter(MillingCutter& c, double r, double zoff)
{
    radiusvec.push_back(r);
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
        lolimit = - 1E-6;
    else
        lolimit = radiusvec[n-1] - 1E-6;
    hilimit = radiusvec[n]+1e-6; // FIXME: really ugly solution this one...
    
    if (d<lolimit)
        return false;
    else if (d>hilimit)
        return false;
    else
        return true;
}

//********   drop-cutter methods ********************** */
int CompoundCutter::vertexDrop(CLPoint &cl, const Triangle &t) const
{
    int result = 0;
    
    for (unsigned int n=0; n<cutter.size(); ++n) { // loop through cutters
        CLPoint cl_tmp = cl + CLPoint(0,0,zoffset[n]);
        //CCPoint cc_tmp;
        if ( cutter[n]->vertexDrop(cl_tmp,t) ) {
            if ( ccValid(n,cl_tmp,cl_tmp.cc) ) { // cc-point is valid
                if (cl.liftZ(cl_tmp.z-zoffset[n],cl_tmp.cc)) { // we need to lift the cutter
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
        Point cl_tmp = cl + Point(0,0,zoffset[n]);
        CCPoint* cc_tmp = new CCPoint();
        if ( cutter[n]->facetDrop(cl_tmp, *cc_tmp,t) ) {
            if ( ccValid(n,cl, *cc_tmp) ) { // cc-point is valid
                if (cl.liftZ(cl_tmp.z-zoffset[n])) { // we need to lift the cutter
                    cc = *cc_tmp;
                    //cc.type = FACET;
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
        Point cl_tmp = cl + Point(0,0,zoffset[n]);
        CCPoint* cc_tmp = new CCPoint();
        if ( cutter[n]->edgeDrop(cl_tmp,*cc_tmp,t) ) { 
            if ( ccValid(n,cl,*cc_tmp) ) { // cc-point is valid
                if (cl.liftZ( cl_tmp.z - zoffset[n] )) { // we need to lift the cutter
                    cc = *cc_tmp;
                    //cc.type = EDGE;
                    result = 1;
                }
            }
        }
    }
    return result;
}

MillingCutter* CompoundCutter::offsetCutter(double d) const{
    return  new CylCutter(); //FIXME!
}

std::string CompoundCutter::str() const {
    std::ostringstream o;
    o << "CompoundCutter with "<< cutter.size() << " cutters:\n";
    for (unsigned int n=0; n<cutter.size(); ++n) { // loop through cutters
        o << " " << n << ":" << cutter[n]->str() << "\n";
        o << "  radius="<< radiusvec[n] << "\n";
        o << "  zoffset="<< zoffset[n] << "\n";
    }
    return o.str();
}

//********   actual compound-cutters ***************************************************** /
// only constructors required, drop-cutter calls handled by base-class


/// define a CylCone cutter which consists of a cylindrical(flat) middle part
/// of diameter diam1, and an outer conical part with a slope angle, and maximum diameter diam2
CylConeCutter::CylConeCutter(double diam1, double diam2, double angle)
{
    MillingCutter* c1 = new CylCutter(diam1);
    MillingCutter* c2 = new ConeCutter(diam2, angle);
    double cone_offset= - (diam1/2)/tan(angle);
    
    addCutter( *c1, diam1/2.0, 0.0 );
    addCutter( *c2, diam2/2.0, cone_offset );
}

/// define a BallCone cutter which consists of a spherical middle part
/// with ball-diameter diam1, and an outer conical part with a slope angle, and maximum diameter diam2
BallConeCutter::BallConeCutter(double diam1, double diam2, double angle)
{
    MillingCutter* c1 = new BallCutter(diam1); // at offset zero
    MillingCutter* c2 = new ConeCutter(diam2, angle);
    
    double cone_offset = - ( (diam1/2.0)/sin(angle) - diam1/2.0);
    double rcontact = (diam1/2.0)*cos(angle);
    
    addCutter( *c1, rcontact, 0.0 );
    addCutter( *c2, diam2/2.0, cone_offset );
}

/// define a BullCone cutter which consists of a toroidal middle part
/// with diameter diam1 and corner radius radius1,
/// and an outer conical part with a slope angle, and maximum diameter diam2
BullConeCutter::BullConeCutter(double diam1, double radius1, double diam2, double angle)
{
    MillingCutter* c1 = new BullCutter(diam1, radius1); // at offset zero
    MillingCutter* c2 = new ConeCutter(diam2, angle);
    
    double h1 = radius1*sin(angle); // the contact point is this much down from the toroid-ring
    double rad = sqrt( square(radius1) - square(h1) );
    double rcontact = (diam1/2.0) - radius1 + rad; // radius of the contact-ring
    double cone_offset= - ( rcontact/tan(angle) - (radius1-h1));
    
    addCutter( *c1, rcontact, 0.0 );
    addCutter( *c2, diam2/2.0, cone_offset );
}

/// define a ConeCone cutter which consists of a conical middle part
/// and a conical outer part. diam2 > diam1.
/// we assume angle2 < angle1
ConeConeCutter::ConeConeCutter(double diam1, double angle1, double diam2, double angle2)
{
    MillingCutter* c1 = new ConeCutter(diam1, angle1); // at offset zero
    MillingCutter* c2 = new ConeCutter(diam2, angle2);
    
    double h1 = (diam1/2.0)/tan(angle1); 
    double h2 = (diam1/2.0)/tan(angle2);
    double cone_offset= - ( h2-h1);
    
    addCutter( *c1, diam1/2.0, 0.0 );
    addCutter( *c2, diam2/2.0, cone_offset );
}


} // end namespace
// end file compoundcutter.cpp
