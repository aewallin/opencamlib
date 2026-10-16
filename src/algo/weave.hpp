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
#ifndef WEAVE_HPP
#define WEAVE_HPP

#include <vector>

#include "fiber.hpp"
#include "weave_typedef.hpp"

namespace ocl {

namespace weave {

    // Abstract base-class for weave-implementations. build() must be implemented in sub-class!
    class Weave {
    public:
        Weave() {}
        virtual ~Weave() {}
        /// add Fiber f to the graph
        /// each fiber should be either in the X or Y-direction
        /// FIXME: separate addXFiber and addYFiber methods?
        void addFiber(Fiber& f);
        /// from the list of fibers, build a graph
        virtual void build() = 0;
        /// run planar_face_traversal to get the waterline loops
        void face_traverse();
        /// return list of loops
        std::vector<std::vector<Point>> getLoops() const;
        /// string representation
        std::string str();
        void printGraph();

        int numVertices() const { return g.num_vertices(); }
        std::vector<Point> getVertices() const;
        std::vector<Point> getVerticesByType(VertexType t) const;
        std::vector<std::pair<Point, Point>> getEdges() const;

    protected:
        WeaveGraph g;                           ///< the weave-graph
        std::vector<std::vector<Vertex>> loops; ///< output: list of loops in this weave
        std::vector<Fiber> xfibers;             ///< the X-fibers
        std::vector<Fiber> yfibers;             ///< the Y-fibers
        std::set<Vertex> clVertexSet;           ///< set of CL-points
    };

} // end weave namespace

} // end ocl namespace
#endif
// end file weave.hpp
