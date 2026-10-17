#include "halfedgediagram.hpp"
#include "check.hpp"

#include <random>
#include <type_traits>

using Vertex = ocl::hedi::VertexDescriptor;
using Edge = ocl::hedi::EdgeDescriptor;
struct VertexProps { int value = 0; };
struct EdgeProps { Edge next, twin; unsigned int face = 0; int value = 0; };
struct FaceProps { Edge edge; unsigned int idx = 0; };
using Graph = ocl::hedi::HEDIGraph<VertexProps, EdgeProps, FaceProps>;

static void storage() {
    static_assert(!std::is_same<Vertex, Edge>::value, "Handles must be distinct");
    static_assert(!std::is_same<Vertex, Graph::Face>::value, "Handles must be distinct");
    Graph g;
    auto a = g.add_vertex(), b = g.add_vertex(), c = g.add_vertex();
    CHECK(a != g.null_vertex());
    auto ab = g.add_edge(a, b), ab2 = g.add_edge(a, b);
    auto ba = g.add_edge(b, a), aa = g.add_edge(a, a), bc = g.add_edge(b, c);
    CHECK(g.num_vertices() == 3 && g.num_edges() == 5);
    CHECK(g.degree(a) == 5 && g.degree(b) == 4 && g.degree(c) == 1);
    CHECK(g.out_edges(a) == Graph::EdgeVector({ab, ab2, aa}));
    const auto& outgoing = g.out_edges_view(a);
    const auto snapshot = g.out_edges(a);
    CHECK(outgoing == snapshot);
    CHECK(g.adjacent_vertices(a) == Graph::VertexVector({b, b, a}));
    g[a].value = 42;
    g[bc].value = 99;
    auto* vertex_property = &g[a];
    auto* edge_property = &g[bc];
    for (int i = 0; i < 2048; ++i) {
        auto v = g.add_vertex();
        auto e = g.add_edge(v, c);
        g[e].next = e;
    }
    CHECK(&g[a] == vertex_property && g[a].value == 42);
    CHECK(&g[bc] == edge_property && g[bc].value == 99);
    g.remove_edge(a, b);
    CHECK(outgoing == Graph::EdgeVector({aa}));
    CHECK(snapshot == Graph::EdgeVector({ab, ab2, aa}));
    CHECK(!g.has_edge(a, b) && g.has_edge(b, a));
    CHECK(g.edge(a, b) == Edge());
    CHECK(g.out_edges(a) == Graph::EdgeVector({aa}));
    g.delete_vertex(a); // Includes incoming edges and a self-loop.
    CHECK(g.num_edges() == 2049 && g.num_vertices() == 2050);
    CHECK(g.out_edges(b) == Graph::EdgeVector({bc}));
    auto reused = g.add_vertex();
    CHECK(g[reused].value == 0 && g.degree(reused) == 0);
    CHECK(reused != b && reused != c);
    auto reused_edge = g.add_edge(reused, b);
    CHECK(g[reused_edge].value == 0 && g[reused_edge].next == Edge());
    CHECK(g.source(reused_edge) == reused && g.target(reused_edge) == b);
    Graph copy = g;
    copy[b].value = 10;
    copy.delete_vertex(c);
    CHECK(g[b].value == 0 && g[bc].value == 99);
    CHECK(copy.num_edges() == 1 && copy.has_edge(reused, b));
    (void)ba;
}

static void check_faces(const Graph& g) {
    for (unsigned int f = 0; f < g.num_faces(); ++f) {
        auto edges = g.face_edges(f);
        CHECK(!edges.empty());
        CHECK(g.face_vertices(f).size() == edges.size());
        for (auto e : edges) {
            CHECK(g[e].face == f);
            CHECK(g.target(e) == g.source(g[e].next));
            CHECK(g[g[e].twin].twin == e);
            CHECK(g.source(e) == g.target(g[e].twin));
            CHECK(g[g.previous_edge(e)].next == e);
        }
    }
}

static void faces() {
    Graph g;
    auto a = g.add_vertex(), b = g.add_vertex(), c = g.add_vertex();
    auto inner = g.add_face(), outer = g.add_face();
    Graph::EdgeVector in{g.add_edge(a, b), g.add_edge(b, c), g.add_edge(c, a)};
    Graph::EdgeVector out{g.add_edge(b, a), g.add_edge(c, b), g.add_edge(a, c)};
    for (std::size_t i = 0; i < 3; ++i) {
        g.twin_edges(in[i], out[i]);
        g[in[i]].face = inner;
        g[out[i]].face = outer;
        g[in[i]].next = in[(i + 1) % 3];
        g[out[i]].next = out[(i + 2) % 3];
    }
    g[inner].edge = in[0];
    g[outer].edge = out[0];
    CHECK(g.adjacent_faces(a) == Graph::FaceVector({inner, outer}));
    check_faces(g);
    for (int i = 0; i < 40; ++i) {
        auto edge = g[inner].edge;
        auto mid = g.add_vertex();
        g.insert_vertex_in_edge(mid, edge);
        CHECK(g.degree(mid) == 4);
        CHECK(g.num_edges() == 8 + 2 * static_cast<unsigned int>(i));
        check_faces(g);
    }
    Graph copy(g);
    check_faces(copy);
    copy.insert_vertex_in_edge(copy.add_vertex(), copy[inner].edge);
    CHECK(copy.num_edges() == g.num_edges() + 2);
    check_faces(copy);
    check_faces(g);

    // A twin pair can bound a single two-edge face (a hairpin).
    Graph hairpin;
    a = hairpin.add_vertex(); b = hairpin.add_vertex();
    auto e = hairpin.add_edge(a, b), t = hairpin.add_edge(b, a);
    auto f = hairpin.add_face();
    hairpin[f].edge = e;
    hairpin.twin_edges(e, t);
    hairpin[e].next = t; hairpin[t].next = e;
    hairpin.insert_vertex_in_edge(hairpin.add_vertex(), e);
    CHECK(hairpin.face_edges(f).size() == 4);
    check_faces(hairpin);
}

static void mutations() {
    // Compare mixed mutations against an independent adjacency-count model.
    Graph g;
    Graph::VertexVector vertices;
    for (int i = 0; i < 12; ++i) vertices.push_back(g.add_vertex());
    unsigned int counts[12][12] = {};
    std::mt19937 random(9481);
    for (int step = 0; step < 3000; ++step) {
        auto a = random() % 12, b = random() % 12;
        if (random() % 3) {
            g.add_edge(vertices[a], vertices[b]);
            ++counts[a][b];
        } else {
            g.remove_edge(vertices[a], vertices[b]);
            counts[a][b] = 0;
        }
        unsigned int total = 0;
        for (unsigned int i = 0; i < 12; ++i) {
            unsigned int degree = 0;
            for (unsigned int j = 0; j < 12; ++j) {
                total += counts[i][j];
                degree += counts[i][j] + counts[j][i];
                CHECK(g.has_edge(vertices[i], vertices[j]) == (counts[i][j] != 0));
            }
            CHECK(g.degree(vertices[i]) == degree);
        }
        CHECK(g.num_edges() == total && g.edges().size() == total);
    }
    for (auto v : g.vertices()) g.delete_vertex(v);
    CHECK(g.num_vertices() == 0 && g.num_edges() == 0);
}

int main() { storage(); faces(); mutations(); }
