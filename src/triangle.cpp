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
//#include <stdio.h>
//#include <sstream>

#include <boost/foreach.hpp>

#include "triangle.h"
#include "point.h"

namespace ocl
{

int Triangle::count=0;

Triangle::Triangle()
{
    setId();
    p[0]=Point(1,0,0);
    p[1]=Point(0,1,0);
    p[2]=Point(0,0,1);
    calcNormal();
    calcBB();
}

Triangle::Triangle(Point p1, Point p2, Point p3)
{
    setId();
    p[0]=p1;
    p[1]=p2;
    p[2]=p3;
    //std::cout << "Triangle(p1,p2,p3) before calcNormal()\n";
    calcNormal();
    calcBB();
    //std::cout << "Triangle(p1,p2,p3) AFTER calcNormal()\n";
}

Triangle::~Triangle()
{
    //delete n;
    //n = 0;
}

/// return vertices in a list to python
boost::python::list Triangle::getPoints()
{
    boost::python::list plist;
    BOOST_FOREACH(Point vertex, p) {
        plist.append(vertex);
    }
    return plist;
}

/// calculate bounding box values
void Triangle::calcBB() {
    bb.addTriangle( *this );
    minx=bb.minpt.x;
    maxx=bb.maxpt.x;
    miny=bb.minpt.y;
    maxy=bb.maxpt.y;
    minz=bb.minpt.z;
    maxz=bb.maxpt.z;
}

void Triangle::calcNormal()
{
    Point v1=p[0]-p[1];
    Point v2=p[0]-p[2];
    
    // the normal is in the direction of the cross product between the edge vectors
    
    Point ntemp = v1.cross(v2); 
    ntemp.normalize(); // FIXME this might fail if norm()==0
     
    //std::cout << "creating normal=" << ntemp << "\n";
    n = new Point(ntemp.x,ntemp.y,ntemp.z);
    //std::cout << "normal is =" << *n << "\n";
    
}

void Triangle::setId()
{
    id=count;
    ++count;
}

std::string Triangle::str() const
{
    std::ostringstream o;
    o << *this;
    return o.str();
}


std::ostream &operator<<(std::ostream &stream, const Triangle t)
{
  stream <<  "T(" << t.id << ") " << t.p[0] << " " << t.p[1] << " " << t.p[2] <<  "n=" << *(t.n) ;
  return stream;
}

}  // end namespace
// end file triangle.cpp
