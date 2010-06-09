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
//#include <stdio.h>
//#include <sstream>
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

// #define DEBUG_SOLVER


//********   Epos ********************** */
Epos::Epos()
{
    setT(1, true);
}

void Epos::setT(double tin, bool side)
{
    if (tin > 1.0)
        tin = 1.0;
    if (tin < -1.0)
        tin = -1.0;
    
    t = tin;
    double ssq = 1 - square(t); 
    if (side == true)
        s =  1 * sqrt(ssq);
    else
        s = -1 * sqrt(ssq);
    
    assert( this->isValid() );
}

bool Epos::isValid()
{
    if ( isZero_tol( square(s) + square(t) - 1.0 ) )
        return true;
    else
        return false;
}

void Epos::setS(double sin, bool side)
{   // this is exactly like setT() but with s<->t swapped
    if (sin > 1.0)
        sin = 1.0;
    if (sin < -1.0)
        sin = -1.0;
    
    s = sin;
    double tsq = 1 - square(s); 
    if (side == true)
        t =  1 * sqrt(tsq);
    else
        t = -1 * sqrt(tsq);
    
    assert( this->isValid() );
}

void Epos::stepTangent(Ellipse e, double delta)
{
    Point tang = e.tangent(*this);
    if ( fabs(tang.x) > fabs(tang.y) ) {
        //  "s-dir step"
        double news = s + delta*tang.x;
        if (t > 0)
            setS(news,1);
        else
            setS(news,0);
    } else {
        // t-dir step"
        double newt = t + delta*tang.y;
        if (s>0)
            setT( newt,1);
        else
            setT( newt,0);
    }
    assert( this->isValid() );
}

Epos& Epos::operator=(const Epos &pos) 
{
    s = pos.s;
    t = pos.t;
    return *this;
}

std::string Epos::str()
{
        std::ostringstream o;
        o << "(" << s << ", " << t << ")";
        return o.str();
}

std::ostream& operator<<(std::ostream &stream, Epos pos)
{
    stream << "("<< pos.s <<" ," << pos.t << ")";
    return stream;
}


//********   Ellipse ********************** */
Ellipse::Ellipse(Point& centerin, double ain, double bin, double offsetin)
{
    center = centerin;
    a = ain;
    b = bin;
    offset = offsetin;
}
        
Point Ellipse::ePoint(Epos& pos)
{
    // (s, t) where:  s^2 + t^2 = 1
    // point of ellipse is:  center + a s + n t
    // tangent at point is:  -a t + b s
    // normal at point is:  j (s / eccen) + n (t * eccen)
    // point on offset-ellipse:  point on ellipse + offrad*norma
    Point p = Point(center);
    p.x += a*pos.s;
    p.y += b*pos.t;
    return p;
}

Point Ellipse::oePoint(Epos& pos)
{
    Point p;
    p = ePoint(pos);
    Point n;
    n = normal(pos);
    p.x += offset*n.x;
    p.y += offset*n.y;
    return p;
}    

Point Ellipse::normal(Epos& pos)
{
    Point n = Point( b*pos.s, a*pos.t, 0);
    n.normalize();
    return n;
}    

Point Ellipse::tangent(Epos& pos)
{
    Point t = Point(- a*pos.t, b*pos.s, 0);
    t.normalize();
    return t;
}        

#define OE_ERROR_TOLERANCE 1e-8  /// \todo magic number tolerance

/// find the epos that makes the offset-ellipse point be at p
int Ellipse::solver(Ellipse& e, Point& p)
{
    Epos pos;
    Epos bestpos;
    double abs_err; 
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
            
        double err = e.error(pos, p);
        
        if (n==0) {
            bestpos = pos; 
            abs_err = fabs(err);
        }
        else if (abs_err > fabs(err) ) {
            abs_err = fabs(err);
            bestpos = pos; // pick the best position to start the iterative-solution with
        }
        #ifdef DEBUG_SOLVER
            std::cout << n << " pos=" << pos << " err=" << err << "\n";
        #endif
    }   
    pos = bestpos;
    #ifdef DEBUG_SOLVER
        std::cout << " selected " << pos << " err=" << e.error(pos, p) << "\n";
    #endif
    
    // newton-rhapson solver to find Epos, starting at pos, which minimizes
    // e.error(p)
    int iters = 0;
    bool endcondition = false;
    double nr_step = 0.1; // arbitrary start-value for NR-step
    double current_error, new_error, deriv, dt;
    Epos epos_tmp;
    current_error = e.error(pos, p);
    
    while (!endcondition) {  
        // #print "current error=", current_error
        epos_tmp.s = pos.s;
        epos_tmp.t = pos.t;
        // # take a small step, to determine rerivative:
        dt = 0.2*nr_step; /// \todo 0.2 value here is quite arbitrary...
        epos_tmp.stepTangent(e,dt);            // a temporary new position, to evaluate derivative
        new_error = e.error(epos_tmp, p);
        deriv = (new_error-current_error)/dt;  // evaluate derivative
        nr_step = (-current_error/deriv);      // Newton-rhapson step
        pos.stepTangent(e, nr_step);           // take step
        
        iters=iters+1;
        current_error = e.error(pos, p);
        // check endcondition    
        if (fabs(current_error) < OE_ERROR_TOLERANCE)    
            endcondition=true;
        if (iters>100) {  // if it goes on and on, stop with an error.
            std::cout << "oellipse.cpp: Newton-Rhapson solver did not converge.\n";
            std::cout << " ellipse-solver target is p= " << p << "\n";
            std::cout << " center= " << e.center << "\n";
            std::cout << " ellipse a=" << e.a << " b=" << e.b << " offset=" << e.offset << "\n";
            std::cout << " current_error= " << current_error << "\n";
            std::cout << " pos = " << pos << " and e.oePoint(pos) = " << e.oePoint(pos) << "\n";
            assert(0);
        }
       
    }
    
    // there are two positions which are optimal
    // cycle the signs of (s,t) and find the other solution
    e.epos1 = pos;
    #ifdef DEBUG_SOLVER
        std::cout << "1st: (s, t)= " << e.epos1 << " oePoint()= " << e.oePoint(e.epos1) << " e=" << e.error(e.epos1, p) << "\n";
    #endif
    
    e.epos2.s=pos.s;  // plus
    e.epos2.t=-pos.t; // minus
    if  ( fabs(e.error(e.epos2, p)) < OE_ERROR_TOLERANCE)     {   
        if ( (fabs(e.epos2.s - e.epos1.s) > 1E-8) || (fabs(e.epos2.t - e.epos1.t) > 1E-8) ) {
            #ifdef DEBUG_SOLVER
                std::cout << "2nd: (s, t)= " << e.epos2 << " oePoint()= " << e.oePoint(e.epos2) << " e=" << e.error(e.epos2, p) << "\n";
            #endif
            return iters;
        }
    }
    
    e.epos2.s=-pos.s; // minus
    e.epos2.t=pos.t;  // plus
    if  ( fabs(e.error(e.epos2, p)) < OE_ERROR_TOLERANCE)     { 
        if ( (fabs(e.epos2.s - e.epos1.s) > 1E-8) || (fabs(e.epos2.t - e.epos1.t) > 1E-8) ) {  
            #ifdef DEBUG_SOLVER
                std::cout << "2nd: (s, t)= " << e.epos2 << " oePoint()= " << e.oePoint(e.epos2) << " e=" << e.error(e.epos2, p) << "\n";
            #endif
            return iters;
        }   
    }
    
    e.epos2.s=-pos.s; // minus
    e.epos2.t=-pos.t; // minus
    if  ( fabs(e.error(e.epos2, p)) < OE_ERROR_TOLERANCE)     {  
        if ( (fabs(e.epos2.s - e.epos1.s) > 1E-8) || (fabs(e.epos2.t - e.epos1.t) > 1E-8) ) {   
            #ifdef DEBUG_SOLVER
                std::cout << "2nd: (s, t)= " << e.epos2 << " oePoint()= " << e.oePoint(e.epos2) << " e=" << e.error(e.epos2, p) << "\n";
            #endif
            return iters;
        }
    }
    
    std::cout << "oellipse.cpp:  Ellipse::solver() error, did not find optimal (s,t) point(s)!";
    assert(0);
    return iters;
}

/// given the two solutions epos1 and epos2
/// and the line through up1 and up2
/// locate the ellipse center correctly
Point Ellipse::calcEcenter(Point& cl, Point& up1, Point& up2, int sln)
{
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
double Ellipse::error(Epos& pos, Point& p)
{
    Point p1 = oePoint(pos);
    double dy = p1.y - p.y;
    return dy;
}

} // end namespace
// end of file oellipse.cpp
