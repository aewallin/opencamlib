find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})

# include dirs
include_directories(${OpenCamLib_SOURCE_DIR}/cutters)
include_directories(${OpenCamLib_SOURCE_DIR}/geo)
include_directories(${OpenCamLib_SOURCE_DIR}/algo)
include_directories(${OpenCamLib_SOURCE_DIR}/dropcutter)
include_directories(${OpenCamLib_SOURCE_DIR}/common)
include_directories(${OpenCamLib_SOURCE_DIR})

add_library(
  ocl
  SHARED
  ${OCL_GEO_SRC}
  ${OCL_CUTTER_SRC}
  ${OCL_ALGO_SRC}
  ${OCL_DROPCUTTER_SRC}
  ${OCL_COMMON_SRC}
)

if(WIN32)
  # Prefix all shared libraries with 'lib'.
  set(CMAKE_SHARED_LIBRARY_PREFIX "lib")
  # Prefix all static libraries with 'lib'.
  set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
endif()

target_link_libraries(
  ocl
  ${Boost_LIBRARIES}
  ${OpenMP_CXX_LIBRARIES}
)

install(
  TARGETS ocl
  LIBRARY
  DESTINATION lib/opencamlib
  ARCHIVE DESTINATION lib/opencamlib
  PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

# this installs the c++ include headers
install(
  FILES ${OCL_INCLUDE_FILES}
  DESTINATION include/opencamlib
  PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
)

if(USE_OPENMP)
  if(APPLE)
    # copy libomp into install directory
    install(
      FILES ${OpenMP_omp_LIBRARY}
      DESTINATION lib/opencamlib
      PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
    )
    # fix loader path
    add_custom_command(TARGET ocl POST_BUILD
      COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change `otool -L $<TARGET_FILE:ocl> | grep libomp | cut -d ' ' -f1 | xargs echo` "@loader_path/libomp.dylib" $<TARGET_FILE:ocl>
    )
  endif()
endif()