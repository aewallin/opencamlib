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
#ifndef WEAVE_TYPEDEF_H
#define WEAVE_TYPEDEF_H

#include "halfedgediagram.hpp"
#include "interval.hpp"

namespace ocl {

namespace weave {


typedef hedi::EdgeDescriptor Edge;

/// vertex type: CL-point, internal point, adjacent point
enum VertexType {CL, CL_DONE, ADJ, TWOADJ, INT, FULLINT};

/// vertex properties
struct VertexProps {
    VertexProps() {
        init();
    }
    /// construct vertex at position p with type t
    VertexProps( Point p, VertexType t) {
        position=p;
        type=t;
        init();
    }
    /// construct vertex at position p with type t
    VertexProps( Point p, VertexType t, std::vector<Interval>::iterator x, std::vector<Interval>::iterator y )
    : xi( x ), yi( y ) {
        position=p;
        type=t;
        init();
    }
    
    void init() {
        index = count;
        count++;
    }
    VertexType type = CL;
// HE data
    /// the position of the vertex
    Point position;
    /// index of vertex
    int index;
    /// global vertex count
    static int count;
    
    // x interval
    std::vector<Interval>::iterator xi;
    // y interval
    std::vector<Interval>::iterator yi;
    
};

/// edge properties
struct EdgeProps {
    EdgeProps() {}
    /// the next edge, counterclockwise, from this edge
    Edge next;
    /// previous edge, to make Weave::build() faster, since we avoid calling hedi::previous_edge() 
    Edge prev;
    /// the twin edge
    Edge twin;
};

typedef unsigned int Face; 

/// properties of a face in the weave
struct FaceProps {
    /// create face with given edge, generator, and type
    FaceProps( Edge e ) {
        edge = e;
    }
    /// face index
    Face idx;
    /// one edge that bounds this face
    Edge edge;
};

 
  
// The graph type for the weave.
typedef hedi::HEDIGraph<VertexProps, EdgeProps, FaceProps> WeaveGraph;
typedef WeaveGraph::Vertex Vertex;
typedef WeaveGraph::VertexItr VertexItr;

/// intersections between intervals are stored as a VertexPair
/// pair.first is a vertex descriptor of the weave graph
/// pair.second is the coordinate along the fiber of the intersection
typedef std::pair< Vertex, double > VertexPair;

/// compare based on pair.second, the coordinate of the intersection
struct VertexPairCompare {
    /// comparison operator
    bool operator() (const VertexPair& lhs, const VertexPair& rhs) const
    { return lhs.second < rhs.second ;}
};

/// intersections stored in this set (for rapid finding of neighbors etc)
typedef std::set< VertexPair, VertexPairCompare > VertexIntersectionSet;

typedef VertexIntersectionSet::iterator VertexPairIterator;    

} // end weave namespace

} // end ocl namespace
#endif
// end file weave_typedef.hpp
