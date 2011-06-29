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


#include <cassert>
// uncomment to disable assert() calls
// #define NDEBUG

#include "point.h"
#include "triangle.h"
#include "millingcutter.h"
#include "cylcutter.h"

#include "numeric.h"
#include "volume.h"

namespace ocl
{



//************* Sphere **************/

/// sphere at center
SphereOCTVolume::SphereOCTVolume() {
    center = Point(0,0,0);
    radius = 1.0;
    calcBB();
    invert = false;
}

double SphereOCTVolume::dist(Point& p ) const {
    double d = (center-p).norm();
    if (invert)
        return -(d-radius);
    else 
        return d-radius;
}

/// set the bounding box values
void SphereOCTVolume::calcBB() {
    bb.clear();
    Point maxpt = Point(center.x + radius, center.y + radius, center.z + radius);
    Point minpt = Point(center.x - radius, center.y - radius, center.z - radius);
    bb.addPoint( maxpt );
    bb.addPoint( minpt );
}

bool SphereOCTVolume::isInside(Point& p) const
{
    if (!isInsideBB(p))
        return false;
    
    if ( (center-p).norm() <= radius ) 
        return true;
    else
        return false;
}


//************* Cube **************/

/// cube at center with side length side
CubeVolume::CubeVolume() {
    center = Point(0,0,0);
    side = 1.234;
    invert = false;
}

/// set bbox values
void CubeVolume::calcBB() {
    bb.clear();
    Point maxpt = Point(center.x + side/2, center.y + side/2, center.z + side/2);
    Point minpt = Point(center.x - side/2, center.y - side/2, center.z - side/2);
    bb.addPoint( maxpt );
    bb.addPoint( minpt );
}

double CubeVolume::dist(Point& p) const {
    // from: http://reocities.com/SiliconValley/port/3456/isosurf/isosurfaces.html
    double d =  std::max( square(p.x)-square(side/2), std::max( square(p.y)-square(side/2), square(p.z)-square(side/2) ) );
    if (invert)
        return -d;
    else
        return d;
}

bool CubeVolume::isInside(Point& p) const {
    bool x,y,z;
    x = ( (p.x >= (center.x-side/2)) && (p.x <= (center.x+side/2)) );
    y = ( (p.y >= (center.y-side/2)) && (p.y <= (center.y+side/2)) );
    z = ( (p.z >= (center.z-side/2)) && (p.z <= (center.z+side/2)) );
    if ( x && y && z )
        return true;
    else
        return false;
}


//************* Box *******************/

BoxOCTVolume::BoxOCTVolume()
{
    corner = Point(0,0,0); 
    v1 = Point(1,0,0); 
    v2 = Point(0,1,0);
    v3 = Point(0,0,1);
}

void BoxOCTVolume::calcBB() {
    bb.clear();
    bb.addPoint(corner);
    bb.addPoint(corner+v1);
    bb.addPoint(corner+v2);
    bb.addPoint(corner+v3);
}

bool BoxOCTVolume::isInside(Point& p) const
{
    /*
    // translate to origo
    Point pt = p - corner;
    
    // box is = a*v1 + b*v2 + c*v3
    // where a,b,c are in [0,1]
    
    // v1 radial
    // v2 along move
    // v3 axial(z-dir)
    
    Point v1xy = v1;
    v1xy.z = 0;
    
    Point v2xy = v2;
    v2xy.z = 0;
    
    // projection along each vector, in turn
    // this only works if the vectors are orthogonal
    double t1 = pt.dot(v1xy)/v1xy.dot(v1xy);
    if ( (t1 < 0.0) || (t1>1.0) )
        return false;
        
    double t2 = pt.dot(v2xy)/v2xy.dot(v2xy);
    if ( (t2 < 0.0) || (t2>1.0) )
        return false;
    
    // this ensures we are OK in the XY plane
    // now check the z-coordinate.
    double zmin = corner.z + t2*v2.z;
    if (p.z < zmin)
        return false;
    double zmax = corner.z + v3.z + t2*v2.z;
    if (p.z > zmax)
        return false;
        
    //t = pt.dot(v3)/v3.dot(v3);
    //if ( (t < 0.0) || (t>1.0) )
    //    return false;
    */
    return (dist(p) <= 0.0);
    
}

//************* Cylinder **************/

CylinderOCTVolume::CylinderOCTVolume()
{
    p1 = Point(0,0,0);
    p2 = Point(1,1,1);
    radius = 1.234;
}

bool CylinderOCTVolume::isInside(Point& p) const 
{
    // closest point on axis
    Point c = p.closestPoint(p1, p2);
    // line = p1 + t*(p2-p1)
    // t is in [0,1] for points on the line
    double t = (c.dot(p2-p1) - p1.dot( p2-p1)) / (p2-p1).dot(p2-p1);
    if ( (t>1.0) || (t < 0.0))
        return false;
        
    if ( (c-p).norm() <= radius)
        return true;
    else
        return false;
}

/// calculate the bounding-box 
void CylinderOCTVolume::calcBB()
{
    // this is an approximate bounding-box, not exact
    Point xmax1 = p1 + Point(radius,0,0);
    Point xmin1 = p1 - Point(radius,0,0);
    Point xmax2 = p2 + Point(radius,0,0);
    Point xmin2 = p2 - Point(radius,0,0);
    
    Point ymax1 = p1 + Point(0,radius,0);
    Point ymin1 = p1 - Point(0,radius,0);
    Point ymax2 = p2 + Point(0,radius,0);
    Point ymin2 = p2 - Point(0,radius,0);    
    
    Point zmax1 = p1 + Point(0,0,radius);
    Point zmin1 = p1 - Point(0,0,radius);
    Point zmax2 = p2 + Point(0,0,radius);
    Point zmin2 = p2 - Point(0,0,radius);  
    
    bb.addPoint( xmax1 );   
    bb.addPoint( xmax2 );
    bb.addPoint( xmin1 );
    bb.addPoint( xmin2 );

    bb.addPoint( ymax1 );   
    bb.addPoint( ymax2 );
    bb.addPoint( ymin1 );
    bb.addPoint( ymin2 );
    
    bb.addPoint( zmax1 );   
    bb.addPoint( zmax2 );
    bb.addPoint( zmin1 );
    bb.addPoint( zmin2 );
    
    
}
//************* EtubeOCTVolume *************/


EtubeOCTVolume::EtubeOCTVolume() 
{
    p1 = Point(0,0,0);
    p2 = Point(1,0,0);
    a = Point(0,0.1,0);
    b = Point(0,0,0.2);
}

EtubeOCTVolume::EtubeOCTVolume(Point& p1in, Point& p2in, Point& ain, Point& bin) 
{
    p1 = p1in;
    p2 = p2in;
    a = ain;
    b = bin;
}


bool EtubeOCTVolume::isInside(Point& p) const 
{
    
    // xy-plane check
    
    //Point v = p2-p1;
    //Point vxy = v;
    //vxy.z =0;
    
    // translate so (0,0) is at p1
    //Point pt = p1 - p;
    
    // restrict to points closer than a.norm() to tool-line   
    double xyd = p.xyDistanceToLine(p1,p2);
    if (xyd > a.norm() )
        return false;
        
    // coordinates along ellipse
    
    // center of ellipse
    Point close = p.closestPoint(p1, p2);
    
    Point ellvec = p-close;
    
    // is ellvec within ellipse?
    double ta = ellvec.dot(a)/a.dot(a);
    double tb = ellvec.dot(b)/b.dot(b);
    if ( (ta>1.0) || (ta<-1.0) )
        return false;
        
    if ( (tb>1.0) || (tb<-1.0) )
        return false;
        
    if ((ta*ta + tb*tb) > 1.0)
        return false;
    
    // projection along 
    // this only works if the vectors are orthogonal
    /*
    double t1 = pt.dot(v1xy)/v1xy.dot(v1xy);
    if ( (t1 < 0.0) || (t1>1.0) )
        return false;
        
    double t2 = pt.dot(v2xy)/v2xy.dot(v2xy);
    if ( (t2 < 0.0) || (t2>1.0) )
        return false;
    */
    
    // z-direction check
    double maxz = p1.z > p2.z ? p1.z : p2.z; 
    double minz = p1.z < p2.z ? p1.z : p2.z;   
    
    if (p.z < minz)
        return false;
        
    if (p.z > maxz)
        return false;
        
    
    //if (p.z 
    // figure out where we are and return true/false
    return true;
}


//************* CylCutterMove **************/


CylMoveOCTVolume::CylMoveOCTVolume(const CylCutter& cin, const Point& p1in, const Point& p2in)
{
    p1 = p1in;
    p2 = p2in;
    c = cin;
    
    Point cutter_zvec = Point(0,0,c.getLength());
    // start of move, extreme point(s)
    bb.addPoint( p1 + c.getRadius()*Point(1,0,0) );
    bb.addPoint( p1 + c.getRadius()*Point(-1,0,0) );
    bb.addPoint( p1 + c.getRadius()*Point(0,1,0) );
    bb.addPoint( p1 + c.getRadius()*Point(0,-1,0) );
    // start of move (top)
    bb.addPoint( p1 + c.getRadius()*Point(1,0,0)  + cutter_zvec );
    bb.addPoint( p1 + c.getRadius()*Point(-1,0,0) + cutter_zvec );
    bb.addPoint( p1 + c.getRadius()*Point(0,1,0)  + cutter_zvec );
    bb.addPoint( p1 + c.getRadius()*Point(0,-1,0) + cutter_zvec );
    // end of move (bottom)
    bb.addPoint( p2 + c.getRadius()*Point(1,0,0) );
    bb.addPoint( p2 + c.getRadius()*Point(-1,0,0) );
    bb.addPoint( p2 + c.getRadius()*Point(0,1,0) );
    bb.addPoint( p2 + c.getRadius()*Point(0,-1,0) );
    // end of move (top)
    bb.addPoint( p2 + c.getRadius()*Point(1,0,0)  + cutter_zvec );
    bb.addPoint( p2 + c.getRadius()*Point(-1,0,0) + cutter_zvec );
    bb.addPoint( p2 + c.getRadius()*Point(0,1,0)  + cutter_zvec );
    bb.addPoint( p2 + c.getRadius()*Point(0,-1,0) + cutter_zvec );
    
    
    // cylinder at start of move
    c1.p1 = p1;
    c1.p2 = p1+Point(0,0,c.getLength());
    c1.radius=c.getRadius();
    // std::cout << " startcyl at " << c1.p1 << " to " << c1.p2 << "radius="<< c1.radius << "\n";
    
    // cylinder at end of move
    c2.p1 = p2;
    c2.p2 = p2+Point(0,0,c.getLength());
    c2.radius=c.getRadius();
    // std::cout << " endcyl at " << c2.p1 << " to " << c2.p2 <<" radius=" << c2.radius << "\n";
    
    // for XY-plane moves, a box:
    Point v = p2-p1; // vector along move
    Point v2 = p2-p1; 
    box.v2 = p2-p1;
    
    v.z = 0;
    v.normalize();
    
    box.corner = p1 + c.getRadius()*v.xyPerp();
    box.v1 = -2*c.getRadius()*v.xyPerp();
    box.v2 = v2;
    box.v3 = Point(0,0,c.getLength());
    // std::cout << " box at corner=" << box.corner << "\n";
    // std::cout << "            v1=" << box.v1 << "\n";
    // std::cout << "            v2=" << box.v2 << "\n";
    // std::cout << "            v3=" << box.v3 << "\n";
    
    // the elliptic tube
    etube = EtubeOCTVolume();
    // set the parameters
    etube.p1 = p1;
    etube.p2 = p2;
    etube.a = c.getRadius()*v.xyPerp();
    //etube.b = Point(0,0,0); //fixme
    // angle of move
    double dz = p2.z-p1.z;
    double length = (p2-p1).norm();
        
    double sin = dz/length;
    //double cos = sqrt( 1.0-sin*sin);
    double baxis = fabs(c.getRadius()*sin);
    // std::cout << " Etube baxis length="<< baxis << "\n";
    // direction is cross product 
    Point bdir = (p2-p1).cross(etube.a);
    bdir.normalize();
    etube.b= baxis*bdir;
    // std::cout << " Etube a="<< etube.a << " b=" << etube.b << "\n";
    
}

bool CylMoveOCTVolume::isInside(Point& p) const 
{
    // CL follows line
    // line = p1 + t*(p2-p1)
    // top of cutter follows same line only c.length() higher
    if (c1.isInside(p)) 
        return true;
    
    if (c2.isInside(p))
        return true;
    
    // for XY-plane moves, a box:
    if (box.isInside(p))
        return true;
    
    // the Elliptic tube
    if (etube.isInside(p))
        return true;
        
    // the default is to return false
    return false;

}

//************* CylCutterVolume **************/

CylCutterVolume::CylCutterVolume() {
    radius = 1.0;
    length = 1.0;
    pos = Point(0,0,0);
    calcBB();
}

void CylCutterVolume::setPos(Point& p) {
    pos = p;
    calcBB();
}

void CylCutterVolume::calcBB() {
    bb.clear();
    bb.addPoint( pos + Point(radius,radius,0) ); //FIXME
    bb.addPoint( pos + Point(-radius,-radius,length) ); // FIXME
}

bool CylCutterVolume::isInside(Point& p) const {
    Point t = p-pos;
    if (t.z < 0 )
        return false;
    else {
        double det = std::max( fabs(t.z-length/2)-length/2 , t.x*t.x+t.y*t.y-radius*radius );
        return (det < 0.0); 
    }
}

double CylCutterVolume::dist(Point& p) const {
    Point t = p-pos;
    if (t.z >= 0 )
        return std::max( fabs(t.z-length/2)-length/2 , t.x*t.x+t.y*t.y-radius*radius );
    else {
        // if we are under the cutter, then return distance to flat cutter bottom
        if ( t.x*t.x+t.y*t.y < radius*radius )
            return -t.z; 
        else { // outside the cutter, return a distance to the outer "ring" of the cutter
            Point nxy = t;
            nxy.xyNormalize();
            nxy = radius * nxy;
            return (t.x-nxy.x)*(t.x-nxy.x) + (t.y-nxy.y)*(t.y-nxy.y) + t.z*t.z;
        }
    }
}

//************* BallCutterVolume **************/

BallCutterVolume::BallCutterVolume() {
    radius = 1.0;
    length = 1.0;
    pos = Point(0,0,0);
}

void BallCutterVolume::setPos(Point& p) {
    pos = p;
    calcBB();
}

void BallCutterVolume::calcBB() {
    bb.clear();
    double safety = 1;
    bb.addPoint( pos + Point(safety*radius,safety*radius,safety*length) );
    bb.addPoint( pos + Point(-safety*radius,-safety*radius,-safety*length) );
}

double BallCutterVolume::dist(Point& p) const {
    Point t = p - pos - Point(0,0,radius);
    if (t.z < 0 )
        return  square(t.x) + square(t.y) + square(t.z) -  square( radius ) ;
    else {
        return std::max( fabs(t.z)-length ,  square(t.x) + square(t.y)  - square(radius) ) ;
    }
}

//************* BullCutterVolume **************/
// TOROID 

BullCutterVolume::BullCutterVolume() {
    r1 = 1.0;
    r2 = r1/3.0;
    radius = r1+r2;
    length = 1.0;
    pos = Point(0,0,0);
}

void BullCutterVolume::setPos(Point& p) {
    pos = p;
    calcBB();
}

void BullCutterVolume::calcBB() {
    bb.clear();
    double safety = 1;
    bb.addPoint( pos + Point(safety*radius,safety*radius,safety*length) );
    bb.addPoint( pos + Point(-safety*radius,-safety*radius,-safety*length) );
}

double BullCutterVolume::dist(Point& p) const {
    Point t = p - pos - Point(0,0,r2); // shift up by tube radius
    if (t.z >= 0.0 ) // cylindrical part, above toroid
        return std::max( fabs(t.z)-length , square(t.x) + square(t.y) - square(r1+r2) );
    else if ( square(t.x)+square(t.y) <= square(r1) ) // cylindrical part, inside toroid
                     //was fabs(t.z)-r2
        return std::max( square(t.z)-square(r2) , square(t.x) + square(t.y) - square( r1 ) );
    else // toroid
        return square( square(t.x) + square(t.y) + square(t.z) + square( r1 ) - square( r2 ) ) - 
               4*square(r1)*(square(t.x)+square(t.y));
}

//************* PlaneVolume **************/

PlaneVolume::PlaneVolume(bool s, unsigned int a, double p) {
    sign = s;
    axis = a;
    position = p; // FIXME, position=0 does not seem to work right!
    calcBB();
}

void PlaneVolume::calcBB() {
    bb.clear();
    Point maxp;
    Point minp;
    double bignum = 1e6;
    maxp = Point(bignum,bignum,bignum);
    minp = Point( -bignum,-bignum,-bignum );
    bb.addPoint( maxp );
    bb.addPoint( minp );
}

double PlaneVolume::dist(Point& p) const {
    if (axis==0u) {
        if (sign)
            return p.x - position;
        else
            return -(p.x - position);
    } else if (axis==1u) {
        if (sign)
            return p.y - position;
        else
            return -(p.y - position);
    } else if (axis==2u) {
        if (sign)
            return p.z - position;
        else
            return -(p.z - position);
    } else {
        assert(0);
        return -1;
    }
}


} // end namespace
// end of file volume.cpp
