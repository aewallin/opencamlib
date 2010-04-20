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


#ifndef OCL_H
#define OCL_H

#include <iostream>
#include <iterator>
#include <algorithm>
#include <string>
#include <list>

#include <math.h>

#include <boost/progress.hpp>
#include <boost/timer.hpp>
#include <boost/foreach.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/class.hpp>
#include <boost/python/wrapper.hpp>
#include <boost/python/call.hpp>

#include "point.h"
#include "triangle.h"
#include "stlsurf.h"
#include "cutter.h"
#include "pfinish.h"
#include "line.h"
#include "arc.h"
#include "path.h"
#include "pathfinish.h"
#include "oellipse.h"
#include "octree.h"
#include "volume.h"
#include "batchdropcutter.h"

#endif

/*
 * some info here: http://www.eventhelix.com/realtimemantra/HeaderFileIncludePatterns.htm
 * 
 */
