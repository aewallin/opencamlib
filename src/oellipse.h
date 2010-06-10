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

#ifndef OELLIPSE_H
#define OELLIPSE_H

//#include <iostream>
#include <list>
#include "point.h"

namespace ocl
{
    

class Ellipse;
///
/// \brief Epos defines a position in (s,t) coordinates on an ellipse.
/// 
/// s^2 + t^2 = 1 should be true at all times.
class Epos {
    public:
        /// create an epos
        Epos();
        
        /// set s=sin and compute t
        void setS(double sin, bool side);
        
        /// set t=tin and compute s
        void setT(double tin, bool side);
        
        /// step along the (s,t)-parameter in the tangent direction 
        void stepTangent(Ellipse e, double delta);
        
        /// s-parameter in [-1, 1]
        double s;
        /// t-parameter in [-1, 1]
        double t;
        
        /// string repr
        friend std::ostream& operator<<(std::ostream &stream, Epos pos);
        
        /// set rhs Epos (s,t) values equal to lhs Epos
        Epos &operator=(const Epos &pos);
        
        /// string repr
        std::string str();
        
        /// check that (s,t) is valid
        bool isValid();
};



/// An Ellipse. 
class Ellipse {
    public:
        /// dummy constructor
        Ellipse(){}; 
        /// create an Ellipse with centerpoint center, X-axis a, Y-axis b, and offset distance offset.
        Ellipse(Point& centerin, double a, double b, double offset);
        
        //std::string str();
        friend std::ostream &operator<<(std::ostream &stream, const Ellipse& e);
        
        /// return a point on the ellipse
        Point ePoint(Epos& position);
        
        /// return a point on the offset-ellipse
        Point oePoint(Epos& position);
         
        /// return a normalized normal of the ellipse at Epos
        Point normal(Epos& position);
        
        /// return a normalized tangent to the ellipse at Epos
        Point tangent(Epos& position);
        
        /// offset-ellipse solver
        static int solver(Ellipse& e, Point& p); // why static?
        
        
        
        
        
        /// error function for the solver
        double error(Epos& position, Point& p);
        
        /// calculate ellipse center
        Point calcEcenter(Point& cl, Point& up1, Point& up2, int sln);
        
        /// the center point of the ellipse
        Point center;
        
        /// a-axis
        double a;
        /// b-axis
        double b;
        /// eccentricity = a/b
        double eccen;
        
        /// offset
        double offset;
        
        /// first Epos solution found by solver()
        Epos epos1;
        /// second Epos solution found by solver()
        Epos epos2;
        
};

} // end namespace
#endif
// end file oellipse.h
