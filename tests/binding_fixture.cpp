#include "pathdropcutter.hpp"
#include "adaptivepathdropcutter.hpp"
#include "cylcutter.hpp"
#include "stlsurf.hpp"
#include "check.hpp"
#include <fstream>
#include <iomanip>

template<class Operation>
void output(double scale, double floor, double expected) {
    ocl::STLSurf surface;
    surface.addTriangle(ocl::Triangle({-10*scale,-10*scale,scale},
        {10*scale,-10*scale,scale}, {0,10*scale,scale}));
    ocl::CylCutter cutter(.5*scale, 5*scale);
    ocl::Path path;
    path.append(ocl::Line({-scale,0,-10*scale}, {scale,0,-10*scale}));
    path.append(ocl::Arc({scale,0,-10*scale}, {-scale,0,-10*scale}, {0,0,-10*scale}, true));
    Operation operation;
    operation.setSTL(surface); operation.setCutter(&cutter); operation.setPath(&path);
    operation.setSampling(.2*scale); operation.setZ(floor); operation.setThreads(1);
    operation.run();
    const auto result = operation.getCLPoints();
    operation.run();
    CHECK(operation.getCLPoints() == result);
    std::cout << '[';
    bool first = true;
    for (const auto& point : result) {
        CHECK(std::abs(point.z-expected) < 1e-9*scale);
        if (!first) std::cout << ',';
        first = false;
        std::cout << '[' << point.x << ',' << point.y << ',' << point.z << ']';
    }
    std::cout << "]\n";
}
int main(int argc, char** argv) {
    CHECK(argc == 2);
    std::ifstream input(argv[1]);
    CHECK(input.good());
    std::cout << std::setprecision(17);
    double scale, floor, expected;
    while (input >> scale >> floor >> expected) {
        output<ocl::PathDropCutter>(scale, floor, expected);
        output<ocl::AdaptivePathDropCutter>(scale, floor, expected);
    }
}
