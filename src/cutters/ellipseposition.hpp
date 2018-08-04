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

#ifndef ELLIPSEPOSITION_H
#define ELLIPSEPOSITION_H

#include <list>

#include "point.hpp"

namespace ocl
{
    
class Ellipse;
///
/// \brief EllipsePosition defines a position in (s,t) coordinates on a unit-circle.
/// The (s,t) pair is used to locate points on an ellipse.
/// 
/// s^2 + t^2 = 1 should be true at all times.
class EllipsePosition {
    public:
        /// create an EllipsePosition
        EllipsePosition();
        /// create EllipsePosition at (s,t)
        EllipsePosition(double sin, double tin){s=sin; t=tin;}
        /// set (s,t) pair to the position corresponding to diangle
        void setDiangle(double dia);
        /// set rhs EllipsePosition (s,t) values equal to lhs EllipsePosition
        EllipsePosition &operator=(const EllipsePosition &pos);
        /// return true if (s,t) is valid, i.e. lies on the unit circle
        /// checks s^2 + t^2 == 1  (to within tolerance) 
        bool isValid() const;
        
        /// string repr
        std::string str() const;
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, EllipsePosition pos);
    
    // DATA
        /// s-parameter in [-1, 1]
        double s;
        /// t-parameter in [-1, 1]
        double t;
        /// diamond angle parameter in [0,4] (modulo 4)
        /// this models an angle [0,2pi] and maps 
        /// from the angle to an (s,t) pair using setD()
        double diangle;
        
    private:
        /// set (s,t) pair to match diangle
        void setD();
};
    
}// end namespace
#endif
// end file EllipsePosition.h
