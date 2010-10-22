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
        virtual Point oePoint(const Epos& position) const;
        /// return a normalized normal vector of the ellipse at the given Epos
        virtual Point normal(const Epos& position) const;

        /// offset-ellipse Brent solver
        int solver_brent();
        /// print out the found solutions
        void print_solutions();
        /// given one epos solution, find the other.
        bool find_epos2();
        /// error function for the solver
        double error(Epos& position) const; 
        /// error function for solver
        virtual double error(double dia) const;
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

        
        /// string repr
        friend std::ostream &operator<<(std::ostream &stream, const Ellipse& e);        
        
        void setA(double ain) {a=ain;}
        void setB(double bin) {b=bin;}
        void setCenter(Point& pin) {center=pin;}
        void setOffset(double ofs) {offset=ofs;}
        void setEccen() {eccen=a/b;}
        inline double getCenterZ() {return center.z;}
        
        /// eccentricity = a/b
        double eccen;
        
    protected:
        /// first Epos solution found by solver()
        Epos epos1;
        /// second Epos solution found by solver()
        Epos epos2;
        /// the higher Epos solution
        Epos epos_hi;
        
        /// the center point of the ellipse
        Point center;
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
        
        Point normal(const Epos& position) const;
        Point ePoint(const Epos& position) const;
        Point oePoint(const Epos& pos) const;
        double error(double dia) const;
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
