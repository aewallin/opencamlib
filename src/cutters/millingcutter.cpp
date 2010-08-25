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

#include <boost/foreach.hpp>

#include "millingcutter.h"


namespace ocl
{
    

//********   MillingCutter ********************** */

MillingCutter::MillingCutter() {   
    diameter =  1.0 ;
    radius = diameter/2.0;
    length = 1.0;
}

/*
void MillingCutter::setDiameter(double d) {
    if ( d >= 0.0 ) {
        diameter=d;
        radius = d/2.0;
    } else {
        std::cout << "cutter.cpp: ERROR, MillingCutter.setDiameter(d) called with d<0 !!";
        diameter=1.0;
        radius = diameter/2.0;
    }
}*/

/*
void MillingCutter::setLength(double l) {
    if ( l > 0.0 ) {
        length=l;
    } else {
        std::cout << "cutter.cpp: ERROR, MillingCutter.setLength(l) called with l<0 !!";
        length=1.0;
    }
}*/


double MillingCutter::getDiameter() const {
    return diameter;
}

double MillingCutter::getRadius() const {
    return radius;
}

double MillingCutter::getLength() const {
    return length;
}

MillingCutter* MillingCutter::offsetCutter(double d) const {
    assert(0); // DON'T call me
    return  NULL;
}

/// general purpose vertex-drop which delegates the this->height(r) to 
/// the specific subclass of cutter 
int MillingCutter::vertexDrop(CLPoint &cl, const Triangle &t) const {
    int result = 0;
    BOOST_FOREACH( const Point& p, t.p) { // test each vertex of triangle
        double q = cl.xyDistance(p); // distance in XY-plane from cl to p
        if ( q <= radius ) { // p is inside the cutter
            CCPoint* cc_tmp = new CCPoint(p);
            if ( cl.liftZ(p.z - this->height(q)) ) { // we need to lift the cutter
                cc_tmp->type = VERTEX;
                cl.cc = cc_tmp;
                result = 1;
            } else {
                delete cc_tmp;
            }
        } 
    }
    return result;
}

/// general purpose facet-drop which calls xy_normal_length(), normal_length(), 
/// and center_height() on the subclass
int MillingCutter::facetDrop(CLPoint &cl, const Triangle &t) const {
    // Drop cutter at (cl.x, cl.y) against facet of Triangle t
    Point normal; // facet surface normal
    if ( isZero_tol( t.n->z ) )  {// vertical surface
        return -1;  //can't drop against vertical surface
    } else if (t.n->z < 0) {  // normal is pointing down
        normal = -1 * (*t.n); // flip normal
    } else {
        normal = *t.n;
    }   
    assert( isPositive( normal.z ) );
    
    // horizontal plane special case
    if ( ( isZero_tol(normal.x) ) && ( isZero_tol(normal.y) ) ) { 
        CCPoint* cc_tmp = new CCPoint(cl.x,cl.y,t.p[0].z);
        if (cc_tmp->isInside(t)) { // assuming cc-point is on the axis of the cutter...       
            if ( cl.liftZ(cc_tmp->z) ) {
                cc_tmp->type = FACET;
                cl.cc = cc_tmp;
                return 1;
            } else {
                delete cc_tmp;
            }
        } else { // not inside facet
            delete cc_tmp;
            return 0;
        }
    } // end horizontal plane case.
    
    // define plane containing facet
    // a*x + b*y + c*z + d = 0, so
    // d = -a*x - b*y - c*z, where
    // (a,b,c) = surface normal
    double a = normal.x;
    double b = normal.y;
    double c = normal.z;
    //double d = - a * t.p[0].x - b * t.p[0].y - c * t.p[0].z;
    double d = - normal.dot(t.p[0]);
    normal.normalize(); // make length of normal == 1.0
    Point xyNormal = normal;
    xyNormal.z = 0;
    xyNormal.xyNormalize();
    
    // define the radiusvector which points from the 
    // cc-point to the cutter-center 
    Point radiusvector = this->xy_normal_length*xyNormal - this->normal_length*normal;
    // find the xy-coordinates of the cc-point
    CCPoint* cc_tmp = new CCPoint();
    *cc_tmp = cl - radiusvector; // NOTE xy-coords right, z-coord is not.
    cc_tmp->z = (1.0/c)*(-d-a*cc_tmp->x-b*cc_tmp->y); // cc-point lies in the plane.
    cc_tmp->type = FACET;
    if (cc_tmp->isInside(t)) {   
        // now find the z-coordinate of the cl-point
        double tip_z = cc_tmp->z + radiusvector.z - this->center_height;
        if ( cl.liftZ(tip_z) ) {
            cl.cc = cc_tmp;
            return 1;
        } else {
            delete cc_tmp;
        }
    } else {
        delete cc_tmp;
        return 0;
    }
    return 0; // we never get here (?)
}

/// general purpose vertexPush, delegates to this->width(h) 
bool MillingCutter::vertexPush(const Fiber& f, Interval& i, const Triangle& t) const {
    bool result = false;
    BOOST_FOREACH( const Point& p, t.p) {
        if ( ( p.z >= f.p1.z ) && ( p.z <= (f.p1.z+getLength()) ) ) { // p.z is within cutter
            Point pq = p.xyClosestPoint(f.p1, f.p2); // closest point on fiber
            double q = (p-pq).xyNorm(); // distance in XY-plane from fiber to p
            double h = p.z - f.p1.z;
            assert( h>= 0.0);
            double cwidth = this->width( h );
            if ( q <= cwidth ) { // we are going to hit the vertex p
                double ofs = sqrt( square( cwidth ) - square(q) ); // distance along fiber 
                Point start = pq - ofs*f.dir;
                Point stop  = pq + ofs*f.dir;
                CCPoint cc_tmp = CCPoint(p);
                cc_tmp.type = VERTEX;
                i.updateUpper( f.tval(stop) , cc_tmp );
                i.updateLower( f.tval(start) , cc_tmp );
                result = true;                
            }             
        }
    }
    return result;
}

/// general purpose facetPush
bool MillingCutter::facetPush(const Fiber& fib, Interval& i,  const Triangle& t) const {
    bool result = false;
    Point normal; // facet surface normal 
    if ( t.n->zParallel() ) { // normal points in z-dir   
        return result; //can't push against horizontal plane, stop here.
    }
    else if (t.n->z < 0) {  // normal is pointing down
        normal = -1* (*t.n); // flip normal
    } else {
        normal = *t.n;
    } // now we know the normal points upwards
    normal.normalize();
    Point xy_normal = normal;
    xy_normal.z = 0;
    xy_normal.xyNormalize();
    
    //   find a point on the plane from which radius2*normal+radius1*xy_normal lands on the fiber+radius2*Point(0,0,1) 
    //   (u,v) locates a point on the triangle facet    v0+ u*(v1-v0)+v*(v2-v0)    u,v in [0,1]
    //   t locates a point along the fiber:             p1 + t*(p2-p1)             t in [0,1]
    // 
    //   facet-point + r2 * n + r1* xy_n = fiber-point + r2*Point(0,0,1)
    //   =>
    //   v0+ u*(v1-v0)+v*(v2-v0) + r2 * n + r1* xy_n  = p1 + t*(p2-p1) + r2*Point(0,0,1)
    //
    //   v0x + u*(v1x-v0x) + v*(v2x-v0x) + r2*nx + r1*xy_n.x  = p1x + t*(p2x-p1x)          p2x-p1x==0 for Y-fiber
    //   v0y + u*(v1y-v0y) + v*(v2y-v0y) + r2*ny + r1*xy_n.y  = p1y + t*(p2y-p1y)          p2y-p1y==0 for X-fiber
    //   v0z + u*(v1z-v0z) + v*(v2z-v0z) + r2*nz              = p1z + t*(p2z-p1z) + r2    (p2z-p1z)==0 for XY-fibers!!
    //   X-fiber:
    //   v0x + u*(v1x-v0x) + v*(v2x-v0x) + r2*nx + r1*xy_n.x  = p1x + t*(p2x-p1x)         
    //   v0y + u*(v1y-v0y) + v*(v2y-v0y) + r2*ny + r1*xy_n.y  = p1y                    solve these  two for (u,v)
    //   v0z + u*(v1z-v0z) + v*(v2z-v0z) + r2*nz              = p1z + r2               and substitute above for t
    //   or
    //   [ (v1y-v0y)    (v2y-v0y) ] [ u ] = [ -v0y - r2*ny - r1*xy_n.y + p1y     ]
    //   [ (v1z-v0z)    (v2z-v0z) ] [ v ] = [ -v0z - r2*nz + p1z + r2            ]
    //
    //   Y-fiber:
    //   [ (v1x-v0x)    (v2x-v0x) ] [ u ] = [ -v0x - r2*nx - r1*xy_n.x + p1x     ]
    
    double a;
    double b;
    double c = t.p[1].z - t.p[0].z;
    double d = t.p[2].z - t.p[0].z;
    double e;
    double f = -t.p[0].z - this->normal_length*normal.z + fib.p1.z + this->center_height;
    double u;
    double v;
    // a,b,e depend on the fiber:
    if ( fib.p1.y == fib.p2.y ) { // XFIBER
        a = t.p[1].y - t.p[0].y;
        b = t.p[2].y - t.p[0].y;
        e = -t.p[0].y - this->normal_length*normal.y - this->xy_normal_length*xy_normal.y + fib.p1.y;
        if (!two_by_two_solver(a,b,c,d,e,f,u,v))
            return result;
        CCPoint cc = t.p[0] + u*(t.p[1]-t.p[0]) + v*(t.p[2]-t.p[0]);
        cc.type = FACET;
        if ( ! cc.isInside( t ) ) 
            return result;
        // v0x + u*(v1x-v0x) + v*(v2x-v0x) + r2*nx + r1*xy_n.x = p1x + t*(p2x-p1x) 
        // =>
        // t = 1/(p2x-p1x) * ( v0x + r2*nx + r1*xy_n.x - p1x +  u*(v1x-v0x) + v*(v2x-v0x)       )
        assert( !isZero_tol( fib.p2.x - fib.p1.x )  );
        double tval = (1.0/( fib.p2.x - fib.p1.x )) * ( t.p[0].x + this->normal_length*normal.x + this->xy_normal_length*xy_normal.x - fib.p1.x 
                                                        + u*(t.p[1].x-t.p[0].x)+v*(t.p[2].x-t.p[0].x) );
        if ( tval < 0.0 || tval > 1.0  ) {
            std::cout << "MillingCutter::facetPush() tval= " << tval << " error!?\n";
        } 
        assert( tval > 0.0 && tval < 1.0 );
        i.updateUpper( tval  , cc );
        i.updateLower( tval  , cc );
        result = true;
    } else if (fib.p1.x == fib.p2.x) { // YFIBER
        a = t.p[1].x - t.p[0].x;
        b = t.p[2].x - t.p[0].x;
        e = -t.p[0].x - this->normal_length*normal.x - this->xy_normal_length*xy_normal.x + fib.p1.x;
        if (!two_by_two_solver(a,b,c,d,e,f,u,v))
            return result;
        CCPoint cc = t.p[0] + u*(t.p[1]-t.p[0]) + v*(t.p[2]-t.p[0]);
        cc.type = FACET;
        if ( ! cc.isInside( t ) ) 
            return result;
        assert( !isZero_tol( fib.p2.y - fib.p1.y )  );
        double tval = (1.0/( fib.p2.y - fib.p1.y )) * ( t.p[0].y + this->normal_length*normal.y + this->xy_normal_length*xy_normal.y - fib.p1.y 
                                                        + u*(t.p[1].y-t.p[0].y)+v*(t.p[2].y-t.p[0].y) );
        if ( tval < 0.0 || tval > 1.0  ) {
            std::cout << "MillingCutter::facetPush() tval= " << tval << " error!?\n";
        } 
        assert( tval > 0.0 && tval < 1.0 );
        i.updateUpper( tval  , cc );
        i.updateLower( tval  , cc );
        result = true;    
    } else {
        assert(0);
    }
    
    
    return result;
}

/// call vertex, facet, and edge drop methods
int MillingCutter::dropCutter(CLPoint &cl, const Triangle &t) const {
    /* template-method, or "self-delegation", pattern */
    if (cl.below(t))
        vertexDrop(cl,t);
        
    // optimisation: if we are now above the triangle we don't need facet and edge
    if ( cl.below(t) ) {
        facetDrop(cl,t); 
        edgeDrop(cl,t);
    }
    return 0; // void would be better, return value not used for anything(?)
}

// TESTING ONLY, don't use for real
int MillingCutter::dropCutterSTL(CLPoint &cl, const STLSurf &s) const {
    /* template-method, or "self-delegation", pattern */
    BOOST_FOREACH( const Triangle& t, s.tris) {
        dropCutter(cl,t);
    }
    return 0; 
}

bool MillingCutter::overlaps(Point &cl, const Triangle &t) const {
    if ( t.maxx < cl.x-radius )
        return false;
    else if ( t.minx > cl.x+radius )
        return false;
    else if ( t.maxy < cl.y-radius )
        return false;
    else if ( t.miny > cl.y+radius )
        return false;
    else
        return true;
}


} // end namespace
// end file cutter.cpp
