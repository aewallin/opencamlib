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
#include "kdtree.h"

///
/// \brief experimental parallel finish toolpath generation class
class ParallelFinish {
    public:
        ParallelFinish();
        
        /// unoptimized drop-cutter 
        void dropCutterSTL1(MillingCutter &cutter);
        
        /// better, kd-tree optimized version      
        void dropCutterSTL2(MillingCutter &cutter);
        
        /// set the STL-surface and build kd-tree to enable optimized algorithm
        void initSTLSurf(STLSurf &s, int bucketSize);
        
        /// initialize a grid of CL-points
        void initCLpoints(double minx,double dx, double maxx, 
                       double miny, double dy,double maxy,double base_z);
<<<<<<< .mine
        
=======
                       
>>>>>>> .r181
        /// return CL-points to Python
        boost::python::list getCLPoints();
        /// return CC-points to Python
        boost::python::list getCCPoints();
        /// return triangles under cutter to Python. Not for CAM-algorithms, more for visualization and demonstration.
        boost::python::list getTrianglesUnderCutter(Point &cl, MillingCutter &cutter);
        
        // DATA
        
        /// the list of CL-points to run drop-cutter on
        std::list<Point> *clpoints;
        /// the cutter-contact points corresponding to the CL-points
        std::list<CCPoint> *ccpoints;
        /// root of the kd-tree
        KDNode *root;
        /// the STLSurf which we test against.
        STLSurf *surf;
        
        /// how many times DropCutter was called. Useful for optimization.
        int dcCalls;
        /// number of threads to use
        int threads;
};


#endif
