/*  Copyright 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com)
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
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <math.h>
#include <boost/progress.hpp>

#include "point.h"
#include "oellipse.h"
#include "numeric.h"



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
    double ssq = 1 - t*t; 
    if (side == true)
        s =  1 * sqrt(ssq);
    else
        s = -1 * sqrt(ssq);
}

void Epos::setS(double sin, bool side)
{   // this is exactly like setT() but with s<->t swapped
    if (sin > 1.0)
        sin = 1.0;
    if (sin < -1.0)
        sin = -1.0;
    
    s = sin;
    double tsq = 1 - s*s; 
    if (side == true)
        t =  1 * sqrt(tsq);
    else
        t = -1 * sqrt(tsq);
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

#define DEBUG_SOLVER
int Ellipse::solver(Ellipse& e, Point& p)
{
    
    // select a "good" initial value
    Epos pos;
    Epos bestpos;
    double abs_err; 
    for (int n=0; n<4 ; n++) {
        double s;

        if ((n%2) == 0 )
            s = 1/sqrt(2);
        else
            s = -1/sqrt(2);
        
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
            bestpos = pos;
        }
        #ifdef DEBUG_SOLVER
            std::cout << n << " pos=" << pos << " err=" << err << "\n";
        #endif
        
        //pos
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
        epos_tmp.stepTangent(e,dt);
        new_error = e.error(epos_tmp, p);
        //#print "new_error=", new_error
        deriv = (new_error-current_error)/dt;
        //#print "derivative = ", deriv
        //# take Newton rhapson step
        nr_step = (-current_error/deriv);
        //#print " NRstep=", NRStep
        //#NRStep=0.05 # debug/demo
        pos.stepTangent(e, nr_step);
        
        iters=iters+1;
        current_error = e.error(pos, p);
        // check endcondition    
        if (fabs(current_error) < OE_ERROR_TOLERANCE)    
            endcondition=true;
        if (iters>125)  /// \todo magix number max_iterations
            endcondition=true;       
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
    
    /*
    Epos pos3;
    pos3.s=-pos.s;
    pos3.t=pos.t;
    Point p3 = e.oePoint(pos3);
    double e3 = e.error(pos3, p);
    if  (fabs(e1) < OE_ERROR_TOLERANCE)     {   
        std::cout << "(s, t)= " << pos << " oePoint()= " << p1 << " e=" << e1 << "\n";
        epos2 = pos2;
        return iters;
    }
    
    Epos pos4;
    pos4.s=-pos.s;
    pos4.t=-pos.t;
    Point p4 = e.oePoint(pos4);
    double e4 = e.error(pos4, p);
    

    if (fabs(e2) < OE_ERROR_TOLERANCE)
        std::cout << "(s, t)= " << pos2 << " oePoint()= " << p2 << " e=" << e2 << "\n";
    if (fabs(e3) < OE_ERROR_TOLERANCE)
        std::cout << "(s, t)= " << pos3 << " oePoint()= " << p3 << " e=" << e3 << "\n";
    if (fabs(e4) < OE_ERROR_TOLERANCE)
        std::cout << "(s, t)= " << pos4 << " oePoint()= " << p4 << " e=" << e4 << "\n";
    */
    
    std::cout << " Ellipse::solver() error, did not find optimal (s,t) point(s)!";
    return iters;
}
        
double Ellipse::error(Epos& pos, Point& p)
{
    Point p1 = oePoint(pos);
    double dy = p1.y - p.y;
    return dy;
}

// end of file oellipse.cpp
