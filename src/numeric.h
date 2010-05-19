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
#ifndef NUMERIC_H
#define NUMERIC_H
#include <cassert>

namespace ocl
{
    
#define PI 3.1415926535897932



///
/// \brief Numeric is a collection of functions for dealing
/// with the joys of floating-point arithmetic.
///

/// return 1 of x>0, return -1 if x<0.
double sign(double x);

inline double square(double x) {  return x*x; };


/// return true if x is negative
bool isNegative(double x);

/// return true if x is negative
bool isPositive(double x);

/// return true if x is zero, to within tolerance
bool isZero_tol(double x);


} // end namespace
#endif
// end file numeric.h
