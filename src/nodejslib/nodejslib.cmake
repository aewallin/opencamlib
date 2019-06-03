message(STATUS "Will build node.js library")

if(CMAKE_BUILD_TYPE EQUAL "Debug")
	message(STATUS "Debug build, passing -g flag to compiler to emit source location (handy for debugging with lldb)")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${OpenCamLib_SOURCE_DIR}/../node_modules/node-addon-api)
include_directories(${CMAKE_JS_INC})

# include dirs
include_directories( ${OpenCamLib_SOURCE_DIR}/cutters )
include_directories( ${OpenCamLib_SOURCE_DIR}/geo )
include_directories( ${OpenCamLib_SOURCE_DIR}/algo )
include_directories( ${OpenCamLib_SOURCE_DIR}/dropcutter )
include_directories( ${OpenCamLib_SOURCE_DIR}/common )
include_directories( ${OpenCamLib_SOURCE_DIR} )

include_directories(${OpenCamLib_SOURCE_DIR}/nodejslib)

add_library(opencamlib
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
	${OpenCamLib_SOURCE_DIR}/nodejslib/adaptivepathdropcutter_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/adaptivewaterline_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/cylcutter_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/ballcutter_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/bullcutter_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/conecutter_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/nodejslib.cpp
)

target_link_libraries(
  opencamlib
  ${Boost_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${CMAKE_JS_LIB}
)

if(WIN32)
	set(NODE_LIB_POSTFIX ".win32.node")
elseif(APPLE)
	set(NODE_LIB_POSTFIX ".darwin.node")
else()
	set(NODE_LIB_POSTFIX ".linux.node")
endif()

set_target_properties(opencamlib PROPERTIES PREFIX "" SUFFIX ${NODE_LIB_POSTFIX})
