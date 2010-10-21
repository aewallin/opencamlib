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

#include <list>
#include "point.h"
#include "epos.h"

namespace ocl
{

class Fiber;

/// An Ellipse. 
class Ellipse {
    public:
        /// dummy constructor
        Ellipse(){}; 
        /// create an Ellipse with centerpoint center, X-axis a, Y-axis b, and offset distance offset.
        Ellipse(Point& centerin, double a, double b, double offset);
        
        /// return a point on the ellipse at given Epos
        virtual Point ePoint(const Epos& position) const;
        /// return a point on the offset-ellipse at given Epos
        Point oePoint(const Epos& position) const;
        /// return a normalized normal vector of the ellipse at the given Epos
        virtual Point normal(const Epos& position) const;
        /// return a normalized tangent vector to the ellipse at the given Epos
        virtual Point tangent(const Epos& position) const;
        /// offset-ellipse Brent solver
        int solver_brent(const Point& p );
        /// print out the found solutions
        void print_solutions(const Point& p);
        /// given one epos solution, find the other.
        bool find_epos2(const Point& p);
        /// error function for the solver
        double error_old(Epos& position, const Point& p); // OLD??
        /// error function for solver
        virtual double error(double dia);
        /// calculate ellipse center
        Point calcEcenter(const Point& up1, const Point& up2, int sln);
        /// set epos_hi to either epos1 or epos2, depending on which
        /// has the center (given by calcEcenter() ) with higher z-coordinate  
        void setEposHi(const Point& u1, const Point& u2);
        /// once epos_hi is set, return an ellipse-point at this position
        Point ePointHi() const;
        Point ePoint1() const;
        Point ePoint2() const;
        Point oePoint1() const;
        Point oePoint2() const;
        /// the center point of the ellipse
        Point center;
        /// eccentricity = a/b
        double eccen;
        
        /// string repr
        friend std::ostream &operator<<(std::ostream &stream, const Ellipse& e);        
        
    protected:
        /// first Epos solution found by solver()
        Epos epos1;
        /// second Epos solution found by solver()
        Epos epos2;
        /// the higher Epos solution
        Epos epos_hi;
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
        AlignedEllipse(Point& centerin, double major_length, double minor_length, double offset, Point& majorDir, Point& minorDir);
        
        Point tangent(const Epos& position) const;
        Point normal(const Epos& position) const;
        Point ePoint(const Epos& position) const;
        
        double error(double dia);
        bool aligned_solver( const Fiber& f );
    private:
        /// direction of the major axis
        Point major_dir;
        /// direction of the minor axis
        Point minor_dir;
        Point error_dir;
};

} // end namespace
#endif
// end file oellipse.h
