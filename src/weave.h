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
#ifndef WEAVE_H
#define WEAVE_H

#include <vector>

#include <boost/graph/adjacency_list.hpp> // graph class
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/python.hpp>

#include "point.h"
#include "ccpoint.h"
#include "numeric.h"
#include "fiber.h"
#include "weave_typedef.h"

namespace ocl
{

template <class NewGraph, class Tag>
struct graph_copier : public boost::base_visitor<graph_copier<NewGraph, Tag> >
{
        typedef Tag event_filter;
        graph_copier(NewGraph& graph) : new_g(graph) { }
        template <class Edge, class Graph>
        void operator()(Edge e, Graph& g) {
            boost::add_edge(boost::source(e, g), boost::target(e, g), new_g);
        } 
    private:
        NewGraph& new_g;
};


template <class NewGraph, class Tag>
inline graph_copier<NewGraph, Tag> 
copy_graph(NewGraph& g, Tag) {
  return graph_copier<NewGraph, Tag>(g);
}

typedef std::pair< std::size_t, Point > TimePointPair;
typedef std::pair< std::size_t, VertexDescriptor > TimeVertexPair;
typedef std::pair< double, VertexDescriptor > DistanceVertexPair;

bool TimeSortPredicate( const TimePointPair& lhs, const TimePointPair& rhs );
bool TimeSortPredicate2( const  TimeVertexPair& lhs, const  TimeVertexPair& rhs );
bool FirstSortPredicate( const  DistanceVertexPair& lhs, const  DistanceVertexPair& rhs );

// see weave_typedef.h for boost-graph classes                         
                    
/// weave-graph
class Weave {
    public:
        Weave();
        virtual ~Weave() {};
        void addFiber(Fiber& f);
        void build();
        void mark_adj_vertices();
        void order_points();
        std::vector<VertexDescriptor> get_neighbors(VertexDescriptor& source);
        VertexDescriptor get_next_vertex(VertexDescriptor& source);
        void split_components();
        
        void invert();
        void sort_fibers();
        unsigned int clpoints_size();
        std::vector<Fiber> fibers;
        std::vector<Fiber> xfibers;
        std::vector<Fiber> yfibers;
        std::string str() const;
        void printGraph() const;
        
        void writeGraph() const; // write to dot file
        // python debug/test interface:
        boost::python::list getCLPoints() const;
        boost::python::list getIPoints() const;
        boost::python::list getADJPoints() const;
        boost::python::list getEdges() const;
        boost::python::list getLoop() const;
        WeaveGraph g;
        std::vector<WeaveGraph> g_components;
        
        std::vector<Point> loop;
};



} // end namespace
#endif
// end file weave.h
