
#include <string>
#include <iostream>
#include <opencamlib/ocl.hpp>
#include <opencamlib/waterline.hpp>
#include <opencamlib/adaptivewaterline.hpp>
#include <opencamlib/pathdropcutter.hpp>
#include <opencamlib/adaptivepathdropcutter.hpp>
#include <opencamlib/stlsurf.hpp>
#include <opencamlib/stlreader.hpp>
#include <opencamlib/cylcutter.hpp>
#include <opencamlib/ballcutter.hpp>
#include <opencamlib/bullcutter.hpp>
#include <opencamlib/conecutter.hpp>
#include <opencamlib/point.hpp>
#include <opencamlib/line.hpp>
#include <opencamlib/path.hpp>

void printXYZ(ocl::Point point)
{
  // printf("X%g ", round(point.x * 100000.0) / 100000.0);
  // printf("Y%g ", round(point.y * 100000.0) / 100000.0);
  // printf("Z%g", round(point.z * 100000.0) / 100000.0);
}

void linear(ocl::Point point)
{
  // printf("G01 ");
  // printXYZ(point);
  // printf("\n");
}

void moveSafely(ocl::Point point)
{
  // printf("G00 Z10\n");
  // printf("G00 ");
  // printf("X%g ", round(point.x * 100000.0) / 100000.0);
  // printf("Y%g\n", round(point.y * 100000.0) / 100000.0);
  // printf("G01 ");
  // printf("Z%g", round(point.z * 100000.0) / 100000.0);
  // printf(" F50\n");
}

void printPoints(std::vector<ocl::Point> points)
{
  for (auto j = 0; j < points.size(); j++)
  {
    auto point = points[j];
    if (j == 0)
      moveSafely(point);
    else
      linear(point);
  }
}

void printPoints(std::vector<ocl::CLPoint> points)
{
  for (auto j = 0; j < points.size(); j++)
  {
    auto point = points[j];
    if (j == 0)
      moveSafely(point);
    else
      linear(point);
  }
}

void printLoops(std::vector<std::vector<ocl::Point>> loops)
{
  for (auto i = 0; i < loops.size(); i++)
  {
    printPoints(loops[i]);
  }
}

void waterline(ocl::STLSurf surface, ocl::MillingCutter *cutter, double z, double sampling)
{
  ocl::Waterline wl = ocl::Waterline();
  wl.setSTL(surface);
  wl.setCutter(cutter);
  wl.setSampling(sampling);
  for (double h = 0; h < z; h = h + 0.1)
  {
    wl.reset();
    wl.setZ(h);
    wl.run();
    auto loops = wl.getLoops();
    printLoops(loops);
  }
}

void adaptiveWaterline(ocl::STLSurf surface, ocl::MillingCutter *cutter, double z, double sampling, double minSampling)
{
  ocl::AdaptiveWaterline awl = ocl::AdaptiveWaterline();
  awl.setSTL(surface);
  awl.setCutter(cutter);
  awl.setSampling(sampling);
  awl.setMinSampling(minSampling);
  for (double h = 0; h < z; h = h + 0.1)
  {
    awl.reset();
    awl.setZ(h);
    awl.run();
    auto loops = awl.getLoops();
    printLoops(loops);
  }
}

void pathDropCutter(ocl::STLSurf surface, ocl::MillingCutter *cutter, double sampling, ocl::Path *path)
{
  ocl::PathDropCutter pdc = ocl::PathDropCutter();
  pdc.setSTL(surface);
  pdc.setCutter(cutter);
  pdc.setPath(path);
  pdc.setSampling(sampling);
  pdc.reset();
  pdc.setZ(0);
  pdc.run();
  auto points = pdc.getPoints();
  printPoints(points);
}

void adaptivePathDropCutter(ocl::STLSurf surface, ocl::MillingCutter *cutter, double sampling, double minSampling, ocl::Path *path)
{
  ocl::AdaptivePathDropCutter apdc = ocl::AdaptivePathDropCutter();
  apdc.setSTL(surface);
  apdc.setCutter(cutter);
  apdc.setPath(path);
  apdc.setSampling(sampling);
  apdc.setMinSampling(minSampling);
  apdc.reset();
  apdc.setZ(0);
  apdc.run();
  auto points = apdc.getPoints();
  printPoints(points);
}

int main()
{
  std::cout << "ocl version: " << ocl::version() << "\n";
  std::cout << "max threads: " << ocl::max_threads() << "\n";
  ocl::STLSurf surface = ocl::STLSurf();
  std::wstring stlPath = L"../../../../stl/gnu_tux_mod.stl";
  ocl::STLReader(stlPath, surface);
  std::cout << "surface size: " << surface.size() << "\n";

  ocl::CylCutter cylCutter = ocl::CylCutter(0.4, 10);
  ocl::BallCutter ballCutter = ocl::BallCutter(4, 20);
  ocl::BullCutter bullCutter = ocl::BullCutter(4, 0.05, 20);
  ocl::ConeCutter coneCutter = ocl::ConeCutter(4, 0.05, 20);
  std::vector<ocl::MillingCutter *> cutters;
  cutters.push_back(&cylCutter);
  cutters.push_back(&ballCutter);
  cutters.push_back(&bullCutter);
  cutters.push_back(&coneCutter);
  double z = 0.5;
  double sampling = 0.1;
  for (auto cutter : cutters)
  {
    std::cout << "WL + Cutter: " << cutter->str() << "\n";
    waterline(surface, cutter, z, sampling);
  }
  double minSampling = 0.01;
  for (auto cutter : cutters)
  {
    std::cout << "AWL + Cutter: " << cutter->str() << "\n";
    adaptiveWaterline(surface, cutter, z, sampling, minSampling);
  }
  ocl::Path path = ocl::Path();
  int i = 0;
  for (double y = 0; y <= 0.2; y = y + 0.1)
  {
    bool ltr = ((int)i % 2) == 0;
    ocl::Point p1 = ocl::Point(ltr ? -2 : 11, y, 0);
    ocl::Point p2 = ocl::Point(ltr ? 11 : -2, y, 0);
    ocl::Line l = ocl::Line(p1, p2);
    path.append(l);
    ocl::Point p3 = ocl::Point(ltr ? 11 : -2, y + 1, 0);
    ocl::Line l2 = ocl::Line(p2, p3);
    path.append(l2);
    i++;
  }
  for (auto cutter : cutters)
  {
    std::cout << "PDC + Cutter: " << cutter->str() << "\n";
    pathDropCutter(surface, cutter, sampling, &path);
  }
  for (auto cutter : cutters)
  {
    std::cout << "APDC: " << cutter->str() << "\n";
    adaptivePathDropCutter(surface, cutter, sampling, minSampling, &path);
  }

  return 0;
}