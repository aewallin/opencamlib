/*  $Id$
 * 
 *  Copyright (c) 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com).
 *  
 *  This file is part of OpenCAMlib 
 *  (see https://github.com/aewallin/opencamlib).
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include <iostream>
#include <sstream>
#include <string>

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



// this allows vertexDrop in the base-class to work as for other cutters
double CompositeCutter::height(double r) const {
    unsigned int idx = radius_to_index(r);
    return cutter[idx]->height(r) + zoffset[idx];
}

// return the width of the cutter at height h. 
double CompositeCutter::width(double h) const {
    unsigned int idx = height_to_index(h);
    // std::cout << "CompositeCutter::width( " << h << " ) idx=" << idx << " zoffset= " << zoffset[idx] << "\n";
    // std::cout << " width  =  " << cutter[idx]->width( h - zoffset[idx] ) << "\n";
    return cutter[idx]->width( h - zoffset[idx] );
}

unsigned int CompositeCutter::height_to_index(double h) const {
    for (unsigned int n=0; n<cutter.size(); ++n) {
        if ( validHeight(n,h) )
            return n;
    }
    // return the last cutter if we get here...
    return cutter.size()-1;
    std::cout << " Error, height= " << h << " has no index \n";
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

bool CompositeCutter::ccValidRadius(unsigned int n, CLPoint& cl) const {
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

bool CompositeCutter::ccValidHeight(unsigned int n, CCPoint& cc, const Fiber& f) const {
    //if (  ((cc.z-f.p1.z) >= 0.0)  && (n == height_to_index(cc.z-f.p1.z)) )
    if (  n == height_to_index(cc.z-f.p1.z) )
        return true;
    else
        return false;
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
    assert( r >= 0.0 );
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
            if ( ccValidRadius(n,cl_tmp) ) { // cc-point is valid
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
            if ( ccValidRadius(n,cl_tmp) ) { // check if cc-point is valid
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

bool CompositeCutter::vertexPush(const Fiber& f, Interval& i, const Triangle& t) const {
    bool result = false;
    std::vector< std::pair<double, CCPoint> > contacts;
    for (unsigned int n=0; n<cutter.size(); ++n) {
        Interval ci;
        Fiber cf(f);
        cf.p1.z = f.p1.z + zoffset[n];
        cf.p2.z = f.p2.z + zoffset[n]; // raised/lowered fiber to push along
        if ( cutter[n]->vertexPush(cf,ci,t) ) {
            if ( ccValidHeight( n, ci.upper_cc, f ) )
                contacts.push_back( std::pair<double,CCPoint>(ci.upper, ci.upper_cc) );
            if ( ccValidHeight( n, ci.lower_cc, f ) )
                contacts.push_back( std::pair<double,CCPoint>(ci.lower, ci.lower_cc) );
        }
    }
    
    for( unsigned int n=0; n<contacts.size(); ++n ) {
        i.update( contacts[n].first, contacts[n].second );
        result = true;
    }
    return result;
}

// push each cutter against facet
//  if the cc-point is valid (at correct height), store interval data
// push all interval data into the original interval
bool CompositeCutter::facetPush(const Fiber& f, Interval& i, const Triangle& t) const {
    // run facetPush for each cutter, retain valid results, and return union of all
    bool result = false;
    std::vector< std::pair<double, CCPoint> > contacts;
    for (unsigned int n=0; n<cutter.size(); ++n) {
        Interval ci;
        Fiber cf(f);
        cf.p1.z = f.p1.z + zoffset[n];
        cf.p2.z = f.p2.z + zoffset[n]; // raised/lowered fiber to push along
        if ( cutter[n]->facetPush(cf,ci,t) ) {
            if ( ccValidHeight( n, ci.upper_cc, f ) )
                contacts.push_back( std::pair<double,CCPoint>(ci.upper, ci.upper_cc) );
            if ( ccValidHeight( n, ci.lower_cc, f ) )
                contacts.push_back( std::pair<double,CCPoint>(ci.lower, ci.lower_cc) );
        }
    }
    
    for( unsigned int n=0; n<contacts.size(); ++n ) {
        i.update( contacts[n].first, contacts[n].second );
        result = true;
    }
    return result;
}



bool CompositeCutter::edgePush(const Fiber& f, Interval& i, const Triangle& t) const {
    bool result = false;
    std::vector< std::pair<double, CCPoint> > contacts;
    for (unsigned int n=0; n<cutter.size(); ++n) {
        Interval ci; // interval for this cutter
        Fiber cf(f); // fiber for this cutter
        cf.p1.z = f.p1.z + zoffset[n];
        cf.p2.z = f.p2.z + zoffset[n]; // raised/lowered fiber to push along
        if ( cutter[n]->edgePush(cf,ci,t) ) {
            if ( ccValidHeight( n, ci.upper_cc, f ) )
                contacts.push_back( std::pair<double,CCPoint>(ci.upper, ci.upper_cc) );
            if ( ccValidHeight( n, ci.lower_cc, f ) )
                contacts.push_back( std::pair<double,CCPoint>(ci.lower, ci.lower_cc) );
        }
    }
    
    for( unsigned int n=0; n<contacts.size(); ++n ) {
        i.update( contacts[n].first, contacts[n].second );
        result = true;
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

   
//  only constructors required, drop-cutter and push-cutter calls handled by base-class

// TESTING
CompCylCutter::CompCylCutter(double diam2, double clength) {
    MillingCutter* shaft = new CylCutter(diam2, clength ); 
    addCutter( *shaft, diam2/2.0, clength , 0.0 );    
    length = clength;
}

// TESTING
CompBallCutter::CompBallCutter(double diam2, double clength) {
    MillingCutter* shaft = new BallCutter(diam2, clength ); 
    addCutter( *shaft, diam2/2.0, clength , 0.0 );    
    length = clength;
}


CylConeCutter::CylConeCutter(double diam1, double diam2, double angle) {
    MillingCutter* cyl = new CylCutter(diam1, 1 );
    MillingCutter* cone = new ConeCutter(diam2, angle);
    MillingCutter* shaft = new CylCutter(diam2, 20 ); // FIXME: dummy height
    
    double cone_offset= - (diam1/2)/tan(angle);
    double cyl_height = 0.0;
    double cone_height = (diam2/2.0)/tan(angle) + cone_offset;
    
    addCutter( *cyl, diam1/2.0, cyl_height, 0.0 );
    addCutter( *cone, diam2/2.0, cone_height, cone_offset );
    addCutter( *shaft, diam2/2.0, (diam2/2.0)/tan(angle) + 20 , cone_height );
    length = cyl_height + cone_height + 10; // Arbitrary 10 here!
}


BallConeCutter::BallConeCutter(double diam1, double diam2, double angle) {
    MillingCutter* c1 = new BallCutter(diam1, 1); // at offset zero 
    MillingCutter* c2 = new ConeCutter(diam2, angle);
    MillingCutter* shaft = new CylCutter(diam2, 20); // FIXME: length
    
    double radius1 = diam1/2.0;
    double radius2 = diam2/2.0;
    double rcontact = radius1*cos(angle);
    double height1 = radius1 - sqrt( square(radius1)-square(rcontact) );
    double cone_offset = - ( (rcontact)/tan(angle) - height1 );
    double height2 = radius2/tan(angle) + cone_offset;
    double shaft_offset = height2;
    
    // cutter, radivec, heightvec, zoffset
    addCutter( *c1, rcontact, height1, 0.0 );
    addCutter( *c2, diam2/2.0, height2, cone_offset );
    addCutter( *shaft, diam2/2.0, height2+20, shaft_offset );
    length = 30;
}

BullConeCutter::BullConeCutter(double diam1, double radius1, double diam2, double angle) {
    MillingCutter* c1 = new BullCutter(diam1, radius1, 1.0); // at offset zero 
    MillingCutter* c2 = new ConeCutter(diam2, angle);
    MillingCutter* shaft = new CylCutter(diam2, 20);
    
    double h1 = radius1*sin(angle); // the contact point is this much down from the toroid-ring
    double rad = sqrt( square(radius1) - square(h1) );
    double rcontact = (diam1/2.0) - radius1 + rad; // radius of the contact-ring
    double cone_offset= - ( rcontact/tan(angle) - (radius1-h1));
    double height1 = radius1 - h1;
    double height2 = (diam2/2.0)/tan(angle) + cone_offset;
    double shaft_offset = height2;
    
    addCutter( *c1, rcontact, height1, 0.0 );
    addCutter( *c2, diam2/2.0, height2, cone_offset );
    addCutter( *shaft, diam2/2.0, height2+20, shaft_offset );
    length=30;
}

ConeConeCutter::ConeConeCutter(double diam1, double angle1, double diam2, double angle2)
{
    MillingCutter* c1 = new ConeCutter(diam1, angle1); // at offset zero
    MillingCutter* c2 = new ConeCutter(diam2, angle2);
    MillingCutter* shaft = new CylCutter(diam2, 20);
    
    double height1 = (diam1/2.0)/tan(angle1); 
    double tmp = (diam1/2.0)/tan(angle2);
    double cone_offset= - ( tmp-height1);
    double height2 = (diam2/2.0)/tan(angle2) + cone_offset;
    double shaft_offset = height2;
    
    addCutter( *c1, diam1/2.0, height1, 0.0 );
    addCutter( *c2, diam2/2.0, height2, cone_offset );
    addCutter( *shaft, diam2/2.0, height2+20, shaft_offset );
    length=30;
}

} // end namespace
// end file compositecutter.cpp
