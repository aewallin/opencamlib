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

#include "compositecutter.hpp"
#include "numeric.hpp"
#include "cylcutter.hpp"
#include "ballcutter.hpp"
#include "bullcutter.hpp"
#include "conecutter.hpp"

namespace ocl
{

CompositeCutter::CompositeCutter() {
    radiusvec = std::vector<double>();
    cutter = std::vector<MillingCutter*>();
    radius=0;
    diameter=0;
}

// add cutter c which is valid until radius=r and height=z with z-offset zoff
void CompositeCutter::addCutter(MillingCutter& c, double r, double h, double zoff) {
    radiusvec.push_back(r);
    heightvec.push_back(h);
    cutter.push_back(&c);
    zoffset.push_back(zoff);
    if (r>radius) {
        radius = r;
        diameter = 2*r;
    }
    // update length also?
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

// this allows vertexDrop in the base-class to work as for other cutters
double CompositeCutter::height(double r) const {
    unsigned int idx = radius_to_index(r);
    return cutter[idx]->height(r) + zoffset[idx];
}

// return the width of the cutter at height h. 
double CompositeCutter::width(double h) const {
    unsigned int idx = height_to_index(h);
    return cutter[idx]->width( h - zoffset[idx] );
}

unsigned int CompositeCutter::height_to_index(double h) const {
    for (unsigned int n=0; n<cutter.size(); ++n) {
        if ( validHeight(n,h) )
            return n;
    }
    assert(0);
    return 0;
}


unsigned int CompositeCutter::radius_to_index(double r) const {
    for (unsigned int n=0; n<cutter.size(); ++n) {
        if ( validRadius(n,r) )
            return n;
    }
    assert(0);
    return 0;
}

// return true if height h belongs to cutter n
bool CompositeCutter::validHeight(unsigned int n, double h) const {
    double lolimit, hilimit;
    if (n==0)
        lolimit = -1E-6;
    else
        lolimit = heightvec[n-1] - 1E-6;
    hilimit = heightvec[n]+1e-6; // FIXME: really ugly solution this one...
    if ( (lolimit<=h) )
        if  (h<=hilimit)
            return true;
    return false;
}


bool CompositeCutter::validRadius(unsigned int n, double r) const {
    double lolimit, hilimit;
    if (n==0)
        lolimit = -1E-6;
    else
        lolimit = radiusvec[n-1] - 1E-6;
    hilimit = radiusvec[n]+1e-6; // FIXME: really ugly solution this one...
    if ( (lolimit<=r) )
        if  (r<=hilimit)
            return true;
    return false;
}

//********   facetDrop  ********************** */

// call facetDrop on each cutter and pick a valid cc-point
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
        if ( cutter[n]->edgeDrop(cl_tmp,t) ) { // drop sub-cutter against edge
            if ( ccValid(n,cl_tmp) ) { // check if cc-point is valid
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

MillingCutter* CompositeCutter::offsetCutter(double d) const {
    std::cout << " ERROR: not implemented.\n";
    assert(0);
    return  new CylCutter(); //FIXME!
}

std::string CompositeCutter::str() const {
    std::ostringstream o;
    o << "CompositeCutter with "<< cutter.size() << " cutters:\n";
    for (unsigned int n=0; n<cutter.size(); ++n) { // loop through cutters
        o << " " << n << ":" << cutter[n]->str() << "\n";
        o << "  radius["<< n << "]="<< radiusvec[n] << "\n";
        o << "  height["<< n << "]="<< heightvec[n] << "\n";
        o << "  zoffset["<< n << "]="<< zoffset[n] << "\n";
    }
    return o.str();
}


//********   actual Composite-cutters  ******************************* */

   
//  only constructors required, drop-cutter calls handled by base-class

CylConeCutter::CylConeCutter(double diam1, double diam2, double angle) {
    MillingCutter* cyl = new CylCutter(diam1, 1 );
    MillingCutter* cone = new ConeCutter(diam2, angle);
    double cone_offset= - (diam1/2)/tan(angle);
    double cyl_height = 0.0;
    double cone_height = (diam2/2.0)/tan(angle) + cone_offset;
    addCutter( *cyl, diam1/2.0, cyl_height, 0.0 );
    addCutter( *cone, diam2/2.0, cone_height, cone_offset );
}

BallConeCutter::BallConeCutter(double diam1, double diam2, double angle) {
    MillingCutter* c1 = new BallCutter(diam1, 1); // at offset zero  FIXME: length
    MillingCutter* c2 = new ConeCutter(diam2, angle);
    double cone_offset = - ( (diam1/2.0)/sin(angle) - diam1/2.0);
    double rcontact = (diam1/2.0)*cos(angle);
    double height1 = 0;
    double height2 = 0;
    
    addCutter( *c1, rcontact, height1, 0.0 );
    addCutter( *c2, diam2/2.0, height2, cone_offset );
}

BullConeCutter::BullConeCutter(double diam1, double radius1, double diam2, double angle) {
    MillingCutter* c1 = new BullCutter(diam1, radius1, 1.0); // at offset zero FIXME: length
    MillingCutter* c2 = new ConeCutter(diam2, angle);
    double h1 = radius1*sin(angle); // the contact point is this much down from the toroid-ring
    double rad = sqrt( square(radius1) - square(h1) );
    double rcontact = (diam1/2.0) - radius1 + rad; // radius of the contact-ring
    double cone_offset= - ( rcontact/tan(angle) - (radius1-h1));
    double height1 = 0;
    double height2 = 0;
    
    addCutter( *c1, rcontact, height1, 0.0 );
    addCutter( *c2, diam2/2.0, height2, cone_offset );
}

ConeConeCutter::ConeConeCutter(double diam1, double angle1, double diam2, double angle2)
{
    MillingCutter* c1 = new ConeCutter(diam1, angle1); // at offset zero
    MillingCutter* c2 = new ConeCutter(diam2, angle2);
    double h1 = (diam1/2.0)/tan(angle1); 
    double h2 = (diam1/2.0)/tan(angle2);
    double cone_offset= - ( h2-h1);
    double height1 = 0;
    double height2 = 0;
    
    addCutter( *c1, diam1/2.0, height1, 0.0 );
    addCutter( *c2, diam2/2.0, height2, cone_offset );
}

} // end namespace
// end file compositecutter.cpp
