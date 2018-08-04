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

#ifndef ELLIPSE_H
#define ELLIPSE_H

#include <list>

#include "point.hpp"
#include "ellipseposition.hpp"

namespace ocl
{

class Fiber;

/// An Ellipse. 
class Ellipse {
    public:
        /// dummy constructor
        Ellipse(){}
        /// create an Ellipse with centerpoint center, X-axis a, Y-axis b, and offset distance offset.
        Ellipse(Point& centerin, double a, double b, double offset);
        
        /// return a point on the ellipse at given EllipsePosition
        virtual Point ePoint(const EllipsePosition& position) const;
        /// return a point on the offset-ellipse at given EllipsePosition
        virtual Point oePoint(const EllipsePosition& position) const;
        /// return a normalized normal vector of the ellipse at the given EllipsePosition
        virtual Point normal(const EllipsePosition& position) const;

        /// offset-ellipse Brent solver
        int solver_brent();
        /// print out the found solutions
        void print_solutions();
        /// given one EllipsePosition solution, find the other.
        bool find_EllipsePosition2();
        /// error function for the solver
        double error(EllipsePosition& position) const; 
        /// error function for solver
        virtual double error(double dia) const;
        /// calculate ellipse center
        Point calcEcenter(const Point& up1, const Point& up2, int sln);
        /// set EllipsePosition_hi to either EllipsePosition1 or EllipsePosition2, depending on which
        /// has the center (given by calcEcenter() ) with higher z-coordinate  
        void setEllipsePositionHi(const Point& u1, const Point& u2);
        /// once EllipsePosition_hi is set, return an ellipse-point at this position
        Point ePointHi() const;
        /// ellipse-point at EllipsePosition1
        Point ePoint1() const;
        /// ellipse-point at EllipsePosition2
        Point ePoint2() const;
        /// offset-ellipse-point at EllipsePosition1
        Point oePoint1() const;
        /// offset-ellipse-point at EllipsePosition2
        Point oePoint2() const;

        
        /// string repr
        friend std::ostream &operator<<(std::ostream &stream, const Ellipse& e);        
        /// set length of ellipse major axis
        void setA(double ain) {a=ain;}
        /// set length of ellipse minor axis
        void setB(double bin) {b=bin;}
        /// set the ellipse center
        void setCenter(Point& pin) {center=pin;}
        /// set offset-ellipse offset distance
        void setOffset(double ofs) {offset=ofs;}
        /// set/calculate the eccentricity
        void setEccen() {eccen=a/b;}
        /// returns the z-coordinate of this->center
        inline double getCenterZ() {return center.z;}
        
        /// eccentricity = a/b
        double eccen;
        
    protected:
        /// first EllipsePosition solution found by solver()
        EllipsePosition EllipsePosition1;
        /// second EllipsePosition solution found by solver()
        EllipsePosition EllipsePosition2;
        /// the higher EllipsePosition solution
        EllipsePosition EllipsePosition_hi;
        
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
        AlignedEllipse(){}
        /// create an aligned ellipse
        AlignedEllipse(Point& centerin, double major_length, double minor_length, 
                       double offset, Point& majorDir, Point& minorDir);
        /// normal vector at given EllipsePosition
        Point normal(const EllipsePosition& position) const;
        /// ellipse-point at given EllipsePosition
        Point ePoint(const EllipsePosition& position) const;
        /// offset-ellipse point at given EllipsePosition
        Point oePoint(const EllipsePosition& pos) const;
        /// error-function for the solver
        double error(double dia) const;
        /// aligned offset-ellipse solver. callsn Numeric::brent_solver()
        bool aligned_solver( const Fiber& f );
    private:
        /// direction of the major axis
        Point major_dir;
        /// direction of the minor axis
        Point minor_dir;
        /// the error-direction for error()
        Point error_dir;
};

} // end namespace
#endif
// end file ellipse.h
