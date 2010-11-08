/*  $Id: $
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

#ifndef POINTDROPCUTTER_H
#define POINTDROPCUTTER_H

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
class PointDropCutter : public Operation {
    public:
        PointDropCutter();
        virtual ~PointDropCutter() {
            delete root;
        }
        /// set the STL-surface and build kd-tree to enable optimized algorithm
        void setSTL(const STLSurf &s);
        /// run drop-cutter on given CLPoint
        void run(CLPoint& cl);
        void run() {
            std::cout << "ERROR: can't call run() on PointDropCutter()\n";
            assert(0);
        }
        
    protected:
        void pointDropCutter1(CLPoint& clp);
};

} // end namespace

#endif
