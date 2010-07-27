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

void Weave::sort_fibers() {
    // sort fibers into X and Y fibers
    xfibers.clear();
    yfibers.clear();
    
    BOOST_FOREACH( Fiber f, fibers ) {
        if ( f.dir.xParallel() && !f.ints.empty() ) {
            xfibers.push_back(f);
        } 
        if ( f.dir.yParallel() && !f.ints.empty() ) {
            yfibers.push_back(f);
        }
    }    
}

void Weave::add_xy_fibers_to_g2() {
    /*
    BOOST_FOREACH( Fiber& f, xfibers ) {
        if (!f.ints.empty() ) {
            BOOST_FOREACH(Interval& i, f.ints) {
                InvVertexDescriptor v;
                v = boost::add_vertex(g2);
                boost::put( boost::vertex_name , g2 , v , i );
                boost::put( boost::vertex_color , g2 , v , true ); //x-intervals are "true"
            }
        }
    }
    BOOST_FOREACH( Fiber& f, yfibers ) {
        if (!f.ints.empty() ) {
            BOOST_FOREACH(Interval& i, f.ints) {
                VertexDescriptor v;
                v = boost::add_vertex(g2);
                boost::put( boost::vertex_name , g2 , v , i );
                boost::put( boost::vertex_color , g2 , v , false ); //y-intervals are "false"
            }
        }
    }*/
}

void Weave::build2() {
    /*
    sort_fibers();
    // put all intervals into g2 as vertices
    add_xy_fibers_to_g2();
    // loop through the x-intervals, finding intesections with y-intervals
    // and adding an edge for each intersection
    InvVertexIterator it_begin, it_end, itrX, itrY;
    boost::tie( it_begin, it_end ) = boost::vertices( g2 );
    for ( itrX=it_begin ; itrX != it_end ; ++itrX ) {
        if ( boost::get( boost::vertex_color, g2, *itrX ) ) { // an X-interval
            Interval xint = boost::get( boost::vertex_name, g2, *itrX );
            double xmin = xint.point(xint.lower).x;
            double xmax = xint.point(xint.upper).x;
            // loop through all Y-intervals    
            for ( itrY=it_begin ; itrY != it_end ; ++itrY ) {
                if ( !boost::get( boost::vertex_color, g2, *itrY ) ) { // an Y-interval
                    Interval yint = boost::get( boost::vertex_name, g2, *itrY );
                    // check for intersection
                    double ymin = yint.point(yint.lower).y;
                    double ymax = yint.point(yint.upper).y;
                    //std::cout << xmin << xmax << ymin << ymax << "\n";
                    if ( (xmin <= yint.p1.x) && ( yint.p1.x <= xmax ) ) { // xint overlaps with y-int x-coord
                        if ( (ymin <= xint.p1.y) && (xint.p1.y <= ymax) ) { // yint overlaps x-int y-coord
                            // add edge
                            InvEdgeDescriptor e;
                            bool success;
                            boost::tie( e, success ) = boost::add_edge(*itrX, *itrY, g2);
                            boost::put( boost::edge_weight, g2, e, Point(yint.p1.x, xint.p1.y, xint.p1.z) );
                        }
                    } 
                }
            }
        }
    }
    */
}

void Weave::invert() {
    /*
    // in g2 each vertex is an interval
    // each edge connects the interval to another interval
    // x-intervals connect only to y-intervals
    // y-intervals connect only to x-intervals
    InvVertexIterator it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( g2 );
    for ( itr=it_begin ; itr != it_end ; ++itr ) { // go through each vertex
        // add all CL-points to g, and connect them
        Interval interval = boost::get( boost::vertex_name, g2, *itr );
        VertexDescriptor v1 = boost::add_vertex( g );
        VertexDescriptor v2 = boost::add_vertex( g );
        boost::put( boost::vertex_name , g , v1 , interval.point(interval.upper) );
        boost::put( boost::vertex_name , g , v2 , interval.point(interval.lower) );
        boost::put( boost::vertex_color , g , v1 , CL );
        boost::put( boost::vertex_color , g , v2 , CL );
        boost::add_edge( v1 , v2 , g );
    }
    
    // edges in g2 connect fibers to eachother
    // for each edge in g2 a new vertex should be inserted into g
    // the vertex should lie on the fiber  
    InvEdgeIterator ite_begin, ite_end, iter;
    boost::tie( ite_begin, ite_end ) = boost::edges( g2 );
    for ( iter = ite_begin ; iter != ite_end ; ++iter) { // process each edge in g2
        VertexDescriptor v = boost::add_vertex( g );
        boost::put( boost::vertex_color , g , v , INT );
        Point p = boost::get( boost::edge_weight, g2, *iter); // this is the intersection Point
        boost::put( boost::vertex_name , g , v , p );
    }
    */
}

void Weave::build() {

    sort_fibers(); // fibers are sorted into xfibers and yfibers
    
    
    bool first_only = false; // for debugging only
    
    BOOST_FOREACH( Fiber& xf, xfibers) {
        if ( !xf.ints.empty() ) {
            BOOST_FOREACH( Interval& xi, xf.ints ) {
                double xmin = xf.point(xi.lower).x;
                double xmax = xf.point(xi.upper).x;
                if (!xi.in_weave) {
                    xi.vert_lower = boost::add_vertex(g);
                    xi.vert_upper = boost::add_vertex(g);
                    boost::put( boost::vertex_name , g , xi.vert_lower , xf.point(xi.lower) );
                    boost::put( boost::vertex_name , g , xi.vert_upper , xf.point(xi.upper) );
                    boost::put( boost::vertex_color , g , xi.vert_lower , CL );
                    boost::put( boost::vertex_color , g , xi.vert_upper, CL );
                    xi.in_weave = true;
                }
                BOOST_FOREACH( Fiber& yf, yfibers ) {
                    if ( (xmin <= yf.p1.x) && ( yf.p1.x <= xmax ) ) {// potential intersection
                        BOOST_FOREACH( Interval& yi, yf.ints ) {
                            double ymin = yf.point(yi.lower).y;
                            double ymax = yf.point(yi.upper).y;
                            if ( (ymin <= xf.p1.y) && (xf.p1.y <= ymax) ) {
                                // X interval xi on fiber xf intersects with Y interval yi on fiber yf
                                // intersection is at ( yf.p1.x, xf.p1.y )
                                // 1) add CL-points of X-fiber (if not already in graph)
                                // 2) add CL-points of Y-fiber (if not already in graph)
                                // 3) add intersection point (if not already in graph) (will allways be new??)
                                // 4) add edges. (if they provide new connections)
                                //      ycl_lower <-> intp <-> ycl_upper
                                //      xcl_lower <-> intp <-> xcl_upper
                                // if this connects points that are already connected, then remove old edge and
                                // provide this "via" connection
                                // 5) remove internal nodes connecting only to non-CL nodes
                                //     'false' node which connects only to 'false' nodes
                                // 6) graph-search to find CL-point order 

                                // 1) CL-points of X-fiber (check if already added, if so, retrieve vertex)

                                // 2) CL-points of Y-fiber (check if already added, if so, retrieve vertex)
                                if (!yi.in_weave) {
                                    //VertexDescriptor  yp1, yp2;
                                    yi.vert_lower = boost::add_vertex(g);
                                    yi.vert_upper = boost::add_vertex(g);
                                    boost::put( boost::vertex_name , g , yi.vert_lower , yf.point(yi.lower) );
                                    boost::put( boost::vertex_name , g , yi.vert_upper , yf.point(yi.upper) );
                                    boost::put( boost::vertex_color , g , yi.vert_lower , CL );
                                    boost::put( boost::vertex_color , g , yi.vert_upper , CL );
                                    yi.in_weave = true;
                                }
                                // 3) intersection point (this is always new, no need to check for existence)
                                VertexDescriptor  v;
                                v = boost::add_vertex(g);
                                boost::put( boost::vertex_name , g , v , Point( yf.p1.x, xf.p1.y , xf.p1.z) ); 
                                boost::put( boost::vertex_color , g , v , INT );
                                // 4) add edges (FIXME)
                                boost::add_edge( xi.vert_lower , v , g );
                                boost::add_edge( xi.vert_upper , v , g );
                                boost::add_edge( yi.vert_lower , v , g );
                                boost::add_edge( yi.vert_upper , v , g );
                                
                                //std::cout << " found intersection!\n";
                            }
                        } // end y interval loon
                    } // end if(potential intersection)
                } // end y fiber loop
                
            } // x interval loop
            
            if (first_only) // debug mechanism...
                break;
        } // end if( x-interval empty)
    } // end X-fiber loop
     
}

void Weave::printGraph() const {
    std::cout << " number of vertices: " << boost::num_vertices( g ) << "\n";
    std::cout << " number of edges: " << boost::num_edges( g ) << "\n";
    VertexIterator it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( g );
    int n=0, n_cl=0, n_internal=0;
    
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        if ( boost::get( boost::vertex_color, g, *itr ) == CL )
            ++n_cl;
        else
            ++n_internal;
        ++n;
    }
    std::cout << " counted " << n << " vertices\n";
    std::cout << "          CL-nodes: " << n_cl << "\n";
    std::cout << "    internal-nodes: " << n_internal << "\n";
}

void Weave::printGraph2() const {
    /*
    std::cout << " number of vertices: " << boost::num_vertices( g2 ) << "\n";
    std::cout << " number of edges: " << boost::num_edges( g2 ) << "\n";
    InvVertexIterator it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( g2 );
    int n=0, nx=0, ny=0;
    
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        if ( boost::get( boost::vertex_color, g2, *itr ) )
            ++nx;
        else
            ++ny;
        ++n;
    }
    std::cout << " counted " << n << " vertices\n";
    std::cout << "          x-nodes: " << nx << "\n";
    std::cout << "          y-nodes: " << ny << "\n";
    */
}


void Weave::writeGraph() const {
    typedef boost::GraphvizGraph gvGraph;
    typedef boost::graph_traits< gvGraph >::vertex_descriptor gvVertex;

    gvGraph g2;
    // edge attributes:    std::map< std::string , std::string >
    //boost::property_map< gvGraph , boost::vertex_attribute_t >::type& 
    //    vertex_attr_map = boost::get( boost::vertex_attribute, g2);

    // write vertices/edges of g to g2
    gvVertex v;
    //boost::property< boost::vertex_name_t, int > vprop(2); 
    v = boost::add_vertex( g2 );
    //vertex_attr_map[v]["name"]="test";
    //boost::put( boost::vertex_property , g2 , v , "v name one" ); //FIXME...
    //boost::put( boost::vertex_name , g2 , v , vprop );
    boost::write_graphviz("weave.dot", g2);
}

boost::python::list Weave::getCLPoints() const {
    boost::python::list plist;
    VertexIterator it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( g );
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        if ( boost::get( boost::vertex_color, g, *itr ) == CL ) // a CL-point
            plist.append( boost::get( boost::vertex_name, g, *itr ) );
    }
    return plist;
}
        
boost::python::list Weave::getIPoints() const {
    boost::python::list plist;
    VertexIterator it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( g );
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        if ( boost::get( boost::vertex_color, g, *itr ) != CL ) // a CL-point
            plist.append( boost::get( boost::vertex_name, g, *itr ) );
    }
    return plist;
}

boost::python::list Weave::getEdges() const {
    boost::python::list edge_list;
    EdgeIterator it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::edges( g );
    for ( itr=it_begin ; itr != it_end ; ++itr ) { // loop through each edge
        boost::python::list point_list; // the endpoints of each edge
        VertexDescriptor v1 = boost::source( *itr, g  );
        VertexDescriptor v2 = boost::target( *itr, g  );
        Point p1 = boost::get( boost::vertex_name, g, v1 );
        Point p2 = boost::get( boost::vertex_name, g, v2 );
        point_list.append(p1);
        point_list.append(p2);
        edge_list.append(point_list);
    }
    return edge_list;
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
