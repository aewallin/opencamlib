#include "kdtree.hpp"
#include "check.hpp"

#include <algorithm>
#include <atomic>
#include <list>
#include <memory>
#include <vector>

struct Object {
    ocl::Bbox bb;
    int id;
    static std::atomic<int> copies;
    Object(const ocl::Bbox& box, int value) : bb(box), id(value) {}
    Object(const Object& other) : bb(other.bb), id(other.id) { ++copies; }
};
std::atomic<int> Object::copies{0};

static void exercise(int plane, int bucket) {
    ocl::KDTree<Object> tree;
    if (plane == 0) tree.setXYDimensions();
    if (plane == 1) tree.setYZDimensions();
    if (plane == 2) tree.setXZDimensions();
    tree.setBucketSize(bucket);
    const ocl::Bbox all(-100,100,-100,100,-100,100);
    std::vector<int> ids;
    tree.visit(all, [&](const Object& o) { ids.push_back(o.id); });
    CHECK(ids.empty());
    tree.build({});
    tree.visit(all, [&](const Object& o) { ids.push_back(o.id); });
    CHECK(ids.empty());

    std::list<Object> input;
    for (int i = 0; i < 60; ++i) {
        double x = i % 7, y = i % 11, z = i % 13;
        input.emplace_back(ocl::Bbox(x, x + (i%3), y, y + (i%4), z, z + (i%5)), i);
    }
    Object::copies = 0;
    tree.build(input);
    CHECK(Object::copies == 60); // one owned snapshot, independent of depth
    Object::copies = 0;
    tree.visit(all, [&](const Object& o) { ids.push_back(o.id); });
    CHECK(Object::copies == 0);
    CHECK(ids.size() == input.size());
    auto sorted = ids;
    std::sort(sorted.begin(), sorted.end());
    for (int i = 0; i < 60; ++i) CHECK(sorted[i] == i);

    for (int q = -2; q < 16; ++q) {
        const ocl::Bbox query(q, q+1, q, q+1, q, q+1);
        std::vector<int> found;
        tree.visit(query, [&](const Object& o) { found.push_back(o.id); });
        CHECK(Object::copies == 0);
        const int axes[3][2] = {{0,2},{2,4},{0,4}};
        for (const auto& o : input) {
            bool overlaps = true;
            for (int axis : axes[plane])
                overlaps &= o.bb[axis] <= query[axis+1] && o.bb[axis+1] >= query[axis];
            if (overlaps) CHECK(std::find(found.begin(),found.end(),o.id) != found.end());
        }
        std::unique_ptr<std::list<Object>> owned(tree.search(query));
        std::vector<int> copied;
        for (const auto& o : *owned) copied.push_back(o.id);
        CHECK(copied == found);
        CHECK(Object::copies == static_cast<int>(found.size()));
        Object::copies = 0;
    }
    input.clear(); // references are owned by the tree, not by its input
    #pragma omp parallel for num_threads(4)
    for (int iteration = 0; iteration < 32; ++iteration) {
        std::vector<int> result;
        tree.visit(all, [&](const Object& o) { result.push_back(o.id); });
        CHECK(result == ids);
    }
    CHECK(Object::copies == 0);
    tree.build({});
    ids.clear();
    tree.visit(all, [&](const Object& o) { ids.push_back(o.id); });
    CHECK(ids.empty());
}

int main() {
    for (int plane = 0; plane < 3; ++plane)
        for (int bucket : {1,4,100}) exercise(plane,bucket);
    // Fixed fixtures lock down hi-before-lo order for both min and max splits.
    for (bool max_split : {false,true}) {
        ocl::KDTree<Object> tree;
        tree.setXYDimensions();
        std::list<Object> input;
        for (int i = 0; i < 4; ++i)
            input.emplace_back(ocl::Bbox(max_split ? 0 : i*2, i*2+1, 0, 0, 0, 0), i);
        tree.build(input);
        std::vector<int> ids;
        tree.visit(ocl::Bbox(-10,10,-10,10,-10,10), [&](const Object& o) { ids.push_back(o.id); });
        CHECK(ids == std::vector<int>({3,2,1,0}));
    }
    // Coincident objects cannot be split, even with bucket size one.
    ocl::KDTree<Object> tree;
    tree.setXYDimensions();
    std::list<Object> input;
    for (int i = 0; i < 5; ++i) input.emplace_back(ocl::Bbox(0,0,0,0,0,0), i);
    tree.build(input);
    int count = 0;
    tree.visit(ocl::Bbox(0,0,0,0,0,0), [&](const Object& o) { CHECK(o.id == count++); });
    CHECK(count == 5);
}
