/*  $Id$
 * 
 *  Copyright (c) 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com).
 *  
 *  This file is part of OpenCAMlib 
 *  (see https://github.com/aewallin/opencamlib).
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include <cassert>

#include <boost/foreach.hpp>

#include "triangle.hpp"
#include "numeric.hpp"

namespace ocl
{

Triangle::Triangle() {
    p[0]=Point(1,0,0);
    p[1]=Point(0,1,0);
    p[2]=Point(0,0,1);
    calcNormal();
    calcBB();
}

Triangle::Triangle(Point p1, Point p2, Point p3) {
    p[0]=p1;
    p[1]=p2;
    p[2]=p3;
    calcNormal();
    calcBB();
}

Triangle::Triangle(const Triangle &t) {
    p[0]=t.p[0];
    p[1]=t.p[1];
    p[2]=t.p[2];
    calcNormal();
    calcBB();
}
 



/// calculate bounding box values
void Triangle::calcBB() {
    bb.clear();
    bb.addTriangle( *this );
}

/// calculate, normalize, and set the Triangle normal
void Triangle::calcNormal() {
    Point v1=p[0]-p[1];
    Point v2=p[0]-p[2];
    Point ntemp = v1.cross(v2);  // the normal is in the direction of the cross product between the edge vectors
    ntemp.normalize(); // FIXME this might fail if norm()==0
    n = Point(ntemp.x,ntemp.y,ntemp.z);
}

Point Triangle::upNormal() const {
    return (n.z < 0) ? -1.0* n : n; 
}

bool Triangle::zslice_verts(Point& p1, Point& p2, double zcut) const {
    if ( (zcut <= this->bb.minpt.z) || ((zcut >= this->bb.maxpt.z)) )
        return false; // no zslice
    // find out how many vertices are below zcut
    std::vector<Point> below;
    std::vector<Point> above;
    for (int m=0;m<3;++m) {
        if ( p[m].z <= zcut )
            below.push_back(p[m]);
        else
            above.push_back(p[m]);
    }
    if ( !(below.size() == 1) && !(below.size() == 2) ) {
        std::cout << "triangle.cpp: zslice_verts() error while trying to z-slice\n";
        std::cout << " triangle=" << *this << "\n";
        std::cout << " zcut=" << zcut << "\n";
        std::cout << above.size() << " above points:\n";
        BOOST_FOREACH(Point p, above) {
            std::cout << "   " << p << "\n";
        }
        std::cout << below.size() << " below points:\n";
        BOOST_FOREACH(Point p, below) {
            std::cout << "   " << p << "\n";
        }
    }
    assert( (below.size() == 1) || (below.size() == 2) );
    
    if ( below.size() == 2 ) {
        assert( above.size() == 1 );
        // find two new intersection points 
        // edge is p1 + t*(p2-p1) = zcut
        // so t = zcut-p1 / (p2-p1)
        double t1 = (zcut - above[0].z) / (below[0].z - above[0].z); // div by zero?!
        double t2 = (zcut - above[0].z) / (below[1].z - above[0].z);
        p1 = above[0] + t1*(below[0] - above[0]);
        p2 = above[0] + t2*(below[1] - above[0]);
        return true;
    } else if ( below.size() == 1 ) {
        assert( above.size() == 2 );
        // find intersection points and add two new triangles
        // t = (zcut -p1) / (p2-p1)
        double t1 = (zcut - above[0].z) / (below[0].z - above[0].z); 
        double t2 = (zcut - above[1].z) / (below[0].z - above[1].z);
        p1 = above[0] + t1*(below[0]-above[0]); 
        p2 = above[1] + t2*(below[0]-above[1]);
        return true;
    } else {
        assert(0);
        return false;
    }
    
}

void Triangle::rotate(double xr, double yr, double zr) {
    for (int n=0;n<3;++n) {
        p[n].xRotate(xr);
        p[n].yRotate(yr);
        p[n].zRotate(zr);
    }
    calcNormal();
    calcBB();
}

std::ostream &operator<<(std::ostream &stream, const Triangle t) {
  stream <<  "T: " << t.p[0] << " " << t.p[1] << " " << t.p[2] <<  "n=" << t.n ;
  return stream;
}

}  // end namespace
// end file triangle.cpp
