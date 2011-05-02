/*  $Id$
 * 
 *  Copyright 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com)
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

#ifndef TSP_H
#define TSP_H

#include <iostream>
#include <vector>
#include <fstream>
#include <set>
#include <ctime>

#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/random.hpp>
#include <boost/timer.hpp>
#include <boost/integer_traits.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/simple_point.hpp>
#include <boost/graph/metric_tsp_approx.hpp>
// #include <boost/graph/graphviz.hpp>

#include <boost/python.hpp>

namespace ocl {

namespace tsp {

// based on metric tsp example:
// http://www.boost.org/doc/libs/1_46_1/libs/graph/test/metric_tsp_approx.cpp

/*
template<typename PointType>
struct cmpPnt {
    bool operator()(const boost::simple_point<PointType>& l, const boost::simple_point<PointType>& r) const
    { return (l.x > r.x); }
};
*/

//add edges to the graph (for each node connect it to all other nodes)
template< typename VertexListGraph, 
          typename PointContainer,
          typename WeightMap, 
          typename VertexIndexMap>
void connectAllEuclidean(VertexListGraph& g,
                        const PointContainer& points,
                        WeightMap wmap,            // Property maps passed by value
                        VertexIndexMap vmap,       // Property maps passed by value
                        int /*sz*/) // not used??
{
    using namespace boost;
    using namespace std;
    typedef typename graph_traits<VertexListGraph>::edge_descriptor Edge;
    typedef typename graph_traits<VertexListGraph>::vertex_iterator VItr;
    Edge e;
    bool inserted;
    pair<VItr, VItr> verts(vertices(g));
    for (VItr src(verts.first); src != verts.second; src++) {
        for (VItr dest(src); dest != verts.second; dest++) {
            if (dest != src) {
                double weight(sqrt(pow(
                    static_cast<double>(points[vmap[*src]].x -
                        points[vmap[*dest]].x), 2.0) +
                    pow(static_cast<double>(points[vmap[*dest]].y -
                        points[vmap[*src]].y), 2.0)));
                boost::tie(e, inserted) = add_edge(*src, *dest, g);
                wmap[e] = weight;
            }
        }
    }
}

class TSPSolver {
    typedef boost::adjacency_matrix< boost::undirectedS, 
                                     boost::no_property,
                                     boost::property< boost::edge_weight_t, double,
                                     boost::property< boost::edge_index_t, long unsigned int> >,
                                     boost::no_property 
                                    > Graph;
    typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
    typedef boost::graph_traits<Graph>::edge_descriptor Edge;
    typedef boost::property_map<Graph, boost::edge_weight_t>::type WeightMap;
    //typedef std::set< boost::simple_point<double>, cmpPnt<double> > PointSet;
    typedef std::vector< boost::simple_point<double> > PointSet;
    typedef std::vector< Vertex > Container;
    
public:
    TSPSolver () {}
    virtual ~TSPSolver() {
        if (g)
            delete g;
    }
    void run() {
        // add a vertex for each point
        //BOOST_FOREACH(boost::simple_point<double> pnt, points) {
        //    boost::add_vertex(g);
        //}
        //Graph g( points.size() );
        g = new Graph( points.size() );
        // connect all vertices;
        WeightMap weight_map = boost::get( boost::edge_weight, *g);
        std::vector< boost::simple_point<double> > point_vec(points.begin(), points.end());
        connectAllEuclidean( *g, point_vec, weight_map, boost::get( boost::vertex_index, *g), points.size() );
        //Container c;
        boost::timer t;
        double len = 0.0;
        // Run the TSP approx, creating the visitor on the fly.
        boost::metric_tsp_approx(*g, boost::make_tsp_tour_len_visitor(*g, std::back_inserter(output), len, weight_map));
        length = len;
        //std::cout << "Number of points: " << boost::num_vertices(*g) << std::endl;
        //std::cout << "Number of edges: " << boost::num_edges(*g) << std::endl;
        //std::cout << "Length of tour: " << len << std::endl;
        //std::cout << "vertices in tour: " << output.size() << std::endl;
        //std::cout << "Elapsed: " << t.elapsed() << std::endl;
        

        //delete g;
    }
    void addPoint(double x, double y) {
        boost::simple_point<double> pnt;
        pnt.x=x;
        pnt.y=y;
        points.push_back(pnt);
    }
    void reset() {
        output.clear();
        points.clear();
        if (g)
            delete g;
    }
    void printOutput() const {
        int n=0;
        BOOST_FOREACH( Vertex v, output ) {
            std::cout << n++ << " : " << v << "\n" ;
        }
    }
    double getLength() const {
        return length;
    }
    boost::python::list getOutput() const {
        boost::python::list plist;
        BOOST_FOREACH(Vertex v, output) {
            plist.append( v );
        }
        return plist;
    }
        
    
protected:

    Container output;
    PointSet points;
    Graph* g;
    double length;
};


} // end tsp namespace

} // end ocl namespace
#endif
