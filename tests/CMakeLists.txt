foreach(test_name halfedge tsp geometry kdtree cutter)
  add_executable(${test_name}_test ${test_name}_test.cpp)
  target_include_directories(${test_name}_test PRIVATE
    ${PROJECT_SOURCE_DIR}/common
    ${PROJECT_SOURCE_DIR}/geo
    ${PROJECT_SOURCE_DIR}/algo
    ${PROJECT_SOURCE_DIR}/cutters
    ${PROJECT_SOURCE_DIR}/dropcutter)
  add_test(NAME ${test_name} COMMAND ${test_name}_test)
endforeach()

foreach(test_name geometry kdtree cutter)
  target_link_libraries(${test_name}_test PRIVATE
    ocl_dropcutter ocl_algo ocl_cutters ocl_geo ocl_common)
  if(USE_OPENMP)
    target_link_libraries(${test_name}_test PRIVATE OpenMP::OpenMP_CXX)
  endif()
endforeach()
