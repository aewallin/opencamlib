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

#ifndef BPC_H
#define BPC_H

#include <vector>

#include "point.hpp"
#include "fiber.hpp"
#include "kdtree.hpp"
#include "operation.hpp"

namespace ocl
{

class STLSurf;
class Triangle;
class MillingCutter;

///
/// BatchPushCutter takes a MillingCutter, an STLSurf, and many Fibers
/// and pushes the cutter along the fibers into contact with the surface.
/// When this runs the Fibers will be updated with the correct interval data.
/// This is then used to build a weave and extract a waterline.
class BatchPushCutter : public Operation {
    public:
        BatchPushCutter();
        virtual ~BatchPushCutter();
        
        /// set the STL-surface and build kd-tree
        void setSTL(const STLSurf& s);

        /// set this bpc to be x-direction
        void setXDirection() {x_direction=true;y_direction=false;}
        /// set this bpc to be Y-direction
        void setYDirection() {x_direction=false;y_direction=true;}
        /// append to list of Fibers to evaluate
        void appendFiber(Fiber& f);

        
        /// run push-cutter
        void run() {this->pushCutter3();}
        //void run() {this->pushCutter1();}
        
        std::vector<Fiber>* getFibers() const {return fibers;}
        void reset();

        std::vector<CLPoint> getCLPoints();

        std::list<Triangle> getOverlapTriangles(Fiber& f);

    protected:
        /// 1st version of algorithm
        void pushCutter1();
        /// 2nd version of algorithm
        void pushCutter2();
        /// 3rd version of algorithm
        void pushCutter3();
        
        /// pointer to list of Fibers
        std::vector<Fiber>* fibers;
        
    // DATA
        /// true if this we have only x-direction fibers
        bool x_direction;
        /// true if we have y-direction fibers
        bool y_direction;
};

} // end namespace

#endif
