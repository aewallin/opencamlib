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

#include "point.h"
#include "triangle.h"
#include "millingcutter.h"
#include "numeric.h"
#include "fiber.h"

// #define EDGEDROP_DEBUG

namespace ocl
{

//********   CylCutter ********************** */
CylCutter::CylCutter() {
    setDiameter(1.0);
}

CylCutter::CylCutter(const double d) {
    setDiameter(d);
}

//********   drop-cutter methods ********************** */
int CylCutter::vertexDrop(CLPoint &cl, const Triangle &t) const
{
    /// loop through each vertex p of Triangle t
    /// drop down cutter at (cl.x, cl.y) against Point p
    int result = 0;
    CCPoint* cc_tmp;
    BOOST_FOREACH( const Point& p, t.p) {
        double q = cl.xyDistance(p); // distance in XY-plane from cl to p
        if (q<= radius) { // p is inside the cutter
            cc_tmp = new CCPoint(p);
            if (cl.liftZ(p.z)) { // we need to lift the cutter
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

int CylCutter::facetDrop(CLPoint &cl, const Triangle &t) const
{
    // Drop cutter at (cl.x, cl.y) against facet of Triangle t
    Point normal; // facet surface normal
    
    if ( isZero_tol( t.n->z ) )  { // vertical surface
        return -1;  // can't drop against vertical surface
    } else if (t.n->z < 0) {  // normal is pointing down
        normal = -1* (*t.n); // flip normal
    } else {
        normal = *t.n;
    }
    
    assert( isPositive( normal.z ) ); // we are in trouble if n.z is not positive by now...
    
    // define plane containing facet
    // a*x + b*y + c*z + d = 0, so
    // d = -a*x - b*y - c*z, where
    // (a,b,c) = surface normal
    double a = normal.x;
    double b = normal.y;
    double c = normal.z;
    //double d = - a * t.p[0].x - b * t.p[0].y - c * t.p[0].z;
    double d = - normal.dot(t.p[0]);
        
    normal.xyNormalize(); // make length of normal in xy plane == 1.0
    
    // the contact point with the plane is on the periphery
    // of the cutter, a length radius from cl in the direction of -n
    CCPoint* cc_tmp = new CCPoint();
    *cc_tmp = cl - (radius)*normal; // Note: at this point the z-coord is rubbish.
    cc_tmp->z = (1.0/c)*(-d-a*cc_tmp->x-b*cc_tmp->y); // NOTE: potential for divide-by-zero (?!)
    
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
    // Drop cutter at (p.x, p.y) against edges of Triangle t
    // strategy:
    // 1) calculate distance to infinite line
    // 2) calculate intersection points w. cutter
    // 3) pick the higher intersection point and test if it is in the edge
    int result = 0;
    
    for (int n=0;n<3;n++) { // loop through all three edges
        // 1) distance from point to line
        int start=n;
        int end=(n+1)%3;
        #ifdef EDGEDROP_DEBUG
            std::cout << "testing points " << start<< " to " << end << " :";
        #endif 
        Point p1 = t.p[start];
        Point p2 = t.p[end];
        
        // check that there is an edge in the xy-plane
        // can't drop against vertical edges!
        if ( !isZero_tol( p1.x - p2.x ) || !isZero_tol( p1.y - p2.y) ) {
            
            double d = cl.xyDistanceToLine(t.p[start],t.p[end]);
            #ifdef EDGEDROP_DEBUG
                std::cout << "xyDistance=" << d ;
            #endif
            
            if (d<=radius) { // potential hit
                //std::cout << " potential hit\n";
                // 2) calculate intersection points with cutter circle.
                // points are on line and diameter/2 from cl.
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
                #ifdef EDGEDROP_DEBUG
                    std::cout << "discr=" << discr << "\n";
                #endif
                
                if ( !isZero_tol(discr) && isNegative(discr) ) {
                    std::cout << "cutter.cpp ERROR: CylCutter::edgeTest discr= "<<discr<<" <0 !!\n";
                    assert(0);
                    return 0;
                    
                } else if ( isZero_tol(discr) ) {// tangent line
                    #ifdef EDGEDROP_DEBUG
                        std::cout << "discr= zero, tangent case.\n";
                    #endif
                    CCPoint* cc_tmp = new CCPoint();
                    cc_tmp->x =  D*dy / dr_sq + cl.x; // translate back to cl
                    cc_tmp->y = -D*dx / dr_sq + cl.y;
                    
                    
                    // 3) check if cc is in edge
                    if ( cc_tmp->isInsidePoints(t.p[start], t.p[end]) ) { 
                        // determine height of point. must be on line, so:
                        // std::cout << "tangent-case: isInside=true!\n";
                        // two point formula for line:
                        // z-z1 = ((z2-z1)/(x2-x1)) * (x - x1)
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
                            assert(0); // trouble.

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
                    #ifdef EDGEDROP_DEBUG
                        std::cout << "discr>0, two intersections\n";
                    #endif
                    CCPoint* cc1 = new CCPoint();
                    CCPoint* cc2 = new CCPoint();
                    double sqrt_discr = sqrt(discr);
                    // remember to translate back to cl
                    cc1->x= ( D*dy + sign(dy)*dx*sqrt_discr) / dr_sq + cl.x; 
                    cc1->y= (-D*dx + fabs(dy)*sqrt_discr   ) / dr_sq + cl.y;
                    cc1->z=0;
                    cc2->x= ( D*dy - sign(dy)*dx*sqrt_discr) / dr_sq + cl.x;
                    cc2->y= (-D*dx - fabs(dy)*sqrt_discr   ) / dr_sq + cl.y;
                    cc2->z=0;
                    #ifdef EDGEDROP_DEBUG
                        std::cout << "cc1= " << *cc1 << "\n";
                        std::cout << "cc2= " << *cc2 << "\n";
                    #endif
                    // 3) check if in edge

                    double x1 = t.p[start].x;
                    double x2 = t.p[end].x;
                    double y1 = t.p[start].y;
                    double y2 = t.p[end].y;
                    double z1 = t.p[start].z;
                    double z2 = t.p[end].z;
                    
                    if ( cc1->isInsidePoints(t.p[start], t.p[end]) ) {
                        // determine height of point. must be on line, so:
                        if (  fabs(x1 - x2) > fabs(y1 - y2)   )   // can compute using x-coords
                            cc1->z = z1 + ((z2-z1)/(x2-x1)) * (cc1->x-x1);
                        else if ( !isZero_tol( fabs(y1 - y2) ) ) // must compute using y-coords
                            cc1->z = z1 + ((z2-z1)/(y2-y1)) * (cc1->y-y1);
                        else { // we are in trouble.
                            std::cout << "cyclutter edge-test, unable to compute cc-point. stop.\n";
                            assert(0);
                        }
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
                        else {// we are in trouble.
                            std::cout << "cyclutter edge-test, unable to compute cc-point. stop.\n";
                            assert(0);
                        }
                        
                        
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
//************** push cutter methods **********************************/
//************** push cutter methods **********************************/
//************** push cutter methods **********************************/


/// push cutter along Fiber against vertices of Triangle t
/// add interfering intervals to the Fiber
int CylCutter::vertexPush(Fiber& f, Interval& i, const Triangle& t) const {
    int result = 0;
    BOOST_FOREACH( const Point& p, t.p)
    {
        //std::cout << "clength=" << getLength() << "\n";
        //std::cout << "zmax=" << f.p1.z+getLength()<< "\n";
        //std::cout << "p.z=" << p.z << "\n";
        if ( ( p.z >= f.p1.z ) && ( p.z <= (f.p1.z+getLength()) ) ) { // p.z is within cutter
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
                result = 1;
                //f.addInt(f.tval(start) , f.tval(stop)) ;
                //    i.lower_cc = p;
                //    i.lower_cc.type = VERTEX;
                //    result=1;
                
            }             
        }
    }
    return result;
}

/// push cutter along Fiber against facet of Triangle t
/// Update Interval i where the cutter interferes
int CylCutter::facetPush(Fiber& f, Interval& i,  const Triangle& t) const {
    int result = 0;
    Point normal; // facet surface normal 
    if ( t.n->zParallel() ) { // normal points in z-dir   
        return result; //can't push against horizontal plane, stop here.
    }
    else if (t.n->z < 0) {  // normal is pointing down
        normal = -1* (*t.n); // flip normal
    } else {
        normal = *t.n;
    }
    assert( isPositive( normal.z ) ); // we are in trouble if n.z is not positive by now...
    
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
    
    namespace bnu = boost::numeric::ublas;
    bnu::matrix<double> A(4,4);
    bnu::matrix<double> B(4,4);
    
    A(0,0) = 1.0;
    A(0,1) = 1.0;
    A(0,2) = 1.0;
    A(0,3) = 1.0;
    A(1,0) = t.p[0].x; // xvals
    A(1,1) = t.p[1].x;
    A(1,2) = t.p[2].x;
    A(1,3) = f.p1.x;
    A(2,0) = t.p[0].y; // yvals
    A(2,1) = t.p[1].y;
    A(2,2) = t.p[2].y;
    A(2,3) = f.p1.y;
    A(3,0) = t.p[0].z; // zvals
    A(3,1) = t.p[1].z;
    A(3,2) = t.p[2].z;
    A(3,3) = f.p1.z;    

    B(0,0) = 1.0;
    B(0,1) = 1.0;
    B(0,2) = 1.0;
    B(0,3) = 0.0;
    B(1,0) = t.p[0].x; // xvals
    B(1,1) = t.p[1].x;
    B(1,2) = t.p[2].x;
    B(1,3) = f.p2.x - f.p1.x;
    B(2,0) = t.p[0].y; // yvals
    B(2,1) = t.p[1].y;
    B(2,2) = t.p[2].y;
    B(2,3) = f.p2.y - f.p1.y;
    B(3,0) = t.p[0].z; // zvals
    B(3,1) = t.p[1].z;
    B(3,2) = t.p[2].z;
    B(3,3) = f.p2.z - f.p1.z;      
    
    //std::cout << A << std::endl;
    
    double detB = determinant(B);
    if ( isZero_tol( detB ) )
        return result; // no intersection btw. line and plane
        
    double detA = determinant(A);
    double q_tval = - detA/detB;
    // std::cout << " tval = " << tval << std::endl;
    Point q = f.p1 + q_tval*(f.p2 - f.p1); // the intersection point.
    
    // compute cc-point from intersection
    // from ip, go a distance radius in the direction of the xy-normal
    normal.z = 0.0;
    normal.xyNormalize();          // normalized normal
    Point tangent = normal.xyPerp();  // normalized tangent
    
    
    // solve linear system
    // using namespace boost::ublas;
    // Ainv = identity_matrix<float>(A.size1());
    // permutation_matrix<size_t> pm(A.size1());
    // lu_factorize(A,pm)
    // lu_substitute(A, pm, Ainv);

    // in the XY plane we move a distance v along the tangent
    // and then a distance r along the normal
    // we should end up on the fiber
    // q + t*tangent + r*normal = p1 + t*(p2-p1)
    // this leads to a matrix equation:
    //
    // [ tang.x  -(p2-p1).x ] [ v ]  = [ (q+r*n-p1).x ]
    // [ tang.y  -(p2-p1).y ] [ t ]  = [ (q+r*n-p1).y ]
    // or
    // Mx=y
    bnu::matrix<double> M(2,2);
    M(0,0) = tangent.x;
    M(0,1) = -(f.p2.x - f.p1.x);
    M(1,0) = tangent.y;
    M(1,1) = -(f.p2.y - f.p1.y);
    bnu::vector<double> y(2);
    y(0) = -q.x - radius*normal.x + f.p1.x;
    y(1) = -q.y - radius*normal.y + f.p1.y;
    bnu::matrix<double> Minv(M.size1(),M.size2());
    Minv = bnu::identity_matrix<double>(M.size1());
    bnu::permutation_matrix<size_t> pm(M.size1());
    bnu::lu_factorize(M,pm);
    bnu::lu_substitute( M, pm, Minv );
    bnu::vector<double> x(2);
    x = bnu::prod( Minv, y );
    // there are two solutions
    double cc_vval1 = x(0);
    double cl_tval1 = x(1);
    CCPoint cc_tmp1 = q + cc_vval1*tangent ;  
    cc_tmp1.type = FACET;
    
    double cc_vval2 = -x(0);
    double cl_tval2 = q_tval + (q_tval-cl_tval1);
    CCPoint cc_tmp2 = q + cc_vval2*tangent ;  
    cc_tmp2.type = FACET;
    
    // check if cc-point is in the facet of the Triangle
    if( cc_tmp1.isInside( t ) ) {
        i.updateUpper( cl_tval1  , cc_tmp1 );
        i.updateLower( cl_tval1  , cc_tmp1 );
        result = 1;
    }
    if( cc_tmp2.isInside( t ) ) {
        i.updateUpper( cl_tval2  , cc_tmp2 );
        i.updateLower( cl_tval2  , cc_tmp2 );
        result = 1;
    }

    return result;
}    

int CylCutter::edgePush(Fiber& f, Interval& i,  const Triangle& t) const {
    int result = 0;
    //FIXME: nothing here yet
    
    
    return result;
}   

MillingCutter* CylCutter::offsetCutter(double d) const {
    return new BullCutter(diameter+2*d, d) ;
}
    
//********  CylCutter string output ********************** */
std::string CylCutter::str() const
{
    std::ostringstream o;
    o << *this;
    return o.str();
}

std::ostream& operator<<(std::ostream &stream, CylCutter c)
{
  stream << "CylCutter (d=" << c.diameter << ")";
  return stream;
}

} // end namespace
// end file cylcutter.cpp
