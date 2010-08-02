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
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/connected_components.hpp>
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

void Weave::add_vertex( Point& position, VertexType t, Interval& i, double ipos) {
    VertexDescriptor  v;
    v = boost::add_vertex(g);
    boost::put( boost::vertex_position , g , v , position );
    boost::put( boost::vertex_type , g , v , t );
    i.intersections.insert( VertexPair( v, ipos) );
}

/// this builds a BGL graph g by looking at xfibers and yfibers
void Weave::build() {
    // 1) add CL-points of X-fiber (if not already in graph)
    // 2) add CL-points of Y-fiber (if not already in graph)
    // 3) add intersection point (if not already in graph) (will allways be new??)
    // 4) add edges. (if they provide new connections)
    //      ycl_lower <-> intp <-> ycl_upper
    //      xcl_lower <-> intp <-> xcl_upper
    // if this connects points that are already connected, then remove old edge and
    // provide this "via" connection
    sort_fibers(); // fibers are sorted into xfibers and yfibers
    BOOST_FOREACH( Fiber& xf, xfibers) {
        if ( !xf.ints.empty() ) {
            BOOST_FOREACH( Interval& xi, xf.ints ) {
                double xmin = xf.point(xi.lower).x;
                double xmax = xf.point(xi.upper).x;
                if (!xi.in_weave) { // add the interval end-points to the weave
                    // 1) CL-points of X-fiber (check if already added)
                    Point p = xf.point(xi.lower);
                    add_vertex( p, CL , xi, p.x );
                    p = xf.point(xi.upper);
                    add_vertex( p, CL , xi, p.x );
                    xi.in_weave = true;
                }
                BOOST_FOREACH( Fiber& yf, yfibers ) {
                    if ( (xmin <= yf.p1.x) && ( yf.p1.x <= xmax ) ) {// potential intersection
                        BOOST_FOREACH( Interval& yi, yf.ints ) {
                            double ymin = yf.point(yi.lower).y;
                            double ymax = yf.point(yi.upper).y;
                            if ( (ymin <= xf.p1.y) && (xf.p1.y <= ymax) ) { // actual intersection
                                // X interval xi on fiber xf intersects with Y interval yi on fiber yf
                                // intersection is at ( yf.p1.x, xf.p1.y , xf.p1.z)

                                // 2) CL-points of Y-fiber (check if already added)
                                if (!yi.in_weave) {
                                    Point p = yf.point(yi.lower);
                                    add_vertex( p, CL , yi, p.y );
                                    p = yf.point(yi.upper);
                                    add_vertex( p, CL , yi, p.y );
                                    yi.in_weave = true;
                                }
                                // 3) intersection point (this is always new, no need to check for existence??)
                                VertexDescriptor  v;
                                v = boost::add_vertex(g);
                                Point v_position = Point( yf.p1.x, xf.p1.y , xf.p1.z) ;
                                boost::put( boost::vertex_position , g , v , v_position ); 
                                boost::put( boost::vertex_type , g , v , INT ); // internal vertex
                                xi.intersections.insert( VertexPair( v, v_position.x ) );
                                yi.intersections.insert( VertexPair( v, v_position.y ) );
                                
                                // 4) add edges 
                                // find the X-vertices above and below the new vertex v.
                                VertexPair v_pair_x( v , v_position.x );
                                VertexPairIterator x_tmp = xi.intersections.lower_bound( v_pair_x );
                                assert(x_tmp != xi.intersections.end() );
                                VertexPairIterator x_above = x_tmp;
                                VertexPairIterator x_below = x_tmp;
                                ++x_above;
                                --x_below;
                                // if x_above and x_below are already connected, we need to remove that edge.
                                boost::remove_edge( x_above->first, x_below->first, g);
                                boost::add_edge( x_above->first , v , g );
                                boost::add_edge( x_below->first , v , g );
                                
                                // now do the same thing in the y-direction.
                                VertexPair v_pair_y( v , v_position.y );
                                VertexPairIterator y_tmp = yi.intersections.lower_bound( v_pair_y );
                                assert(y_tmp != yi.intersections.end() );
                                VertexPairIterator y_above = y_tmp;
                                VertexPairIterator y_below = y_tmp;
                                ++y_above;
                                --y_below;
                                // if y_above and y_below are already connected, we need to remove that edge.
                                boost::remove_edge( y_above->first, y_below->first, g);
                                boost::add_edge( y_above->first , v , g );
                                boost::add_edge( y_below->first , v , g );
                            }
                        } // end y interval loop
                    } // end if(potential intersection)
                } // end y fiber loop
            } // x interval loop
        } // end if( x-interval empty)
    } // end X-fiber loop
     
}

/// assuming that the graph is built,
/// find out the number of components,
/// and split and return a list of the the unconnected compontents
std::vector<Weave> Weave::split_components() {
    typedef boost::property_map< WeaveGraph, boost::vertex_component_t>::type ComponentMap;
    ComponentMap comp_map = boost::get( boost::vertex_component, g);
    //std::vector<int> component( boost::num_vertices(g) );
    std::size_t N = boost::connected_components( g, comp_map );
    std::cout << " graph has " << N << " components\n";
    WeaveGraph gcomp;
    std::vector<Weave> outw;
    for( unsigned int m=0;m<N;++m) {
        gcomp = g; // copy everything into g_components
        VertexIterator it, it_end;
        boost::tie( it, it_end ) = boost::vertices( gcomp );
        for(  ; it != it_end ; ++it ) {
            std::size_t v_comp = boost::get( boost::vertex_component, gcomp, *it); // get component number
            if ( v_comp != m ) {
                boost::clear_vertex( *it , gcomp ); // this removes all edges
                boost::put( boost::vertex_type, gcomp, *it, INT); // mark INT, so we don't start at a false CL-point
            }
        }
        std::cout << "comp " << m << " verts=" << boost::num_vertices(gcomp) << " edges=" << boost::num_edges(gcomp) << "\n";
        // now create an new Weave
        Weave* w = new Weave();
        w->g = gcomp;
        outw.push_back(*w);
    }
    return outw;
}

/// split the weave into components and return a list
/// of sub-weaves to python
boost::python::list Weave::get_components() {
    boost::python::list wlist;
    std::vector<Weave> weaves = split_components();
    BOOST_FOREACH( Weave w, weaves ) {
        wlist.append( w );
    }
    return wlist;
}


void Weave::add_loop_edges() {
    // find and add edges that make up the toolpath loop
    VertexIterator it, it_end;
    boost::tie( it, it_end ) = boost::vertices( g );
    std::vector<VertexDescriptor> mark_cl;
    for(  ; it != it_end ; ++it ) {
        if ( boost::get( boost::vertex_type, g, *it) == ADJ ) {
            // ADJ vertices that have two CL-neighbors
            AdjacencyIterator adj_it, adj_end;
            boost::tie(adj_it, adj_end) = boost::adjacent_vertices(*it, g);
            std::vector<VertexDescriptor> cln; // adjacent cl-points
            std::vector<VertexDescriptor> adjn; // adjacent adj-points
            for (  ; adj_it != adj_end ; ++adj_it ) {
                if ( boost::get( boost::vertex_type, g, *adj_it) == CL ) {
                    cln.push_back(*adj_it);
                } else if ( boost::get( boost::vertex_type, g, *adj_it) == ADJ ) {
                    adjn.push_back(*adj_it);
                }
            }
            if (cln.size()==2) { 
                bool added;
                EdgeDescriptor e;
                boost::tie( e, added ) = boost::add_edge( cln[0] , cln[1], g);
                boost::put( boost::edge_color, g, e, true);
                mark_cl.push_back(*it);
            } 
        }
    }
    BOOST_FOREACH( VertexDescriptor v, mark_cl) {
        boost::put( boost::vertex_type, g, v, CL);
    }
}

/// return a vector of vertices which are closest to source
std::vector<VertexDescriptor> Weave::get_neighbors(VertexDescriptor& source) {
    typedef boost::property_map< WeaveGraph, boost::vertex_distance_t>::type dist_map_t;
    dist_map_t dmap = boost::get( boost::vertex_distance, g);
    WeaveGraph G_copy( boost::num_vertices( g ) );
    std::vector<VertexDescriptor> p( boost::num_vertices(g) );
    
    boost::breadth_first_search( g, 
                                 source ,
                                 boost::visitor( 
                                 boost::make_bfs_visitor(
                                    std::make_pair( boost::record_distances(dmap, boost::on_tree_edge() )  ,
                                        std::make_pair(boost::record_predecessors( &p[0] , boost::on_tree_edge() ),
                                                        copy_graph(G_copy, boost::on_examine_edge())
                                                       ) 
                                                   )
                                    )
                                 )
                                 );
                                 
    // put all CL-points in a list
    VertexIterator it_begin, it_end, it, first;
    boost::tie( it_begin, it_end ) = boost::vertices( g );
    std::vector<TimeVertexPair> neighbors;
    for( it = it_begin ; it != it_end ; ++it ) {
        if ( boost::get( boost::vertex_type , g , *it ) == CL  ) {
            TimeVertexPair pair( boost::get( boost::vertex_distance, g, *it) ,*it);
            neighbors.push_back( pair );
        }
    }
    // sort the neigbors 
    std::sort( neighbors.begin(), neighbors.end(), TimeSortPredicate2);
    
    std::vector<VertexDescriptor> nearest_neighbors;
    std::vector<TimeVertexPair>::iterator pi;
    VertexSize min_dist = boost::get( boost::vertex_distance, g, neighbors[0].second );
    //std::cout << "the min time is=" << min_dist << "\n";
    
    for( pi=neighbors.begin() ; pi!=neighbors.end() ; ++pi) {
        if ( pi->first == min_dist )
            nearest_neighbors.push_back( pi->second );
    }
    //std::cout << nearest_neighbors.size() << " nn at dist=" << min_time << "\n";
    return nearest_neighbors;
}

/// given the source vertex, find the next vertex
VertexDescriptor Weave::get_next_vertex(VertexDescriptor& source) {
    std::vector<VertexDescriptor> nn = get_neighbors(source);
    if (nn.size() == 1) {// this is the easy case
        boost::put( boost::vertex_type, g, nn[0], CL_DONE);
        return nn[0];
    } else { // the harder case
        // choose based on euclidean distance??
        Point sp = boost::get( boost::vertex_position, g, source);
        
        std::vector<DistanceVertexPair> nvector;
        BOOST_FOREACH( VertexDescriptor v, nn) {
            Point p = boost::get( boost::vertex_position, g, v);
            // std::cout << " dist to" << p << " is " << (sp-p).norm() << "\n";
            nvector.push_back( DistanceVertexPair( (sp-p).norm() , v ) );
        }
        // sort the list
        std::sort( nvector.begin(), nvector.end(), FirstSortPredicate);
        //std::cout << " choosing dist=" << nvector[0].first << " \n";
        boost::put( boost::vertex_type, g, nvector[0].second, CL_DONE);
        return nvector[0].second; 
    }
}

/// count the number of cl-points in the weave
unsigned int Weave::clpoints_size() {
    VertexIterator it_begin, it_end, it, first;
    boost::tie( it_begin, it_end ) = boost::vertices( g );
    unsigned int ncl=0;
    for( it = it_begin ; it != it_end ; ++it ) {
        if ( boost::get( boost::vertex_type , g , *it ) == CL ) {
            ++ncl;
        }
    }
    return ncl;
}

/// find a first CL point, and call get_next_vertex() until we are done
void Weave::order_points() {
    // find a first CL-point
    VertexIterator it_begin, it_end, it, first;
    boost::tie( it_begin, it_end ) = boost::vertices( g );

    // put all CL-points into a vector
    std::vector<VertexDescriptor> clpts;
    for( it = it_begin ; it != it_end ; ++it ) {
        if ( boost::get( boost::vertex_type , g , *it ) == CL ) {
            clpts.push_back( *it );
        }
    }
    VertexDescriptor startvertex = clpts[0];
    loop.push_back(boost::get ( boost::vertex_position, g, startvertex ) );
    boost::put( boost::vertex_type, g, startvertex, CL_DONE); 
       
    VertexDescriptor nextvertex;
    VertexDescriptor currentvertex = startvertex;
    int niterations=0;
    while ( clpoints_size()>0 ) {
        //std::cout << " finding neigbors from " << currentvertex << " \n ";
        nextvertex = get_next_vertex(currentvertex);
        // << nextvertex << "\n";
        loop.push_back( boost::get( boost::vertex_position, g, nextvertex  ) );
        currentvertex = nextvertex;
        ++niterations;
    }
    
}


bool TimeSortPredicate2( const  TimeVertexPair& lhs, const  TimeVertexPair& rhs ) {
    return lhs.first < rhs.first;
}
bool FirstSortPredicate( const  DistanceVertexPair& lhs, const  DistanceVertexPair& rhs ) {
    return lhs.first < rhs.first;
}

void Weave::mark_adj_vertices() {
    // go through the vertices and mark the ones that connect to CL-points
    // as being of type ADJ
    VertexIterator it_begin, it_end, it;
    boost::tie( it_begin, it_end ) = boost::vertices( g );
    for( it = it_begin ; it != it_end ; ++it ) {
        AdjacencyIterator adj_begin, adj_end, adj_itr;
        boost::tie( adj_begin, adj_end ) = boost::adjacent_vertices( *it , g );
        bool has_CL_adjacent = false;
        for ( adj_itr = adj_begin ; adj_itr != adj_end ; ++adj_itr ) {
            if ( boost::get( boost::vertex_type , g , *adj_itr ) == CL )
                has_CL_adjacent = true;
        }
        if (has_CL_adjacent && (boost::get( boost::vertex_type , g , *it ) == INT) )
            boost::put( boost::vertex_type, g , *it , ADJ);
    }
}




void Weave::printGraph() const {
    std::cout << " number of vertices: " << boost::num_vertices( g ) << "\n";
    std::cout << " number of edges: " << boost::num_edges( g ) << "\n";
    VertexIterator it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( g );
    int n=0, n_cl=0, n_internal=0;
    
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        if ( boost::get( boost::vertex_type, g, *itr ) == CL )
            ++n_cl;
        else
            ++n_internal;
        ++n;
    }
    std::cout << " counted " << n << " vertices\n";
    std::cout << "          CL-nodes: " << n_cl << "\n";
    std::cout << "    internal-nodes: " << n_internal << "\n";
}

/// return CL-points to python
boost::python::list Weave::getCLPoints() const {
    boost::python::list plist;
    VertexIterator it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( g );
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        if ( boost::get( boost::vertex_type, g, *itr ) == CL ) // a CL-point
            plist.append( boost::get( boost::vertex_position, g, *itr ) );
    }
    return plist;
}

/// return the internal points of the weave to python
boost::python::list Weave::getIPoints() const {
    boost::python::list plist;
    VertexIterator it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( g );
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        if ( boost::get( boost::vertex_type, g, *itr ) == INT ) 
            plist.append( boost::get( boost::vertex_position, g, *itr ) );
    }
    return plist;
}

/// return the ADJ points of the weave to python
boost::python::list Weave::getADJPoints() const {
    boost::python::list plist;
    VertexIterator it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( g );
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        if ( boost::get( boost::vertex_type, g, *itr ) == ADJ ) 
            plist.append( boost::get( boost::vertex_position, g, *itr ) );
    }
    return plist;
}


/// put all edges in a list of lists for output to python
/// format is [ [p1,p2] , [p3,p4] , ... ]
boost::python::list Weave::getEdges() const {
    boost::python::list edge_list;
    EdgeIterator it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::edges( g );
    for ( itr=it_begin ; itr != it_end ; ++itr ) { // loop through each edge
        if ( ! boost::get( boost::edge_color, g, *itr ) ) {
            boost::python::list point_list; // the endpoints of each edge
            VertexDescriptor v1 = boost::source( *itr, g  );
            VertexDescriptor v2 = boost::target( *itr, g  );
            Point p1 = boost::get( boost::vertex_position, g, v1 );
            Point p2 = boost::get( boost::vertex_position, g, v2 );
            point_list.append(p1);
            point_list.append(p2);
            edge_list.append(point_list);
        }
    }
    return edge_list;
}


boost::python::list Weave::getCLEdges() const {
    boost::python::list edge_list;
    EdgeIterator it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::edges( g );
    for ( itr=it_begin ; itr != it_end ; ++itr ) { // loop through each edge
        if ( boost::get( boost::edge_color, g, *itr ) ) {
            boost::python::list point_list; // the endpoints of each edge
            VertexDescriptor v1 = boost::source( *itr, g  );
            VertexDescriptor v2 = boost::target( *itr, g  );
            Point p1 = boost::get( boost::vertex_position, g, v1 );
            Point p2 = boost::get( boost::vertex_position, g, v2 );
            point_list.append(p1);
            point_list.append(p2);
            edge_list.append(point_list);
        }
    }
    return edge_list;
}

/// output points from variable this->loop to python
boost::python::list Weave::getLoop() const {
    boost::python::list plist;
    BOOST_FOREACH( Point p, loop ) {
        plist.append( p );
    }
    return plist;
}
        

std::string Weave::str() const {
    std::ostringstream o;
    o << "Weave\n";
    o << "  " << fibers.size() << " fibers\n";
    o << "  " << xfibers.size() << " X-fibers\n";
    o << "  " << yfibers.size() << " Y-fibers\n";
    return o.str();
}

/// experimental graphviz output. FIXME
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


} // end namespace
// end file weave.cpp
