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

#ifndef POINTDROPCUTTER_H
#define POINTDROPCUTTER_H

#include <iostream>
#include <string>
#include <vector>

#include "clpoint.h"
#include "millingcutter.h"
#include "kdtree.h"
#include "operation.h"

namespace ocl
{

class STLSurf;
class Triangle;

/// run drop-cutter on an STL-surface at a single input CLPoint
class PointDropCutter : public Operation {
    public:
        PointDropCutter();
        virtual ~PointDropCutter() {
            std::cout << " ~PointDropCutter() \n";
            delete root;
        }
        void setSTL(const STLSurf &s);
        void run(CLPoint& cl);
        void run() {
            std::cout << "ERROR: can't call run() on PointDropCutter()\n";
            assert(0);
        }
        
    protected:
        /// first simple implementation of this operation
        void pointDropCutter1(CLPoint& clp);
};

} // end namespace

#endif
