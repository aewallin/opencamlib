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



#include "path.hpp"

namespace ocl
{

Path::Path() {
}

Path::Path(const Path &p) {
}

Path::~Path() {
}

void Path::append(const Line &l) {
        span_list.push_back(new LineSpan(l));
}

void Path::append(const Arc &a) {
        span_list.push_back(new ArcSpan(a));
}

} // end namespace
// end file path.cpp
