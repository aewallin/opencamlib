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

#include <cmath>


#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/lu.hpp>



#include "numeric.h"

namespace ocl
{

namespace bnu = boost::numeric::ublas;


#define TOLERANCE 0.0000001


double sign(double x) {
    if (x<0.0)
        return -1;
    else
        return 1;
}

bool isPositive(double x) {
    if (x > 0.0 )
        return true;
    else
        return false;
}

bool isNegative(double x) {
    if ( x < 0.0 )
        return true;
    else
        return false;
}

bool isZero_tol(double x) {
    if (fabs(x)<TOLERANCE)
        return true;
    else
        return false;
}

int determinant_sign(const bnu::permutation_matrix<std::size_t>& pm)
{
    int pm_sign=1;
    for (std::size_t i = 0; i < pm.size(); ++i)
        if (i != pm(i))
            pm_sign *= -1.0; // swap_rows would swap a pair of rows here, so we change sign
    return pm_sign;
}

double determinant( bnu::matrix<double>& m ) {
    bnu::permutation_matrix<std::size_t> pm(m.size1());
    double det = 1.0;
    if( bnu::lu_factorize(m,pm) ) {
        det = 0.0;
    } else {
        for(unsigned i = 0; i < m.size1(); i++) 
            det *= m(i,i); // multiply by elements on diagonal
        det = det * determinant_sign( pm );
    }
    return det;
}

/// return machine epsilon
double eps() {
  double r;
  r = 1.0;
  while ( 1.0 <  (1.0 + r) )
    r = r / 2.0;
  return ( 2.0 * r );
}

/// Brent's root finding algorithm
double brent_zero( double a, double b, double eps, double t, 
  double f(double x)) {
    // f must have unequal sign at a and b, i.e.
    // f(a)*f(b) < 0
    // the location of a root c where f(c)=0 is located
    // to within 6*eps*abs(c)+2*t tolerance 
    double c,d,e;
    double fa,fb,fc; // function values
    double m,p,q,r,s;
    double tol;
    //double sa,sb,tol;
      
    //sa = a;
    //sb = b;
    fa = f(a);
    fb = f(b);
    if (fa*fb >= 0.0) // check for opposite signs
        assert(0);
        
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
        if (fabs(m) <= tol || fb == 0.0) // end-condition for the infinite loop
            break; // either within tolerance, or found exact zero fb
        
        if ( fabs(e) < tol || fabs(fa) <= fabs(fb) ) {
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
            if ( 2.0*p < 3*m*q-fabs(tol*q) && p<fabs(0.5*s*q) ) {
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
            b = b+tol; // otherwise, push root by tol
        else
            b = b-tol;
        
        fb = f(b);
        
        if ( ((fb>0.0) && (fc>0.0)) || ((fb<=0.0) && (fc<=0.0)) ) {
            // fb and fc have the same sign
            c = a;  // so change c to a
            fc = a;
            e = b-a; // interval width
            d = e;
        }
        
    } // end iteration-loop

    return b;
}

} // end namespace
// end file numeric.cpp
