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
#include <cassert>
#include "numeric.h"

namespace ocl
{

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

} // end namespace
// end file numeric.cpp
