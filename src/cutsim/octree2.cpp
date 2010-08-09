/*  $Id: $
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


#include <list>
// uncomment to disable assert() calls
// #define NDEBUG
#include <cassert>

#include <boost/python.hpp>
#include <boost/foreach.hpp>

#include "point.h"
#include "triangle.h"
#include "millingcutter.h"
#include "numeric.h"
#include "octree2.h"
//#include "ocode.h"

//#define DEBUG_BUILD_OCT

namespace ocl
{

Point Octnode::direction[8] = {
                    Point( 1, 1,-1),
                    Point(-1, 1,-1),
                    Point(-1,-1,-1),
                    Point( 1,-1,-1),
                    Point( 1, 1, 1),
                    Point(-1, 1, 1),
                    Point(-1,-1, 1),
                    Point( 1,-1, 1)
                    };


//**************** LinOCT ********************/

Octnode::Octnode(Octnode* nodeparent, Point& centerp, double nodescale, unsigned int nodedepth) {
    parent = nodeparent;
    center = &centerp;
    scale = nodescale;
    depth = nodedepth;
    setvertices();
}

/// create 8 children of this node
void Octnode::subdivide() {
    for( int n=0;n<8;++n ) {
        Point childc = this->childcenter(n);
        child[n] = new Octnode( this, childc , scale/2 , depth+1 );
        // inherit the surface property here...
    }
}

/// return centerpoint of child n
Point Octnode::childcenter(int n) {
    return  *center + 0.5*scale * direction[n];
}
/// set the vertex positions
void Octnode::setvertices() {
    for ( int n=0;n<8;++n) {
        vertex[n] = new Point(*center + scale*direction[n] ) ;
    }
}


/// string repr
std::ostream& operator<<(std::ostream &stream, const Octnode &n) {
    stream << " Octnode: N=";     
    return stream;
}


/// string repr
std::string Octnode::str() const {
    std::ostringstream o;
    o << *this;
    return o.str();
}

} // end namespace
// end of file octree2.cpp
