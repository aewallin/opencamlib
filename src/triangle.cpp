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
    calcNormal();
    calcBB();
}

Triangle::~Triangle()
{
    //delete n;
    //n = 0;
}

/// return vertices in a list to python
boost::python::list Triangle::getPoints() const
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
    
    // consider re-writing this behavior: (?)
    minx=bb.minpt.x;
    maxx=bb.maxpt.x;
    
    miny=bb.minpt.y;
    maxy=bb.maxpt.y;
    
    minz=bb.minpt.z;
    maxz=bb.maxpt.z;
}

void Triangle::calcNormal() {
    Point v1=p[0]-p[1];
    Point v2=p[0]-p[2];
    // the normal is in the direction of the cross product between the edge vectors
    Point ntemp = v1.cross(v2); 
    ntemp.normalize(); // FIXME this might fail if norm()==0
    n = new Point(ntemp.x,ntemp.y,ntemp.z);
}

std::vector<Triangle>* Triangle::zslice(const double zcut) const {
    std::vector<Triangle>* tlist = new std::vector<Triangle>();
    if ( (zcut < this->bb.minpt.z) || ((zcut > this->bb.maxpt.z)) ) {
        //std::cout << "zcut outside bbox, nothing to do.\n";
        tlist->push_back(*this);
        return tlist;
    } else {
        // find out how many vertices are below zcut
        std::vector<Point> below;
        std::vector<Point> above;
        for (int m=0;m<3;++m) {
            if ( p[m].z <= zcut )
                below.push_back(p[m]);
            else
                above.push_back(p[m]);
        }
        //std::cout << below.size() << " vertices below zcut\n";
        assert( (below.size() == 1) || (below.size() == 2) );
        
        if ( below.size() == 2 ) {
            assert( above.size() == 1 );
            // find two new intersection points 
            // edge is p1 + t*(p2-p1) = zcut
            // so t = zcut-p1 / (p2-p1)
            double t1 = (zcut - above[0].z) / (below[0].z - above[0].z); // div by zero?!
            double t2 = (zcut - above[0].z) / (below[1].z - above[0].z);
            Point p1 = above[0];
            Point p2 = above[0] + t1*(below[0] - above[0]);
            Point p3 = above[0] + t2*(below[1] - above[0]);
            tlist->push_back( Triangle( p1 , p2 , p3) );
            return tlist;
            
        } else if ( below.size() == 1 ) {
            assert( above.size() == 2 );
            // find intersection points and add two new triangles
            // t = (zcut -p1) / (p2-p1)
            double t1 = (zcut - above[0].z) / (below[0].z - above[0].z); 
            double t2 = (zcut - above[1].z) / (below[0].z - above[1].z);
            Point p1 = above[0] + t1*(below[0]-above[0]); 
            Point p2 = above[1] + t2*(below[0]-above[1]);
            Point p3 = above[0];
            Point p4 = above[1];
            tlist->push_back( Triangle(p1,p2,p3) );
            tlist->push_back( Triangle(p1,p3,p4) );
            return tlist;
        } else {
            assert(0);
        }
        
    }
    
    return tlist;
}

boost::python::list Triangle::getZslice(const double zcut) const {
    boost::python::list py_tlist;
    std::vector<Triangle>* tlist;
    tlist = this->zslice(zcut);
    BOOST_FOREACH(Triangle t, *tlist) {
        py_tlist.append(t);
    }
    return py_tlist;
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
