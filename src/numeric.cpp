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

#include "oellipse.h"


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


/// find an intersection point in the XY-plane between two lines
/// first line:   p1 + v*(p2-p1)
/// second line:  p3 + t*(p4-p3)
/// returns true if an intersection was found 
bool xy_line_line_intersection( Point& p1, Point& p2, double& v,
                                Point& p3, Point& p4, double& t) {
    // in the XY plane we move a distance v along l1
    // and a distance t along line2
    // we should end up at the same point
    // p1 + v*(p2-p1) = p3 + t*(p4-p3)
    // this leads to a matrix equation:
    // [ (p2-p1).x  -(p4-p3).x ] [ v ]  = [ (p3-p1).x ]
    // [ (p2-p1).y  -(p4-p3).y ] [ t ]  = [ (p3-p1).y ]
    // or
    // Mx=y
    namespace bnu = boost::numeric::ublas;
    bnu::matrix<double> M(2,2);
    M(0,0) = (p2.x-p1.x);
    M(0,1) = (p4.x - p3.x);
    M(1,0) = (p2.y-p1.y);
    M(1,1) = (p4.y - p3.y);
    double detM = determinant(M);
    if ( isZero_tol( detM ) )
        return false; // parallell lines, no intersection
    
    double v_numer = (p4.x-p3.x)*(p1.y-p3.y) - (p4.y-p3.y)*(p1.x-p3.x);
    double t_numer = (p2.x-p1.x)*(p1.y-p3.y) - (p2.y-p1.y)*(p1.x-p3.x);
    if( isZero_tol( t_numer ) && isZero_tol( v_numer) )
        return false;
        
    t = t_numer/detM;
    v = v_numer/detM;
    return true;
}

} // end namespace
// end file numeric.cpp
