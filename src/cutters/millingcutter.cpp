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
#include "numeric.h"

namespace ocl
{

MillingCutter* MillingCutter::offsetCutter(double d) const {
    assert(0); // DON'T call me
    return  NULL;
}

// general purpose vertex-drop which delegates the this->height(r) to 
// the specific subclass of cutter 
bool MillingCutter::vertexDrop(CLPoint &cl, const Triangle &t) const {
    bool result = false;
    BOOST_FOREACH( const Point& p, t.p) {           // test each vertex of triangle
        double q = cl.xyDistance(p);                // distance in XY-plane from cl to p
        if ( q <= radius ) {                        // p is inside the cutter
            CCPoint cc_tmp(p, VERTEX);
            result = result || cl.liftZ( p.z - this->height(q), cc_tmp );
        } 
    }
    return result;
}

// general purpose facet-drop which calls xy_normal_length(), normal_length(), 
// and center_height() on the subclass
bool MillingCutter::facetDrop(CLPoint &cl, const Triangle &t) const {
    // Drop cutter at (cl.x, cl.y) against facet of Triangle t
    Point normal = t.upNormal(); // facet surface normal
    if ( isZero_tol( normal.z ) )  // vertical surface
        return false;  //can't drop against vertical surface
    assert( isPositive( normal.z ) );
    
    if ( ( isZero_tol(normal.x) ) && ( isZero_tol(normal.y) ) ) { // horizontal plane special case
        CCPoint cc_tmp( cl.x, cl.y, t.p[0].z, FACET);
        return cl.liftZ_if_inFacet(cc_tmp.z, cc_tmp, t);
    } else { // general case
        // plane containing facet:  a*x + b*y + c*z + d = 0, so
        // d = -a*x - b*y - c*z, where  (a,b,c) = surface normal
        double d = - normal.dot(t.p[0]);
        normal.normalize(); // make length of normal == 1.0
        Point xyNormal( normal.x, normal.y, 0.0);
        xyNormal.xyNormalize();
        // define the radiusvector which points from the cc-point to the cutter-center 
        Point radiusvector = this->xy_normal_length*xyNormal + this->normal_length*normal;
        CCPoint cc_tmp = cl - radiusvector; // NOTE xy-coords right, z-coord is not.
        cc_tmp.z = (1.0/normal.z)*(-d-normal.x*cc_tmp.x-normal.y*cc_tmp.y); // cc-point lies in the plane.
        cc_tmp.type = FACET;
        double tip_z = cc_tmp.z + radiusvector.z - this->center_height;
        return cl.liftZ_if_inFacet(tip_z, cc_tmp, t);
    }
}

// edge-drop function which calls the sub-class MillingCutter::singleEdgeDrop on each 
// edge of the input Triangle t.
bool MillingCutter::edgeDrop(CLPoint &cl, const Triangle &t) const {
    //return this->singleEdgeDrop(cl,t.p[0],t.p[1]) || this->singleEdgeDrop(cl,t.p[1],t.p[2]) || this->singleEdgeDrop(cl,t.p[2],t.p[0]);
    bool result = false;
    for (int n=0;n<3;n++) { // loop through all three edges
        int start=n;      // index of the start-point of the edge
        int end=(n+1)%3;  // index of the end-point of the edge
        const Point p1 = t.p[start];
        const Point p2 = t.p[end];
        if ( !isZero_tol( p1.x - p2.x) || !isZero_tol( p1.y - p2.y) ) {
            const double d = cl.xyDistanceToLine(p1,p2);
            if (d<=radius)  // potential contact with edge
                if ( this->singleEdgeDrop(cl,p1,p2,d) )
                    result=true;
        }
    }
    return result;
}

// "dual" edge-drop problems
// cylinder: zero diam edge/ellipse, r-radius cylinder, find r-offset == cl  (ITO surface XY-slice is a circle)
// sphere: zero diam cylinder. ellipse around edge, find offset == cl (ITO surface slice is ellipse) (?)
// toroid: radius2 diam edge, radius1 cylinder, find radius1-offset-ellipse=cl (ITO surf slice is offset ellipse) (this is the offset-ellipse problem)
// cone: ??? (how is this an ellipse??)
bool MillingCutter::singleEdgeDrop(CLPoint& cl, const Point& p1, const Point& p2, const double d) const {    
    Point v = p2 - p1; // vector along edge, from p1 -> p2
    Point vxy( v.x, v.y, 0.0);
    vxy.xyNormalize(); // normalized XY edge vector
    // figure out u-coordinates of p1 and p2 (i.e. x-coord in the rotated system)
    Point sc = cl.xyClosestPoint( p1, p2 );   
    assert( ( (cl-sc).xyNorm() - d ) < 1E-6 );
    // edge endpoints in the new coordinate system, in these coordinates, CL is at origo
    Point up1( (p1-sc).dot(vxy) , d, p1.z); // d, distance to line, is the y-coord in the rotated system
    Point up2( (p2-sc).dot(vxy) , d, p2.z);

    CC_CLZ_Pair contact = this->singleEdgeContact( up1, up2 );
    // translate back into original coord-system
    CCPoint cc_tmp( sc + contact.first * vxy, EDGE);
    cc_tmp.z_projectOntoEdge(p1,p2);
    return cl.liftZ_if_InsidePoints( contact.second , cc_tmp , p1, p2);
}

// dummy implementation, override in sub-class.
CC_CLZ_Pair MillingCutter::singleEdgeContact( const Point& u1, const Point& u2) const {
    // CCPoint cc_tmp;
    return CC_CLZ_Pair( 0.0, 0.0);
}

// general purpose vertexPush, delegates to this->width(h) 
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
                CCPoint cc_tmp( p, VERTEX );
                i.updateUpper( f.tval(stop) , cc_tmp );
                i.updateLower( f.tval(start) , cc_tmp );
                result = true;                
            }             
        }
    }
    return result;
}

// general purpose facetPush
bool MillingCutter::facetPush(const Fiber& fib, Interval& i,  const Triangle& t) const {
    bool result = false;
    Point normal = t.upNormal(); // facet surface normal, pointing up 
    if ( t.n->zParallel() ) // normal points in z-dir   
        return result; //can't push against horizontal plane, stop here.
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
    double f = -t.p[0].z - this->normal_length*normal.z + fib.p1.z + this->center_height; // note: the xy_normal does not have a z-component, so omitted here.
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
        i.update( tval, cc );
        result = true;    
    } else {
        assert(0);
    }
    
    return result;
}

// call vertex, facet, and edge drop methods on input Triangle t
bool MillingCutter::dropCutter(CLPoint &cl, const Triangle &t) const {
    bool facet, vertex, edge;
    /* // alternative ordering of the tests:
    if (cl.below(t))
        vertexDrop(cl,t);
        
    // optimisation: if we are now above the triangle we don't need facet and edge
    if ( cl.below(t) ) {
        facetDrop(cl,t); 
        edgeDrop(cl,t);
    }*/
    
    if (cl.below(t)) {
        facet = facetDrop(cl,t);
        if (!facet) {
            vertex = vertexDrop(cl,t);
            if ( cl.below(t) ) {
                edge = edgeDrop(cl,t);
            }
        }
    }
    
    return ( facet || vertex || edge ); 
}

// TESTING ONLY, don't use for real
bool MillingCutter::dropCutterSTL(CLPoint &cl, const STLSurf &s) const {
    bool result=false;
    BOOST_FOREACH( const Triangle& t, s.tris) {
        if ( this->dropCutter(cl,t) )
            result = true;
    }
    return result; 
}

// overlap test: does cutter at cl.x cl.y overlap in the xy-plane with triangle t
bool MillingCutter::overlaps(Point &cl, const Triangle &t) const {
    if ( t.bb.maxpt.x < cl.x-radius )
        return false;
    else if ( t.bb.minpt.x > cl.x+radius )
        return false;
    else if ( t.bb.maxpt.y < cl.y-radius )
        return false;
    else if ( t.bb.minpt.y > cl.y+radius )
        return false;
    else
        return true;
}


} // end namespace
// end file cutter.cpp
