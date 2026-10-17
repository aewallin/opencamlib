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
#ifndef HALFEDGEDIAGRAM_HPP
#define HALFEDGEDIAGRAM_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <deque>
#include <limits>
#include <ostream>
#include <set>
#include <vector>

namespace ocl {
namespace hedi { // half-edge diagram

/// Distinct, graph-local handles. A default handle is invalid.
/// Deleting an element invalidates its handle; its slot can subsequently be reused.
template <class Tag>
class Descriptor {
public:
    Descriptor() : index_((std::numeric_limits<std::size_t>::max)()) {}
    explicit Descriptor(std::size_t index) : index_(index) {}
    std::size_t index() const { return index_; }
    friend bool operator==(Descriptor a, Descriptor b) { return a.index_ == b.index_; }
    friend bool operator!=(Descriptor a, Descriptor b) { return !(a == b); }
    friend bool operator<(Descriptor a, Descriptor b) { return a.index_ < b.index_; }
    friend std::ostream& operator<<(std::ostream& out, Descriptor value) {
        return out << value.index_;
    }
private:
    std::size_t index_;
};

struct VertexTag;
struct EdgeTag;
using VertexDescriptor = Descriptor<VertexTag>;
using EdgeDescriptor = Descriptor<EdgeTag>;

/// Directed half-edge storage with stable handles and property references.
/// Block storage avoids one allocation per graph element, and vacant slots are
/// reused. Small contiguous adjacency lists retain insertion order and support
/// parallel edges and self-loops. Enumeration returns snapshots so callers may
/// delete elements while traversing them. Copies preserve all internal handles.
/// Face, next, previous and twin relationships are maintained by the caller,
/// except in twin_edges() and insert_vertex_in_edge().
template <class TVertexProperties, class TEdgeProperties, class TFaceProperties>
class HEDIGraph {
public:
    using Vertex = VertexDescriptor;
    using Edge = EdgeDescriptor;
    using Face = unsigned int;
    using VertexVector = std::vector<Vertex>;
    using EdgeVector = std::vector<Edge>;
    using FaceVector = std::vector<Face>;
    using VertexItr = typename VertexVector::iterator;
    using EdgeItr = typename EdgeVector::iterator;
    using OutEdgeItr = EdgeItr;
    using AdjacencyItr = VertexItr;

    TVertexProperties& operator[](Vertex v) { return vertex_record(v).properties; }
    const TVertexProperties& operator[](Vertex v) const { return vertex_record(v).properties; }
    TEdgeProperties& operator[](Edge e) { return edge_record(e).properties; }
    const TEdgeProperties& operator[](Edge e) const { return edge_record(e).properties; }
    TFaceProperties& operator[](Face f) { return faces[f]; }
    const TFaceProperties& operator[](Face f) const { return faces[f]; }

    std::vector<TFaceProperties> faces;

    static Vertex null_vertex() { return Vertex(); }

    Vertex add_vertex() {
        std::size_t index;
        if (free_vertices_.empty()) {
            index = vertices_.size();
            vertices_.emplace_back();
        } else {
            index = free_vertices_.back();
            vertices_[index].properties = TVertexProperties();
            free_vertices_.pop_back();
        }
        vertices_[index].active = true;
        return Vertex(index);
    }

    Edge add_edge(Vertex from, Vertex to) {
        auto& source_record = vertex_record(from);
        auto& target_record = vertex_record(to);
        // Allocate adjacency capacity before changing the graph.
        grow_adjacency(source_record.out);
        grow_adjacency(target_record.in);
        std::size_t index;
        if (free_edges_.empty()) {
            index = edges_.size();
            edges_.emplace_back();
        } else {
            index = free_edges_.back();
            edges_[index].properties = TEdgeProperties();
            free_edges_.pop_back();
        }
        auto& record = edges_[index];
        record.from = from;
        record.to = to;
        record.active = true;
        Edge e(index);
        source_record.out.push_back(e);
        target_record.in.push_back(e);
        return e;
    }

    Face add_face() {
        faces.emplace_back();
        Face index = static_cast<Face>(faces.size() - 1);
        faces.back().idx = index;
        return index;
    }

    void twin_edges(Edge a, Edge b) {
        (*this)[a].twin = b;
        (*this)[b].twin = a;
    }

    Vertex source(Edge e) const { return edge_record(e).from; }
    Vertex target(Edge e) const { return edge_record(e).to; }

    VertexVector vertices() const {
        VertexVector result;
        result.reserve(num_vertices());
        for (std::size_t i = 0; i < vertices_.size(); ++i)
            if (vertices_[i].active)
                result.emplace_back(i);
        return result;
    }

    EdgeVector edges() const {
        EdgeVector result;
        result.reserve(num_edges());
        for (std::size_t i = 0; i < edges_.size(); ++i)
            if (edges_[i].active)
                result.emplace_back(i);
        return result;
    }

    EdgeVector out_edges(Vertex v) const { return vertex_record(v).out; }

    /// Borrow the adjacency list without allocating a snapshot. Its iterators
    /// and entries must not be retained across changes to this vertex's edges.
    const EdgeVector& out_edges_view(Vertex v) const { return vertex_record(v).out; }

    VertexVector adjacent_vertices(Vertex v) const {
        VertexVector result;
        const auto& out = vertex_record(v).out;
        result.reserve(out.size());
        for (Edge e : out)
            result.push_back(target(e));
        return result;
    }

    unsigned int degree(Vertex v) const {
        const auto& record = vertex_record(v);
        return static_cast<unsigned int>(record.in.size() + record.out.size());
    }
    unsigned int num_vertices() const {
        return static_cast<unsigned int>(vertices_.size() - free_vertices_.size());
    }
    unsigned int num_edges() const {
        return static_cast<unsigned int>(edges_.size() - free_edges_.size());
    }
    unsigned int num_faces() const { return static_cast<unsigned int>(faces.size()); }

    /// Returns an invalid handle when no directed edge exists.
    Edge edge(Vertex from, Vertex to) const {
        for (Edge e : vertex_record(from).out)
            if (target(e) == to)
                return e;
        return Edge();
    }
    bool has_edge(Vertex from, Vertex to) const { return edge(from, to) != Edge(); }

    EdgeVector face_edges(Face f) const {
        EdgeVector result;
        Edge start = faces[f].edge;
        Edge current = start;
        do {
            result.push_back(current);
            current = (*this)[current].next;
        } while (current != start);
        return result;
    }

    VertexVector face_vertices(Face f) const {
        VertexVector result;
        for (Edge e : face_edges(f))
            result.push_back(target(e));
        return result;
    }

    Edge previous_edge(Edge e) const {
        Edge previous = (*this)[e].next;
        while ((*this)[previous].next != e)
            previous = (*this)[previous].next;
        return previous;
    }

    FaceVector adjacent_faces(Vertex v) const {
        std::set<Face> unique;
        for (Edge e : vertex_record(v).out)
            unique.insert((*this)[e].face);
        return FaceVector(unique.begin(), unique.end());
    }

    /// Split both halves of an edge, preserving face cycles and twin links.
    void insert_vertex_in_edge(Vertex v, Edge e) {
        Edge twin = (*this)[e].twin;
        Vertex from = source(e), to = target(e);
        assert(from == target(twin) && to == source(twin));
        assert(e != twin);
        Face face = (*this)[e].face, twin_face = (*this)[twin].face;
        Edge previous = previous_edge(e), twin_previous = previous_edge(twin);
        Edge next = (*this)[e].next, twin_next = (*this)[twin].next;

        Edge e1 = add_edge(from, v), e2 = add_edge(v, to);
        Edge t1 = add_edge(to, v), t2 = add_edge(v, from);
        (*this)[e1].face = (*this)[e2].face = face;
        (*this)[t1].face = (*this)[t2].face = twin_face;
        // The old edges may be neighbors, or form an entire two-edge face.
        auto replacement = [=](Edge old) { return old == e ? e1 : (old == twin ? t1 : old); };
        (*this)[e1].next = e2;
        (*this)[e2].next = replacement(next);
        (*this)[t1].next = t2;
        (*this)[t2].next = replacement(twin_next);
        if (previous != e && previous != twin)
            (*this)[previous].next = e1;
        if (twin_previous != e && twin_previous != twin)
            (*this)[twin_previous].next = t1;
        twin_edges(e1, t2);
        twin_edges(e2, t1);
        faces[face].edge = e1;
        faces[twin_face].edge = t1;
        remove_edge(e);
        remove_edge(twin);
    }

    void remove_edge(Edge e) {
        auto& record = edge_record(e);
        free_edges_.push_back(e.index());
        erase(vertex_record(record.from).out, e);
        erase(vertex_record(record.to).in, e);
        record.active = false;
    }

    /// Remove all parallel edges from v1 to v2.
    void remove_edge(Vertex v1, Vertex v2) {
        const auto& out = vertex_record(v1).out;
        for (std::size_t i = 0; i < out.size();) {
            Edge e = out[i];
            if (target(e) == v2)
                remove_edge(e); // The following entry shifts into index i.
            else
                ++i;
        }
    }

    void clear_vertex(Vertex v) {
        auto& record = vertex_record(v);
        while (!record.out.empty())
            remove_edge(record.out.back());
        while (!record.in.empty())
            remove_edge(record.in.back());
    }

    /// The vertex must already be isolated. Use delete_vertex for connected vertices.
    void remove_vertex(Vertex v) {
        auto& record = vertex_record(v);
        assert(record.in.empty() && record.out.empty());
        free_vertices_.push_back(v.index());
        record.active = false;
    }
    void delete_vertex(Vertex v) {
        clear_vertex(v);
        remove_vertex(v);
    }

private:
    struct VertexRecord {
        TVertexProperties properties{};
        EdgeVector out, in;
        bool active = false;
    };
    struct EdgeRecord {
        TEdgeProperties properties{};
        Vertex from, to;
        bool active = false;
    };
    // Deques keep references valid on growth, while descriptors remain small
    // integer indices. These storage and adjacency choices are local to this class.
    std::deque<VertexRecord> vertices_;
    std::deque<EdgeRecord> edges_;
    std::vector<std::size_t> free_vertices_, free_edges_;

    VertexRecord& vertex_record(Vertex v) {
        assert(v.index() < vertices_.size() && vertices_[v.index()].active);
        return vertices_[v.index()];
    }
    const VertexRecord& vertex_record(Vertex v) const {
        assert(v.index() < vertices_.size() && vertices_[v.index()].active);
        return vertices_[v.index()];
    }
    EdgeRecord& edge_record(Edge e) {
        assert(e.index() < edges_.size() && edges_[e.index()].active);
        return edges_[e.index()];
    }
    const EdgeRecord& edge_record(Edge e) const {
        assert(e.index() < edges_.size() && edges_[e.index()].active);
        return edges_[e.index()];
    }
    static void erase(EdgeVector& list, Edge e) {
        auto it = std::find(list.begin(), list.end(), e);
        assert(it != list.end());
        list.erase(it);
    }
    static void grow_adjacency(EdgeVector& list) {
        if (list.size() == list.capacity())
            list.reserve(std::max<std::size_t>(4, list.capacity() * 2));
    }
};

} // namespace hedi
} // namespace ocl
#endif
