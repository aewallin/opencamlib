/*  $Id$
 * 
 *  Copyright (c) 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com).
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

#ifndef LINE_CL_FILTER_H
#define LINE_CL_FILTER_H

#include "clfilter.hpp"

namespace ocl
{  

/// LineCLFilter takes a sequence of cutter-location (CL) points
/// as input and produces another sequence as output.
///
/// The number of CL-points is reduced by finding co-linear points, 
/// to within a set tolerance, and deleting redundant ones.
///
class LineCLFilter : public CLFilter  { 
    public:
        LineCLFilter();
        ~LineCLFilter(){}; 
        void addCLPoint(const CLPoint& p); 
        void setTolerance(const double tol);
        void run();
};



} // end namespace
#endif
// end file lineclfilter.h
