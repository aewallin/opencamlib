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
// uncomment to disable assert() calls
// #define NDEBUG
#include <cassert>

#include "point.h"
#include "oellipse.h"
#include "numeric.h"

namespace ocl
{



//********   Ellipse ********************** */
Ellipse::Ellipse(Point& centerin, double ain, double bin, double offsetin)
{
    center = centerin;
    a = ain;
    b = bin;
    assert( b > 0.0 );
    eccen = a/b;
    offset = offsetin;
}
        
Point Ellipse::ePoint(Epos& pos) const {
    // (s, t) where:  s^2 + t^2 = 1
    // a and b are the orthogonal axes of the ellipse
    // point of ellipse is:  center + a s + b t
    // tangent at point is:  -a t + b s
    // normal at point is:    b s + a t 
    // point on offset-ellipse:  point on ellipse + offset*normal
    Point p = Point(center);
    p.x += a*pos.s;
    p.y += b*pos.t;
    return p;
}

Point Ellipse::oePoint(Epos& pos) const {
    return ePoint(pos) + offset * normal(pos); // offset-point  = ellipse-point + offset*normal 
}    

Point Ellipse::normal(Epos& pos) const {
    assert( pos.isValid() );
    Point n = Point( b*pos.s, a*pos.t, 0);
    n.normalize();
    return n;
}    

Point Ellipse::tangent(Epos& pos) const {
    assert( pos.isValid() );
    Point t = Point( -a*pos.t, b*pos.s, 0);
    t.normalize();
    return t;
}        

#define OE_ERROR_TOLERANCE 1e-10  /// \todo magic number tolerance

// #define DEBUG_SOLVER

/// find the epos that makes the offset-ellipse point be at p
int Ellipse::solver_nr(Point& p)
{
    Epos pos;
    Epos bestpos;
    double best_err; 
    // check the four different sign-permutations of (s,t)
    // to select a "good" initial value
    for (int n=0; n<4 ; n++) {
        double s;

        if ((n%2) == 0 )
            s = 1/sqrt(2.0);
        else
            s = -1/sqrt(2.0);
        
        if ( n < 2 )
            pos.setS(s, true);
        else
            pos.setS(s, false);
            
        double err = this->error(pos, p);
        
        if (n==0) { // on first loop-iteration, store values
            bestpos = pos; 
            best_err = fabs(err);
        }
        else if ( fabs(err) < best_err ) { // update if better pos is found
            best_err = fabs(err);
            bestpos = pos; // pick the best position to start the iterative-solution with
        }
        #ifdef DEBUG_SOLVER
            std::cout << n << " pos=" << pos << " err=" << err << "\n";
        #endif
    }   
    pos = bestpos; // starting position for solver
    assert( pos.isValid() );
    
    
    //bool debugNR = false;
    //if (e.eccen > 370000) {
    //    std::cout << "solver: high eccen case e.eccen= " << e.eccen << "\n";
    //    debugNR = true;
    //}
    
    #ifdef DEBUG_SOLVER
        std::cout << " selected " << pos << " err=" << this->error(pos, p) << "\n";
    #endif
    
    // solver to find Epos, starting at pos, which minimizes
    // e.error(p)
    int iters = 0;                               // number of iterations
    bool endcondition = false;                   // stop when true
    double nr_step = 0.01;                        // arbitrary start-value for NR-step
    double current_error, new_error, deriv, dt;
    Epos epos_tmp;
    current_error = this->error(pos, p);
    
    while (!endcondition) {
        // Newton-Rhapson solver for ellipses with low eccentricity
        epos_tmp = pos;
        Epos tmp2 = pos;
        // take a small step, to determine derivative:
        dt = (0.01/(this->a+this->b)); //*nr_step; //  (0.2/(e.a+e.b))*nr_step; /// \todo 0.2 value here is quite arbitrary...
        assert( dt != 0.0 );
        epos_tmp.stepTangent(*this,dt);            // a temporary new position, to evaluate derivative
        new_error = this->error(epos_tmp, p);
        deriv = (new_error-current_error)/dt;  // evaluate derivative
        nr_step = (-current_error/deriv);      // Newton-Rhapson step
        pos.stepTangent(*this, nr_step);           // take step
        assert( pos.isValid() );
        current_error = this->error(pos, p);
        iters=iters+1;
        
        //if (debugNR) {
        //    std::cout << iters << ": c_err=" << current_error << " pos=" << pos << " oldpos=" << tmp2 << " ";
        //    std::cout << " n_err=" << new_error << " nr_step=" << nr_step << " dt=" << dt << " deriv="<< deriv << "\n";
        //}
        // check endcondition    
        if (fabs(current_error) < OE_ERROR_TOLERANCE)    
            endcondition=true;
        if (iters>200) {  // if it goes on and on, stop with an error.
            std::cout << "oellipse.cpp: Newton-Rhapson solver did not converge.\n";
            std::cout << " iters= " << iters << "\n";
            std::cout << " ellipse-solver target is p= " << p << "\n";
            std::cout << " center= " << this->center << "\n";
            std::cout << " ellipse a=" << this->a << " b=" << this->b << " offset=" << this->offset << "\n";
            std::cout << " current_error= " << current_error << "\n";
            std::cout << " new_error= " << new_error << "\n";
            std::cout << " dt= " << dt << "\n";
            std::cout << " pos = " << pos << " and e.oePoint(pos) = " << this->oePoint(pos) << "\n";
            std::cout << " e.tanget(pos) = " << this->tangent(pos) << "\n";
            std::cout << " nr_step =" << nr_step << "\n";
            assert(0);
        }
    }
    
    // there are two positions which are optimal
    // cycle the signs of (s,t) and find the other solution
    
    this->epos1 = pos; // this is the first solution
    // the code below finds e.epos2 which is the other solution
    
    #ifdef DEBUG_SOLVER
        std::cout << "1st: (s, t)= " << this->epos1 << " oePoint()= " << this->oePoint(this->epos1) << " e=" << this->error(this->epos1, p) << "\n";
    #endif
    
    if (find_epos2(p)) 
        return iters;

    std::cout << "oellipse.cpp:  Ellipse::solver() error, did not find optimal (s,t) point(s)!";
    assert(0);
    return iters;
}

bool Ellipse::find_epos2(Point& p) {
    assert( epos1.isValid() );
    double err1 = fabs(this->error(this->epos1, p));
    this->epos2.s =  this->epos1.s;  // plus
    this->epos2.t = -this->epos1.t; // minus
    if  ( fabs(this->error(this->epos2, p)) < err1+OE_ERROR_TOLERANCE)     {   
        if ( (fabs(this->epos2.s - this->epos1.s) > 1E-8) || (fabs(this->epos2.t - this->epos1.t) > 1E-8) ) {
            #ifdef DEBUG_SOLVER
                std::cout << "2nd: (s, t)= " << this->epos2 << " oePoint()= " << this->oePoint(this->epos2) << " e=" << this->error(this->epos2, p) << "\n";
            #endif
            return true;
        }
    }
    
    this->epos2.s = -this->epos1.s;  
    this->epos2.t =  this->epos1.t; 
    if  ( fabs(this->error(this->epos2, p)) < err1+OE_ERROR_TOLERANCE)     { 
        if ( (fabs(this->epos2.s - this->epos1.s) > 1E-8) || (fabs(this->epos2.t - this->epos1.t) > 1E-8) ) {  
            #ifdef DEBUG_SOLVER
                std::cout << "2nd: (s, t)= " << this->epos2 << " oePoint()= " << this->oePoint(this->epos2) << " e=" << this->error(this->epos2, p) << "\n";
            #endif
            return true;
        }   
    }
    
    this->epos2.s = -this->epos1.s;  
    this->epos2.t = -this->epos1.t; 
    if  ( fabs(this->error(this->epos2, p)) < err1+OE_ERROR_TOLERANCE)     {  
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
    if  ( fabs(this->error(this->epos2, p)) < err1+OE_ERROR_TOLERANCE)     {  
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
int Ellipse::solver_brent (Point& p) {
    int iters = 1;
    // Brent's method requires bracketing the root
    Epos a, b;
    a.diangle = 0.0;
    b.diangle = 3.0;
    a.setD();
    assert( a.isValid() );
    b.setD();
    assert( b.isValid() );
    if ( fabs( this->error(a,p) ) < OE_ERROR_TOLERANCE ) {
        this->epos1 = a;
        if( find_epos2(p) ) {
            //std::cout << "brent: direct a found!\n";
            //print_solutions(p);
        } else {
            assert(0);
        }
        return 1;
    } else if ( fabs( this->error(b,p) ) < OE_ERROR_TOLERANCE ) {
        this->epos1 = b;
        if ( find_epos2(p) ) {
            //std::cout << "brent: direct b found!\n";
            //print_solutions(p);
        } else {
            //std::cout << "brent: ERROR direct b found!\n";
            //print_solutions(p);
            assert(0);
        }   
        return 1;
    }  
    int brack_iters=0;
    while ( this->error(a,p) * this->error(b,p) >= 0.0 ) {
        b.diangle += 1.0;
        b.setD();
        brack_iters++;
        if (brack_iters > 10)
            assert(0);
    }
    /*
    std::cout << " a.err("<<a.diangle<<")="<< this->error(a,p) << " b.err("<< b.diangle <<")=" << this->error(b,p) << "brack_iters=" << brack_iters << "\n";
    std::cout << " prod=" <<  this->error(a,p) * this->error(b,p) << "\n";
    */
    // root is now bracketed.
    
    target = p; // the target point
    
    double dia_sln = brent_zero( a.diangle, b.diangle , 3E-16, OE_ERROR_TOLERANCE, this ); 
    a.diangle = dia_sln;
    a.setD();
    //std::cout << " found sln a.err(" << a.diangle<<")=" << this->error(a,p) << "\n";
    epos1 = a;
    if (!find_epos2( p )) {
        print_solutions(p);
        assert(0);
    }
        
    //print_solutions( p );
    if (epos1.s == epos2.s && epos1.t==epos2.t) {
        if ( !isZero_tol(epos1.s) && !isZero_tol(epos1.t) ) {
            std::cout << "identical solutions!\n";
            std::cout << "epos1=" << epos1 << " epos2="<< epos2 << "\n";
        }
    }
    return iters;
}

void Ellipse::print_solutions(Point& p) {
    std::cout << "1st: (s, t)= " << this->epos1 << " oePoint()= " << this->oePoint(this->epos1) << " e=" << this->error(this->epos1, p) << "\n";
    std::cout << "2nd: (s, t)= " << this->epos2 << " oePoint()= " << this->oePoint(this->epos2) << " e=" << this->error(this->epos2, p) << "\n";
}

double Ellipse::error_brent( double dia ) {
    epos1.diangle = dia;
    epos1.setD();
    Point p1 = oePoint(epos1);
    return p1.y - target.y;
}


/// given the two solutions epos1 and epos2
/// and the line through up1 and up2
/// locate the ellipse center correctly
Point Ellipse::calcEcenter(Point& cl, Point& up1, Point& up2, int sln) {
    Epos pos;
    if (sln == 1)
        pos = epos1;
    else
        pos = epos2;
        
    //Point cce = ePoint(pos);
    Point cle = oePoint(pos);
    double xoffset = - cle.x;
    // x-coord on line is
    // x = up1.x + t*(up2.x-up1.x) = center.x+offset 
    double tparam = (center.x + xoffset - up1.x) / (up2.x - up1.x);
    
    return up1 + tparam*(up2-up1); // return a point on the line
}
    
/// error-function for the offset-ellipse solver
/// here we use only the y-coordinate error         
double Ellipse::error(Epos& pos, Point& p) {
    assert( pos.isValid() );
    Point p1 = oePoint(pos);
    double dy = p1.y - p.y;
    return dy;
}

/// Ellipse string output
std::ostream& operator<<(std::ostream &stream, const Ellipse& e) {
  stream << "Ellipse: cen=" << e.center << " a=" << e.a << " b=" << e.b << " ofs=" << e.offset ; 
  return stream;
}

} // end namespace
// end of file oellipse.cpp
