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
#include <sstream>
#include <cmath>
#include <string>

#include <cassert>

#include "point.h"
#include "oellipse.h"
#include "numeric.h"
#include "brent_zero.h"
#include "fiber.h"

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
}

AlignedEllipse::AlignedEllipse(Point& centerin, double ain, double bin, double offsetin, Point& major, Point& minor) {
    center = centerin;
    a = ain;
    b = bin;
    assert( b > 0.0 );
    eccen = a/b;
    offset = offsetin;
    major_dir=major;
    minor_dir=minor;
}

Point Ellipse::ePoint1() const {
    return this->ePoint(epos1);
}

Point Ellipse::ePoint2() const {
    return this->ePoint(epos2);
}

Point Ellipse::oePoint1() const {
    return this->oePoint(epos1);
}

Point Ellipse::oePoint2() const {
    return this->oePoint(epos2);
}

Point Ellipse::ePoint(const Epos& pos) const {
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

Point AlignedEllipse::ePoint(const Epos& pos) const {
    Point p = center + a*pos.s*major_dir + b*pos.t*minor_dir; // point of ellipse is:  center + a s + b t
    return p;
}


Point Ellipse::oePoint(const Epos& pos) const {
    return ePoint(pos) + offset * normal(pos); // offset-point  = ellipse-point + offset*normal 
}  
  
Point AlignedEllipse::oePoint(const Epos& pos) const {
    return ePoint(pos) + offset * normal(pos); // offset-point  = ellipse-point + offset*normal 
}  

Point Ellipse::normal(const Epos& pos) const {
    assert( pos.isValid() );
    Point n = Point( b*pos.s, a*pos.t, 0);
    n.normalize();
    return n;
}    

Point AlignedEllipse::normal(const Epos& pos) const { // normal at point is:    b s + a t 
    Point n = pos.s*b*major_dir + pos.t*a*minor_dir; 
    n.normalize();
    return n;
}    



     

#define OE_ERROR_TOLERANCE 1e-10  /// \todo magic number tolerance

bool Ellipse::find_epos2(const Point& p) { // a horrible horrible function... :(
    assert( epos1.isValid() );
    double err1 = fabs(this->error_old(this->epos1, p));
    this->epos2.s =  this->epos1.s;  // plus
    this->epos2.t = -this->epos1.t; // minus
    if  ( fabs(this->error_old(this->epos2, p)) < err1+OE_ERROR_TOLERANCE)     {   
        if ( (fabs(this->epos2.s - this->epos1.s) > 1E-8) || (fabs(this->epos2.t - this->epos1.t) > 1E-8) ) {
            #ifdef DEBUG_SOLVER
                std::cout << "2nd: (s, t)= " << this->epos2 << " oePoint()= " << this->oePoint(this->epos2) << " e=" << this->error(this->epos2, p) << "\n";
            #endif
            return true;
        }
    }
    
    this->epos2.s = -this->epos1.s;  
    this->epos2.t =  this->epos1.t; 
    if  ( fabs(this->error_old(this->epos2, p)) < err1+OE_ERROR_TOLERANCE)     { 
        if ( (fabs(this->epos2.s - this->epos1.s) > 1E-8) || (fabs(this->epos2.t - this->epos1.t) > 1E-8) ) {  
            #ifdef DEBUG_SOLVER
                std::cout << "2nd: (s, t)= " << this->epos2 << " oePoint()= " << this->oePoint(this->epos2) << " e=" << this->error(this->epos2, p) << "\n";
            #endif
            return true;
        }   
    }
    
    this->epos2.s = -this->epos1.s;  
    this->epos2.t = -this->epos1.t; 
    if  ( fabs(this->error_old(this->epos2, p)) < err1+OE_ERROR_TOLERANCE)     {  
        if ( (fabs(this->epos2.s - this->epos1.s) > 1E-8) || (fabs(this->epos2.t - this->epos1.t) > 1E-8) ) {   
            #ifdef DEBUG_SOLVER
                std::cout << "2nd: (s, t)= " << this->epos2 << " oePoint()= " << this->oePoint(this->epos2) << " e=" << this->error(this->epos2, p) << "\n";
            #endif
            return true;
        }
    }
    
    // last desperate attempt is identical solutions
    this->epos2.s = this->epos1.s;  
    this->epos2.t = this->epos1.t; 
    if  ( fabs(this->error_old(this->epos2, p)) < err1+OE_ERROR_TOLERANCE)     {  
        // DON'T require solutions to differ
        // if ( (fabs(this->epos2.s - this->epos1.s) > 1E-8) || (fabs(this->epos2.t - this->epos1.t) > 1E-8) ) {   
            #ifdef DEBUG_SOLVER
                std::cout << "2nd: (s, t)= " << this->epos2 << " oePoint()= " << this->oePoint(this->epos2) << " e=" << this->error(this->epos2, p) << "\n";
            #endif
            //std::cout << " find_epos2() desperate-mode\n";
            return true;
        //}
    }
    
    return false;
}

/// offfset-ellipse solver using Brent's method
/// find the epos that makes the offset-ellipse point be at p
/// this is a zero of Ellipse::error()
/// returns number of iterations
int Ellipse::solver_brent(const Point& p) {
    int iters = 1;
    Epos apos, bpos; // Brent's method requires bracketing the root in [apos.diangle, bpos.diangle]
    apos.diangle = 0.0;
    bpos.diangle = 3.0;
    apos.setD();           assert( apos.isValid() );
    bpos.setD();           assert( bpos.isValid() );
    if ( fabs( this->error_old(apos,p) ) < OE_ERROR_TOLERANCE ) {
        this->epos1 = apos;
        if( !find_epos2(p) ) 
            assert(0);
        return iters;
    } else if ( fabs( this->error_old(bpos,p) ) < OE_ERROR_TOLERANCE ) {
        this->epos1 = bpos;
        if ( !find_epos2(p) ) 
            assert(0);
        return iters;
    }  
    int brack_iters=0;
    while ( this->error_old(apos,p) * this->error_old(bpos,p) >= 0.0 ) {
        bpos.diangle += 1.0;
        bpos.setD();
        brack_iters++;
        if (brack_iters > 10)
            assert(0);
    }
    // root is now bracketed.
    target = p; // the target point
    double dia_sln = brent_zero( apos.diangle, bpos.diangle , 3E-16, OE_ERROR_TOLERANCE, this ); 
    apos.diangle = dia_sln;
    apos.setD();       assert( apos.isValid() );
    epos1 = apos;
    if (!find_epos2( p )) {
        print_solutions(p);
        assert(0);
    }
    /* // what good does this do?
    if (epos1.s == epos2.s && epos1.t==epos2.t) {
        if ( !isZero_tol(epos1.s) && !isZero_tol(epos1.t) ) {
            std::cout << "identical solutions!\n";
            std::cout << "epos1=" << epos1 << " epos2="<< epos2 << "\n";
        }
    }*/
    return iters;
}



bool AlignedEllipse::aligned_solver( const Fiber& f ) {
    error_dir = f.dir.xyPerp(); // now calls to error(diangle) will give the right error
    target = f.p1; // target is either x or y-coord of f.p1
    
    // bracket root
    Epos tmp,apos,bpos;
    // find position(s) where ellipse tangent is parallel to fiber. Here error() will be minimized/maximized.
    // tangent at point is:  -a t + b s
    // or -a*major_dir*t + b*minor_dir*s 
    // -at ma.y + bs mi.y = 0   for X-fiber
    // s = sqrt(1-t^2)
    //  -a*ma.y * t + b*mi.y* sqrt(1-t^2) = 0
    // or t^2 = b^2 / (a^2 + b^2)
    bool y_fiber;
    if (f.p1.y == f.p2.y)
        y_fiber = true;
    else
        y_fiber = false;
        
    if (y_fiber) {
        double t1 = sqrt( square( b*minor_dir.y ) / ( square( a*major_dir.y ) + square( b*minor_dir.y ) ) );
        double s1 = sqrt(1.0-square(t1));
        bool found_positive=false;
        bool found_negative=false;
        
        tmp.diangle = xyVectorToDiangle(s1,t1);
        tmp.setD();
        if (error(tmp.diangle) > 0) {
            found_positive = true;
            apos = tmp;
        } else if (error(tmp.diangle) < 0) {
            found_negative = true;
            bpos = tmp;
        }
        tmp.diangle = xyVectorToDiangle(s1,-t1);
        tmp.setD();
        if (error(tmp.diangle) > 0) {
            found_positive = true;
            apos = tmp;
        }
        else if (error(tmp.diangle) < 0) {
            found_negative = true;
            bpos = tmp;
        }
        
        tmp.diangle = xyVectorToDiangle(-s1,t1);
        tmp.setD();
        if (error(tmp.diangle) > 0) {
            found_positive = true;
            apos = tmp;
        }
        else if (error(tmp.diangle) < 0) {
            found_negative = true;
            bpos = tmp;
        }
        
        tmp.diangle = xyVectorToDiangle(-s1,-t1);
        tmp.setD();
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
                double lolim, hilim;
                if (apos.diangle > bpos.diangle ) {
                    lolim = bpos.diangle;
                    hilim = apos.diangle;
                } else if (bpos.diangle > apos.diangle) {
                    hilim = bpos.diangle;
                    lolim = apos.diangle;
                }
                double dia_sln = brent_zero( lolim, hilim , 3E-16, OE_ERROR_TOLERANCE, this );
                double dia_sln2 = brent_zero( hilim-4.0, lolim , 3E-16, OE_ERROR_TOLERANCE, this );
                // assert( lolim < dia_sln ); assert( dia_sln < hilim ); 
                apos.diangle = dia_sln;
                apos.setD();       assert( apos.isValid() );
                epos1 = apos;
                assert( isZero_tol( error(epos1.diangle) ) );
                bpos.diangle = dia_sln2;
                bpos.setD();       assert( bpos.isValid() );
                epos2 = bpos;
                assert( isZero_tol( error(epos2.diangle) ) );
                return true;
            }
        }
        
    } else { // an x-fiber
        assert( f.p1.x == f.p2.x );
        double t1 = sqrt( square( b*minor_dir.x ) / ( square( a*major_dir.x ) + square( b*minor_dir.x ) ) );
        double s1 = sqrt(1.0-square(t1));
        bool found_positive=false;
        bool found_negative=false;
        
        tmp.diangle = xyVectorToDiangle(s1,t1);
        tmp.setD();
        if (error(tmp.diangle) > 0) {
            found_positive = true;
            apos = tmp;
        } else if (error(tmp.diangle) < 0) {
            found_negative = true;
            bpos = tmp;
        }
        tmp.diangle = xyVectorToDiangle(s1,-t1);
        tmp.setD();
        if (error(tmp.diangle) > 0) {
            found_positive = true;
            apos = tmp;
        }
        else if (error(tmp.diangle) < 0) {
            found_negative = true;
            bpos = tmp;
        }
        
        tmp.diangle = xyVectorToDiangle(-s1,t1);
        tmp.setD();
        if (error(tmp.diangle) > 0) {
            found_positive = true;
            apos = tmp;
        }
        else if (error(tmp.diangle) < 0) {
            found_negative = true;
            bpos = tmp;
        }
        
        tmp.diangle = xyVectorToDiangle(-s1,-t1);
        tmp.setD();
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
                double lolim, hilim;
                if (apos.diangle > bpos.diangle ) {
                    lolim = bpos.diangle;
                    hilim = apos.diangle;
                } else if (bpos.diangle > apos.diangle) {
                    hilim = bpos.diangle;
                    lolim = apos.diangle;
                }
                double dia_sln = brent_zero( lolim, hilim , 3E-16, OE_ERROR_TOLERANCE, this );
                double dia_sln2 = brent_zero( hilim-4.0, lolim , 3E-16, OE_ERROR_TOLERANCE, this );
                // assert( lolim < dia_sln ); assert( dia_sln < hilim ); 
                apos.diangle = dia_sln;
                apos.setD();       assert( apos.isValid() );
                epos1 = apos;
                assert( isZero_tol( error(epos1.diangle) ) );
                bpos.diangle = dia_sln2;
                bpos.setD();       assert( bpos.isValid() );
                epos2 = bpos;
                assert( isZero_tol( error(epos2.diangle) ) );
                return true;
            }
        }
    }
    
    return false;
}

double AlignedEllipse::error(double diangle) const {
    Epos tmp;
    tmp.diangle = diangle;
    tmp.setD();
    Point p = this->oePoint(tmp);
    Point errorVec = target-p;
    return errorVec.dot(error_dir);
}



double Ellipse::error(double diangle ) const {
    Epos tmp;
    tmp.diangle = diangle; 
    tmp.setD();
    Point p1 = oePoint(tmp);
    return p1.y - target.y;
}

// consider removing??
double Ellipse::error_old(Epos& pos, const Point& p) {
    assert( pos.isValid() );
    Point p1 = oePoint(pos);
    double dy = p1.y - p.y;
    return dy;
}


/// given the two solutions epos1 and epos2 and the edge up1-up2
/// locate the ellipse center correctly
Point Ellipse::calcEcenter(const Point& up1, const Point& up2, int sln) {
    Epos pos;
    if (sln == 1)
        pos = epos1;
    else
        pos = epos2;
    Point cle = oePoint(pos);
    double xoffset = - cle.x;
    // x-coord on line is  x = up1.x + t*(up2.x-up1.x) = center.x+offset 
    double t = (center.x + xoffset - up1.x) / (up2.x - up1.x);
    return up1 + t*(up2-up1); // return a point on the line
}
Point Ellipse::ePointHi() const {
    return ePoint( epos_hi );
}

void Ellipse::setEposHi(const Point& u1, const Point& u2) {
    Point ecen1 = calcEcenter( u1, u2, 1);
    Point ecen2 = calcEcenter( u1, u2, 2);
    if (ecen1.z >= ecen2.z) { // we want the higher center
        epos_hi = epos1;
        center = ecen1;
    } else {
        epos_hi = epos2;
        center = ecen2;
    } 
}

void Ellipse::print_solutions(const Point& p) {
    std::cout << "1st: (s, t)= " << this->epos1 << " oePoint()= " << this->oePoint(this->epos1) << " e=" << this->error_old(this->epos1, p) << "\n";
    std::cout << "2nd: (s, t)= " << this->epos2 << " oePoint()= " << this->oePoint(this->epos2) << " e=" << this->error_old(this->epos2, p) << "\n";
}

/// Ellipse string output
std::ostream& operator<<(std::ostream &stream, const Ellipse& e) {
  stream << "Ellipse: cen=" << e.center << " a=" << e.a << " b=" << e.b << " ofs=" << e.offset ; 
  return stream;
}

} // end namespace
// end of file oellipse.cpp
