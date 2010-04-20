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

#ifndef BDC_H
#define BDC_H
#include <boost/foreach.hpp>
#include <boost/python.hpp>
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include "point.h"
#include "triangle.h"
#include "stlsurf.h"
#include "cutter.h"
#include "kdtree.h"

///
/// \brief experimental parallel finish toolpath generation class
class BatchDropCutter {
    public:
        BatchDropCutter();
        
        /// unoptimized drop-cutter,  tests against all triangles of surface
        void dropCutter1();
        
        /// better, kd-tree optimized version      
        void dropCutter2();
        
        /// kd-tree and explicit overlap test      
        void dropCutter3();
        
        /// use OpenMP for multi-threading     
        void dropCutter4();
        
        /// set the STL-surface and build kd-tree to enable optimized algorithm
        void setSTL(STLSurf &s, int bucketSize = 1);
        
        /// set the MillingCutter to use
        void setCutter(MillingCutter *cutter);
        
        /// set number of threads to use in OpenMP
        void setThreads(int n);
        /// append to list of CL-points to evaluate
        void appendPoint(Point& p);
        
        
        /// return CL-points to Python
        boost::python::list getCLPoints();
        /// return CC-points to Python
        boost::python::list getCCPoints();
        /// return triangles under cutter to Python. Not for CAM-algorithms, more for visualization and demonstration.
        boost::python::list getTrianglesUnderCutter(Point &cl, MillingCutter &cutter);
        
        // DATA
        
        /// the MillingCutter used
        MillingCutter *cutter;
        
        /// the list of CL-points to run drop-cutter on
        std::vector<Point> *clpoints;
        
        /// the cutter-contact points corresponding to the CL-points
        std::vector<CCPoint> *ccpoints;
        
        /// root of the kd-tree
        KDNode *root;
        
        /// the STLSurf which we test against.
        STLSurf *surf;
        
        /// how many times DropCutter was called. Useful for optimization.
        int dcCalls;
        
        /// number of threads to use
        int nthreads;
};


#endif
