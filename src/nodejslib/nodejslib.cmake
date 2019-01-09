message("Creating node.js library")

include(${OpenCamLib_SOURCE_DIR}/nodejslib/nodemodule.cmake)

set(NODE_MODULE_MINIMUM_ABI 67)

if (APPLE)
	set(INSTALL_PATH  "nodejslib/opencamlib.darwin.node")
else()
	set(INSTALL_PATH  "nodejslib/opencamlib.linux.node")
endif()
message(${INSTALL_PATH})
add_node_module(opencamlib
	NAN_VERSION 2.12.1
	INSTALL_PATH ${INSTALL_PATH}
)
foreach(TARGET IN LISTS opencamlib::targets)
	message(${TARGET})
	target_link_libraries(${TARGET} ${Boost_LIBRARIES})
	include_directories(${TARGET} SYSTEM PRIVATE ${OpenCamLib_SOURCE_DIR}/../node_modules/node-addon-api)
	target_include_directories(${TARGET} SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
endforeach()
target_sources(opencamlib INTERFACE
	${OCL_GEO_SRC}
	${OCL_CUTTER_SRC}
	${OCL_DROPCUTTER_SRC}
	${OCL_ALGO_SRC}
	${OCL_COMMON_SRC}
	${OpenCamLib_SOURCE_DIR}/nodejslib/point_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/stlsurf_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/stlreader_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/triangle_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/bbox_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/cylcutter_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/waterline_js.cpp
	${OpenCamLib_SOURCE_DIR}/nodejslib/nodejslib.cpp
)
