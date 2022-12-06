find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
find_package(Boost COMPONENTS python${Python_VERSION_MAJOR}${Python_VERSION_MINOR} REQUIRED)

# include dirs
include_directories(${OpenCamLib_SOURCE_DIR}/cutters)
include_directories(${OpenCamLib_SOURCE_DIR}/geo)
include_directories(${OpenCamLib_SOURCE_DIR}/algo)
include_directories(${OpenCamLib_SOURCE_DIR}/dropcutter)
include_directories(${OpenCamLib_SOURCE_DIR}/common)
include_directories(${OpenCamLib_SOURCE_DIR})

# this makes the ocl Python module
Python_add_library(
  ocl
MODULE
  pythonlib/ocl_cutters.cpp
  pythonlib/ocl_geometry.cpp
  pythonlib/ocl_algo.cpp
  pythonlib/ocl_dropcutter.cpp
  pythonlib/ocl.cpp
)

target_link_libraries(
  ocl
PRIVATE
  ocl_common
  ocl_dropcutter
  ocl_cutters
  ocl_geo
  ocl_algo
  Boost::python${Python_VERSION_MAJOR}${Python_VERSION_MINOR}
  Python::Module
)

if(USE_OPENMP)
  target_link_libraries(ocl PRIVATE OpenMP::OpenMP_CXX)
endif()

install(TARGETS ocl LIBRARY DESTINATION "opencamlib")
if(NOT SKBUILD)
  install(
    DIRECTORY pythonlib/opencamlib/
    DESTINATION "opencamlib"
  )
endif()

if(USE_OPENMP AND APPLE)
  # copy libomp into install directory
  install(
    FILES ${OpenMP_omp_LIBRARY}
    DESTINATION "opencamlib"
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
  )
  # fix loader path
  add_custom_command(TARGET ocl POST_BUILD
    COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change `otool -L $<TARGET_FILE:ocl> | grep libomp | cut -d ' ' -f1 | xargs echo` "@loader_path/libomp.dylib" $<TARGET_FILE:ocl>
  )
endif()
