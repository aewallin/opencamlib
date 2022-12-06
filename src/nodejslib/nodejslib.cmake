message(STATUS "Will build node.js library")

include_directories(${OpenCamLib_SOURCE_DIR}/nodejslib/node_modules/node-addon-api)
include_directories(${CMAKE_JS_INC})

# include dirs
include_directories(${OpenCamLib_SOURCE_DIR}/cutters)
include_directories(${OpenCamLib_SOURCE_DIR}/geo)
include_directories(${OpenCamLib_SOURCE_DIR}/algo)
include_directories(${OpenCamLib_SOURCE_DIR}/dropcutter)
include_directories(${OpenCamLib_SOURCE_DIR}/common)
include_directories(${OpenCamLib_SOURCE_DIR})

include_directories(${OpenCamLib_SOURCE_DIR}/nodejslib)

add_library(ocl
	SHARED
	${OCL_GEO_SRC}
	${OCL_CUTTER_SRC}
	${OCL_DROPCUTTER_SRC}
	${OCL_ALGO_SRC}
	${OCL_COMMON_SRC}
	${OpenCamLib_SOURCE_DIR}/nodejslib/point_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/triangle_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/bbox_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/cylcutter_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/line_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/path_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/stlsurf_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/stlreader_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/waterline_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/pathdropcutter_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/adaptivepathdropcutter_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/adaptivewaterline_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/cylcutter_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/ballcutter_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/bullcutter_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/conecutter_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/nodejslib.cpp
)

target_link_libraries(
  ocl
  ${Boost_LIBRARIES}
  ${OpenMP_CXX_LIBRARIES}
  ${CMAKE_JS_LIB}
)

set_target_properties(ocl PROPERTIES PREFIX "" SUFFIX ".node")

add_definitions(-DNAPI_VERSION=3)

if(USE_OPENMP AND APPLE)
  # copy libomp into install directory
  install(
    FILES ${OpenMP_omp_LIBRARY}
    DESTINATION "."
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
  )
  # fix loader path
  add_custom_command(TARGET ocl POST_BUILD
    COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change `otool -L $<TARGET_FILE:ocl> | grep libomp | cut -d ' ' -f1 | xargs echo` "@loader_path/libomp.dylib" $<TARGET_FILE:ocl>
  )
endif()

install(
	TARGETS ocl
	DESTINATION "."
)
