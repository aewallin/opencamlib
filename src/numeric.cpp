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
#define NEGATIVE_TOL -TOLERANCE

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

} // end namespace
// end file numeric.cpp
