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
#ifndef CLSURFACE_H
#define CLSURFACE_H

#include <vector>

#include "halfedgediagram.hpp"
#include "operation.hpp"
#include "point.hpp"

namespace ocl
{

namespace clsurf
{





typedef boost::adjacency_list_traits<boost::listS, 
                                     boost::listS, 
                                     boost::bidirectionalS, 
                                     boost::listS >::edge_descriptor CLSEdge;


typedef boost::adjacency_list_traits<boost::listS, 
                                     boost::listS, 
                                     boost::bidirectionalS, 
                                     boost::listS >::vertex_descriptor CLSVertex;
//typedef CLSGraph::Edge CLSEdge;
//typedef CLSGraph::Vertex CLSVertex;
typedef unsigned int CLSFace;
//typedef hedi::HEDIGraph::Face CLSFace;

typedef std::vector<CLSVertex> CLSVertexVector;
typedef std::vector<CLSFace> CLSFaceVector;
typedef std::vector<CLSEdge> CLSEdgeVector;  


struct CLSVertexProps {
    CLSVertexProps() {
        init();
    }
    /// construct vertex at position p with type t
    CLSVertexProps( Point p) {
        position=p;
        init();
    }
    void init() {
        index = count;
        count++;
    }
// HE data
    Point position;     ///< the position of the vertex
    int index; ///< index of vertex
    static int count; ///< global vertex count
};

int CLSVertexProps::count = 0;

struct CLSEdgeProps {
    CLSEdgeProps() {}
    /// create edge with given next, twin, and face
    CLSEdgeProps(CLSEdge n, CLSEdge t, CLSFace f) { 
        next = n;
        twin = t;
        face = f;
    }
    CLSEdge next; ///< the next edge, counterclockwise, from this edge
    CLSEdge twin; ///< the twin edge
    CLSFace face;  ///< the face to which this edge belongs
};

/// properties of a face 
struct CLSFaceProps {
    /// create face with given edge, generator, and type
    CLSFaceProps() {}
    CLSFaceProps( CLSEdge e ) {
        edge = e;
    }
    
    CLSFace idx; ///< face index
    CLSEdge edge;   ///< one edge that bounds this face

};


// the cutter location surface graph
typedef hedi::HEDIGraph<  boost::listS,             // out-edge storage
                          boost::listS,             // vertex set storage
                          boost::bidirectionalS,    // bidirectional graph.
                          CLSVertexProps,           // vertex properties
                          CLSEdgeProps,             // edge properties
                          CLSFaceProps,             // face properties
                          boost::no_property,       // graph properties
                          boost::listS              // edge storage
                          > CLSGraph;



/// \brief cutter location surface.
///
/// 1) start with a square sized like the bounding-box of the surface
/// 2) recursively subdivide until we reach sampling
/// 3) run drop cutter to project the surface
/// 4) adaptively subdivide until min_sampling where required
/// 5) do something:
///    - constant step-over (propagating geodesic windows on square grid is easy?)
///    - slicing (?)
///    - classify into steep/flat
///    - use for identifying rest-machining areas?
///
///    geodesic papers: "Fast Exact and Approximate Geodesics on Meshes"
///    doi 10.1145/1073204.1073228
///     http://research.microsoft.com/en-us/um/people/hoppe/geodesics.pdf
///
///    Accurate Computation of Geodesic Distance Fields for Polygonal Curves on Triangle Meshes
///    http://www.graphics.rwth-aachen.de/uploads/media/bommes_07_VMV_01.pdf
///
class CutterLocationSurface : public Operation {
    public:
        CutterLocationSurface() {
            far=1.0;
            init();
        }
        /// create diagram with given far-radius and number of bins
        CutterLocationSurface(double f) {
            far=f;
            init();
        }
        virtual ~CutterLocationSurface() {  
        }
        
        void init() {
            // initialize cl-surf
            //
            //    b  e1   a
            //    e2      e4
            //    c   e3  d
            CLSVertex a = g.add_vertex(); // VertexProps( Point(far,far,0) ), g);
            g[a].position = Point(far,far,0);
            CLSVertex b = g.add_vertex(); // VertexProps( Point(-far,far,0) ), g);
            g[b].position = Point(-far,far,0);
            CLSVertex c = g.add_vertex(); // VertexProps( Point(-far,-far,0) ), g);
            g[c].position = Point(-far,-far,0);
            CLSVertex d = g.add_vertex(); // VertexProps( Point(far,-far,0) ), g);
            g[c].position = Point(far,-far,0);
            CLSFace f_outer= g.add_face( );
            CLSFace f_inner= g.add_face( );
            CLSEdge e1 =  g.add_edge(a , b );
            CLSEdge e1t = g.add_edge(b , a );
            CLSEdge e2 =  g.add_edge(b , c );
            CLSEdge e2t = g.add_edge(c , b );
            CLSEdge e3 =  g.add_edge(c , d );
            CLSEdge e3t = g.add_edge(d , c );
            CLSEdge e4 =  g.add_edge(d , a );
            CLSEdge e4t = g.add_edge(a , d );
            
            g[f_inner].edge = e1;
            g[f_outer].edge = e1t;
            out_face = f_outer;
            // twin edges
            g.twin_edges(e1,e1t);
            g.twin_edges(e2,e2t);
            g.twin_edges(e3,e3t);
            g.twin_edges(e4,e4t);
            // inner face:
            g[e1].face = f_inner;
            g[e2].face = f_inner;
            g[e3].face = f_inner;
            g[e4].face = f_inner;
            // outer face:
            g[e1t].face = f_outer;
            g[e2t].face = f_outer;
            g[e3t].face = f_outer;
            g[e4t].face = f_outer;
            // next-pointers:
            g[e1].next = e2;
            g[e2].next = e3;
            g[e3].next = e4;
            g[e4].next = e1;
            // outer next-pointers:
            g[e1t].next = e4t;
            g[e4t].next = e3t;
            g[e3t].next = e2t;
            g[e2t].next = e1t;
            
            subdivide();
    
        }
        
        void subdivide() {
            for( CLSFace f=0; f< g.num_faces() ; ++f ) {
                // subdivide each face
                if ( f!= out_face ) {
                    subdivide_face(f);
                }
            }
        }
        
        void subdivide_face(CLSFace f) {
            CLSEdgeVector f_edges = g.face_edges(f);
            assert( f_edges.size() == 4 );
            CLSVertex center = g.add_vertex();
            for (CLSEdge e : f_edges) {
                CLSVertex src = g.source(e);
                CLSVertex trg = g.target(e);
                // new vertex at mid-point of each edge
                Point mid = 0.5*(g[src].position+g[trg].position);
                g[center].position += 0.25*g[src].position; // average of four corners
                CLSVertex v = g.add_vertex();
                g[v].position = mid;
                g.insert_vertex_in_edge(v,e); // this also removes the old edges...
            }
            // now loop through edges again:
            f_edges = g.face_edges(f);
            assert( f_edges.size() == 8 );
            // for (CLSEdge e : f_edges) {
            //     std::cout << e << "\n";
            // }
        }
        
        virtual void run() {
        }
        void setMinSampling(double s) {
            min_sampling=s;
        }
        
    // PYTHON
        std::vector<Point> getVertices()
        {
            std::vector<Point> vertices;
            for (CLSVertex v : g.vertices()) {
                vertices.push_back(g[v].position);
            }
            return vertices;
        }

        std::vector<std::pair<Point, Point>> getEdges()
        {
            std::vector<std::pair<Point, Point>> edges;
            for (CLSEdge edge : g.edges()) {
                CLSVertex v1 = g.source(edge);
                CLSVertex v2 = g.target(edge);
                edges.emplace_back(std::make_pair(g[v1].position, g[v2].position));
            }
            return edges;
        }

        /// string repr
        std::string str() const {
            std::ostringstream o;
            o << "CutterLocationSurface (nVerts=" << g.num_vertices()
              << " , nEdges=" << g.num_edges() << "\n";
            return o.str();
        }

    protected:
        
    // DATA
        /// the half-edge diagram 
        CLSGraph g;
        double min_sampling;
        double far;
        CLSFace out_face;
};


}// end clsurf namespace

} // end ocl namespace
#endif
// end clsurface.h
