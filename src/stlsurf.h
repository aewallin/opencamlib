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
#ifndef STLSURF_H
#define STLSURF_H

#include <list>

#include "point.h"
#include "triangle.h"
#include "kdtree.h"

namespace ocl
{
    


/// \brief STL surface, essentially an unordered list of Triangle objects
///
/// STL surfaces consist of triangles. There is by definition no structure
/// or order among the triangles, i.e. they can be positioned or connected in arbitrary ways.
class STLSurf {
    public:
        /// Create an empty STL-surface
        STLSurf();
        /// destructor
        ~STLSurf();
        
        /// read STL surface from file
        STLSurf(const std::wstring &stl_file_path);
                
        /// add Triangle t to this surface
        void addTriangle(const Triangle &t);
        
        /// string repr
        std::string str();
        /// string repr
        friend std::ostream &operator<<(std::ostream &stream, const STLSurf s);
        
        /// return number of triangles in surface
        int size();
        
        /// id-number count (obsolete?)
        static int count;
        /// id-number (obsolete?)
        int id;
        /// list of Triangles in this surface
        std::list<Triangle> tris; 
        
        /// build a kd-tree from the triangles in the surface
        void build_kdtree();
        
        /// jump up in the kd-tree
        int jump_kd_up();
        /// jump hi
        int jump_kd_hi();
        /// jumo lo
        int jump_kd_lo();
        /// set node = root
        void jump_kd_reset();
        /// return level of node
        int get_kd_level();
        /// return triangles in node
        boost::python::list get_kd_triangles();
        /// return cut-dim and value of node
        boost::python::list get_kd_cut();
        
        /// return list of triangles under cutter
        boost::python::list getTrianglesUnderCutter(Point &cl, MillingCutter &cutter);
        
        /// root of kd-tree
        KDNode *root;
        
        /// node of kd-tree
        KDNode *node;

    private:
        /// set the id-number
        void setId();
        /// read STL-surface from file
        void read_from_file(const wchar_t* filepath);
};

} // end namespace
#endif
// end file stlsurf.h
