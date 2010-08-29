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
#include "epos.h"

namespace ocl
{
    
/// An Ellipse. 
class Ellipse {
    public:
        /// dummy constructor
        Ellipse(){}; 
        /// create an Ellipse with centerpoint center, X-axis a, Y-axis b, and offset distance offset.
        Ellipse(Point& centerin, double a, double b, double offset);
        
        /// return a point on the ellipse at given Epos
        Point ePoint(const Epos& position) const;
        /// return a point on the offset-ellipse at given Epos
        Point oePoint(const Epos& position) const;
        /// return a normalized normal vector of the ellipse at the given Epos
        Point normal(const Epos& position) const;
        /// return a normalized tangent vector to the ellipse at the given Epos
        Point tangent(const Epos& position) const;
        /// offset-ellipse Brent solver
        int solver_brent( Point& p );
        /// print out the found solutions
        void print_solutions( Point& p);
        
        /// given one epos solution, find the other.
        bool find_epos2(Point& p);
        
        /// error function for the solver
        double error_old(Epos& position, Point& p);
        /// error function for solver
        virtual double error(const double dia);
        
        /// calculate ellipse center
        Point calcEcenter(Point& cl, Point& up1, Point& up2, int sln);
        
        /// the center point of the ellipse
        Point center;
        /// eccentricity = a/b
        double eccen;
        
        /// first Epos solution found by solver()
        Epos epos1;
        /// second Epos solution found by solver()
        Epos epos2;
        /// string repr
        friend std::ostream &operator<<(std::ostream &stream, const Ellipse& e);        
        
        
    protected:
        /// a-axis, in the X-direction
        double a;  
        /// b-axis, in the Y-direction
        double b;
        /// offset
        double offset;
        /// the target Point for the error-function
        Point target;

        
};

/// an aligned ellipse, used by the edgePush function of BullCutter
class AlignedEllipse : public Ellipse {
    public:
        AlignedEllipse(){}; 
        /// create an aligned ellipse
        AlignedEllipse(Point& centerin, double a, double b, double offset, Point& major, Point& minor);
        double error(const double dia);
    private:
        /// direction of the major axis
        Point major_dir;
        /// direction of the minor axis
        Point minor_dir;
};

} // end namespace
#endif
// end file oellipse.h
