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

int Ellipse::solver(Ellipse& e, Epos& pos, Point& p)
{
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
        if (fabs(current_error) < 1e-8)    /// \todo magic number tolerance
            endcondition=true;
        if (iters>125)  /// \todo magix number max_iterations
            endcondition=true;
            
    }
    
    // sometimes we find the wrone pos
    // we want the pos which also minimizes the x-distance to p
    Point p1 = e.oePoint(pos);
    Epos pos2;
    pos2.s=pos.s;
    pos2.t=-pos.t;
    Point p2 = e.oePoint(pos2);
    std::cout << "solutions: " << p1 << " and " << p2 << "\n";
    
    return iters;
}
        
double Ellipse::error(Epos& pos, Point& p)
{
    Point p1 = oePoint(pos);
    double dy = p1.y - p.y;
    return dy;
}

// end of file oellipse.cpp
