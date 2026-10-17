#include "adaptivepathdropcutter.hpp"
#include "adaptivewaterline.hpp"
#include "cylcutter.hpp"
#include "ballcutter.hpp"
#include "bullcutter.hpp"
#include "conecutter.hpp"
#include "compositecutter.hpp"
#include "pathdropcutter.hpp"
#include "stlreader.hpp"
#include "waterline.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

using Clock = std::chrono::steady_clock;
struct Result {
    double setup_ms = 0, run_ms = 0, total_ms = 0, checksum = 0;
    std::size_t points = 0;
    void add(const ocl::Point& p) { ++points; checksum += p.x + p.y + p.z; }
};
double elapsed(Clock::time_point start) {
    return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
}
template<class Operation>
void waterline(ocl::STLSurf& surface, ocl::MillingCutter& cutter,
               int threads, Result& result) {
    Operation operation;
    auto start = Clock::now();
    operation.setSTL(surface);
    result.setup_ms += elapsed(start);
    operation.setCutter(&cutter); operation.setThreads(threads);
    operation.setSampling(.1); operation.setZ(1);
    start = Clock::now();
    operation.run();
    result.run_ms += elapsed(start);
    for (const auto& loop : operation.getLoops())
        for (const auto& p : loop) result.add(p);
}
template<class Operation>
void dropcutter(ocl::STLSurf& surface, ocl::MillingCutter& cutter,
                ocl::Path& path, int threads, Result& result) {
    Operation operation;
    auto start = Clock::now();
    operation.setSTL(surface);
    result.setup_ms += elapsed(start);
    operation.setCutter(&cutter); operation.setThreads(threads);
    operation.setSampling(.1); operation.setZ(-1); operation.setPath(&path);
    start = Clock::now();
    operation.run();
    result.run_ms += elapsed(start);
    for (const auto& p : operation.getPoints()) result.add(p);
}
int main(int argc, char** argv) {
    if (argc != 5 && argc != 6) {
        std::cerr << "Usage: cutter_benchmark model.stl "
                     "waterline|adaptive-waterline|dropcutter|adaptive-dropcutter threads repetitions "
                     "[cyl|ball|bull|cone|cyl-cone]\n";
        return 1;
    }
    const std::string filename = argv[1], mode = argv[2];
    const int threads = std::stoi(argv[3]), repetitions = std::stoi(argv[4]);
    if (threads < 1 || repetitions < 1) return 1;
    ocl::STLSurf surface;
    ocl::STLReader(std::wstring(filename.begin(), filename.end()), surface);
    const std::string cutter_type = argc == 6 ? argv[5] : "cyl";
    std::unique_ptr<ocl::MillingCutter> cutter;
    if (cutter_type == "cyl") cutter.reset(new ocl::CylCutter(4,20));
    else if (cutter_type == "ball") cutter.reset(new ocl::BallCutter(4,20));
    else if (cutter_type == "bull") cutter.reset(new ocl::BullCutter(4,1,20));
    else if (cutter_type == "cone") cutter.reset(new ocl::ConeCutter(4,.7,20));
    else if (cutter_type == "cyl-cone") cutter.reset(new ocl::CylConeCutter(2,4,.7));
    else {
        std::cerr << "Unknown cutter: " << cutter_type << '\n';
        return 1;
    }
    ocl::Path path;
    for (int row = 0; row < 25; ++row) {
        const double y = surface.bb.minpt.y + (surface.bb.maxpt.y - surface.bb.minpt.y) * row / 24;
        path.append(ocl::Line(ocl::Point(surface.bb.minpt.x - 2, y, -1),
                              ocl::Point(surface.bb.maxpt.x + 2, y, -1)));
    }
    Result result;
    for (int r = 0; r < repetitions; ++r) {
        const auto start = Clock::now();
        if (mode == "waterline") waterline<ocl::Waterline>(surface,*cutter,threads,result);
        else if (mode == "adaptive-waterline") waterline<ocl::AdaptiveWaterline>(surface,*cutter,threads,result);
        else if (mode == "dropcutter") dropcutter<ocl::PathDropCutter>(surface,*cutter,path,threads,result);
        else if (mode == "adaptive-dropcutter") dropcutter<ocl::AdaptivePathDropCutter>(surface,*cutter,path,threads,result);
        else return 1;
        result.total_ms += elapsed(start);
    }
    std::cout << std::setprecision(17)
              << "{\"setup_ms\":" << result.setup_ms/repetitions
              << ",\"run_ms\":" << result.run_ms/repetitions
              << ",\"total_ms\":" << result.total_ms/repetitions
              << ",\"points\":" << result.points/repetitions
              << ",\"checksum\":" << result.checksum/repetitions
              << ",\"triangles\":" << surface.size() << "}\n";
}
