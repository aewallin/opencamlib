find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${OpenCamLib_SOURCE_DIR}/../node_modules/node-addon-api)

# this branches into the dirs and compiles stuff there
add_subdirectory( ${OpenCamLib_SOURCE_DIR}/cutters  )
add_subdirectory( ${OpenCamLib_SOURCE_DIR}/geo  )
add_subdirectory( ${OpenCamLib_SOURCE_DIR}/algo  ) 
add_subdirectory( ${OpenCamLib_SOURCE_DIR}/dropcutter  ) 
add_subdirectory( ${OpenCamLib_SOURCE_DIR}/common  ) 

# include dirs
include_directories( ${OpenCamLib_SOURCE_DIR}/cutters )
include_directories( ${OpenCamLib_SOURCE_DIR}/geo )
include_directories( ${OpenCamLib_SOURCE_DIR}/algo )
include_directories( ${OpenCamLib_SOURCE_DIR}/dropcutter )
include_directories( ${OpenCamLib_SOURCE_DIR}/common )
include_directories( ${OpenCamLib_SOURCE_DIR} )

include_directories(${CMAKE_JS_INC})
add_library(${PROJECT_NAME} SHARED
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

if(WIN32)
	set(NODE_LIB_POSTFIX ".win32.node")
elseif(APPLE)
	set(NODE_LIB_POSTFIX ".darwin.node")
else()
	set(NODE_LIB_POSTFIX ".linux.node")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ${NODE_LIB_POSTFIX})
link_libraries(${CMAKE_JS_LIB})
link_libraries(${Boost_LIBRARIES})