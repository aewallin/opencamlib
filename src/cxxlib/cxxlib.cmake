add_library(ocl
  SHARED
    ${OCL_SRC}
    ${OCL_GEO_SRC}
    ${OCL_CUTTER_SRC}
    ${OCL_ALGO_SRC}
    ${OCL_DROPCUTTER_SRC}
    ${OCL_COMMON_SRC}
)

if(WIN32)
  # on windows, prefix the library with lib, and make sure the .lib file is installed as well
  set_target_properties(ocl PROPERTIES
      PREFIX "lib"
      WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

include(GNUInstallDirs)

# add headers
set_property(TARGET ocl PROPERTY PUBLIC_HEADER ${OCL_INCLUDE_FILES})

# add include directories
target_include_directories(ocl
  PRIVATE
    ${PROJECT_SOURCE_DIR}/cutters
    ${PROJECT_SOURCE_DIR}/geo
    ${PROJECT_SOURCE_DIR}/algo
    ${PROJECT_SOURCE_DIR}/dropcutter
    ${PROJECT_SOURCE_DIR}/common
    ${PROJECT_SOURCE_DIR}
  PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# disable /GL and enable /LTCG (see https://github.com/luxonis/depthai-core/issues/334)
if(WIN32 AND MSVC) # AND CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS
  get_target_property(_INTER_OPT ocl INTERPROCEDURAL_OPTIMIZATION)
  if(_INTER_OPT)
    message(STATUS "Workaround MSVC dll exports with INTERPROCEDURAL_OPTIMIZATION")
    set_target_properties(ocl PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF)
    target_link_options(ocl PRIVATE /LTCG)
  endif()
  unset(_INTER_OPT)
endif()

# link with Boost and optionally with OpenMP
target_link_libraries(ocl PUBLIC Boost::boost)
if(USE_OPENMP)
  target_link_libraries(ocl PRIVATE OpenMP::OpenMP_CXX)
endif()

# this installs the ocl library
install(
  TARGETS ocl
  EXPORT ocltargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/opencamlib
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/opencamlib
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/opencamlib
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/opencamlib
  PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

if(APPLE)
  target_compile_options(ocl PRIVATE -g)
  # if(CMAKE_CXX_FLAGS MATCHES "-flto")
    set(lto_object ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/ocl-lto.o)
    set_property(TARGET ocl APPEND_STRING PROPERTY
      LINK_FLAGS " -Wl,-object_path_lto -Wl,${lto_object}")
  # endif()
  add_custom_command(TARGET ocl POST_BUILD
    COMMAND xcrun dsymutil $<TARGET_FILE:ocl>
    COMMAND xcrun strip -Sl $<TARGET_FILE:ocl>)
  install(
    FILES $<TARGET_FILE:ocl>.dSYM
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/opencamlib
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
  )
endif()

# this install the cmake targets
install(
  EXPORT ocltargets
  FILE OpenCAMLibTargets.cmake
  DESTINATION share/cmake/OpenCAMLib
  NAMESPACE OpenCAMLib::
)

# this creates the cmake config
include(CMakePackageConfigHelpers)
configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/../cmake/${PROJECT_NAME}Config.cmake.in"
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake
)

# this installs the cmake config
install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/${PROJECT_NAME}
)

if(USE_OPENMP AND APPLE)
  # add homebrew libomp paths to the INSTALL_RPATH, and the @loader_path last as a fallback.
  set_target_properties(ocl PROPERTIES
    INSTALL_RPATH "/opt/homebrew/opt/libomp/lib;/usr/local/opt/libomp/lib;@loader_path")
  # copy libomp into install directory
  install(
    FILES ${OpenMP_CXX_LIBRARIES}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/opencamlib
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
  )
  # fix loader path
  add_custom_command(TARGET ocl POST_BUILD
    COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change `otool -L $<TARGET_FILE:ocl> | grep libomp | cut -d ' ' -f1 | xargs echo` "@rpath/libomp.dylib" $<TARGET_FILE:ocl>
  )
endif()