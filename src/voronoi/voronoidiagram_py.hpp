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
#ifndef VODI_PY_H
#define VODI_PY_H


#include "voronoidiagram.hpp"

namespace ocl
{

/// \brief python wrapper for VoronoiDiagram
///
class VoronoiDiagram_py : public VoronoiDiagram {
    public:
        VoronoiDiagram_py() : VoronoiDiagram() {}
        /// create diagram with given far-radius and number of bins
        VoronoiDiagram_py(double far, unsigned int n_bins) 
            : VoronoiDiagram( far, n_bins) {}
        
        // for visualizing the closest face (returns it's generator)
        Point getClosestFaceGenerator( const Point p ) {
            HEFace closest_face = fgrid->grid_find_closest_face( p );
            return g[closest_face].generator;
        }
        
        // for visualizing seed-vertex
        Point getSeedVertex( const Point p ) {
            HEFace closest_face = fgrid->grid_find_closest_face( p );
            HEVertex v = findSeedVertex(closest_face, p);
            return g[ v ].position;
        }
        
        // for visualizing the delete-set
        boost::python::list getDeleteSet( Point p ) { // no const here(?)
            boost::python::list out;
            HEFace closest_face = fgrid->grid_find_closest_face( p );
            HEVertex v_seed = findSeedVertex(closest_face, p);
            g[v_seed].type = IN;
            VertexVector v0;
            v0.push_back(v_seed); 
            augment_vertex_set_M(v0, p);
            BOOST_FOREACH( HEVertex v, v0) {
                boost::python::list vert;
                vert.append( g[ v ].position );
                vert.append( g[ v ].type );
                out.append( vert );
            }
            reset_labels();            
            return out;
            
        }
        /// for visualizing the delete-edges
        boost::python::list getDeleteEdges( Point p ) {
            boost::python::list out;
            HEFace closest_face = fgrid->grid_find_closest_face( p );
            HEVertex v_seed = findSeedVertex(closest_face, p);
            g[v_seed].type = IN;
            VertexVector v0;
            v0.push_back(v_seed); 
            augment_vertex_set_M(v0, p);
            EdgeVector del = find_edges(v0, IN);
            BOOST_FOREACH( HEEdge e, del) {
                boost::python::list edge;
                HEVertex src = hedi::source(e,g);
                HEVertex trg = hedi::target(e,g);
                edge.append( g[ src ].position );
                edge.append( g[ trg ].position );
                out.append( edge );
            }
            reset_labels();            
            return out;
        }
        /// for visualizing the edges to be modified
        boost::python::list getModEdges( Point p ) {
            boost::python::list out;
            HEFace closest_face = fgrid->grid_find_closest_face( p );
            HEVertex v_seed = findSeedVertex(closest_face, p);
            g[v_seed].type = IN;
            VertexVector v0;
            v0.push_back(v_seed); 
            augment_vertex_set_M(v0, p);
            EdgeVector del = find_edges(v0, OUT);
            BOOST_FOREACH( HEEdge e, del) {
                boost::python::list edge;
                HEVertex src = hedi::source(e,g);
                HEVertex trg = hedi::target(e,g);
                edge.append( g[ src ].position );
                edge.append( g[ trg ].position );
                out.append( edge );
            }
            reset_labels();            
            return out;
        }

        /// return list of generators to python
        boost::python::list getGenerators()  {
            boost::python::list plist;
            for ( HEFace f=0;f<hedi::num_faces(g);++f ) {
                plist.append( g[f].generator  );
            }
            return plist;
        }
        /// return list of vd vertices to python
        boost::python::list getVoronoiVertices() const {
            boost::python::list plist;
            BOOST_FOREACH( HEVertex v, hedi::vertices(g) ) {
                if ( hedi::degree( v, g ) == 6 ) {
                    plist.append( g[v].position );
                }
            }
            return plist;
        }
        /// return list of the three special far-vertices to python
        boost::python::list getFarVoronoiVertices() const {
            boost::python::list plist;
            BOOST_FOREACH( HEVertex v, hedi::vertices(g) ) {
                if ( hedi::degree( v , g) == 4 ) {
                    plist.append( g[v].position );
                }
            }
            return plist;
        }
        /// return list of vd-edges to python
        boost::python::list getVoronoiEdges() const {
            boost::python::list edge_list;
            BOOST_FOREACH( HEEdge edge, hedi::edges(g) ) { // loop through each edge
                    boost::python::list point_list; // the endpoints of each edge
                    HEVertex v1 = hedi::source( edge, g );
                    HEVertex v2 = hedi::target( edge, g );
                    point_list.append( g[v1].position );
                    point_list.append( g[v2].position );
                    edge_list.append(point_list);
            }
            return edge_list;
        }
        /// return edges and generators to python
        boost::python::list getEdgesGenerators()  {
            boost::python::list edge_list;
            BOOST_FOREACH( HEEdge edge, hedi::edges(g) ) {
                    boost::python::list point_list; // the endpoints of each edge
                    HEVertex v1 = hedi::source( edge, g );
                    HEVertex v2 = hedi::target( edge, g );
                    Point src = g[v1].position;
                    Point tar = g[v2].position;
                    int src_idx = g[v1].index;
                    int trg_idx = g[v2].index;
                    point_list.append( src );
                    point_list.append( tar );
                    point_list.append( src_idx );
                    point_list.append( trg_idx );
                    edge_list.append(point_list);
            }
            return edge_list;
        }

        
};


} // end namespace
#endif
// end voronoidiagram_py.h
