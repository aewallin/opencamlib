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


/// \brief STL surface.
/// STL surfaces consist of triangles. There is by definition no structure
/// or order among the triangles, i.e. they can be positioned or connected in arbitrary ways.
class STLSurf {
    public:
        /// Create an empty STL-surface
        STLSurf();
        ~STLSurf();
        
        /// read STL surface from file
        STLSurf(const std::wstring &stl_file_path);
                
        /// add Triangle t to this surface
        void addTriangle(const Triangle &t);

        std::string str();
        friend std::ostream &operator<<(std::ostream &stream, const STLSurf s);
        
        /// return number of triangles in surface
        int size();

        static int count;
        int id;
        /// list of Triangles in this surface
        std::list<Triangle> tris; 

    private:
        void setId();
        void read_from_file(const wchar_t* filepath);
};

#endif
