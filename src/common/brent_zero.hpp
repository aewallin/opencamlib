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
#ifndef BRENT_ZERO_H
#define BRENT_ZERO_H

namespace ocl
{

/// Brent's root finding algorithm
/// http://en.wikipedia.org/wiki/Brent's_method
///
/// find a zero of function f in the interval [a,b]
/// a and b must bracket the root, i.e. f(a) must have different sign than f(b)
/// needs a pointer to an ErrObj which must provide a function
/// double ErrObj::error(double x) for which we try to find a zero
///
/// FIXME: describe tolerance parameters eps and t
template <class ErrObj>
double brent_zero( double a, double b, double eps, double t, ErrObj* ell) {
    // f must have unequal sign at a and b, i.e.
    // f(a)*f(b) < 0
    // returns the location of a root c where f(c)=0 
    // to within 6*eps*abs(c)+2*t tolerance 
    double c,d,e;
    double fa,fb,fc; // function values
    double m,p,q,r,s;
    double tol;
    fa = ell->error(a); // f(a);
    fb = ell->error(b); // f(b);
    if (fa*fb >= 0.0) {// check for opposite signs
        std::cout << " brent_zero() called with invalid interval [a,b] !\n";
        assert(0);
    }
    c  = a; // set c sln equal to a sln
    fc = fa; 
    e  = b-a; // interval width
    d  = e; // interval width
    while (true) {
        if (fabs(fc)<fabs(fb)) { // sln at c is better than at b
            a = b;  // a is the old solution
            b = c;  // b is the best root so far
            c  = a;
            fa = fb;
            fb = fc; // swap so that fb is the best solution
            fc = fa;
        }
        tol = 2.0*eps*fabs(b)+t;
        m = 0.5*(c-b); // half of step from c to b sln 
        if ( (fabs(m) <= tol) || (fb == 0.0) ) // end-condition for the infinite loop
            break; // either within tolerance, or found exact zero fb
        
        if ( (fabs(e) < tol) || (fabs(fa) <= fabs(fb)) ) {
            // step from c->b was small, or fa is a better solution
            e = m; 
            d = e; // bisection?
        } else {
            s = fb/fa;
            if (a == c) {
                p=2.0*m*s;
                q=1.0-s;
            } else {
                q = fa/fc;
                r = fb/fc;
                p = s*(2.0*m*a*(q-r)-(b-a)*(r-1.0));
                q = (q-1.0)*(r-1.0)*(s-1.0);
            }
            
            if (p>0.0)
                q=-q;
            else
                p=-p; // make p negative
            
            s=e;
            e=d;
            if ( (2.0*p < (3.0*m*q-fabs(tol*q))) && (p<fabs(0.5*s*q)) ) {
                d = p/q;
            } else {
                e = m;
                d = e;
            }                
        }
        a = b; // store the old b-solution in a
        fa = fb;
        if (fabs(d) > tol ) // if d is "large"
            b = b+d; // push the root by d
        else if ( m > 0.0 )
            b = b+tol; // otherwise, push root by tol in direction of m
        else
            b = b-tol;
        // std::cout << " brent_zero b=" << b << "\n";
        
        fb = ell->error(b); // f(b);
        
        if ( ((fb>0.0) && (fc>0.0)) || ((fb<=0.0) && (fc<=0.0)) ) {
            // fb and fc have the same sign
            c = a;  // so change c to a
            fc = fa;
            e = b-a; // interval width
            d = e;
        }
    } // end iteration-loop
    return b;
}



} // end namespace
#endif
// end file brent_zero.hpp
