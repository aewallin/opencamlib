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
    loops.clear();
}

void Weave::addFiber(Fiber& f) {
     fibers.push_back(f);
}

void Weave::sort_fibers() {
    // sort fibers into X and Y fibers
    xfibers.clear();
    yfibers.clear();
    BOOST_FOREACH( Fiber f, fibers ) {
        if ( f.dir.xParallel() && !f.empty() ) {
            xfibers.push_back(f);
        } 
        if ( f.dir.yParallel() && !f.empty() ) {
            yfibers.push_back(f);
        }
    }    
}

void Weave::add_vertex( Point& position, WeaveVertexType t, Interval& i, double ipos) {
    WeaveVertex  v = boost::add_vertex(g);
    //boost::put( boost::vertex_position , g , v , position ); // 
    g[v].position = position;
    //boost::put( boost::vertex_type , g , v , t );            // 
    g[v].type = t;
    i.intersections.insert( VertexPair( v, ipos) );
}

void Weave::print_embedding(WeavePlanarEmbedding& e) {
    unsigned int m = e.size();
    std::cout << " graph has " << boost::num_vertices(g) << " vertices\n";
    std::cout << " e has " << m << " rows\n";
    for (unsigned int i=0; i<m ; ++i ) {
        unsigned int N = e[i].size();
        std::cout << i << " : "; // vertex index
        for (unsigned int j=0; j<N ; ++j) {
            std::cout << (e[i])[j] << " " ; // edges connecting to this vertex, in the right order
        }
        std::cout << "\n";
    }
}


void Weave::build_embedding(WeavePlanarEmbedding& e) {
    e = WeavePlanarEmbedding( boost::num_vertices(g) ); // one row for each vertex
    // each row has adjacent edges for this vertex, in the correct order
    VertexIterator it, it_end;
    for( boost::tie(it, it_end)=boost::vertices(g); it!=it_end ; ++it ) {
        //int idx = boost::get( boost::vertex_index, g, *it); // 
        int vertexIndex = g[*it].index;
        //Point vertexPos = boost::get( boost::vertex_position, g, *it); // 
        Point vertexPos = g[*it].position;
        OutEdgeIterator edgeItr, eit_end;
        typedef std::pair< bool, WeaveEdge > BoolEdgePair;
        // edges can go in four directions: N, E, S, W
        std::vector< BoolEdgePair > ordered_edges(4, BoolEdgePair(false,  WeaveEdge() ) ); // store possible out-edges in a size=4 vector
        for ( boost::tie(edgeItr,eit_end)=boost::out_edges(*it,g); edgeItr!=eit_end ; ++edgeItr) { // look at each edge connecting to this vertex
            WeaveVertex targetVertex = boost::target( *edgeItr, g);
            WeaveVertex source = boost::source( *edgeItr, g);
            assert( *it == source );
            
            //Point targetPos = boost::get( boost::vertex_position, g, targetVertex); // 
            Point targetPos = g[targetVertex].position;
            Point diff = vertexPos - targetPos;
            if (diff.y > 0) // in the N direction
                ordered_edges[0] = BoolEdgePair(true, *edgeItr); 
            else if (diff.x > 0) // in the E direction
                ordered_edges[1] = BoolEdgePair(true, *edgeItr); 
            else if (diff.y < 0) // in the S direction
                ordered_edges[2] = BoolEdgePair(true, *edgeItr); 
            else if (diff.x < 0) // in the W direction
                ordered_edges[3] = BoolEdgePair(true, *edgeItr); 
            else {
                std::cout << " diff.x="<<diff.x<<" diff.y="<<diff.y<<"\n";
                assert(0);
            }
        }
        BOOST_FOREACH( BoolEdgePair p, ordered_edges) {
            if (p.first)
                e[vertexIndex].push_back(p.second);
        }
    }
}

bool Weave::isPlanar() const {
    WeavePlanarEmbedding e_tmp(boost::num_vertices(g));
    if ( boost::boyer_myrvold_planarity_test( boost::boyer_myrvold_params::graph = g,
                                   boost::boyer_myrvold_params::embedding = &e_tmp[0]) ) { // NOTE: This will build a new WRONG planar embedding of the graph!!
        std::cout << "Input graph is planar" << std::endl;
        return true;
    } else {
        std::cout << "Input graph is not planar" << std::endl;
        return false;
    }
}

void Weave::face_traverse() {
    // Initialize the interior edge index
    boost::property_map<WeaveGraph, boost::edge_index_t>::type e_index = boost::get(boost::edge_index, g);
    boost::graph_traits<WeaveGraph>::edges_size_type edge_count = 0;
    EdgeIterator ei, ei_end;
    for(boost::tie(ei, ei_end) = boost::edges(g); ei != ei_end; ++ei)
        boost::put(e_index, *ei, edge_count++); // build an edge index
    
    // initialize the vertex index
    std::size_t vertex_count = 0;
    VertexIterator vertexItr, vertexEnd;
    for(boost::tie(vertexItr, vertexEnd) = boost::vertices(g); vertexItr != vertexEnd; ++vertexItr)
        g[*vertexItr].index = vertex_count++;
        
        //boost::put(e_index, *ei, edge_count++); // build an edge index
    
    // build a planar embedding   
    assert( isPlanar() ); 
    WeavePlanarEmbedding embedding(boost::num_vertices(g));
    build_embedding(embedding); 
        
    std::cout << " edge index built \n";
    vertex_output_visitor vertex_visitor(this, g);
    // void planar_face_traversal(const Graph& g, PlanarEmbedding embedding, PlanarFaceVisitor& visitor, EdgeIndexMap em);
    boost::planar_face_traversal(g, &embedding[0], vertex_visitor);
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
        assert( !xf.empty() ); // sort_fibers() ensures no empty fibers remain
        BOOST_FOREACH( Interval& xi, xf.ints ) {
            double xmin = xf.point(xi.lower).x;
            double xmax = xf.point(xi.upper).x;
            assert( !xi.in_weave );
            // add the interval end-points to the weave
            Point p = xf.point(xi.lower);
            add_vertex( p, CL , xi, p.x ); // add_vertex( Point& position, VertexType t, Interval& i, double ipos)
            p = xf.point(xi.upper);
            add_vertex( p, CL , xi, p.x );

            BOOST_FOREACH( Fiber& yf, yfibers ) {
                if ( (xmin <= yf.p1.x) && ( yf.p1.x <= xmax ) ) {// potential intersection
                    BOOST_FOREACH( Interval& yi, yf.ints ) {
                        double ymin = yf.point(yi.lower).y;
                        double ymax = yf.point(yi.upper).y;
                        if ( (ymin <= xf.p1.y) && (xf.p1.y <= ymax) ) { // actual intersection
                            // X interval xi on fiber xf intersects with Y interval yi on fiber yf
                            // intersection is at ( yf.p1.x, xf.p1.y , xf.p1.z)
                            if (!yi.in_weave) { // add y-interval endpoints to weave
                                Point p = yf.point(yi.lower);
                                add_vertex( p, CL , yi, p.y );
                                p = yf.point(yi.upper);
                                add_vertex( p, CL , yi, p.y );
                                yi.in_weave = true;
                            }
                            // 3) intersection point (this is always new, no need to check for existence??)
                            WeaveVertex  v;
                            v = boost::add_vertex(g);
                            Point v_position = Point( yf.p1.x, xf.p1.y , xf.p1.z) ;
                            g[v].position = v_position;
                            g[v].type = INT;
                            //boost::put( boost::vertex_position , g , v , v_position ); 
                            //boost::put( boost::vertex_type , g , v , INT ); // internal vertex
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
       
    } // end X-fiber loop
     
}

/// assuming that the graph is built,
/// find out the number of components,
/// and split and return a list of the the unconnected compontents
std::vector<Weave> Weave::split_components() {
    //typedef boost::property_map< WeaveGraph, int WeaveVertexProps::* >::type ComponentMap;
    //ComponentMap comp_map = boost::get( &WeaveVertexProps::component , g);

    std::size_t N = boost::connected_components( g, boost::get( &WeaveVertexProps::component , g) );

    WeaveGraph gcomp;
    std::vector<Weave> outw;
    for( unsigned int m=0;m<N;++m) {
        gcomp = g; // copy everything into gcomp
        VertexIterator it, it_end;
        boost::tie( it, it_end ) = boost::vertices( gcomp );
        for(  ; it != it_end ; ++it ) {
            //std::size_t v_comp = boost::get( boost::vertex_component, gcomp, *it); // get component number
            std::size_t v_comp = gcomp[*it].component;
            if ( v_comp != m ) {
                boost::clear_vertex( *it , gcomp ); // this removes all edges
                //boost::put( boost::vertex_type, gcomp, *it, INT); // mark INT, so we don't start at a false CL-point
                gcomp[*it].type = INT;
            }
        }
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
        //if ( boost::get( boost::vertex_type, g, *itr ) == CL ) // 
        if ( g[*itr].type == CL )
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
    BOOST_FOREACH( std::vector<WeaveVertex> loop, loops ) {
        std::vector<Point> point_list;
        BOOST_FOREACH( WeaveVertex v, loop ) {
            //Point p = boost::get( boost::vertex_position, g, v); // 
            Point p = g[v].position;
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
