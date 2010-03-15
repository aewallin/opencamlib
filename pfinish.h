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

#ifndef PFINISH_H
#define PFINISH_H
#include <boost/foreach.hpp>
#include <boost/python.hpp>
#include <iostream>
#include <string>
#include <list>
#include "point.h"
#include "triangle.h"
#include "stlsurf.h"
#include "cutter.h"
///
/// \brief parallel finish toolpath generation
      
class ParallelFinish {
    public:
        ParallelFinish();
        
        void dropCutterSTL1(MillingCutter &cutter, STLSurf &s);      
        void dropCutterSTL2(MillingCutter &cutter, STLSurf &s); 
          
        void initCLpoints(double minx,double dx, double maxx, 
                       double miny, double dy,double maxy,double base_z);
        boost::python::list getCLPoints();
        boost::python::list getCCPoints();
        
        std::list<Point> *clpoints;
        std::list<CCPoint> *ccpoints;
        int dcCalls;
};


#endif
