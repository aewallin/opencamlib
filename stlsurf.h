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
#ifndef STLSURF_H
#define STLSURF_H

#include "point.h"
#include "triangle.h"
#include <list>

///
/// \brief STL surface
///
class STLSurf {
	public:
		STLSurf();
        ~STLSurf();
        /// add Triangle t to this surface
        void addTriangle(const Triangle &t);
		std::string str();
		friend std::ostream &operator<<(std::ostream &stream, const STLSurf s);
		
        static int count;
        int id;
        /// list of Triangles in this surface
        std::list<Triangle> tris; 
        
    private:
        void setId();
};

#endif
