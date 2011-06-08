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
#ifndef CLSURFACE_H
#define CLSURFACE_H

#include <vector>
#include <list>
#include <stack>
#include <queue>

#include "point.h"
#include "halfedgediagram.hpp"


namespace ocl
{

namespace clsurf
{

struct VertexProps; 
struct EdgeProps;
struct FaceProps;
typedef unsigned int Face;  
  
// extra storage in graph:
typedef HEDIGraph<     boost::listS,             // out-edge storage
                       boost::listS,             // vertex set storage
                       boost::bidirectionalS,    // bidirectional graph.
                       VertexProps,              // vertex properties
                       EdgeProps,                // edge properties
                       FaceProps,                // face properties
                       boost::no_property,       // graph properties
                       boost::listS             // edge storage
                       > CLSGraph;

typedef boost::graph_traits< CLSGraph >::vertex_descriptor  Vertex;
typedef boost::graph_traits< CLSGraph >::vertex_iterator    VertexItr;
typedef boost::graph_traits< CLSGraph >::edge_descriptor    Edge;
typedef boost::graph_traits< CLSGraph >::edge_iterator      EdgeItr;
typedef boost::graph_traits< CLSGraph >::out_edge_iterator  OutEdgeItr;
typedef boost::graph_traits< CLSGraph >::adjacency_iterator AdjacencyItr;

typedef std::vector<Vertex> VertexVector;
typedef std::vector<Face> FaceVector;
typedef std::vector<Edge> EdgeVector;  


struct VertexProps {
    VertexProps() {
        init();
    }
    /// construct vertex at position p with type t
    VertexProps( Point p) {
        position=p;
        init();
    }
    void init() {
        index = count;
        count++;
    }
    //VertexType type;
// HE data
    /// the position of the vertex
    Point position;
    /// index of vertex
    int index;
    /// global vertex count
    static int count;
};

int VertexProps::count = 0;

struct EdgeProps {
    EdgeProps() {}
    /// create edge with given next, twin, and face
    EdgeProps(Edge n, Edge t, Face f) { 
        next = n;
        twin = t;
        face = f;
    }
    /// the next edge, counterclockwise, from this edge
    Edge next; 
    /// the twin edge
    Edge twin;
    /// the face to which this edge belongs
    Face face; 
};
/// types of faces in the weave
//enum 2FaceType {INCIDENT, NONINCIDENT};
/// properties of a face in the weave
struct FaceProps {
    /// create face with given edge, generator, and type
    FaceProps() {}
    FaceProps( Edge e ) {
        edge = e;
        //type = t;
    }
    /// face index
    Face idx;
    /// one edge that bounds this face
    Edge edge;
    /// face type
    //W2FaceType type;
};


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
            Vertex a = hedi::add_vertex( VertexProps( Point(far,far,0) ), g);
            Vertex b = hedi::add_vertex( VertexProps( Point(-far,far,0) ), g);
            Vertex c = hedi::add_vertex( VertexProps( Point(-far,-far,0) ), g);
            Vertex d = hedi::add_vertex( VertexProps( Point(far,-far,0) ), g);
            Face f_outer= hedi::add_face( FaceProps(), g);
            Face f_inner= hedi::add_face( FaceProps(), g);
            Edge e1 = hedi::add_edge(a , b , g);
            Edge e1t = hedi::add_edge(b , a , g);
            Edge e2 = hedi::add_edge(b , c , g);
            Edge e2t = hedi::add_edge(c , b , g);
            Edge e3 = hedi::add_edge(c , d , g);
            Edge e3t = hedi::add_edge(d , c , g);
            Edge e4 = hedi::add_edge(d , a , g);
            Edge e4t = hedi::add_edge(a , d , g);
            
            g[f_inner].edge = e1;
            g[f_outer].edge = e1t;
            out_face = f_outer;
            // twin edges
            hedi::twin_edges(e1,e1t,g);
            hedi::twin_edges(e2,e2t,g);
            hedi::twin_edges(e3,e3t,g);
            hedi::twin_edges(e4,e4t,g);
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
            for( Face f=0; f< hedi::num_faces(g) ; ++f ) {
                // subdivide each face
                if ( f!= out_face ) {
                    subdivide_face(f);
                }
            }
        }
        
        void subdivide_face(Face f) {
            EdgeVector f_edges = hedi::face_edges(f,g);
            assert( f_edges.size() == 4 );
            Vertex center = hedi::add_vertex( g );
            BOOST_FOREACH( Edge e, f_edges ) {
                Vertex src = hedi::source(e,g);
                Vertex trg = hedi::target(e,g);
                // new vertex at mid-point of each edge
                Point mid = 0.5*(g[src].position+g[trg].position);
                g[center].position += 0.25*g[src].position; // average of four corners
                Vertex v = hedi::add_vertex( g );
                g[v].position = mid;
                hedi::insert_vertex_in_edge(v,e,g); // this also removes the old edges...
            }
            // now loop through edges again:
            f_edges = hedi::face_edges(f,g);
            assert( f_edges.size() == 8 );
            BOOST_FOREACH( Edge e, f_edges ) {
                std::cout << e << "\n";
            }
        }
        
        virtual void run() {
        }
        void setMinSampling(double s) {
            min_sampling=s;
        }
        
    // PYTHON
        boost::python::list getVertices() const {
            boost::python::list plist;
            BOOST_FOREACH( Vertex v, hedi::vertices(g) ) {
                plist.append( g[v].position );
            }
            return plist;
        }
        
        boost::python::list getEdges() const {
            boost::python::list edge_list;
            BOOST_FOREACH( Edge edge, hedi::edges(g) ) { // loop through each edge
                    boost::python::list point_list; // the endpoints of each edge
                    Vertex v1 = hedi::source( edge, g );
                    Vertex v2 = hedi::target( edge, g );
                    point_list.append( g[v1].position );
                    point_list.append( g[v2].position );
                    edge_list.append(point_list);
            }
            return edge_list;
        }
        
        /// string repr
        std::string str() const {
            std::ostringstream o;
            o << "CutterLocationSurface (nVerts="<< hedi::num_vertices(g) << " , nEdges="<< hedi::num_edges(g) <<"\n";
            return o.str();
        }

    protected:
        
    // DATA
        /// the half-edge diagram 
        CLSGraph g;
        double min_sampling;
        double far;
        Face out_face;
};


}// end clsurf namespace

} // end ocl namespace
#endif
// end clsurface.h
