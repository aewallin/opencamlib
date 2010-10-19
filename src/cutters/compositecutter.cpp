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

#include "compositecutter.h"
#include "numeric.h"
#include "cylcutter.h"
#include "ballcutter.h"
#include "bullcutter.h"
#include "conecutter.h"

namespace ocl
{

CompositeCutter::CompositeCutter() {
    radiusvec = std::vector<double>();
    cutter = std::vector<MillingCutter*>();
}

void CompositeCutter::addCutter(MillingCutter& c, double r, double zoff) {
    radiusvec.push_back(r);
    cutter.push_back(&c);
    zoffset.push_back(zoff);
}

bool CompositeCutter::ccValid(int n, CLPoint& cl) const {
    if (cl.cc->type == NONE)
        return false;
    double d = cl.xyDistance(*cl.cc);
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

// delegate to the sub-cutters, and pick the right one.
bool CompositeCutter::vertexDrop(CLPoint &cl, const Triangle &t) const {
    bool result = false;
    for (unsigned int n=0; n<cutter.size(); ++n) { // loop through cutters
        CLPoint cl_tmp = cl + CLPoint(0,0,zoffset[n]);
        CCPoint* cc_tmp; 
        if ( cutter[n]->vertexDrop(cl_tmp,t) ) {  // if we hit a vertex with this cutter
            assert( cl_tmp.cc != 0);
            if ( ccValid(n,cl_tmp) ) { // and cc-point is valid
                cc_tmp =  new CCPoint(*cl_tmp.cc);
                if (cl.liftZ( cl_tmp.z-zoffset[n] )) { // and we need to lift the cutter
                    cc_tmp->type = VERTEX;
                    cl.cc = cc_tmp;
                    result = true;
                } else {
                    delete cc_tmp;
                }
            } 
        } 
    }
    return result;
}

//********   facet ********************** */
bool CompositeCutter::facetDrop(CLPoint &cl, const Triangle &t) const {
    bool result = false;
    for (unsigned int n=0; n<cutter.size(); ++n) { // loop through cutters
        CLPoint cl_tmp = cl + CLPoint(0,0,zoffset[n]);
        CCPoint* cc_tmp;
        if ( cutter[n]->facetDrop(cl_tmp, t) ) {
            assert( cl_tmp.cc != 0);
            if ( ccValid(n,cl_tmp) ) { // cc-point is valid
                cc_tmp = new CCPoint(*cl_tmp.cc);
                if (cl.liftZ( cl_tmp.z - zoffset[n] )) { // we need to lift the cutter
                    cc_tmp->type = FACET;
                    cl.cc = cc_tmp;
                    result = true;
                } else {
                    delete cc_tmp;
                }
            }
        }
    }
    return result;
}

//********   edge **************************************************** */
bool CompositeCutter::edgeDrop(CLPoint &cl, const Triangle &t) const {
    bool result = false;
    for (unsigned int n=0; n<cutter.size(); ++n) { // loop through cutters
        CLPoint cl_tmp = cl + Point(0,0,zoffset[n]);
        CCPoint* cc_tmp;
        if ( cutter[n]->edgeDrop(cl_tmp,t) ) { 
            if ( ccValid(n,cl_tmp) ) { // cc-point is valid
                cc_tmp = new CCPoint(*cl_tmp.cc);
                if (cl.liftZ( cl_tmp.z - zoffset[n] ) ) { // we need to lift the cutter
                    cc_tmp->type = EDGE;
                    cl.cc = cc_tmp;
                    result = true;
                } else {
                    delete cc_tmp;
                }
            }
        }
    }
    return result;
}

MillingCutter* CompositeCutter::offsetCutter(const double d) const {
    std::cout << " ERROR: not implemented.\n";
    assert(0);
    return  new CylCutter(); //FIXME!
}

std::string CompositeCutter::str() const {
    std::ostringstream o;
    o << "CompositeCutter with "<< cutter.size() << " cutters:\n";
    for (unsigned int n=0; n<cutter.size(); ++n) { // loop through cutters
        o << " " << n << ":" << cutter[n]->str() << "\n";
        o << "  radius="<< radiusvec[n] << "\n";
        o << "  zoffset="<< zoffset[n] << "\n";
    }
    return o.str();
}



//  actual Composite-cutters  
//  only constructors required, drop-cutter calls handled by base-class

CylConeCutter::CylConeCutter(double diam1, double diam2, double angle) {
    MillingCutter* c1 = new CylCutter(diam1, 1 );
    MillingCutter* c2 = new ConeCutter(diam2, angle);
    double cone_offset= - (diam1/2)/tan(angle);
    addCutter( *c1, diam1/2.0, 0.0 );
    addCutter( *c2, diam2/2.0, cone_offset );
}

BallConeCutter::BallConeCutter(double diam1, double diam2, double angle) {
    MillingCutter* c1 = new BallCutter(diam1, 1); // at offset zero  FIXME: length
    MillingCutter* c2 = new ConeCutter(diam2, angle);
    double cone_offset = - ( (diam1/2.0)/sin(angle) - diam1/2.0);
    double rcontact = (diam1/2.0)*cos(angle);
    addCutter( *c1, rcontact, 0.0 );
    addCutter( *c2, diam2/2.0, cone_offset );
}

BullConeCutter::BullConeCutter(double diam1, double radius1, double diam2, double angle) {
    MillingCutter* c1 = new BullCutter(diam1, radius1, 1.0); // at offset zero FIXME: length
    MillingCutter* c2 = new ConeCutter(diam2, angle);
    double h1 = radius1*sin(angle); // the contact point is this much down from the toroid-ring
    double rad = sqrt( square(radius1) - square(h1) );
    double rcontact = (diam1/2.0) - radius1 + rad; // radius of the contact-ring
    double cone_offset= - ( rcontact/tan(angle) - (radius1-h1));
    addCutter( *c1, rcontact, 0.0 );
    addCutter( *c2, diam2/2.0, cone_offset );
}

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
// end file compositecutter.cpp
