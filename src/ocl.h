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
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/class.hpp>
#include <boost/python/wrapper.hpp>
#include <boost/python/call.hpp>

#include "point.h"
#include "triangle.h"
#include "stlsurf.h"
#include "millingcutter.h"
#include "line.h"
#include "arc.h"
#include "path.h"
#include "pathdropcutter.h"
#include "oellipse.h"
#include "octree.h"
#include "volume.h"
#include "batchdropcutter.h"
#include "fiber.h"
<<<<<<< .mine
#include "clfilter.h"
=======
#include "stlreader.h"
>>>>>>> .r275

#endif

/*
 * some info here: http://www.eventhelix.com/realtimemantra/HeaderFileIncludePatterns.htm
 * 
 */
