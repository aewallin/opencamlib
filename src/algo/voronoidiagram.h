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
#ifndef VODI_H
#define VODI_H

#include <vector>
#include <list>
#include <stack>
#include <queue>

#include "point.h"
#include "voronoidiagram_graph.h"
#include "voronoidiagram_checker.h"
#include "halfedgediagram2.h"
#include "facegrid.h"

namespace ocl
{

class VoronoiDiagramChecker;

/// \brief Voronoi diagram.
///
/// see http://en.wikipedia.org/wiki/Voronoi_diagram
/// 
/// the dual of a voronoi diagram is the delaunay diagram(triangulation).
///  voronoi-faces are dual to delaunay-vertices.
///  vornoi-vertices are dual to delaunay-faces 
///  voronoi-edges are dual to delaunay-edges
class VoronoiDiagram {
    public:
        /// ctor
        VoronoiDiagram() {}
        /// create diagram with given far-radius and number of bins
        VoronoiDiagram(double far, unsigned int n_bins);
        /// dtor
        virtual ~VoronoiDiagram();
        
        /// add a vertex generator at given position
        void addVertexSite(const Point& p);

        /// string repr
        std::string str() const;
        /// return the far radius
        double getFarRadius() const {return far_radius;}
        
        friend class VoronoiDiagramChecker;
        
    protected:
        /// initialize the diagram with three generators
        void init();
        /// among the vertices of f, find the one with the lowest detH value
        HEVertex findSeedVertex(HEFace f, const Point& p);

        /// breadth-first search based Tree-expansion algorithm
        void augment_vertex_set_M(VertexVector& q, const Point& p);
        EdgeVector find_edges(VertexVector& q, VoronoiVertexType vtype);
        int outVertexCount(HEFace f);
        int adjacentInCount(HEVertex v);
        FaceVector adjacentIncidentFaces(HEVertex v);
        bool incidentFacesHaveAdjacentInVertex(HEVertex v);
        bool faceVerticesConnected( HEFace f, VoronoiVertexType Vtype );
        void markAdjecentFacesIncident(HEVertex v);
        void pushAdjacentVertices(  HEVertex v , std::queue<HEVertex>& Q);
        
        
        /// add the new vertices  
        void add_new_voronoi_vertices(VertexVector& v, const Point& p);
        /// split faces when adding new generator p
        HEFace split_faces(const Point& p);
        /// split the face
        void split_face(HEFace new_f, HEFace f);
        /// remove vertices in the set
        void remove_vertex_set(VertexVector& v0 , HEFace newface);
        /// set all vertices to UNDECIDED and all faces to NONINCIDENT
        void reset_labels();
        
    // PRINT
        void printFaceVertexTypes(HEFace f);
        void printVertices(VertexVector& q);
        
    // DATA
        /// the half-edge diagram of the vd
        HEGraph g;

        /// a grid which allows fast nearest-neighbor search
        FaceGrid* fgrid; // for grid-search
        /// the voronoi diagram is constructed for sites within a circle with radius far_radius
        double far_radius;
        /// special initial vertex
        HEVertex v01;
        /// special initial vertex
        HEVertex v02;
        /// special initial vertex
        HEVertex v03;
        
        // initial generators
        Point gen1;
        Point gen2;
        Point gen3;
        /// the number of generators
        int gen_count;
        /// temporary variable for incident faces
        FaceVector incident_faces;
        /// temporary variable for in-vertices, out-vertices that need to be reset
        /// after each generator has been inserted
        VertexVector in_vertices;
        /// sanity-checks on the diagram are done by this helper class
        VoronoiDiagramChecker vdChecker;
};


} // end namespace
#endif
// end voronoidiagram.h
