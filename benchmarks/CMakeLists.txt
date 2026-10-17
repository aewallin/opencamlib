add_executable(cutter_benchmark cutter_benchmark.cpp)
target_include_directories(cutter_benchmark PRIVATE
  ${PROJECT_SOURCE_DIR}/common
  ${PROJECT_SOURCE_DIR}/geo
  ${PROJECT_SOURCE_DIR}/algo
  ${PROJECT_SOURCE_DIR}/cutters
  ${PROJECT_SOURCE_DIR}/dropcutter)
target_link_libraries(cutter_benchmark PRIVATE
  ocl_dropcutter ocl_algo ocl_cutters ocl_geo ocl_common)
if(USE_OPENMP)
  target_link_libraries(cutter_benchmark PRIVATE OpenMP::OpenMP_CXX)
endif()
