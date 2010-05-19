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
#ifndef FIBER_H
#define FIBER_H

#include <iostream>
#include <vector>
#include <boost/numeric/interval.hpp>


#include "point.h"
#include "numeric.h"

namespace ocl
{

typedef boost::numeric::interval<double> dinterval;

class Fiber {
    public:
		Fiber(){ };
        Fiber(const Point &p1, const Point &p2);
        virtual ~Fiber() {};
        
        
        void printInts();
        void addInt(double t1, double t2);
        void condense();
        double tval(Point& p) const;
        boost::python::list getInts();
        Point point(double t);
        
        /// start point
        Point p1;
        /// end point
        Point p2;
        Point dir;
        std::vector<dinterval> ints;
    private:
        void calcDir();
};

} // end namespace
#endif
// end file fiber.h
