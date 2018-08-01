/*  $Id$
 * 
 *  Copyright (c) 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com).
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

#include <iostream>
#include <sstream>
#include <string>

#include "smart_weave.hpp"

namespace ocl
{

namespace weave
{

// given a VertexPair and an Interval, in the Interval find the Vertex above and below the given vertex
std::pair<Vertex,Vertex> SmartWeave::find_neighbor_vertices( VertexPair v_pair, Interval& ival, bool above_equality ) { 
    Interval::VertexPairIterator itr = ival.intersections2.lower_bound( v_pair ); // returns first that is not less than argument (equal or greater)
    assert( itr != ival.intersections2.end() ); // we must find a lower_bound
    Interval::VertexPairIterator v_above; 
    if ( above_equality ) 
        v_above = itr; // lower_bound returns one beyond the give key, i.e. what we want
    else {
        v_above = ++itr;
        --itr;
    }
    Interval::VertexPairIterator v_below = --itr; // this is the vertex below the given vertex
    std::pair<Vertex,Vertex> out;
    out.first = v_above->first; // vertex above v (xu)
    out.second = v_below->first; // vertex below v (xl)
    return out;
}


// this is the new smarter build() which uses less RAM
void SmartWeave::build() {
    std::cout << " SimpleWeave::build()... \n";
    
    // this adds all CL-vertices from x-intervals
    // it also populates the xi.intersections_fibers set of intersecting y-fibers
    // also add the first-crossing vertex and the last-crossing vertex
    
    //std::cout << " build2() add_vertices_x() ... " << std::flush ;
    add_vertices_x();
    //std::cout << " done.\n" << std::flush ;
    // the same for y-intervals, add all CL-points, and intersections to the set.
    //std::cout << " build2() add_vertices_y() ... " << std::flush ;
    add_vertices_y();
    //std::cout << " done.\n" << std::flush ;
    
    //std::cout << " build2() looping over xfibers ... " << std::flush ;
    BOOST_FOREACH( Fiber& xf, xfibers ) {
        std::vector<Interval>::iterator xi;
        for( xi = xf.ints.begin(); xi < xf.ints.end(); xi++ ) {
            std::set<std::vector<Fiber>::iterator>::const_iterator current, prev;
            if( xi->intersections_fibers.size() > 1 ) {
                current = xi->intersections_fibers.begin();
                prev = current++;
                for( ; current != xi->intersections_fibers.end(); current++ ) {
                    // for each x-interval, loop through the intersecting y-fibers
                    if( (*current - *prev) > 1 ) {
                        std::vector<Interval>::iterator yi = find_interval_crossing_x( xf, *(*prev + 1) );
                        add_vertex( xf, *(*prev + 1), xi , yi, FULLINT );
                        if( (*current - *prev) > 2 ) {
                            yi = find_interval_crossing_x( xf, *(*current - 1) );
                            add_vertex( xf, *(*current - 1), xi, yi, FULLINT );
                        }
                    }
                    prev = current;
                }
            }
        }
    }
    //std::cout << " done.\n" << std::flush ;
    
    //std::cout << " build2() looping over yfibers ... " << std::flush ;
    //std::cout << yfibers.size() << " fibers to loop over \n"<< std::flush ; 
    //int ny = 0;
    BOOST_FOREACH( Fiber& yf, yfibers ) {
        //std::cout << " fiber nr: " << ny++ << " has " << yf.ints.size() << " intervals\n" << std::flush ;
        std::vector<Interval>::iterator yi;
        //int ny_int=0;
        for( yi = yf.ints.begin(); yi < yf.ints.end(); yi++ ) {
            //std::cout << "  interval nr: " << ny_int++ << " has yi->intersections_fibers.size()= " << yi->intersections_fibers.size() << "\n" << std::flush;
            std::set<std::vector<Fiber>::iterator>::iterator current, prev;
            if( yi->intersections_fibers.size() > 1 ) {
                current = yi->intersections_fibers.begin();
                prev = current++;
                for( ; current != yi->intersections_fibers.end(); current++ ) {
                    if( (*current - *prev) > 1 ) {
                        std::vector<Interval>::iterator xi = find_interval_crossing_y( *(*prev + 1), yf );
                        add_vertex( *(*prev + 1), yf, xi , yi, FULLINT );
                        if( (*current - *prev) > 2 ) {
                            xi = find_interval_crossing_y( *(*current - 1), yf );
                            add_vertex( *(*current - 1), yf, xi, yi, FULLINT );
                        }
                    }
                    prev = current;
                }
            }
        }
    }
    //std::cout << " done.\n" << std::flush ;
    
    std::cout << " SmartWeave::build() add_all_edges()... " << std::flush ;
    add_all_edges();
    std::cout << " done.\n" << std::flush ;
}

// add a new CL-vertex to Weave, also adding it to the interval intersection-set, and to clVertices
Vertex SmartWeave::add_cl_vertex( const Point& position, Interval& ival, double ipos) {
    Vertex  v = g.add_vertex(); 
    g[v].position = position;
    g[v].type = CL;
    ival.intersections2.insert( VertexPair( v, ipos) ); // ?? this makes Interval depend on the WeaveGraph type
    clVertexSet.insert(v);
    return v;
}

void SmartWeave::add_vertices_x() {
    std::vector<Fiber>::iterator xf;
    for( xf = xfibers.begin(); xf < xfibers.end(); xf++ ) {
        std::vector<Interval>::iterator xi;
        for( xi = xf->ints.begin(); xi < xf->ints.end(); xi++ ) {
            // find first and last fiber crossing this interval
            std::vector<Fiber>::iterator yf = yfibers.begin();
            std::vector<Interval>::iterator yi;

            bool is_crossing = crossing_x( *yf, yi, *xi, *xf );
            while( (yf<yfibers.end()) && !is_crossing ) {// first crossing 
                yf++;
                if( yf<yfibers.end() ) is_crossing = crossing_x( *yf, yi, *xi, *xf );
            }

            if( yf < yfibers.end() ) {
                Point lower( xf->point( xi->lower ) );
                add_cl_vertex( lower, *xi, lower.x );
                Point upper( xf->point( xi->upper ) );
                add_cl_vertex( upper, *xi, upper.x );

                add_vertex( *xf, *yf, xi, yi, INT ); // the first crossing vertex
                xi->intersections_fibers.insert( yf );
                yi->intersections_fibers.insert( xf );

                is_crossing = crossing_x( *yf, yi, *xi, *xf );
                while( (yf<yfibers.end()) && is_crossing ) {// last crossing 
                    yf++;
                    if( yf<yfibers.end() ) is_crossing = crossing_x( *yf, yi, *xi, *xf );
                }
                add_vertex( *xf, *(--yf), xi, yi, INT ); // the last crossing vertex
                xi->intersections_fibers.insert( yf );
                yi->intersections_fibers.insert( xf );
            }
        }// end foreach x-interval
    }// end foreach x-fiber
}

void SmartWeave::add_vertices_y() {
    std::vector<Fiber>::iterator yf;
    for( yf = yfibers.begin(); yf < yfibers.end(); yf++ ) {
        std::vector<Interval>::iterator yi;
        for( yi = yf->ints.begin(); yi < yf->ints.end(); yi++ ) {
            std::vector<Fiber>::iterator xf = xfibers.begin();
            std::vector<Interval>::iterator xi;
            
            // find first and last fiber crossing this interval
            bool is_crossing = crossing_y( *xf, xi, *yi, *yf );
            while ( (xf<xfibers.end()) && !is_crossing ) {
                xf++;
                if( xf<xfibers.end() ) is_crossing = crossing_y( *xf, xi, *yi, *yf );
            }

            if( xf < xfibers.end() ) {
                Point lower( yf->point( yi->lower ) );
                add_cl_vertex( lower, *yi, lower.y );
                Point upper( yf->point( yi->upper ) );
                add_cl_vertex( upper, *yi, upper.y );

                if( add_vertex( *xf, *yf, xi, yi, INT ) ) { // add_vertex returns false if vertex already exists
                    xi->intersections_fibers.insert( yf );
                    yi->intersections_fibers.insert( xf );
                }

                bool is_crossing = crossing_y( *xf, xi, *yi, *yf );
                while ( (xf<xfibers.end()) && is_crossing ) { 
                    xf++;
                    if( xf<xfibers.end() ) is_crossing = crossing_y( *xf, xi, *yi, *yf );
                }
                if( add_vertex( *(--xf), *yf, xi, yi, INT ) ) {
                    xi->intersections_fibers.insert( yf );
                    yi->intersections_fibers.insert( xf );
                }
            }
        }// end foreach x-interval
    }// end foreach x-fiber
}

//crossing_x fiber
bool SmartWeave::crossing_x( Fiber& yf, std::vector<Interval>::iterator& yi, Interval& xi, Fiber& xf ) {
    //if the FIBER crosses the xi interval
    if( (yf.p1.x >= xf.point( xi.lower ).x) && (yf.p1.x <= xf.point( xi.upper ).x) ) {
        //for all the intervals of this y-fiber...
        std::vector<Interval>::iterator it;
        for( it = yf.ints.begin(); it < yf.ints.end(); it++ ) {
            //find the first INTERVAL which crosses our xi interval
            if( (yf.point( it->lower ).y <= xf.p1.y) && (yf.point( it->upper ).y >= xf.p1.y) ) {
                //save the y-interval iterator
                //and return true because we found an interval
                yi = it;
                return true;
            }
        }
        //return false, there is no y-interval on this y-fiber crossing our xi interval 
        return false;
    }
    //this y-fiber doesn't cross our xi interval
    else 
        return false;
}

//crossing_y fiber
bool SmartWeave::crossing_y( Fiber& xf, std::vector<Interval>::iterator& xi, Interval& yi, Fiber& yf ) {
    if( (xf.p1.y >= yf.point( yi.lower ).y) && (xf.p1.y <= yf.point( yi.upper ).y) ) {
        std::vector<Interval>::iterator it;
        for( it = xf.ints.begin(); it < xf.ints.end(); it++ ) {
            if( (xf.point( it->lower ).x <= yf.p1.x) && (xf.point( it->upper ).x >= yf.p1.x) ) {
                xi = it;
                return true;
            }
        }
        return false;
    }
    else 
        return false;
}

//find_interval_crossing_x
std::vector<Interval>::iterator SmartWeave::find_interval_crossing_x( Fiber& xf, Fiber& yf ) {
    std::vector<Interval>::iterator yi, xi;
    yi = yf.ints.begin();
    while( (yi<yf.ints.end()) && !crossing_y( xf, xi, *yi, yf ) ) 
        yi++;
    return yi;
}

//crossing_y interval
std::vector<Interval>::iterator SmartWeave::find_interval_crossing_y( Fiber& xf, Fiber& yf ) {
    std::vector<Interval>::iterator xi, yi;
    xi = xf.ints.begin();
    while( (xi<xf.ints.end()) && !crossing_x( yf, yi, *xi, xf ) ) 
        xi++;
    return xi;
}

//add_vertex
bool SmartWeave::add_vertex( Fiber& xf, Fiber& yf,
                        std::vector<Interval>::iterator xi, 
                        std::vector<Interval>::iterator yi,
                        enum VertexType type ) {
    //test if vertex exists
    BOOST_FOREACH( std::vector<Fiber>::iterator it_xf, yi->intersections_fibers ) {
        if( *it_xf == xf )
            return false;
    }
    Point v_position( yf.p1.x, xf.p1.y, xf.p1.z );
    Vertex v =g.add_vertex(); 
    g[v].position = v_position;
    g[v].type = type;
    g[v].xi= xi;
    g[v].yi= yi;
    xi->intersections2.insert( VertexPair( v, v_position.x ) );
    yi->intersections2.insert( VertexPair( v, v_position.y ) );
    return true;
}

//add_all_edges
void SmartWeave::add_all_edges()
{
    std::vector<Vertex> vertices = g.vertices(); 

    std::cout << "There are " << vertices.size() << " vertices.\n";
    BOOST_FOREACH( Vertex& vertex, vertices ) {
        if( (g[vertex].type == INT) || (g[vertex].type == FULLINT) ) {
            std::vector<Vertex>                adjacent_vertices;
            std::vector<Vertex>::iterator    adj_itr;
            std::vector<Edge>                in_edges, out_edges;
            std::vector<Edge>::iterator        in_edge_itr, out_edge_itr;

            Vertex x_u, x_l, y_u, y_l;
            boost::tie( x_u, x_l ) = find_neighbor_vertices( VertexPair(vertex, g[vertex].position.x), *(g[vertex].xi), false );
            boost::tie( y_u, y_l ) = find_neighbor_vertices( VertexPair(vertex, g[vertex].position.y), *(g[vertex].yi), false );
            
            adjacent_vertices.push_back( x_l );
            adjacent_vertices.push_back( y_u );
            adjacent_vertices.push_back( x_u );
            adjacent_vertices.push_back( y_l );

            for( adj_itr=adjacent_vertices.begin(); adj_itr<adjacent_vertices.end(); adj_itr++ ) {
                Edge in, out;
                if( g.has_edge( *adj_itr, vertex ) ) { 
                    in =  g.edge( *adj_itr, vertex );
                    out = g.edge( vertex, *adj_itr );
                    in_edges.push_back( in );
                    out_edges.push_back( out );
                }
                else {
                    in = g.add_edge( *adj_itr, vertex );
                    out = g.add_edge( vertex, *adj_itr );
                    in_edges.push_back( in );
                    out_edges.push_back( out );
                }

                if( g[*adj_itr].type == CL ) {
                    g[in].prev = out;
                    g[out].next = in;
                }
            }

            for(    in_edge_itr = in_edges.begin(),    out_edge_itr = out_edges.begin();
                    in_edge_itr < in_edges.end(),    out_edge_itr < out_edges.end();
                    in_edge_itr++,                    out_edge_itr++ )
            {
                if( in_edge_itr == in_edges.begin() ) {
                    g[*in_edge_itr].next = *(out_edge_itr+1);
                    g[*out_edge_itr].prev = *(in_edges.end()-1);
                }
                else if( in_edge_itr == (in_edges.end() - 1) ) {
                    g[*in_edge_itr].next = *(out_edges.begin());
                    g[*out_edge_itr].prev = *(in_edge_itr-1);
                }
                else {
                    g[*in_edge_itr].next = *(out_edge_itr+1);
                    g[*out_edge_itr].prev = *(in_edge_itr-1);
                }
            }
        }
        /*else if( g[vertex].type == FULLINT ) {
            std::vector<Vertex> adjacent_vertices;
            Vertex x_u, x_l, y_u, y_l;
            boost::tie( x_u, x_l ) = find_neighbor_vertices( VertexPair(vertex, g[vertex].position.x), *(g[vertex].xi), false );
            boost::tie( y_u, y_l ) = find_neighbor_vertices( VertexPair(vertex, g[vertex].position.y), *(g[vertex].yi), false );

            if( g[x_l].type == INT ) adjacent_vertices.push_back( x_l );
            if( g[y_u].type == INT ) adjacent_vertices.push_back( y_u );
            if( g[x_u].type == INT ) adjacent_vertices.push_back( x_u );
            if( g[y_l].type == INT ) adjacent_vertices.push_back( y_l );
            
            if( adjacent_vertices.size() == 1 ) {
                Edge in, out;
                std::vector<Vertex>::iterator adj_itr = adjacent_vertices.begin();

                if( hedi::has_edge( *adj_itr, vertex, g ) ) {
                    in  = hedi::edge( *adj_itr, vertex, g );
                    out = hedi::edge( vertex, *adj_itr, g );
                }
                else {
                    in  = hedi::add_edge( *adj_itr, vertex, g );
                    out = hedi::add_edge( vertex, *adj_itr, g );
                }
                g[in].next = out;
                g[out].prev = in;
            }
            else {
                std::vector<Vertex>::iterator adj_itr;
                std::vector<Edge> out_edges, in_edges;
                std::vector<Edge>::iterator edge_itr;

                for( adj_itr=adjacent_vertices.begin(); adj_itr<adjacent_vertices.end(); adj_itr++ ) {
                    if( !hedi::has_edge( *adj_itr, vertex, g ) ) {    
                        in_edges.push_back( hedi::add_edge( *adj_itr, vertex, g ) );
                        out_edges.push_back( hedi::add_edge( vertex, *adj_itr, g ) );
                    }
                    else {
                        in_edges.push_back( hedi::edge( *adj_itr, vertex, g ) );
                        out_edges.push_back( hedi::edge( vertex, *adj_itr, g ) );
                    }
                }

                for( unsigned int i=0; i<in_edges.size(); i++ ) {
                    if( i == (in_edges.size() - 1) ) {
                        g[in_edges[i] ].next = out_edges[ 0 ];
                        g[out_edges[i]].prev =  in_edges[i-1];
                    }
                    else if( i == 0 ) {
                        g[in_edges[i] ].next = out_edges[i+1];
                        g[out_edges[i]].prev =  in_edges[in_edges.size()-1];
                    }
                    else {
                        g[in_edges[i] ].next = out_edges[i+1];
                        g[out_edges[i]].prev =  in_edges[i-1];
                    }
                }
            }

        }*/
    }
}



} // end weave namespace

} // end ocl namespace
// end file smart_weave.cpp
