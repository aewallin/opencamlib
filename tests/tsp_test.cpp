#include "tsp.hpp"
#include "check.hpp"

#include <algorithm>
#include <random>
#include <utility>

using Points = std::vector<std::pair<double, double>>;
static double distance(const Points& p, std::size_t a, std::size_t b) {
    return std::hypot(p[a].first - p[b].first, p[a].second - p[b].second);
}

static void verify(const Points& points) {
    ocl::tsp::TSPSolver solver;
    for (auto p : points) solver.addPoint(p.first, p.second);
    solver.run();
    auto output = solver.getOutput();
    if (points.empty()) {
        CHECK(output.empty() && solver.getLength() == 0.0);
        return;
    }
    CHECK(output.size() == points.size() + 1);
    CHECK(output.front() == 0 && output.back() == 0);
    auto sorted = output;
    sorted.pop_back();
    std::sort(sorted.begin(), sorted.end());
    for (std::size_t i = 0; i < points.size(); ++i) CHECK(sorted[i] == i);
    double length = 0;
    for (std::size_t i = 1; i < output.size(); ++i)
        length += distance(points, output[i - 1], output[i]);
    CHECK(std::abs(length - solver.getLength()) <= 1e-12 * std::max(1.0, length));

    // Brute force a separate optimum for small cases, checking the approximation bound.
    if (points.size() <= 8) {
        double optimal = (std::numeric_limits<double>::infinity)();
        do {
            double candidate = distance(points, sorted.back(), sorted.front());
            for (std::size_t i = 1; i < sorted.size(); ++i)
                candidate += distance(points, sorted[i - 1], sorted[i]);
            optimal = std::min(optimal, candidate);
        } while (std::next_permutation(sorted.begin() + 1, sorted.end()));
        CHECK(length <= 2 * optimal + 1e-10 * std::max(1.0, optimal));
    }
    solver.run();
    CHECK(solver.getOutput() == output && solver.getLength() == length);
    auto copy = solver;
    copy.reset();
    copy.addPoint(9, 9);
    copy.run();
    CHECK(copy.getOutput() == std::vector<std::size_t>({0, 0}));
    CHECK(solver.getOutput() == output);
    solver.addPoint(3, 8);
    solver.run();
    CHECK(solver.getOutput().size() == points.size() + 2);
    solver.reset();
    solver.run();
    CHECK(solver.getOutput().empty() && solver.getLength() == 0.0);
}

int main() {
    verify({});
    verify({{1, 2}});
    verify({{0, 0}, {3, 4}});
    verify({{0, 0}, {1, 0}, {1, 1}, {0, 1}});
    verify({{0, 0}, {0, 0}, {0, 0}, {0, 0}});
    verify({{-5, 0}, {0, 0}, {4, 0}, {-1, 0}, {0, 0}});
    verify({{0, 0}, {1e150, 0}, {1e150, 1e150}, {0, 1e150}});
    std::mt19937 random(8793);
    for (int trial = 0; trial < 30; ++trial) {
        Points p;
        for (int i = 0; i < 8; ++i)
            p.emplace_back(static_cast<int>(random() % 200) - 100,
                           static_cast<int>(random() % 200) - 100);
        verify(p);
    }
    Points line;
    for (int i = 0; i < 2000; ++i) line.emplace_back(i, 0);
    verify(line); // Deep tree: traversal must not depend on the call stack.
}
