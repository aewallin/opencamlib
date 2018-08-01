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

#ifndef FIBERPUSHCUTTER_H
#define FIBERPUSHCUTTER_H

#include <iostream>
#include <string>
#include <vector>

#include "point.hpp"
#include "fiber.hpp"
#include "operation.hpp"

namespace ocl
{


///
/// \brief run push-cutter on a single input fiber

class FiberPushCutter : public Operation {
    public:
        FiberPushCutter();
        virtual ~FiberPushCutter();
        
        /// set the STL-surface and build kd-tree
        void setSTL(const STLSurf& s);

        /// set this bpc to be x-direction
        void setXDirection() {x_direction=true;y_direction=false;}
        /// set this bpc to be Y-direction
        void setYDirection() {x_direction=false;y_direction=true;}
        /// run() is an error.
        void run() {assert(0);}
        void run(Fiber& f) {pushCutter2(f);}
        
    protected:
        /// input fiber is tested against all triangles of surface
        void pushCutter1(Fiber& f);
        /// use kd-tree search to find overlapping triangles
        void pushCutter2(Fiber& f);
        
    // DATA
        /// true if this we have only x-direction fibers
        bool x_direction;
        /// true if we have y-direction fibers
        bool y_direction;
};

} // end namespace

#endif
