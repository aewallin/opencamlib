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
#ifndef VODI_CHECK_H
#define VODI_CHECK_H

#include "voronoidiagram_graph.hpp"

namespace ocl
{
class VoronoiDiagram;

/// this class provides sanity-checks for the VoronoiDiagram class
class VoronoiDiagramChecker {
public:
    VoronoiDiagramChecker() {}
    ~VoronoiDiagramChecker() {}
    
    /// sanity-check for the diagram, calls other sanity-check functions
    bool isValid(VoronoiDiagram* vd);
    
    /// check that number of faces equals the number of generators
    bool face_count_equals_generator_count(VoronoiDiagram* vd);
    
    /// the diagram should be of degree three (at least with point generators)
    bool isDegreeThree(VoronoiDiagram* vd);
    
    /// traverse the incident faces and check next-pointers
    bool allIncidentFacesOK(VoronoiDiagram* vd);
    
    /// check that all vertices in the input vector are of type IN
    bool allIn(VoronoiDiagram* vd, const VertexVector& q);

    /// check that no undecided vertices remain in the face
    bool  noUndecidedInFace( VoronoiDiagram* vd, HEFace f );
    
};

} // end namespace
#endif
// end voronoidiagram_checker.h
