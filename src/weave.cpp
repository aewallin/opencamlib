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
#include <boost/python.hpp>

#include "weave.h"


namespace ocl
{

Weave::Weave() {
    fibers.clear();
    xfibers.clear();
    yfibers.clear();
}

void Weave::addFiber(Fiber& f) {
     fibers.push_back(f);
}

void Weave::build() {
    // sort fibers into X and Y fibers
    BOOST_FOREACH( Fiber f, fibers ) {
        if ( f.dir.xParallel() && !f.ints.empty() ) {
            xfibers.push_back(f);
        } 
        if ( f.dir.yParallel() && !f.ints.empty() ) {
            yfibers.push_back(f);
        }
    }
    
    // process fibers
    bool first_only = false;
    BOOST_FOREACH( Fiber f, xfibers) {
        if ( !f.ints.empty() ) {
            std::cout << " processing xfiber "<< f << "\n";
            BOOST_FOREACH( Interval i, f.ints ) {
                std::cout <<" int=" << i.str() << "\n";
                double xmin = f.point(i.lower).x;
                double xmax = f.point(i.upper).x;
                std::cout << "  xmin=" << xmin << " xmax=" << xmax << "\n";
                // for this X-inteval, find potential y-fibers
                std::vector<Fiber> yhits;
                BOOST_FOREACH( Fiber yf, yfibers ) {
                    if ( (xmin <= yf.p1.x) && ( yf.p1.x <= xmax ) )
                        yhits.push_back(yf);
                }
                std::cout << " found " << yhits.size() << " potential y-fibers\n";
                
                // go through potential fibers, finding intersections
                BOOST_FOREACH( Fiber yf, yhits ) {
                    BOOST_FOREACH( Interval yi, yf.ints ) {
                        double ymin = yf.point(yi.lower).y;
                        double ymax = yf.point(yi.upper).y;
                        if ( (ymin <= f.p1.y) && (f.p1.y <= ymax) ) {
                            // X interval i on fiber f
                            // intersects with
                            // Y interval yi on fiber yf
                            // 1) add CL-points of X-fiber (if not already in graph)
                            //      (maintain a CL-point set which keeps track of this) 
                            // 2) add CL-points of Y-fiber (if not already in graph)
                            // 3) add intersection point (if not already in graph) will allways be new??
                            // 4) add edges. (if they provide new connections)
                            //      ycl_lower <-> intp <-> ycl_upper
                            //      xcl_lower <-> intp <-> xcl_upper
                            // if this connects points that are already connected, then remove old edge and
                            // provide this "via" connection
                            //               
                            VertexDescriptor  v;
                            v = boost::add_vertex(g);
                            boost::put( boost::vertex_name , g , v , f.point(i.lower) );
                            boost::put( boost::vertex_color , g , v , true );
                            
                            std::cout << " found intersection!\n";
                        }
                    }
                }
                
            }
            
            if (first_only) // debug mechanism...
                break;
        }
    }
     
}

void Weave::printGraph() const {
    std::cout << " number of vertices: " << boost::num_vertices( g ) << "\n";
    std::cout << " number of edges: " << boost::num_edges( g ) << "\n";
}


std::string Weave::str() const {
    std::ostringstream o;
    o << "Weave\n";
    o << "  " << fibers.size() << " fibers\n";
    o << "  " << xfibers.size() << " X-fibers\n";
    o << "  " << yfibers.size() << " Y-fibers\n";
    return o.str();
}




} // end namespace
// end file weave.cpp
