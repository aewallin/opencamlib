/*  $Id$
 * 
 *  Copyright (c) 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com).
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

#include <boost/foreach.hpp>

#include "millingcutter.hpp"
#include "numeric.hpp"

namespace ocl
{

MillingCutter* MillingCutter::offsetCutter(double d) const {
    assert(0); // DON'T call me
    return  NULL;
}

// general purpose vertex-drop which delegates to this->height(r) of subclass 
bool MillingCutter::vertexDrop(CLPoint &cl, const Triangle &t) const {
    bool result = false;
    BOOST_FOREACH( const Point& p, t.p) {           // test each vertex of triangle
        double q = cl.xyDistance(p);                // distance in XY-plane from cl to p
        if ( q <= radius ) {                        // p is inside the cutter
            CCPoint cc_tmp(p, VERTEX);
            if ( cl.liftZ( p.z - this->height(q), cc_tmp ) )
                result = true;
        } 
    }
    return result;
}

// general purpose facet-drop which calls xy_normal_length(), normal_length(), 
// and center_height() on the subclass
bool MillingCutter::facetDrop(CLPoint &cl, const Triangle &t) const { // Drop cutter at (cl.x, cl.y) against facet of Triangle t
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

// 1) translate the geometry so that in the XY plane cl = (0,0) 
// 2) rotate the p1-p2 edge so that a new edge u1-u2 lies along the x-axis
// 3) call singleEdgeDropCanonical(), implemented in the sub-class.
//    this returns the x-coordinate of the CC point and cl.z
// 4) rotate/translate back to the original geometry
// 5) update cl.z if required and if CC lies within the edge 
//
// The edge test can be done in a "dual" geometry.
// instead of testing the original cutter against an infinitely thin edge 
// we can test a virtual CylCutter with radius VR against an infinite ER-radius cylinder around the edge.
// This reduces to a 2D problem in the XY plane, where section of the ER-radius cylinder is an ellipse.
// in the general case CL lies on an offset ellipse with offset OR.
// The general cases only applies for BullCutter(R1,R2) where R1 is the shaft radius
// and R2 is the corner radius. 
// For CylCutter and BallCutter there are simple analytic solutions and an offset ellipse approach is not required.
//
// The dual problem for each cutter type is:
// 
//      CylCutter(R): VR = R, ER = 0, OR=R  (the z-position of VR is the same as for CylCutter)
//     BallCutter(R): VR = 0, ER = R, OR=0 (the z-position of VR is a distance R up from the tip of BallCutter)
// BullCutter(R1,R2): VR=R1-R2, ER=R2, OR=R1-R2 (z-position of VR is a distance R2 up from the tip of BullCutter)
//
// cone: ??? (how is this an ellipse??)
//
// d is the distance from the p1-p2 line to cl, in the 2D XY plane
bool MillingCutter::singleEdgeDrop(CLPoint& cl, const Point& p1, const Point& p2, double d) const {    
    Point v = p2 - p1;          // vector along edge, from p1 -> p2
    Point vxy( v.x, v.y, 0.0);  // XY projection
    vxy.xyNormalize();          // normalized XY edge vector
    // figure out u-coordinates of p1 and p2 (i.e. x-coord in the rotated system)
    Point sc = cl.xyClosestPoint( p1, p2 );
    assert( ( (cl-sc).xyNorm() - d ) < 1E-6 );
    // edge endpoints in the new coordinate system, in these coordinates, CL is at origo
    Point u1( (p1-sc).dot(vxy) , d, p1.z); // d, the distance to line, is the y-coord in the rotated system
    Point u2( (p2-sc).dot(vxy) , d, p2.z);
    CC_CLZ_Pair contact = this->singleEdgeDropCanonical( u1, u2 ); // the subclass handles this
    CCPoint cc_tmp( sc + contact.first * vxy, EDGE); // translate back into original coord-system
    cc_tmp.z_projectOntoEdge(p1,p2);
    // update cl.z if required, and the cc-point lies inside the p1-p2 edge.
    return cl.liftZ_if_InsidePoints( contact.second , cc_tmp , p1, p2);
}

// general purpose vertexPush, delegates to this->width(h) 
bool MillingCutter::vertexPush(const Fiber& f, Interval& i, const Triangle& t) const {
    bool result = false;
    BOOST_FOREACH( const Point& p, t.p) {
        if (this->singleVertexPush(f,i,p, VERTEX) )
            result = true;
    }
    return result;
}

bool MillingCutter::singleVertexPush(const Fiber& f, Interval& i, const Point& p, CCType cctyp) const {
    bool result = false;
    if ( ( p.z >= f.p1.z ) && ( p.z <= (f.p1.z+ this->getLength()) ) ) { // p.z is within cutter
        Point pq = p.xyClosestPoint(f.p1, f.p2); // closest point on fiber
        double q = (p-pq).xyNorm(); // distance in XY-plane from fiber to p
        double h = p.z - f.p1.z;
        assert( h>= 0.0);
        double cwidth = this->width( h );
        if ( q <= cwidth ) { // we are going to hit the vertex p
            double ofs = sqrt( square( cwidth ) - square(q) ); // distance along fiber 
            Point start = pq - ofs*f.dir;
            Point stop  = pq + ofs*f.dir;
            CCPoint cc_tmp( p, cctyp );
            i.updateUpper( f.tval(stop) , cc_tmp );
            i.updateLower( f.tval(start) , cc_tmp );
            result = true;                
        }             
    }
    return result;
}

bool MillingCutter::facetPush(const Fiber& fib, Interval& i,  const Triangle& t) const {
    return generalFacetPush(this->normal_length,
                            this->center_height,
                            this->xy_normal_length,
                            fib,i,t);
}
    
// general purpose facetPush
bool MillingCutter::generalFacetPush(double normal_length,
                                     double center_height,
                                     double xy_normal_length,
                                     const Fiber& fib, 
                                     Interval& i,  
                                     const Triangle& t) 
                                     const {
    bool result = false;
    Point normal = t.upNormal(); // facet surface normal, pointing up 
    if ( normal.zParallel() ) // normal points in z-dir   
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
    double f = -t.p[0].z - normal_length*normal.z + fib.p1.z + center_height; 
    // note: the xy_normal does not have a z-component, so omitted here.
    
    double u, v; // u and v are coordinates of the cc-point within the triangle facet
    // a,b,e depend on the fiber:
    if ( fib.p1.y == fib.p2.y ) { // XFIBER
        a = t.p[1].y - t.p[0].y;
        b = t.p[2].y - t.p[0].y;
        e = -t.p[0].y - normal_length*normal.y - xy_normal_length*xy_normal.y + fib.p1.y;
        if (!two_by_two_solver(a,b,c,d,e,f,u,v))
            return result;
        CCPoint cc = t.p[0] + u*(t.p[1]-t.p[0]) + v*(t.p[2]-t.p[0]);
        cc.type = FACET;
        if ( ! cc.isInside( t ) ) 
            return result;
        // v0x + u*(v1x-v0x) + v*(v2x-v0x) + r2*nx + r1*xy_n.x = p1x + t*(p2x-p1x) 
        // =>
        // t = 1/(p2x-p1x) * ( v0x + r2*nx + r1*xy_n.x - p1x +  u*(v1x-v0x) + v*(v2x-v0x)       )
        assert( !isZero_tol( fib.p2.x - fib.p1.x )  ); // guard against division by zero
        double tval = (1.0/( fib.p2.x - fib.p1.x )) * ( t.p[0].x + normal_length*normal.x + xy_normal_length*xy_normal.x - fib.p1.x 
                                                        + u*(t.p[1].x-t.p[0].x)+v*(t.p[2].x-t.p[0].x) );
        if ( tval < 0.0 || tval > 1.0  ) {
            std::cout << "MillingCutter::facetPush() tval= " << tval << " error!?\n";
            //std::cout << " cutter: " << *this << "\n";
            std::cout << " triangle: " << t << "\n";
            std::cout << " fiber: " << fib << "\n";
        } 
        assert( tval > 0.0 && tval < 1.0 );
        i.update( tval, cc );
        result = true;
    } else if (fib.p1.x == fib.p2.x) { // YFIBER
        a = t.p[1].x - t.p[0].x;
        b = t.p[2].x - t.p[0].x;
        e = -t.p[0].x - normal_length*normal.x - xy_normal_length*xy_normal.x + fib.p1.x;
        if (!two_by_two_solver(a,b,c,d,e,f,u,v))
            return result;
        CCPoint cc = t.p[0] + u*(t.p[1]-t.p[0]) + v*(t.p[2]-t.p[0]);
        cc.type = FACET;
        if ( ! cc.isInside( t ) ) 
            return result;
        assert( !isZero_tol( fib.p2.y - fib.p1.y )  );
        double tval = (1.0/( fib.p2.y - fib.p1.y )) * ( t.p[0].y + normal_length*normal.y + xy_normal_length*xy_normal.y - fib.p1.y 
                                                        + u*(t.p[1].y-t.p[0].y)+v*(t.p[2].y-t.p[0].y) );
        if ( tval < 0.0 || tval > 1.0  ) {
            std::cout << "MillingCutter::facetPush() tval= " << tval << " error!?\n";
            std::cout << " (most probably a user error, the fiber is too short compared to the STL model?)\n";
        } 
        assert( tval > 0.0 && tval < 1.0 );
        i.update( tval, cc );
        result = true;    
    } else {
        assert(0);
    }
    
    return result;
}

bool MillingCutter::edgePush(const Fiber& f, Interval& i,  const Triangle& t) const {
    bool result = false;
    for (int n=0;n<3;n++) { // loop through all three edges
        int start=n;
        int end=(n+1)%3;
        const Point p1 = t.p[start]; // edge is from p1 to p2
        const Point p2 = t.p[end];
        if ( this->singleEdgePush(f,i,p1,p2))
            result = true;
    } 
    return result;
}

bool MillingCutter::singleEdgePush(const Fiber& f, Interval& i,  const Point& p1, const Point& p2) const {
    bool result = false;
    if ( this->horizEdgePush(f,i,p1,p2) )
        result = true;
    else {
        if ( this->shaftEdgePush(f,i,p1,p2) )
            result = true;
        if ( this->generalEdgePush(f,i,p1,p2) )
            result = true;
    }
    return result;
}

// this is used for the cylindrical shaft of Cyl, Ball, Bull, Cone
bool MillingCutter::shaftEdgePush(const Fiber& f, Interval& i,  const Point& p1, const Point& p2) const {
    // push cutter along Fiber f in contact with edge p1-p2
    // contact with cylindrical cutter shaft
    double u,v;
    bool result = false;
    if ( xy_line_line_intersection(p1, p2, u, f.p1, f.p2, v ) ) { // find XY-intersection btw fiber and edge
        Point q = p1 + u*(p2-p1); // edge/fiber intersection point, on edge
        // Point q = f.p1 + v*(f.p2-f.p1); // q on fiber
        // from q, go v_cc*xy_tangent, then r*xy_normal, and end up on fiber:
        // q + v_cc*tangent + r*xy_normal = p1 + t_cl*(p2-p1)
        Point xy_tang=p2-p1;
        xy_tang.z=0;
        xy_tang.xyNormalize();
        Point xy_normal = xy_tang.xyPerp();
        Point q1 = q  + radius*xy_normal;
        Point q2 = q1 + (p2-p1);
        double u_cc, t_cl;
        if ( xy_line_line_intersection( q1 , q2, u_cc, f.p1, f.p2, t_cl ) ) {
            double t_cl1 = t_cl;            // cc_tmp1 = q +/- u_cc*(p2-p1);
            double t_cl2 = v + (v-t_cl);
            if ( calcCCandUpdateInterval(t_cl1, u_cc, q, p1, p2, f, i, f.p1.z+center_height, EDGE_SHAFT) )
                result = true;
            if ( calcCCandUpdateInterval(t_cl2, -u_cc, q, p1, p2, f, i, f.p1.z+center_height, EDGE_SHAFT) )
                result = true;
        }
    }
    //std::cout << " shaftEdgePush = " << result << "\n";
    return result;
}

// this is the horizontal edge case
bool MillingCutter::horizEdgePush(const Fiber& f, Interval& i,  const Point& p1, const Point& p2) const {
    bool result=false;
    double h = p1.z - f.p1.z; // height of edge above fiber
    if ( (h > 0.0) ) {
        if ( isZero_tol( p2.z-p1.z ) ) { // this is the horizontal-edge special case
            double eff_radius = this->width( h ); // the cutter acts as a cylinder with eff_radius 
            // contact this cylinder/circle against edge in xy-plane
            double qt;      // fiber is f.p1 + qt*(f.p2-f.p1)
            double qv;      // line  is p1 + qv*(p2-p1)
            if (xy_line_line_intersection( p1 , p2, qv, f.p1, f.p2, qt ) ) {
                Point q = p1 + qv*(p2-p1); // the intersection point
                // from q, go v-units along tangent, then eff_r*normal, and end up on fiber:
                // q + ccv*tangent + r*normal = p1 + clt*(p2-p1)
                double ccv, clt;
                Point xy_tang=p2-p1;
                xy_tang.z=0;
                xy_tang.xyNormalize();
                Point xy_normal = xy_tang.xyPerp();
                Point q1 = q+eff_radius*xy_normal;
                Point q2 = q1+(p2-p1);
                if ( xy_line_line_intersection( q1 , q2, ccv, f.p1, f.p2, clt ) ) {
                    double t_cl1 = clt;
                    double t_cl2 = qt + (qt - clt );
                    if ( calcCCandUpdateInterval(t_cl1, ccv, q, p1, p2, f, i, f.p1.z, EDGE_HORIZ) )
                        result = true;
                    if ( calcCCandUpdateInterval(t_cl2, -ccv, q, p1, p2, f, i, f.p1.z, EDGE_HORIZ) )
                        result = true;
                }
            }
        }
    }
    //std::cout << " horizEdgePush = " << result << "\n";
    return result;
}

bool MillingCutter::calcCCandUpdateInterval( double t, double u, const Point& q, const Point& p1, const Point& p2, 
                                             const Fiber& f, Interval& i, double height, CCType cctyp) const {
    CCPoint cc_tmp = q+u*(p2-p1);
    cc_tmp.type = cctyp;
    return i.update_ifCCinEdgeAndTrue( t, cc_tmp, p1, p2, (cc_tmp.z >= height)  );
} 

bool MillingCutter::pushCutter(const Fiber& f, Interval& i, const Triangle& t) const {
    bool v = vertexPush(f,i,t); 
    bool fa = facetPush(f,i,t);
    bool e = edgePush(f,i,t);
    return v || fa || e;
}


// call vertex, facet, and edge drop methods on input Triangle t
bool MillingCutter::dropCutter(CLPoint &cl, const Triangle &t) const {
    bool facet(false), vertex(false), edge(false);
    /* // alternative ordering of the tests:
    if (cl.below(t))
        vertexDrop(cl,t);
        
    // optimisation: if we are now above the triangle we don't need facet and edge
    if ( cl.below(t) ) {
        facetDrop(cl,t); 
        edgeDrop(cl,t);
    }*/
    
    if (cl.below(t)) {
        facet = facetDrop(cl,t); // if we make contact with the facet...
        if (!facet) {            // ...then we will not hit an edge/vertex, so don't check for that
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
