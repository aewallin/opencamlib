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

#include "cylcutter.h"
#include "bullcutter.h" // for offsetCutter()

namespace ocl
{

//********   CylCutter ********************** */
CylCutter::CylCutter() {
    setDiameter(1.0);
}

CylCutter::CylCutter(const double d) {
    if (d>0.0)
        setDiameter(d);
}

/// offset of CylCutter is BullCutter
MillingCutter* CylCutter::offsetCutter(const double d) const {
    return new BullCutter(diameter+2*d, d) ;
}

double CylCutter::height(const double r) const {
    if ( r <= radius )
        return 0.0;
    else {
        assert(0);
        return -1;
    }
}

//********   drop-cutter methods ********************** */
/*
int CylCutter::vertexDrop(CLPoint &cl, const Triangle &t) const {
    int result = 0;
    CCPoint* cc_tmp;
    BOOST_FOREACH( const Point& p, t.p) {
        double q = cl.xyDistance(p); // distance in XY-plane from cl to p
        if (q<= radius) { // p is inside the cutter
            cc_tmp = new CCPoint(p);
            if (cl.liftZ(p.z - this->height(q) )) { // we need to lift the cutter
                cc_tmp->type = VERTEX;
                cl.cc = cc_tmp;
                result = 1;
            } else {
                delete cc_tmp;
            }
        } 
    }
    return result;
}*/

int CylCutter::facetDrop(CLPoint &cl, const Triangle &t) const {
    Point normal; // facet surface normal
    if ( isZero_tol( t.n->z ) )  { // vertical surface
        return -1;  // can't drop against vertical surface
    } else if (t.n->z < 0) {  // normal is pointing down
        normal = -1* (*t.n); // flip normal
    } else {
        normal = *t.n;
    }
    assert(  normal.z > 0.0 ); // we are in trouble if n.z is not positive by now...
    // define plane containing facet
    // a*x + b*y + c*z + d = 0, so
    // d = -a*x - b*y - c*z, where
    // (a,b,c) = surface normal
    double a = normal.x;
    double b = normal.y;
    double c = normal.z;
    double d = - normal.dot(t.p[0]); //double d = - a * t.p[0].x - b * t.p[0].y - c * t.p[0].z;
    normal.xyNormalize(); // make length of normal in xy plane == 1.0
    // the contact point with the plane is on the periphery
    // of the cutter, a length radius from cl in the direction of -normal
    CCPoint* cc_tmp = new CCPoint();
    *cc_tmp = cl - (radius)*normal; // NOTE: at this point the z-coord is rubbish.
    cc_tmp->z = (1.0/c)*(-d-a*cc_tmp->x-b*cc_tmp->y); // locate on the plane
    if (cc_tmp->isInside(t)) { 
        if (cl.liftZ(cc_tmp->z)) {
            cc_tmp->type = FACET;
            cl.cc = cc_tmp;
            return 1;
        } else {
            delete cc_tmp;
        }
    } else {
        delete cc_tmp;
        return 0;
    }
    return 0;
}

int CylCutter::edgeDrop(CLPoint &cl, const Triangle &t) const {
    // strategy:
    // 1) calculate distance to infinite line
    // 2) calculate intersection points w. cutter (xy plane?)
    // 3) pick the higher intersection point and test if it is in the edge
    int result = 0;
    for (int n=0;n<3;n++) { // loop through all three edges
        int start=n;
        int end=(n+1)%3;
        Point p1 = t.p[start];
        Point p2 = t.p[end];
        // check that there is an edge in the xy-plane
        // can't drop against vertical edges!
        if ( !isZero_tol( p1.x - p2.x ) || !isZero_tol( p1.y - p2.y) ) {
            double d = cl.xyDistanceToLine(t.p[start],t.p[end]);// 1) distance from point to line
            if (d<=radius) { // potential hit
                // 2) calculate intersection points with cutter circle.
                // points are on line and radius from cl.
                // see http://mathworld.wolfram.com/Circle-LineIntersection.html
                double x1 = t.p[start].x - cl.x; // translate to cl=(0,0)
                double y1 = t.p[start].y - cl.y;
                double x2 = t.p[end].x - cl.x;
                double y2 = t.p[end].y - cl.y;
                double dx = x2-x1;
                double dy = y2-y1;
                double dr_sq = dx*dx + dy*dy;
                double dr = sqrt( dr_sq );
                double D = x1*y2 - x2*y1;
                double discr = square( radius ) * square(dr) - square(D);
                if ( !isZero_tol(discr) && isNegative(discr) ) {
                    std::cout << "cutter.cpp ERROR: CylCutter::edgeTest discr= "<<discr<<" <0 !!\n";
                    assert(0);
                    return 0;
                } else if ( isZero_tol(discr) ) {// discr==0.0 means line is tangent to cutter circle
                    CCPoint* cc_tmp = new CCPoint();
                    cc_tmp->x =  D*dy / dr_sq + cl.x; // translate back to cl
                    cc_tmp->y = -D*dx / dr_sq + cl.y;
                    // 3) check if cc is in edge
                    if ( cc_tmp->isInsidePoints(t.p[start], t.p[end]) ) { 
                        // determine height of point. must be on line, so:
                        // z-z1 = ((z2-z1)/(x2-x1)) * (x - x1)  // two point formula for line:
                        // z = z1 + ((z2-z1)/(x2-x1)) * (x-x1)
                        double z1 = t.p[start].z;
                        double z2 = t.p[end].z;
                        double x1 = t.p[start].x;
                        double x2 = t.p[end].x;
                        double y1 = t.p[start].y;
                        double y2 = t.p[end].y;
                        // use either x-coord or y-coord to calculate z-height
                        if ( fabs(x1 - x2) > fabs(y2 - y1) ) 
                            cc_tmp->z = z1 + ((z2-z1)/(x2-x1)) * (cc_tmp->x-x1);
                        else if ( !isZero_tol( y2-y1) ) // guard against division by zero
                            cc_tmp->z = z1 + ((z2-z1)/(y2-y1)) * (cc_tmp->y-y1);
                        else 
                            assert_msg(0, "CylCutter::edgeDrop(), tangent case, cannot compute cc_tmp.z"); // trouble.

                        if ( cl.liftZ(cc_tmp->z) ) {
                            cc_tmp->type = EDGE;
                            cl.cc = cc_tmp;
                            result = 1;
                        } else {
                            delete cc_tmp;
                        }
                    } else {
                            delete cc_tmp;
                    }
                } else { // discr > 0, two intersection points
                    assert( discr > 0.0 );
                    CCPoint* cc1 = new CCPoint();
                    CCPoint* cc2 = new CCPoint();
                    double sqrt_discr = sqrt(discr);
                    cc1->x= ( D*dy + sign(dy)*dx*sqrt_discr) / dr_sq + cl.x; // remember to translate back to cl
                    cc1->y= (-D*dx + fabs(dy)*sqrt_discr   ) / dr_sq + cl.y;
                    cc1->z=0;
                    cc2->x= ( D*dy - sign(dy)*dx*sqrt_discr) / dr_sq + cl.x;
                    cc2->y= (-D*dx - fabs(dy)*sqrt_discr   ) / dr_sq + cl.y;
                    cc2->z=0;
                    double x1 = t.p[start].x;
                    double x2 = t.p[end].x;
                    double y1 = t.p[start].y;
                    double y2 = t.p[end].y;
                    double z1 = t.p[start].z;
                    double z2 = t.p[end].z;
                    if ( cc1->isInsidePoints(t.p[start], t.p[end]) ) { // 3) check if in edge
                        // determine height of point. must be on line, so:
                        if (  fabs(x1 - x2) > fabs(y1 - y2)   )   //  compute using x-coords
                            cc1->z = z1 + ((z2-z1)/(x2-x1)) * (cc1->x-x1);
                        else if ( !isZero_tol( fabs(y1 - y2) ) ) //  compute using y-coords
                            cc1->z = z1 + ((z2-z1)/(y2-y1)) * (cc1->y-y1);
                        else  // we are in trouble.
                            assert_msg( 0, "CylCutter::edgeDrop(), general case, unable to compute cc1.z. stop.\n");
                        
                        if (cl.liftZ(cc1->z)) {
                            cc1->type = EDGE;
                            cl.cc = cc1;
                            result = 1;
                        } else {
                            delete cc1;
                        }
                    } else {
                        delete cc1;
                    }
                    if ( cc2->isInsidePoints(t.p[start], t.p[end]) ) {
                        if ( fabs(x1 - x2) > fabs(y1 - y2)  )  // determine z- height of cc point
                            cc2->z = z1 + ((z2-z1)/(x2-x1)) * (cc2->x-x1);
                        else if ( !isZero_tol( fabs(y1 - y2) )  ) 
                            cc2->z = z1 + ((z2-z1)/(y2-y1)) * (cc2->y-y1);
                        else // we are in trouble.
                            assert_msg(0, "cyclutter edge-test, unable to compute cc-point. stop.\n");
                        
                        if (cl.liftZ(cc2->z)) {     
                            cc2->type = EDGE;
                            cl.cc = cc2;                     
                            result=1;
                        } else {
                            delete cc2;
                        }
                    } else { // end cc2.isInside()
                        delete cc2;
                    }
                } //end two intersection points case
            }// end if(potential hit)
        } // end if(vertical edge)
    } // end loop through all edges
    return result;
}

//************** push cutter methods **********************************/

/// push cutter along Fiber against vertices of Triangle t
/// update Interval i 
bool CylCutter::vertexPush(const Fiber& f, Interval& i, const Triangle& t) const {
    bool result = false;
    std::vector<Point> verts;
    verts.push_back(t.p[0]);
    verts.push_back(t.p[1]);
    verts.push_back(t.p[2]);
    // if the triangle is sliced, we have two more verts to test:
    Point p1,p2;
    if ( t.zslice_verts(p1, p2, f.p1.z) ) {
        p1.z = p1.z + 1E-3; // dirty trick...
        p2.z = p2.z + 1E-3; // ...which will not affect results, unless cutter.length < 1E-3
        verts.push_back(p1);
        verts.push_back(p2);
    }
    BOOST_FOREACH( const Point& p, verts) {
        if ( (p.z >= f.p1.z) && (p.z <= (f.p1.z+getLength())) ) { // p.z is within cutter
            Point pq = p.xyClosestPoint(f.p1, f.p2);
            double q = (p-pq).xyNorm(); // distance in XY-plane from fiber to p
            if ( q <= radius ) { // p is inside the cutter
                double ofs = sqrt( square(radius) - square(q) ); // distance along fiber 
                Point start = pq - ofs*f.dir;
                Point stop  = pq + ofs*f.dir;
                CCPoint cc_tmp = CCPoint(p);
                cc_tmp.type = VERTEX;
                //std::cout << "updating with " << f.tval(stop) << " to " << f.tval(start) << "\n";
                i.updateUpper( f.tval(stop) , cc_tmp );
                i.updateLower( f.tval(start) , cc_tmp );
                result = true;                
            }             
        }
    }
    return result;
}

/// push cutter along Fiber against facet of Triangle t
/// Update Interval i where the cutter interferes
bool CylCutter::facetPush(const Fiber& fib, Interval& intv,  const Triangle& t) const {
    bool result = false;
    Point normal; // facet surface normal 
    if ( t.n->zParallel() ) { // normal points in z-dir   
        return result; //can't push against horizontal plane, stop here.
    }
    else if (t.n->z < 0) {  // normal is pointing down
        normal = -1* (*t.n); // flip normal
    } else {
        normal = *t.n;
    }

    // find the intersection of the Fiber and the plane
    // http://mathworld.wolfram.com/Line-PlaneIntersection.html
    // plane defined by p1, p2, p3
    // line through p4, p5
    // intersection on line = p4 + t*(p5-p4)
    // and
    // t = - det(A) / det(B)
    // where A =
    // 1  1  1  1
    // x1 x2 x3 x4
    // y1 y2 y3 y4
    // z1 z2 z3 z4
    // and B =
    // 1  1  1  0
    // x1 x2 x3 x5-x4
    // y1 y2 y3 y5-y4
    // z1 z2 z3 z5-z4
    //
    //  if det(B)==0 there is no intersection
    
    // 1  1  1  1
    // x1 x2 x3 x4
    // y1 y2 y3 y4
    // z1 z2 z3 z4
    // =
    // +1 * ( x2(y3*z4-z3*y4)-x3(y2*z4-z2*y4)+x4(y2*z3-z2*y3) )
    // -1 * (-x1(y3*z4-z3*z4)
    
    // a b c d   
    // e f g h 
    // i j k l 
    // m n o p 
    //
    //    [ f g h ]     [ e g h ]     [ e f h ]    [ e f g ]
    // +a [ j k l ]  -b [ i k l ]  +c [ i j l ] -d [ i j k ]
    //    [ n o p ]     [ m o p ]     [ m n p ]    [ m n o ]
    //
    //
    // a*(f*(k*p-o*l)-g*(j*p-n*l)+h*(j*o-n*k))
    //-b*(e*(k*p-o*l)-g*(i*p-m*l)+h*(i*o-m*k))
    //+c*(e*(j*p-n*l)-f*(i*p-m*l)+h*(i*n-m*j))
    //-d*(e*(j*o-n*k)-f*(i*o-m*k)+g*(i*n-m*j))
    double a = 1.0;
    double b = 1.0;
    double c = 1.0;
    double d = 1.0;
    double e = t.p[0].x; // xvals
    double f = t.p[1].x;
    double g = t.p[2].x;
    double h = fib.p1.x;
    double i = t.p[0].y; // yvals
    double j = t.p[1].y;
    double k = t.p[2].y;
    double l = fib.p1.y;
    double m = t.p[0].z; // zvals
    double n = t.p[1].z;
    double o = t.p[2].z;
    double p = fib.p1.z;  
    double detA = +a*(f*(k*p-o*l)-g*(j*p-n*l)+h*(j*o-n*k))
                  -b*(e*(k*p-o*l)-g*(i*p-m*l)+h*(i*o-m*k))
                  +c*(e*(j*p-n*l)-f*(i*p-m*l)+h*(i*n-m*j))
                  -d*(e*(j*o-n*k)-f*(i*o-m*k)+g*(i*n-m*j));
    a = 1.0;
    b = 1.0;
    c = 1.0;
    d = 0.0;
    e = t.p[0].x; // xvals
    f = t.p[1].x;
    g = t.p[2].x;
    h = fib.p2.x - fib.p1.x;
    i = t.p[0].y; // yvals
    j = t.p[1].y;
    k = t.p[2].y;
    l = fib.p2.y - fib.p1.y;
    m = t.p[0].z; // zvals
    n = t.p[1].z;
    o = t.p[2].z;
    p = fib.p2.z - fib.p1.z;    
    double detB = +a*(f*(k*p-o*l)-g*(j*p-n*l)+h*(j*o-n*k))
                  -b*(e*(k*p-o*l)-g*(i*p-m*l)+h*(i*o-m*k))
                  +c*(e*(j*p-n*l)-f*(i*p-m*l)+h*(i*n-m*j))
                  -d*(e*(j*o-n*k)-f*(i*o-m*k)+g*(i*n-m*j));

    if ( isZero_tol( detB ) )
        return result; // no intersection btw. line and plane
        

    double q_tval = - detA/detB;
    Point q = fib.p1 + q_tval*(fib.p2 - fib.p1); // the intersection point.
    
    // compute cc-point from intersection
    normal.z = 0.0;
    normal.xyNormalize();          // normalized normal
    Point tangent = normal.xyPerp();  // normalized tangent
    // in the XY plane we move a distance v along the tangent
    // and then a distance r along the normal
    // we should end up on the fiber
    // q + t*tangent + r*normal = p1 + t*(p2-p1)
    // from q, go v-units along tangent, then r*normal, and end up on fiber:
    // q + v*tangent + r*normal = p1 + t*(p2-p1)
    // in matrix form:
    // [ tang.x  -(p2-p1).x ] [ v ]  = [ (-q.x -r*n.x +p1.x ) ]
    // [ tang.y  -(p2-p1).y ] [ t ]  = [ (-q.y -r*n.y +p1.y ) ]

    double v_cc, t_cl;
    Point q1 = q+radius*normal;
    Point q2 = q1+tangent;
    if ( !xy_line_line_intersection( q1 , q2, v_cc, fib.p1, fib.p2, t_cl ) )
        assert(0); 
    
    // there are two solutions
    double cc_vval1 = v_cc;
    double cl_tval1 = t_cl;
    CCPoint cc_tmp1 = q + cc_vval1*tangent ;  
    cc_tmp1.type = FACET;
    
    double cc_vval2 = -v_cc;
    double cl_tval2 = q_tval + (q_tval-cl_tval1);
    CCPoint cc_tmp2 = q + cc_vval2*tangent ;  
    cc_tmp2.type = FACET;
    
    // check if cc-point is in the facet of the Triangle
    if( cc_tmp1.isInside( t ) ) {
        intv.updateUpper( cl_tval1  , cc_tmp1 );
        intv.updateLower( cl_tval1  , cc_tmp1 );
        result = true;
    }
    if( cc_tmp2.isInside( t ) ) {
        intv.updateUpper( cl_tval2  , cc_tmp2 );
        intv.updateLower( cl_tval2  , cc_tmp2 );
        result = true;
    }
    return result;
}

bool CylCutter::edgePush(const Fiber& f, Interval& i,  const Triangle& t) const {
    bool result = false;
    for (int n=0;n<3;n++) { // loop through all three edges
        int start=n;
        int end=(n+1)%3;
        Point p1 = t.p[start];
        Point p2 = t.p[end];
        // check that there is an edge in the xy-plane
        // can't push against vertical edges ??
        if ( !isZero_tol( p1.x - p2.x ) || !isZero_tol( p1.y - p2.y) ) {
            // find XY-intersection btw fiber and edge
            // fiber is f.p1 + t*(f.p2-f.p1)
            // line  is p1 + v*(p2-p1)
            double tq, v;
            if ( xy_line_line_intersection(p1, p2, v, f.p1, f.p2, tq ) ){
                Point q = p1 + v*(p2-p1); // intersection point, on edge
                // Point q = f.p1 + tq*(f.p2-f.p1);
                // from q, go v-units along tangent, then r*normal, and end up on fiber:
                // q + t*tangent + r*normal = p1 + t*(p2-p1)
                double v_cc, t_cl;
                Point xy_tang=p2-p1;
                xy_tang.z=0;
                xy_tang.xyNormalize();
                Point xy_normal = xy_tang.xyPerp();
                Point q1 = q+radius*xy_normal;
                Point q2 = q1+(p2-p1);
                if ( xy_line_line_intersection( q1 , q2, v_cc, f.p1, f.p2, t_cl ) ) {
                    double t_cl1 = t_cl;
                    double t_cl2 = tq + (tq-t_cl );
                    CCPoint cc_tmp1 = q+v_cc*(p2-p1);
                    CCPoint cc_tmp2 = q-v_cc*(p2-p1); 
                    cc_tmp1.type = EDGE;
                    cc_tmp2.type = EDGE;
                    if( cc_tmp1.isInsidePoints(p1,p2) && (cc_tmp1.z >= f.p1.z) ) {
                        i.updateUpper( t_cl1  , cc_tmp1 );
                        i.updateLower( t_cl1  , cc_tmp1 );
                        result = true;
                    }
                    if( cc_tmp2.isInsidePoints( p1,p2 ) && (cc_tmp2.z >= f.p1.z) ) {
                        i.updateUpper( t_cl2  , cc_tmp2 );
                        i.updateLower( t_cl2  , cc_tmp2 );
                        result = true;
                    }
                }
            } // end if(fiber and edge intersect)
        } // end if(vertical edge)
    } // end loop through all edges
    return result;
}   

//********  CylCutter string output ********************** */
std::string CylCutter::str() const {
    std::ostringstream o;
    o << *this;
    return o.str();
}

std::ostream& operator<<(std::ostream &stream, CylCutter c) {
  stream << "CylCutter (d=" << c.diameter << ")";
  return stream;
}

} // end namespace
// end file cylcutter.cpp
