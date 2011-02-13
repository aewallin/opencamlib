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
#ifndef CLSURFACE_H
#define CLSURFACE_H

#include <vector>
#include <list>
#include <stack>
#include <queue>

#include "point.h"
#include "halfedgediagram.h"


namespace ocl
{

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
        CutterLocationSurface() {}
        /// create diagram with given far-radius and number of bins
        CutterLocationSurface(double far) {
        }
        virtual ~CutterLocationSurface() {  }
        virtual void run() {
        }
        void setMinSampling(double s) {min_sampling=s;}
        
    // PYTHON
        boost::python::list getVertices() const {
            boost::python::list plist;
            BOOST_FOREACH( HEVertex v, hed.vertices() ) {
                plist.append( hed[v].position );
            }
            return plist;
        }
        
        boost::python::list getEdges() const {
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
        
        /// string repr
        std::string str() const {
            std::ostringstream o;
            o << "CutterLocationSurface (nVerts="<< hed.num_vertices() << " , nEdges="<< hed.num_edges() <<"\n";
            return o.str();
        }

    protected:
        
    // DATA
        /// the half-edge diagram 
        HalfEdgeDiagram hed;
        double min_sampling;
};


} // end namespace
#endif
// end clsurface.h
