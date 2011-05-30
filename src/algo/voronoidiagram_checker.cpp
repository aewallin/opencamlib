/*  $Id:  $
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

//#include <vector>
//#include <list>
//#include <stack>
//#include <queue>

//#include "point.h"
#include "voronoidiagram_checker.h"
//#include "voronoidiagram_graph.h"
#include "voronoidiagram.h"

//#include "halfedgediagram2.h"
//#include "facegrid.h"

namespace ocl
{

    
    /// sanity-check for the diagram, calls other sanity-check functions
    bool VoronoiDiagramChecker::isValid(VoronoiDiagram* vd) {
        if (!isDegreeThree(vd) )
            return false;
        if (!face_count_equals_generator_count(vd))
            return false;
        return true;
    }
    
    
    
    /// check that number of faces equals the number of generators
    bool VoronoiDiagramChecker::face_count_equals_generator_count(VoronoiDiagram* vd) {
        // Euler formula for planar graphs
        // v - e + f = 2
        // in a half-edge diagram all edges occur twice, so:
        // f = 2-v+e
        int vertex_count = hedi::num_vertices(vd->g);
        int face_count = (vertex_count- 4)/2 + 3; // degree three graph
        //int face_count = hed.num_faces();
        if (face_count != vd->gen_count) {
            std::cout << " face_count_equals_generator_count() ERROR:\n";
            std::cout << " num_vertices = " << vertex_count << "\n";
            std::cout << " gen_count = " << vd->gen_count << "\n";
            std::cout << " face_count = " << face_count << "\n";
        }
        return ( face_count == vd->gen_count );
    }

    
    
    /// the diagram should be of degree three (at least with point generators)
    bool VoronoiDiagramChecker::isDegreeThree(VoronoiDiagram* vd) {
        // the outermost init() vertices have special degree, all others == 6
        BOOST_FOREACH(HEVertex v, hedi::vertices(vd->g) ) {
            if ( hedi::degree( v, vd->g ) != 6 ) {
                if ( (v != vd->v01) && (v != vd->v02) && (v != vd->v03) )
                    return false;
            }
        }
        return true;
    }
    
    
    /// traverse the incident faces and check next-pointers
    bool VoronoiDiagramChecker::allIncidentFacesOK(VoronoiDiagram* vd) {
        // all incident faces should pass the sanity-check
        BOOST_FOREACH( HEFace f, vd->incident_faces ) {
            if ( !vd->faceVerticesConnected(  f, IN ) )
                return false; // IN vertices should be connected
            if ( !vd->faceVerticesConnected(  f, OUT ) )  // OUT vertices should be connected
                return false;
            if ( !noUndecidedInFace(vd, f ) )            // no UNDECIDED vertices should remain
                return false;
        }
        return true;
}
    
    
    /// check that all vertices in the input vector are of type IN
    bool VoronoiDiagramChecker::allIn(VoronoiDiagram* vd, const VertexVector& q) {
        BOOST_FOREACH( HEVertex v, q) {
            if ( vd->g[v].type != IN )
                return false;
        }
        return true;
    }

    /// check that no undecided vertices remain in the face
    bool  VoronoiDiagramChecker::noUndecidedInFace( VoronoiDiagram* vd, HEFace f ) {
        VertexVector face_verts = hedi::face_vertices(f, vd->g);
        BOOST_FOREACH( HEVertex v, face_verts ) {
            if ( vd->g[v].type == UNDECIDED )
                return false;
        }
        return true;
    }
        
    


} // end namespace
