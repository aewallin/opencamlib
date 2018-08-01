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
#include <sstream>
#include <cmath>
#include <string>

#include <cassert>


#include "ellipse.hpp"
#include "numeric.hpp"
#include "brent_zero.hpp"
#include "fiber.hpp"

namespace ocl
{


//********   Ellipse ********************** */
Ellipse::Ellipse(Point& centerin, double ain, double bin, double offsetin) {
    center = centerin;
    a = ain;
    b = bin;
    assert( b > 0.0 );
    eccen = a/b;
    offset = offsetin;
    target = Point(0,0,0);
}

AlignedEllipse::AlignedEllipse(Point& centerin, double ain, double bin, double offsetin, Point& major, Point& minor) {
    center = centerin;
    a = ain; // major axis of ellipse
    b = bin; // minor axis of ellipse
    assert( b > 0.0 );
    eccen = a/b;
    offset = offsetin;
    major_dir=major;
    minor_dir=minor;
}

Point Ellipse::ePoint1() const {
    return this->ePoint(EllipsePosition1);
}

Point Ellipse::ePoint2() const {
    return this->ePoint(EllipsePosition2);
}

Point Ellipse::oePoint1() const {
    return this->oePoint(EllipsePosition1);
}

Point Ellipse::oePoint2() const {
    return this->oePoint(EllipsePosition2);
}

Point Ellipse::ePoint(const EllipsePosition& pos) const {
    // (s, t) where:  s^2 + t^2 = 1
    // a and b are the orthogonal axes of the ellipse
    // point of ellipse is:  center + a s + b t               s=cos(theta) t=sin(theta)
    // tangent at point is:  -a t + b s
    // normal at point is:    b s + a t 
    // point on offset-ellipse:  point on ellipse + offset*normal
    Point p(center);
    p.x += a*pos.s;  // a is in X-direction
    p.y += b*pos.t;  // b is in Y-direction
    return p;
}

Point AlignedEllipse::ePoint(const EllipsePosition& pos) const {
    Point p = center + a*pos.s*major_dir + b*pos.t*minor_dir; // point of ellipse is:  center + a s + b t
    return p;
}


Point Ellipse::oePoint(const EllipsePosition& pos) const {
    return ePoint(pos) + offset * normal(pos); // offset-point  = ellipse-point + offset*normal 
}  
  
Point AlignedEllipse::oePoint(const EllipsePosition& pos) const {
    return ePoint(pos) + offset * normal(pos); // offset-point  = ellipse-point + offset*normal 
}  

Point Ellipse::normal(const EllipsePosition& pos) const {
    assert( pos.isValid() );
    Point n = Point( b*pos.s, a*pos.t, 0);
    n.normalize();
    return n;
}    

Point AlignedEllipse::normal(const EllipsePosition& pos) const { 
    // normal at point is:    b s + a t 
    Point n = pos.s*b*major_dir + pos.t*a*minor_dir; 
    n.normalize();
    return n;
}    


#define OE_ERROR_TOLERANCE 1e-10  /// \todo magic number tolerance
// #define DEBUG_SOLVER
//
// given a known EllipsePosition1, look for the other symmetric
// solution EllipsePosition2
bool Ellipse::find_EllipsePosition2() { // a horrible horrible function... :(
    assert( EllipsePosition1.isValid() );
    double err1 = fabs(this->error(this->EllipsePosition1));
    this->EllipsePosition2.s =  this->EllipsePosition1.s;  // plus
    this->EllipsePosition2.t = -this->EllipsePosition1.t; // minus
    if  ( fabs(this->error(this->EllipsePosition2)) < err1+OE_ERROR_TOLERANCE)     {   
        if ( (fabs(this->EllipsePosition2.s - this->EllipsePosition1.s) > 1E-8) || (fabs(this->EllipsePosition2.t - this->EllipsePosition1.t) > 1E-8) ) {
            #ifdef DEBUG_SOLVER
                std::cout << "2nd: (s, t)= " << this->EllipsePosition2 << " oePoint()= " << this->oePoint(this->EllipsePosition2) << " e=" << this->error(this->EllipsePosition2) << "\n";
            #endif
            return true;
        }
    }
    
    this->EllipsePosition2.s = -this->EllipsePosition1.s;  
    this->EllipsePosition2.t =  this->EllipsePosition1.t; 
    if  ( fabs(this->error(this->EllipsePosition2)) < err1+OE_ERROR_TOLERANCE)     { 
        if ( (fabs(this->EllipsePosition2.s - this->EllipsePosition1.s) > 1E-8) || (fabs(this->EllipsePosition2.t - this->EllipsePosition1.t) > 1E-8) ) {  
            #ifdef DEBUG_SOLVER
                std::cout << "2nd: (s, t)= " << this->EllipsePosition2 << " oePoint()= " << this->oePoint(this->EllipsePosition2) << " e=" << this->error(this->EllipsePosition2) << "\n";
            #endif
            return true;
        }   
    }
    
    this->EllipsePosition2.s = -this->EllipsePosition1.s;  
    this->EllipsePosition2.t = -this->EllipsePosition1.t; 
    if  ( fabs(this->error(this->EllipsePosition2)) < err1+OE_ERROR_TOLERANCE)     {  
        if ( (fabs(this->EllipsePosition2.s - this->EllipsePosition1.s) > 1E-8) || (fabs(this->EllipsePosition2.t - this->EllipsePosition1.t) > 1E-8) ) {   
            #ifdef DEBUG_SOLVER
                std::cout << "2nd: (s, t)= " << this->EllipsePosition2 << " oePoint()= " << this->oePoint(this->EllipsePosition2) << " e=" << this->error(this->EllipsePosition2) << "\n";
            #endif
            return true;
        }
    }
    
    // last desperate attempt is identical solutions
    this->EllipsePosition2.s = this->EllipsePosition1.s;  
    this->EllipsePosition2.t = this->EllipsePosition1.t; 
    if  ( fabs(this->error(this->EllipsePosition2)) < err1+OE_ERROR_TOLERANCE)     {  
        // DON'T require solutions to differ
        // if ( (fabs(this->EllipsePosition2.s - this->EllipsePosition1.s) > 1E-8) || (fabs(this->EllipsePosition2.t - this->EllipsePosition1.t) > 1E-8) ) {   
            #ifdef DEBUG_SOLVER
                std::cout << "2nd: (s, t)= " << this->EllipsePosition2 << " oePoint()= " << this->oePoint(this->EllipsePosition2) << " e=" << this->error(this->EllipsePosition2) << "\n";
            #endif
            //std::cout << " find_EllipsePosition2() desperate-mode\n";
            return true;
        //}
    }
    std::cout << "Ellipse::find_EllipsePosition2 cannot find EllipsePosition2! \n"; 
    std::cout << "ellipse= "<< *this << "\n"; 
    print_solutions();
    assert(0); // serious error if we get here!
        
    return false;
}

/// offfset-ellipse solver using Brent's method
/// find the EllipsePosition that makes the offset-ellipse point be at p
/// this is a zero of Ellipse::error()
/// returns number of iterations
///
/// called (only?) by BullCutter::singleEdgeDropCanonical()   
int Ellipse::solver_brent() {
    int iters = 1;
    EllipsePosition apos, bpos; // Brent's method requires bracketing the root in [apos.diangle, bpos.diangle]
    apos.setDiangle( 0.0 );         assert( apos.isValid() );
    bpos.setDiangle( 3.0 );         assert( bpos.isValid() );
    if ( fabs( error(apos) ) < OE_ERROR_TOLERANCE ) { // if we are lucky apos is the solution
        EllipsePosition1 = apos;                      // and we do not need to search further.
        find_EllipsePosition2();
        return iters;
    } else if ( fabs( error(bpos) ) < OE_ERROR_TOLERANCE ) { // or bpos might be the solution?
        EllipsePosition1 = bpos;
        find_EllipsePosition2(); 
        return iters;
    }
    // neither apos nor bpos is the solution
    // but root is now bracketed, so we can use brent_zero
    assert( error(apos) * error(bpos) < 0.0  );
    // this looks for the diangle that makes the offset-ellipse point y-coordinate zero
    double dia_sln = brent_zero( apos.diangle, bpos.diangle , 3E-16, OE_ERROR_TOLERANCE, this ); // brent_zero.hpp
    EllipsePosition1.setDiangle( dia_sln );     assert( EllipsePosition1.isValid() );
    // because we only work with the y-coordinate of the offset-ellipse-point, there are two symmetric solutions
    find_EllipsePosition2();
    return iters;
}


// used by BullCutter pushcutter edge-test
bool AlignedEllipse::aligned_solver( const Fiber& f ) {
    error_dir = f.dir.xyPerp(); // now calls to error(diangle) will give the right error
    assert( error_dir.xyNorm() > 0.0 );
    target = f.p1; // target is either x or y-coord of f.p1
    // find position(s) where ellipse tangent is parallel to fiber. Here error() will be minimized/maximized.
    // tangent at point is:  -a t + b s = -a*major_dir*t + b*minor_dir*s 
    // -at ma.y + bs mi.y = 0   for X-fiber
    // s = sqrt(1-t^2)
    //  -a*ma.y * t + b*mi.y* sqrt(1-t^2) = 0
    //  =>  t^2 = b^2 / (a^2 + b^2)
    double t1(0.0);
    if (f.p1.y == f.p2.y)
        t1 = sqrt( square( b*minor_dir.y ) / ( square( a*major_dir.y ) + square( b*minor_dir.y ) ) );
    else if (f.p1.x == f.p2.x)
        t1 = sqrt( square( b*minor_dir.x ) / ( square( a*major_dir.x ) + square( b*minor_dir.x ) ) );
    else
        assert(0);
    // bracket root
    EllipsePosition tmp,apos,bpos;
    double s1 = sqrt(1.0-square(t1));
    bool found_positive=false;
    bool found_negative=false;
    tmp.setDiangle( xyVectorToDiangle(s1,t1) );
    if (error(tmp.diangle) > 0) {
        found_positive = true;
        apos = tmp;
    } else if (error(tmp.diangle) < 0) {
        found_negative = true;
        bpos = tmp;
    }
    tmp.setDiangle( xyVectorToDiangle(s1,-t1) );
    if (error(tmp.diangle) > 0) {
        found_positive = true;
        apos = tmp;
    }
    else if (error(tmp.diangle) < 0) {
        found_negative = true;
        bpos = tmp;
    }    
    tmp.setDiangle( xyVectorToDiangle(-s1,t1) );
    if (error(tmp.diangle) > 0) {
        found_positive = true;
        apos = tmp;
    }
    else if (error(tmp.diangle) < 0) {
        found_negative = true;
        bpos = tmp;
    }
    tmp.setDiangle( xyVectorToDiangle(-s1,-t1) );
    if (error(tmp.diangle) > 0) {
        found_positive = true;
        apos = tmp;
    }
    else if (error(tmp.diangle) < 0) {
        found_negative = true;
        bpos = tmp;
    }
    
    if (found_positive) {
        if (found_negative) {
            assert( this->error(apos.diangle) * this->error(bpos.diangle) < 0.0 ); // root is now bracketed.
            double lolim(0.0), hilim(0.0);
            if (apos.diangle > bpos.diangle ) {
                lolim = bpos.diangle;
                hilim = apos.diangle;
            } else if (bpos.diangle > apos.diangle) {
                hilim = bpos.diangle;
                lolim = apos.diangle;
            }
            double dia_sln = brent_zero( lolim, hilim , 3E-16, OE_ERROR_TOLERANCE, this );
            double dia_sln2 = brent_zero( hilim-4.0, lolim , 3E-16, OE_ERROR_TOLERANCE, this );
            
            EllipsePosition1.setDiangle( dia_sln );  
            EllipsePosition2.setDiangle( dia_sln2 );   
                   
            assert( EllipsePosition1.isValid() );
            assert( EllipsePosition2.isValid() );
            /*
            // FIXME. This assert fails in some cases (30sphere.stl z=0, for example)
            // FIXME. The allowed error should probably be in proportion to the difficulty of the case.
            
            if (!isZero_tol( error(EllipsePosition1.diangle) )) {
                std::cout << "AlignedEllipse::aligned_solver() ERROR \n";
                std::cout << "error(EllipsePosition1.diangle)= "<< error(EllipsePosition1.diangle) << " (expected zero)\n";
                
            }         
            assert( isZero_tol( error(EllipsePosition1.diangle) ) );
            assert( isZero_tol( error(EllipsePosition2.diangle) ) );
            */
            
            return true;
        }
    }
    
    return false;
}

double AlignedEllipse::error(double diangle) const {
    EllipsePosition tmp;
    tmp.setDiangle( diangle );
    Point p = this->oePoint(tmp);
    Point errorVec = target-p;
    return errorVec.dot(error_dir);
}

double Ellipse::error(double diangle ) const {
    EllipsePosition tmp;
    tmp.setDiangle( diangle ); 
    return error(tmp);
}

double Ellipse::error(EllipsePosition& pos) const {
    Point p1 = oePoint(pos);
    return p1.y;
}


/// given the two solutions EllipsePosition1 and EllipsePosition2 and the edge up1-up2
/// locate the ellipse center correctly
Point Ellipse::calcEcenter(const Point& up1, const Point& up2, int sln) {
    Point cle = ( sln == 1 ? oePoint1() : oePoint2() );
    double xoffset = - cle.x;
    // x-coord on line is  x = up1.x + t*(up2.x-up1.x) = center.x+offset 
    double t = (center.x + xoffset - up1.x) / (up2.x - up1.x);
    return up1 + t*(up2-up1); // return a point on the line
}
Point Ellipse::ePointHi() const {
    return ePoint( EllipsePosition_hi );
}

void Ellipse::setEllipsePositionHi(const Point& u1, const Point& u2) {
    Point ecen1 = calcEcenter( u1, u2, 1);
    Point ecen2 = calcEcenter( u1, u2, 2);
    if (ecen1.z >= ecen2.z) { // we want the higher center
        EllipsePosition_hi = EllipsePosition1;
        center = ecen1;
    } else {
        EllipsePosition_hi = EllipsePosition2;
        center = ecen2;
    } 
}

void Ellipse::print_solutions() {
    std::cout << "1st: (s, t)= " << EllipsePosition1 << " oePoint()= " << oePoint(EllipsePosition1) << " e=" << error(EllipsePosition1) << "\n";
    std::cout << "2nd: (s, t)= " << EllipsePosition2 << " oePoint()= " << oePoint(EllipsePosition2) << " e=" << error(EllipsePosition2) << "\n";
}

/// Ellipse string output
std::ostream& operator<<(std::ostream &stream, const Ellipse& e) {
  stream << "Ellipse: cen=" << e.center << " a=" << e.a << " b=" << e.b << " ofs=" << e.offset ; 
  return stream;
}

} // end namespace
// end of file oellipse.cpp
