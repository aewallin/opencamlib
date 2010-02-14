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



#ifndef CUTTER_H
#define CUTTER_H
#include <boost/foreach.hpp>
#include <iostream>
#include <string>
#include "point.h"
#include "triangle.h"
///
/// \brief MillingCutter is a base-class for all milling cutters
///
      
class MillingCutter {
    public:
        MillingCutter();
        
        void setDiameter(double d);
        double getDiameter();
        void setLength(double l);
        double getLength();
        void setId();
        
        // drop-cutter methods
        /// \brief drop cutter at (cl.x, cl.y) against vertices of Triangle t.
        /// returns the cc point
        virtual Point vertexDrop(Point &cl, const Triangle &t) = 0;
        /// drop cutter at (cl.x, cl.y) against facet of Triangle t
        virtual Point facetDrop(Point &cl, const Triangle &t) = 0;
        /// drop cutter at (cl.x, cl.y) against edges of Triangle t
        virtual Point edgeDrop(Point &cl, const Triangle &t) = 0;
        
        //virtual Point dropCutter(Point &cl, const Triangle &t) = 0;
        
    protected:
        static int count;
        /// cutter id number
        int id;
        /// diameter of cutter
        double diameter;
        /// length of cutter
        double length;
};


///
/// \brief Cylindrical milling cutter (flat-endmill)
///
class CylCutter : public MillingCutter {
	public:
		/// create CylCutter with radius = 1.0
		CylCutter();
		/// create CylCutter with diameter = d
		CylCutter(const double d);
        
        // dropCutter methods
        /// drop cutter at (cl.x, cl.y) against vertices of Triangle t
        Point vertexDrop(Point &cl, const Triangle &t);
        /// drop cutter at (cl.x, cl.y) against facet of Triangle t
        Point facetDrop(Point &cl, const Triangle &t);
        /// drop cutter at (cl.x, cl.y) against edges of Triangle t
        Point edgeDrop(Point &cl, const Triangle &t);
        
        // text output
        friend std::ostream& operator<<(std::ostream &stream, CylCutter c);
        std::string str();
		
};

///
/// \brief Ball/Spherical milling cutter (ball-nose endmill)
///
class BallCutter : public MillingCutter {
    public:
        BallCutter();
        Point vertexDrop(Point &cl, const Triangle &t);
        Point facetDrop(Point &cl, const Triangle &t);
        Point edgeDrop(Point &cl, const Triangle &t);
        
        
};

#endif
