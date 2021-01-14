message(STATUS "Will build emscripten js library")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g4 -O0 -s DISABLE_EXCEPTION_CATCHING=0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 --closure 1")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --bind -s MODULARIZE=1 -s ASSERTIONS=1 -s ALLOW_MEMORY_GROWTH=1")

# include dirs
include_directories( ${OpenCamLib_SOURCE_DIR}/cutters )
include_directories( ${OpenCamLib_SOURCE_DIR}/geo )
include_directories( ${OpenCamLib_SOURCE_DIR}/algo )
include_directories( ${OpenCamLib_SOURCE_DIR}/dropcutter )
include_directories( ${OpenCamLib_SOURCE_DIR}/common )
include_directories( ${OpenCamLib_SOURCE_DIR} )

include_directories( ${OpenCamLib_SOURCE_DIR}/emscriptenlib )

add_executable(opencamlib
	# SHARED
	${OCL_GEO_SRC}
	${OCL_CUTTER_SRC}
	${OCL_DROPCUTTER_SRC}
	${OCL_ALGO_SRC}
	${OCL_COMMON_SRC}
	${OpenCamLib_SOURCE_DIR}/emscriptenlib/emscriptenlib.cpp
)

message(STATUS "Boost_LIBRARIES:" ${Boost_LIBRARIES})
target_link_libraries(
  opencamlib
  ${Boost_LIBRARIES}
)
