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

#ifndef BDC_H
#define BDC_H

#include <iostream>
#include <string>
#include <vector>

#include "clpoint.h"
#include "millingcutter.h"
#include "kdtree3.h"
#include "operation.h"

namespace ocl
{

class STLSurf;
class Triangle;


///
/// BatchDropCutter takes a MillingCutter, an STLSurf, and a list of CLPoint's
/// and calls MillingCutter::dropCutter() for each CLPoint.
/// To find triangles overlapping the cutter a kd-tree data structure is used.
/// The list of CLPoint's will be updated with the correct z-height as well
/// as corresponding CCPoint's
/// Some versions of this algorithm use OpenMP for multi-threading.
class BatchDropCutter : public Operation {
    public:
        BatchDropCutter();
        virtual ~BatchDropCutter() {delete root;}
        /// set the STL-surface and build kd-tree to enable optimized algorithm
        void setSTL(const STLSurf &s);
        /// append to list of CL-points to evaluate
        void appendPoint(CLPoint& p);
        /// run drop-cutter on all clpoints
        void run() {this->dropCutter5();};
    // getters and setters
        /// return a vector of CLPoints, the result of this operation
        std::vector<CLPoint> getCLPoints() {return *clpoints;}
        
    protected:
        /// unoptimized drop-cutter,  tests against all triangles of surface
        void dropCutter1();
        /// better, kd-tree optimized version      
        void dropCutter2();
        /// kd-tree and explicit overlap test      
        void dropCutter3();
        /// use OpenMP for multi-threading     
        void dropCutter4();
        /// version 5 of the algorithm
        void dropCutter5();
    // DATA
        /// pointer to list of CL-points on which to run drop-cutter.
        std::vector<CLPoint>* clpoints;

};

} // end namespace

#endif
