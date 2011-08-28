/*  
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

#ifndef VODI_VERTEX_HPP
#define VODI_VERTEX_HPP

#include "point.hpp"


namespace ocl {

// base-class for generators
class VoronoiGenerator {
    public:
        VoronoiGenerator();
        virtual ~VoronoiGenerator();
        virtual Point get_apex(const Point& p) const = 0;
    private:
        
};

class PointGenerator : public VoronoiGenerator {
    public:
        PointGenerator(Point p) {
            position = p;
        }
        Point get_apex(const Point& p) const {
            return position;
        }
    private:
        Point position;
};

/// voronoi-vertices can have one of these four different states
/// as we incrementally construct the diagram the type is updated as follows:
/// OUT-vertices will not be deleted
/// IN-vertices will be deleted
/// UNDECIDED-vertices have not been examied yet
/// NEW-vertices are constructed on OUT-IN edges
enum VoronoiVertexStatus {OUT, IN, UNDECIDED, NEW };

/// properties of a vertex in the voronoi diagram
class VoronoiVertex {
public:
    VoronoiVertex();
    /// construct vertex at position p with type t
    VoronoiVertex( Point p, VoronoiVertexStatus st);
    void init();
    void reset();
    void set_generators(const Point& pi, const Point& pj, const Point& pkin);
    
    /// based on precalculated J2, J3, J4, calculate the H determinant for input Point pl
    /// Eq.(20) from Sugihara&Iri 1994
    double detH(const Point& pl) const;
    /// index of vertex
    int index;
    /// vertex status. when the incremental algorithm runs
    /// vertices are marked: undecided, in, out, or new
    VoronoiVertexStatus status;
    bool in_queue;
    /// the position of the vertex
    Point position;
protected:
    /// based on previously calculated J2, J3, and J4, set the position of the vertex
    /// Eq.(24) from Sugihara&Iri 1994
    void set_position();
    /// set the J values
    void set_J(const Point& pi, const Point& pj, const Point& pkin);
    /// calculate J2
    /// Eq(21) from Sugihara&Iri 1994
    /// see also Eq(4.6.4), page 256, of Okabe et al book
    double detH_J2(Point& pi, Point& pj, Point& pk);
    /// calculate J3
    /// Eq(22) from Sugihara&Iri 1994
    /// see also Eq(4.6.5), page 257, of Okabe et al book
    double detH_J3(Point& pi, Point& pj, Point& pk);
    /// calculate J4
    /// Eq(23) from Sugihara&Iri 1994
    /// see also Eq(4.6.6), page 257, of Okabe et al book
    double detH_J4(Point& pi, Point& pj, Point& pk);
// private DATA
    /// the reference point for J-calculations
    Point _pk;
    /// J2 determinant
    double J2;
    /// J3 determinant
    double J3;
    /// J4 determinant
    double J4;
    /// global vertex count
    static int count;
};


} // end ocl namespace
#endif
