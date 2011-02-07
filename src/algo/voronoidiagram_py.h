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
#ifndef VODI_PY_H
#define VODI_PY_H


#include "voronoidiagram.h"

namespace ocl
{

/// \brief Voronoi diagram.
///
/// the dual of a voronoi diagram is the delaunay diagram(triangulation).
///  voronoi-faces are dual to delaunay-vertices.
///  vornoi-vertices are dual to delaunay-faces 
///  voronoi-edges are dual to delaunay-edges
class VoronoiDiagram_py : public VoronoiDiagram {
    public:
        VoronoiDiagram_py() : VoronoiDiagram() {}
        /// create diagram with given far-radius and number of bins
        VoronoiDiagram_py(double far, unsigned int n_bins) 
            : VoronoiDiagram( far, n_bins) {}
        
        // for visualizing the delete-set
        boost::python::list getDeleteSet( Point p ) { // no const here(?)
            boost::python::list out;
            HEFace closest_face = fgrid->grid_find_closest_face( p );
            HEVertex v_seed = find_seed_vertex(closest_face, p);
            hed[v_seed].type = IN;
            VertexVector v0;
            v0.push_back(v_seed); 
            augment_vertex_set_B(v0, p);
            BOOST_FOREACH( HEVertex v, v0) {
                boost::python::list vert;
                vert.append( hed[ v ].position );
                vert.append( hed[ v ].type );
                out.append( vert );
            }
            reset_labels();            
            return out;
            
        }

        boost::python::list getDelaunayEdges()  {
            boost::python::list edge_list;
            BOOST_FOREACH( HEEdge edge, dt->edges() ) {
                    boost::python::list point_list; // the endpoints of each edge
                    HEVertex v1 = dt->source( edge );
                    HEVertex v2 = dt->target( edge );
                    Point src = (*dt)[v1].position;
                    Point tar = (*dt)[v2].position;
                    point_list.append( src );
                    point_list.append( tar );
                    edge_list.append(point_list);
            }
            return edge_list;
        }
        /// return list of generators to python
        boost::python::list getGenerators()  {
            boost::python::list plist;
            for ( HEFace f=0;f<hed.num_faces();++f ) {
                plist.append( hed[f].generator  );
            }
            return plist;
        }
        /// return list of vd vertices to python
        boost::python::list getVoronoiVertices() const {
            boost::python::list plist;
            BOOST_FOREACH( HEVertex v, hed.vertices() ) {
                if ( hed.degree( v ) == 6 ) {
                    plist.append( hed[v].position );
                }
            }
            return plist;
        }
        /// return list of the three special far-vertices to python
        boost::python::list getFarVoronoiVertices() const {
            boost::python::list plist;
            BOOST_FOREACH( HEVertex v, hed.vertices() ) {
                if ( hed.degree( v ) == 4 ) {
                    plist.append( hed[v].position );
                }
            }
            return plist;
        }
        /// return list of vd-edges to python
        boost::python::list getVoronoiEdges() const {
            boost::python::list edge_list;
            BOOST_FOREACH( HEEdge edge, hed.edges() ) { // loop through each edge
                    boost::python::list point_list; // the endpoints of each edge
                    HEVertex v1 = hed.source( edge );
                    HEVertex v2 = hed.target( edge );
                    point_list.append( hed[v1].position );
                    point_list.append( hed[v2].position );
                    edge_list.append(point_list);
            }
            return edge_list;
        }
        /// return edges and generators to python
        boost::python::list getEdgesGenerators()  {
            boost::python::list edge_list;
            BOOST_FOREACH( HEEdge edge, hed.edges() ) {
                    boost::python::list point_list; // the endpoints of each edge
                    HEVertex v1 = hed.source( edge );
                    HEVertex v2 = hed.target( edge );
                    Point src = hed[v1].position;
                    Point tar = hed[v2].position;
                    int src_idx = hed[v1].index;
                    int trg_idx = hed[v2].index;
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
