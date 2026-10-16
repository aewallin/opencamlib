find_package(Python3 COMPONENTS Interpreter Development.Module REQUIRED)
if(Python3_FOUND)
  message(STATUS "Found Python: " ${Python3_VERSION})
  message(STATUS "Python libraries: " ${Python3_LIBRARIES})
  message(STATUS "Python executable: " ${Python3_EXECUTABLE})
  message(STATUS "Python (arch-dependant) module destination: " ${Python3_SITEARCH})
endif()

find_package(pybind11 REQUIRED)

# include dirs
include_directories(${PROJECT_SOURCE_DIR}/cutters)
include_directories(${PROJECT_SOURCE_DIR}/geo)
include_directories(${PROJECT_SOURCE_DIR}/algo)
include_directories(${PROJECT_SOURCE_DIR}/dropcutter)
include_directories(${PROJECT_SOURCE_DIR}/common)
include_directories(${PROJECT_SOURCE_DIR})

# this makes the ocl Python module
pybind11_add_module(
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
)

if(USE_OPENMP)
  target_link_libraries(ocl PRIVATE OpenMP::OpenMP_CXX)
endif()

if(SKBUILD)
  install(TARGETS ocl LIBRARY DESTINATION "opencamlib")
else()
  install(TARGETS ocl LIBRARY DESTINATION "${Python3_SITEARCH}/opencamlib")
  install(
    DIRECTORY pythonlib/opencamlib/
    DESTINATION "${Python3_SITEARCH}/opencamlib"
  )
endif()

if(USE_OPENMP AND APPLE)
  # add homebrew libomp paths to the INSTALL_RPATH, and the @loader_path last as a fallback.
  set_target_properties(ocl PROPERTIES
    INSTALL_RPATH "/opt/homebrew/opt/libomp/lib;/usr/local/opt/libomp/lib;@loader_path")
  # copy libomp into install directory
  install(
    FILES ${OpenMP_CXX_LIBRARIES}
    DESTINATION "opencamlib"
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
  )
  # fix loader path
  add_custom_command(TARGET ocl POST_BUILD
    COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change `otool -L $<TARGET_FILE:ocl> | grep libomp | cut -d ' ' -f1 | xargs echo` "@rpath/libomp.dylib" $<TARGET_FILE:ocl>
  )
endif()
