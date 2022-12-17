
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
#include <opencamlib/point.hpp>
#include <opencamlib/line.hpp>
#include <opencamlib/path.hpp>

void moveSafely(ocl::Point point)
{
  printf("G00 Z10\n");
  printf("G00 ");
  printf("X%g ", round(point.x * 100000.0) / 100000.0);
  printf("Y%g\n", round(point.y * 100000.0) / 100000.0);
  printf("G01 ");
  printf("Z%g", round(point.z * 100000.0) / 100000.0);
  printf(" F50\n");
}

void waterline(ocl::STLSurf surface, ocl::MillingCutter *cutter, double z, double sampling)
{
  ocl::Waterline wl = ocl::Waterline();
  wl.setSTL(surface);
  wl.setCutter(cutter);
  wl.setZ(z);
  wl.setSampling(sampling);
  wl.run();
  auto loops = wl.getLoops();
  // std::cout << "loops: " << loops.size() << "\n";
  for (auto i = 0; i < loops.size(); i++)
  {
    for (auto j = 0; j < loops[i].size(); j++)
    {
      auto point = loops[i][j];
      if (j == 0) {
        moveSafely(point);
      } else {
        printf("G01 ");
        printf("X%g ", round(point.x * 100000.0) / 100000.0);
        printf("Y%g ", round(point.y * 100000.0) / 100000.0);
        printf("Z%g\n", round(point.z * 100000.0) / 100000.0);
      }
    }
  }
}

void adaptiveWaterline(ocl::STLSurf surface, ocl::MillingCutter *cutter, double z, double sampling, double minSampling)
{
  ocl::AdaptiveWaterline awl = ocl::AdaptiveWaterline();
  awl.setSTL(surface);
  awl.setCutter(cutter);
  awl.setZ(z);
  awl.setSampling(sampling);
  awl.setMinSampling(minSampling);
  awl.run();
  auto loops = awl.getLoops();
  // std::cout << "loops: " << loops.size() << "\n";
  for (auto i = 0; i < loops.size(); i++)
  {
    for (auto j = 0; j < loops[i].size(); j++)
    {
      auto point = loops[i][j];
      if (j == 0) {
        moveSafely(point);
      } else {
        printf("G01 ");
        printf("X%g ", round(point.x * 100000.0) / 100000.0);
        printf("Y%g ", round(point.y * 100000.0) / 100000.0);
        printf("Z%g\n", round(point.z * 100000.0) / 100000.0);
      }
    }
  }
}

void pathDropCutter(ocl::STLSurf surface, ocl::MillingCutter *cutter, double sampling, ocl::Path *path)
{
  ocl::PathDropCutter pdc = ocl::PathDropCutter();
  pdc.setSTL(surface);
  pdc.setCutter(cutter);
  pdc.setPath(path);
  pdc.setZ(0);
  pdc.setSampling(sampling);
  pdc.run();
  auto points = pdc.getPoints();
  // std::cout << "points: " << points.size() << "\n";
  for (auto i = 0; i < points.size(); i++)
  {
    auto point = points[i];
    if (i == 0) {
      moveSafely(point);
    } else {
      printf("G01 ");
      printf("X%g ", round(point.x * 100000.0) / 100000.0);
      printf("Y%g ", round(point.y * 100000.0) / 100000.0);
      printf("Z%g\n", round(point.z * 100000.0) / 100000.0);
    }
  }
}

void adaptivePathDropCutter(ocl::STLSurf surface, ocl::MillingCutter *cutter, double sampling, double minSampling, ocl::Path *path)
{
  ocl::AdaptivePathDropCutter apdc = ocl::AdaptivePathDropCutter();
  apdc.setSTL(surface);
  apdc.setCutter(cutter);
  apdc.setPath(path);
  apdc.setZ(0);
  apdc.setSampling(sampling);
  apdc.setMinSampling(minSampling);
  apdc.run();
  auto points = apdc.getPoints();
  // std::cout << "points: " << points.size() << "\n";
  for (auto i = 0; i < points.size(); i++)
  {
    auto point = points[i];
    if (i == 0) {
      moveSafely(point);
    } else {
      printf("G01 ");
      printf("X%g ", round(point.x * 100000.0) / 100000.0);
      printf("Y%g ", round(point.y * 100000.0) / 100000.0);
      printf("Z%g\n", round(point.z * 100000.0) / 100000.0);
    }
  }
}

int main()
{
  // std::cout << "ocl version: " << ocl::version() << "\n";
  // std::cout << "max threads: " << ocl::max_threads() << "\n";
  ocl::STLSurf surface = ocl::STLSurf();
  std::wstring stlPath = L"../../../../stl/gnu_tux_mod.stl";
  ocl::STLReader(stlPath, surface);
  // std::cout << "surface size: " << surface.size() << "\n";
  ocl::CylCutter cutter = ocl::CylCutter(4, 20);
  double z = 1;
  double sampling = 0.1;
  waterline(surface, &cutter, z, sampling);
  // std::cout << "waterline done.\n";
  double minSampling = 0.001;
  ocl::CylCutter cutter2 = ocl::CylCutter(4, 20);
  adaptiveWaterline(surface, &cutter2, z, sampling, minSampling);
  // std::cout << "adaptivewaterline done.\n";
  ocl::Path path = ocl::Path();
  ocl::Point p1 = ocl::Point(-2, 4, 0);
  ocl::Point p2 = ocl::Point(11, 4, 0);
  ocl::Line l = ocl::Line(p1, p2);
  path.append(l);
  ocl::CylCutter cutter3 = ocl::CylCutter(4, 20);
  pathDropCutter(surface, &cutter3, sampling, &path);
  // std::cout << "pathdropcutter done.\n";
  ocl::CylCutter cutter4 = ocl::CylCutter(4, 20);
  adaptivePathDropCutter(surface, &cutter4, sampling, minSampling, &path);
  // std::cout << "adaptivepathdropcutter done.\n";
  return 0;
}

