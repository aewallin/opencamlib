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
#ifndef TSP_H
#define TSP_H

#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <vector>

namespace ocl {
namespace tsp {

/// Euclidean metric TSP approximation: preorder traversal of a minimum
/// spanning tree, shortcutting repeated vertices. For finite Euclidean distances
/// the closed tour is at most twice the optimal length. Dense Prim takes O(n^2)
/// time and O(n) auxiliary space; no complete graph or distance matrix is stored.
class TSPSolver {
    using Vertex = std::size_t;
    struct Point { double x, y; };
    using PointSet = std::vector<Point>;
    using Container = std::vector<Vertex>;

public:
    TSPSolver() : length(0.0) {}
    virtual ~TSPSolver() = default;

    void run() {
        output.clear();
        length = 0.0;
        const std::size_t n = points.size();
        if (n == 0)
            return;

        std::vector<double> nearest(n, (std::numeric_limits<double>::infinity)());
        Container parent(n, 0);
        std::vector<bool> visited(n, false);
        nearest[0] = 0.0;
        for (std::size_t step = 0; step < n; ++step) {
            Vertex current = n;
            // Resolve equal weights by vertex index for reproducible tours.
            for (Vertex v = 0; v < n; ++v)
                if (!visited[v] && (current == n || nearest[v] < nearest[current]))
                    current = v;
            visited[current] = true;
            for (Vertex v = 0; v < n; ++v) {
                if (visited[v])
                    continue;
                double weight = distance(current, v);
                if (weight < nearest[v]) {
                    nearest[v] = weight;
                    parent[v] = current;
                }
            }
        }

        // Child lists in flat arrays avoid a separate allocation for each node.
        Container first_child(n, n), next_sibling(n, n);
        for (Vertex v = n - 1; v > 0; --v) {
            next_sibling[v] = first_child[parent[v]];
            first_child[parent[v]] = v;
        }
        // Iterative preorder also handles arbitrarily deep trees.
        output.reserve(n + 1);
        Vertex current = 0;
        for (;;) {
            output.push_back(current);
            if (first_child[current] != n) {
                current = first_child[current];
                continue;
            }
            while (current != 0 && next_sibling[current] == n)
                current = parent[current];
            if (current == 0)
                break;
            current = next_sibling[current];
        }
        output.push_back(0);
        for (std::size_t i = 1; i < output.size(); ++i)
            length += distance(output[i - 1], output[i]);
    }

    void addPoint(double x, double y) { points.push_back({x, y}); }
    void reset() {
        output.clear();
        points.clear();
        length = 0.0;
    }
    void printOutput() const {
        std::size_t n = 0;
        for (Vertex v : output)
            std::cout << n++ << " : " << v << "\n";
    }
    double getLength() const { return length; }
    auto getOutput() const { return output; }

protected:
    Container output;
    PointSet points;
    double length;

private:
    double distance(Vertex a, Vertex b) const {
        return std::hypot(points[a].x - points[b].x, points[a].y - points[b].y);
    }
};

} // namespace tsp
} // namespace ocl
#endif
