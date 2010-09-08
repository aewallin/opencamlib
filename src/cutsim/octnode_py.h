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

#ifndef OCTNODE_PY_H
#define OCTNODE_PY_H

#include <boost/python.hpp>

#include "octnode.h"

namespace ocl
{

class Octnode_py : public Octnode {
    public:
        Octnode_py() : Octnode() {};
        /// create suboctant idx of parent with scale nodescale and depth nodedepth
        Octnode_py(Octnode* parent, unsigned int idx, double nodescale, unsigned int nodedepth)
            : Octnode(parent,idx,nodescale,nodedepth) {};
        
        /// return center of this node
        Point py_get_center() const {
            return *center;
        };
        /// return vertices to python
        boost::python::list py_get_vertices() const {
            boost::python::list vlist;
            for ( int n=0;n<8;++n) 
                vlist.append( *(vertex[n]) );
            return vlist;
        };
};

} // end namespace
#endif
// end file octnode_py.h
