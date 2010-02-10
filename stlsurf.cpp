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
#include <iostream>
#include <iterator>
#include <algorithm>
#include <stdio.h>
#include <sstream>
#include <math.h>
#include <string>
#include <list>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>

#include "point.h"
#include "triangle.h"
#include "stlsurf.h"

int STLSurf::count=0;

STLSurf::STLSurf() {
    setId();
	std::cout << "STLSurf default constructor" << std::endl;
}


void STLSurf::setId()
{      
    id=count;
    ++count;
}

void STLSurf::addTriangle(Triangle t)
{
	tris.push_back(t);
	return;
}

std::string STLSurf::str()
{
	std::ostringstream o;
	o << "S"<< id <<"(N="<< tris.size() << ")";
	return o.str();
}

std::ostream &operator<<(std::ostream &stream, const STLSurf s)
{
  stream << "STLSurf(id=" << s.id << ", N="<< s.tris.size() <<")";
  return stream;
}
