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

#ifndef OCTREE_PY_H
#define OCTREE_PY_H

#include <boost/python.hpp>
#include <boost/foreach.hpp>

#include "octree.h"
#include "triangle_py.h"

namespace ocl
{

class Octree_py : public Octree {
    public:
        Octree_py() : Octree() {};
        Octree_py(double root_scale, unsigned int max_depth, Point& centerp) 
            : Octree(root_scale, max_depth, centerp) {};
        // python interface
        /// return python-list of leaf nodes
        boost::python::list py_get_leaf_nodes() const {
            std::vector<Octnode*> nodelist;
            Octree::get_leaf_nodes(root, nodelist);
            boost::python::list pynodelist;
            BOOST_FOREACH(Octnode* n, nodelist) {
                pynodelist.append( *n );
            }
            return pynodelist;
        };
        /// return python-list of marching-cubes triangles
        boost::python::list py_mc_triangles() {
            std::vector<Triangle> mc_triangles = mc();
            boost::python::list tlist;
            BOOST_FOREACH( Triangle t, mc_triangles ) {
                Triangle_py t2(t);
                tlist.append( t2 );
            }
            return tlist;
        };
        /// return python-list of side-trianges
        boost::python::list py_s_triangles() {
            std::vector<Triangle> s_triangles = side_triangles();
            boost::python::list tlist;
            BOOST_FOREACH( Triangle t, s_triangles ) {
                Triangle_py t2(t);
                tlist.append( t2 );
            }
            return tlist;
        };
};

} // end namespace
#endif
// end file octree_py.h
