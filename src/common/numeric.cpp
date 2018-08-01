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
#include <iostream>
#include <cmath> // for fabs()
#include <cassert>

#include "numeric.hpp"
#include "point.hpp"

#ifdef _WIN32 // Windows platform problem: error C3861: 'isnan': identifier not found, use the Boost version instead
#include <boost/math/special_functions/fpclassify.hpp> // isnan
#endif

namespace ocl {

#define TOLERANCE 0.0000001

double xyVectorToDiangle(double x, double y) {
    double diangle;
    if (y >= 0)
        diangle = (x >= 0 ? y/(x+y) : 1-x/(-x+y));
    else
        diangle = (x < 0 ? 2-y/(-x-y) : 3+x/(x-y));
#ifdef _WIN32
	if ((boost::math::isnan)(diangle)) { // Use the Boost version 
#else
    if (std::isnan(diangle) ) { // Use the std version
#endif
        std::cout << "numeric::xyVectorToDiangle() error (x,y)= ("<< x << " , " << y  << " ) and diangle=" << diangle << "\n";
        assert(0);
    }
    return diangle;
}


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

/// return machine epsilon
double eps() {
    double r;
    r = 1.0;
    while ( 1.0 <  (1.0 + r) )
        r = r / 2.0;
    return ( 2.0 * r );
}

double epsD(double x) {
    double r;
    r = 1000.0;
    while ( x <  (x + r) )
        r = r / 2.0;
    return ( 2.0 * r );
}

float epsF(float x) {
    float r;
    r = 1000.0;
    while ( x <  (x + r) )
        r = r / (float)2.0;
    return ( (float)2.0 * r );
}

void assert_msg( bool assertion, std::string message) {
    if ( !assertion ) {
        std::cout << message;
        assert( assertion );
    }
}


/// solve system Ax = y by inverting A
/// x = Ainv * y
/// returns false if det(A)==0, i.e. no solution found
bool two_by_two_solver( const double& a, 
                        const double& b, 
                        const double& c,
                        const double& d,
                        const double& e,
                        const double& f,
                        double& u,
                        double& v) {
    //  [ a  b ] [u] = [ e ]
    //  [ c  d ] [v] = [ f ]
    // matrix inverse is
    //          [ d  -b ]
    //  1/det * [ -c  a ]
    //  so
    //  [u]              [ d  -b ] [ e ]
    //  [v]  =  1/det *  [ -c  a ] [ f ]
    double det = a*d-c*b;
    if (isZero_tol(det))
        return false;
    u = (1.0/det) * (d*e - b*f);
    v = (1.0/det) * (-c*e + a*f);
    return true;
}


/// find an intersection point in the XY-plane between two lines
/// first line:   p1 + v*(p2-p1)
/// second line:  p3 + t*(p4-p3)
/// sets (v,t) to the intersection point and returns true if an intersection was found 
bool xy_line_line_intersection( const Point& p1, const Point& p2, double& v,
                                const Point& p3, const Point& p4, double& t) {
    // p1 + v*(p2-p1) = p3 + t*(p4-p3)
    // =>
    // [ (p2-p1).x  -(p4-p3).x ] [ v ]  = [ (p3-p1).x ]
    // [ (p2-p1).y  -(p4-p3).y ] [ t ]  = [ (p3-p1).y ]
    return two_by_two_solver( (p2-p1).x , -(p4-p3).x , (p2-p1).y , -(p4-p3).y,  (p3-p1).x, (p3-p1).y, v, t);
}


} // end namespace
// end file numeric.cpp
