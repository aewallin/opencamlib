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

#include <fstream>  // required by read_from_file()
#include <list>

#include <boost/foreach.hpp>

#include "point.h"
#include "triangle.h"
#include "stlsurf.h"

namespace ocl
{
    

int STLSurf::count=0;

STLSurf::STLSurf() {
    setId();
}



STLSurf::~STLSurf() {
    //delete tris;
}

void STLSurf::setId() {      
    id=count;
    ++count;
}

void STLSurf::addTriangle(const Triangle &t) {
    tris.push_back(t);
    bb.addTriangle(t);
    return;
}

int STLSurf::size()
{
    return tris.size();
}

void STLSurf::build_kdtree()
{
    root = KDNode::build_kdtree( &tris );
}

int STLSurf::get_kd_level() 
{
    return node->level;
}

int STLSurf::jump_kd_up()
{
    if (node->up) {
        node = node->up;
        return 1;
    } else {
        return 0;
    }
}

int STLSurf::jump_kd_hi()
{
    if (node->hi) {
        node = node->hi;
        return 1;
    } else {
        return 0;
    }
}

int STLSurf::jump_kd_lo()
{
    if (node->lo) {
        node = node->lo;
        return 1;
    } else {
        return 0;
    }
}

void STLSurf::jump_kd_reset() {
    node = root;
}

boost::python::list STLSurf::get_kd_cut()
{
    boost::python::list cutlist;
    cutlist.append( node->dim );
    cutlist.append( node->cutval );
    return cutlist;
}


boost::python::list STLSurf::get_kd_triangles()
{
    boost::python::list trilist;
    std::list<Triangle> *triangles = new std::list<Triangle>();
    KDNode::getTriangles( triangles, node);
    BOOST_FOREACH(Triangle t, *triangles) {
        trilist.append(t);
    }
    return trilist;
}

boost::python::list STLSurf::getTrianglesUnderCutter(const Point &cl, const MillingCutter &cutter) const {
    boost::python::list trilist;
    std::list<Triangle> *triangles_under_cutter = new std::list<Triangle>();
    KDNode::search_kdtree( triangles_under_cutter, cl, cutter, root);
    BOOST_FOREACH(Triangle t, *triangles_under_cutter) {
        trilist.append(t);
    }
    delete triangles_under_cutter;
    return trilist;
}

boost::python::list STLSurf::getBounds() const
{
    boost::python::list bounds;
    bounds.append( bb.minpt.x );
    bounds.append( bb.maxpt.x );
    bounds.append( bb.minpt.y );
    bounds.append( bb.maxpt.y );
    bounds.append( bb.minpt.z );
    bounds.append( bb.maxpt.z );
    return bounds;
}

std::string STLSurf::str() const {
    std::ostringstream o;
    o << *this;
    return o.str();
}

std::ostream &operator<<(std::ostream &stream, const STLSurf s)
{
  stream << "STLSurf(id=" << s.id << ", N="<< s.tris.size() <<")";
  return stream;
}









} // end namespace
// end file stlsurf.cpp
