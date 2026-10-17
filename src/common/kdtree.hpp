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

#ifndef KDTREE_H
#define KDTREE_H

#include <array>
#include <algorithm>
#include <stdexcept>
#include <cassert>
#include <memory>
#include <vector>
#include <iostream>
#include <list>


#include "kdnode.hpp"
#include "bbox.hpp"
#include "millingcutter.hpp"
#include "clpoint.hpp"
#include "numeric.hpp"

namespace ocl
{
    
class Point;
class CLPoint;
class Triangle;
class MillingCutter;

/// \brief KDTree spread, a measure of how spread-out a list of triangles are.
///
/// simple struct-like class for storing the "spread" or maximum 
/// extent of a list of triangles. Used by the kd-tree algorithm.
class Spread {
    public:
        /// constructor
        Spread(int dim, double v, double s) {
            d = dim;
            val = v;
            start = s;
        };
        /// dimension
        int d;
        /// spread-value
        double val;
        /// minimum or start value
        double start;
        /// comparison of Spread objects. Used for finding the largest spread
        /// along which the next partition/cut is made.
        static bool spread_compare(Spread *x, Spread *y) {
            if (x->val > y->val)
                return true;
            else
                return false;
        };
};

/// a kd-tree for storing triangles and fast searching for triangles
/// that overlap the cutter
template <class BBObj>
class KDTree {
public:
    using Snapshot = std::shared_ptr<const std::vector<BBObj>>;
    KDTree() = default;
    virtual ~KDTree() = default;
    KDTree(const KDTree&) = delete;
    KDTree& operator=(const KDTree&) = delete;
    KDTree(KDTree&&) = default;
    KDTree& operator=(KDTree&&) = default;
    void setBucketSize(int size) {
        if (size <= 0) throw std::invalid_argument("Bucket size must be positive");
        bucketSize = static_cast<unsigned int>(size);
    }
    void setXYDimensions() { dimensions = {0,1,2,3}; }
    void setYZDimensions() { dimensions = {2,3,4,5}; }
    void setXZDimensions() { dimensions = {0,1,4,5}; }

    /// Copy input once into immutable contiguous storage.
    void build(const std::list<BBObj>& input) {
        buildSnapshot(std::make_shared<const std::vector<BBObj>>(input.begin(), input.end()));
    }
    /// Reuse immutable geometry between independently indexed search planes.
    /// The snapshot is retained by the tree and must never be mutated by its creator.
    void buildSnapshot(Snapshot input) {
        if (dimensions.empty()) throw std::logic_error("Set search dimensions before building");
        if (!input) throw std::invalid_argument("Snapshot must not be null");
        KDTree replacement;
        replacement.bucketSize = bucketSize;
        replacement.dimensions = dimensions;
        replacement.objects = std::move(input);
        replacement.order.resize(replacement.objects->size());
        for (std::size_t i = 0; i < replacement.order.size(); ++i)
            replacement.order[i] = &(*replacement.objects)[i];
        if (replacement.order.size() > replacement.nodes.max_size()/2)
            throw std::length_error("Too many objects for KDTree");
        replacement.nodes.reserve(replacement.order.size()*2);
        std::vector<const BBObj*> scratch(replacement.order.size());
        if (!replacement.order.empty()) replacement.partition(0, replacement.order.size(), scratch);
        objects.swap(replacement.objects);
        order.swap(replacement.order);
        nodes.swap(replacement.nodes);
    }
    /// Owning compatibility API; prefer visit() to avoid allocating result copies.
    std::list<BBObj>* search(const Bbox& bb) {
        auto result = std::make_unique<std::list<BBObj>>();
        visit(bb, [&](const BBObj& object) { result->push_back(object); });
        return result.release();
    }
    std::list<BBObj>* search_cutter_overlap(const MillingCutter* cutter, CLPoint* cl) {
        const double r = cutter->getRadius();
        return search(Bbox(cl->x-r, cl->x+r, cl->y-r, cl->y+r, cl->z, cl->z+cutter->getLength()));
    }
    /// Concurrent visits are safe when callbacks do not modify shared state.
    /// References remain valid until the snapshot is released.
    template<class Visitor>
    void visit(const Bbox& bb, Visitor&& visitor) const {
        if (dimensions.empty()) throw std::logic_error("Set search dimensions before querying");
        if (!nodes.empty()) visit_node(bb, nodes.data(), visitor);
    }
    template<class Visitor>
    void visit_cutter_overlap(const MillingCutter* cutter, const CLPoint* cl, Visitor&& visitor) const {
        const double r = cutter->getRadius();
        visit(Bbox(cl->x-r, cl->x+r, cl->y-r, cl->y+r, cl->z, cl->z+cutter->getLength()), visitor);
    }
    std::string str() const {
        return "KDTree(objects=" + std::to_string(order.size()) + ", nodes=" + std::to_string(nodes.size()) + ")";
    }
private:
    struct Node {
        int dim;
        double cut;
        std::size_t first, last;
        const Node* hi = nullptr;
        const Node* lo = nullptr;
        bool leaf = true;
    };
    Spread spread(std::size_t first, std::size_t last) const {
        std::array<double, 6> minval{}, maxval{};
        for (int dim : dimensions) minval[dim] = maxval[dim] = order[first]->bb[dim];
        for (auto i = first; i < last; ++i) {
            const auto& object = *order[i];
            for (int dim : dimensions) {
                const double value = object.bb[dim];
                if (value > maxval[dim]) maxval[dim] = value;
                if (value < minval[dim]) minval[dim] = value;
            }
        }
        int widest = dimensions.front();
        double width = 0;
        for (int dim : dimensions) {
            const double value = maxval[dim] - minval[dim];
            if (value > width) { width = value; widest = dim; }
        }
        return Spread(widest, maxval[widest]-minval[widest], minval[widest]);
    }
    const Node* partition(std::size_t first, std::size_t last, std::vector<const BBObj*>& scratch) {
        const auto extent = spread(first, last);
        Node node{extent.d, extent.start + extent.val/2, first, last};
        const auto index = nodes.size();
        nodes.push_back(node);
        if (last-first <= bucketSize || isZero_tol(extent.val)) return &nodes[index];
        std::size_t lower = 0;
        for (auto i = first; i < last; ++i)
            lower += order[i]->bb[node.dim] <= node.cut;
        // Rounded midpoints may equal an endpoint; never recurse without progress.
        if (lower == 0 || lower == last-first) return &nodes[index];
        auto lo = first, hi = first + lower;
        for (auto i = first; i < last; ++i) {
            const auto object = order[i];
            scratch[object->bb[node.dim] > node.cut ? hi++ : lo++] = object;
        }
        std::copy(scratch.begin()+first, scratch.begin()+last, order.begin()+first);
        node.leaf = false;
        node.hi = partition(first+lower, last, scratch);
        node.lo = partition(first, first+lower, scratch);
        nodes[index] = node;
        return &nodes[index];
    }
    template<class Visitor>
    void visit_node(const Bbox& bb, const Node* current, Visitor& visitor) const {
        const auto& node = *current;
        if (node.leaf) {
            for (auto i = node.first; i < node.last; ++i) visitor(*order[i]);
        } else if (node.dim % 2 == 0 && node.cut > bb[node.dim+1]) {
            visit_node(bb, node.lo, visitor);
        } else if (node.dim % 2 != 0 && node.cut < bb[node.dim-1]) {
            visit_node(bb, node.hi, visitor);
        } else {
            // Stable partitioning and hi-before-lo traversal preserve contact ties.
            visit_node(bb, node.hi, visitor);
            visit_node(bb, node.lo, visitor);
        }
    }
    unsigned int bucketSize = 1;
    std::vector<int> dimensions;
    Snapshot objects;
    std::vector<const BBObj*> order;
    std::vector<Node> nodes;
};

} // namespace ocl
#endif
