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
#include <boost/graph/adjacency_list.hpp> 
#include <boost/graph/connected_components.hpp>
#include <boost/graph/planar_face_traversal.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>

#include "weave.h"
#include "pft_visitor.h"

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

void Weave::print_embedding(PlanarEmbedding& e) {
    unsigned int m = e.size();
    std::cout << " graph has " << boost::num_vertices(g) << " vertices\n";
    std::cout << " e has " << m << " rows\n";
    for (unsigned int i=0; i<m ; ++i ) {
        unsigned int N = e[i].size();
        std::cout << i << " : ";
        for (unsigned int j=0; j<N ; ++j) {
            std::cout << (e[i])[j] << " " ;
        }
        std::cout << "\n";
    }
}


void Weave::build_embedding(PlanarEmbedding& e) {
    e = PlanarEmbedding(boost::num_vertices(g)); // one row for each vertex
    // each row has adjacent edges for this vertex, in the correct order
    VertexIterator it, it_end;
    for( boost::tie(it, it_end)=boost::vertices(g); it!=it_end ; ++it ) {
        int idx = boost::get( boost::vertex_index, g, *it);
        Point v_pos = boost::get( boost::vertex_position, g, *it);
        OutEdgeIterator eit, eit_end;
        typedef std::pair< bool, EdgeDescriptor > BoolEdgePair;
        // edges can go in four directions: N, E, S, W
        std::vector< BoolEdgePair > ordered_edges(4, BoolEdgePair(false,  EdgeDescriptor() ) ); // store possible out-edges in a size=4 vector
        for ( boost::tie(eit,eit_end)=boost::out_edges(*it,g); eit!=eit_end ; ++eit) { // look at each edge connecting to this vertex
            VertexDescriptor adjacent = boost::target( *eit, g);
            Point adj_pos = boost::get( boost::vertex_position, g, adjacent);
            Point diff = v_pos-adj_pos;
            if (diff.y > 0) // in the N direction
                ordered_edges[0] = BoolEdgePair(true, *eit); // naive
            else if (diff.x > 0) // in the E direction
                ordered_edges[1] = BoolEdgePair(true, *eit); 
            else if (diff.y < 0) // in the S direction
                ordered_edges[2] = BoolEdgePair(true, *eit); 
            else if (diff.x < 0) // in the W direction
                ordered_edges[3] = BoolEdgePair(true, *eit); 
            else {
                std::cout << " diff.x="<<diff.x<<" diff.y="<<diff.y<<"\n";
                assert(0);
            }
        }
        BOOST_FOREACH( BoolEdgePair p, ordered_edges) {
            if (p.first)
                e[idx].push_back(p.second);
        }
    }
}
/*
 * This shows an example,using print_embedding() of planar emebddings of a simple graph generated with
 * the boost planarity test, or simply adding edges in the order of iteration, or by figuring
 * out the NESW, direction correctly:
 * boyer_myrvold_planarity_test
0 : (0,4) 
1 : (7,1) 
2 : (4,2) 
3 : (13,3) 
4 : (4,13) (4,7) (4,2) (0,4) 
5 : (7,5) 
6 : (7,6) 
7 : (7,5) (7,6) (7,1) (4,7) 
8 : (12,8) 
9 : (13,9) 
10 : (12,10) 
11 : (12,11) 
12 : (12,10) (12,11) (12,8) (13,12) 
13 : (13,3) (13,12) (13,9) (4,13) 

naive
0 : (0,4) 
1 : (1,7) 
2 : (2,4) 
3 : (3,13) 
4 : (4,0) (4,2) (4,7) (4,13) 
5 : (5,7) 
6 : (6,7) 
7 : (7,1) (7,4) (7,6) (7,5) 
8 : (8,12) 
9 : (9,13) 
10 : (10,12) 
11 : (11,12) 
12 : (12,8) (12,11) (12,10) (12,13) 
13 : (13,9) (13,12) (13,3) (13,4) 

correct:
0 : (0,4) 
1 : (1,7) 
2 : (2,4) 
3 : (3,13) 
4 : (4,2) (4,0) (4,13) (4,7) 
5 : (5,7) 
6 : (6,7) 
7 : (7,5) (7,4) (7,6) (7,1) 
8 : (8,12) 
9 : (9,13) 
10 : (10,12) 
11 : (11,12) 
12 : (12,10) (12,8) (12,11) (12,13) 
13 : (13,4) (13,12) (13,3) (13,9) 
 */

void Weave::face_traverse() {
    // void planar_face_traversal(const Graph& g, PlanarEmbedding embedding, PlanarFaceVisitor& visitor, EdgeIndexMap em);
    // std::cout << " face_traverse() \n";
    // Initialize the interior edge index
    boost::property_map<WeaveGraph, boost::edge_index_t>::type e_index = boost::get(boost::edge_index, g);
    boost::graph_traits<WeaveGraph>::edges_size_type edge_count = 0;
    EdgeIterator ei, ei_end;
    for(boost::tie(ei, ei_end) = boost::edges(g); ei != ei_end; ++ei)
        boost::put(e_index, *ei, edge_count++); // build an edge index
    
    // test for planarity and build embedding as side-effect
    PlanarEmbedding e(boost::num_vertices(g));
    build_embedding(e); // build a planar embedding
    std::cout << " planar_embedding() done.\n";
    //print_embedding(e);
    std::cout << " graph has " << boost::num_vertices(g) << " vertices \n";
    
    
    if ( boost::boyer_myrvold_planarity_test( boost::boyer_myrvold_params::graph = g,
                                   boost::boyer_myrvold_params::embedding = &e[0]) )
        std::cout << "Input graph is planar" << std::endl;
    else {
        std::cout << "Input graph is not planar" << std::endl;
        assert(0);
    }
    //print_embedding(e);
    
    //std::cout << std::endl << "Vertices on the faces: " << std::endl;
    
    vertex_output_visitor v_vis(this, g);
    std::cout << " calling planar_face_traversal() \n";
    boost::planar_face_traversal(g, &e[0], v_vis);
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
    //std::cout << " graph has " << N << " components\n";
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
        //std::cout << "comp " << m << " verts=" << boost::num_vertices(gcomp) << " edges=" << boost::num_edges(gcomp) << "\n";
        // now create an new Weave
        Weave* w = new Weave();
        w->g = gcomp;
        outw.push_back(*w);
    }
    return outw;
}

/// print out information about the graph
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

/// return loops as vector of vector<Point>
std::vector< std::vector<Point> > Weave::getLoops() const {
    std::vector< std::vector<Point> > loop_list;
    BOOST_FOREACH( std::vector<VertexDescriptor> loop, loops ) {
        std::vector<Point> point_list;
        BOOST_FOREACH( VertexDescriptor v, loop ) {
            Point p = boost::get( boost::vertex_position, g, v);
            point_list.push_back( p );
        }
        loop_list.push_back(point_list);
    }
    return loop_list;
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
