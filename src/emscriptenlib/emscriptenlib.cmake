message(STATUS "Will build emscripten js library")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g4 -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --bind -s MODULARIZE=1 -s SINGLE_FILE=1 -s ALLOW_MEMORY_GROWTH=1")

find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})

# include dirs
include_directories( ${OpenCamLib_SOURCE_DIR}/cutters )
include_directories( ${OpenCamLib_SOURCE_DIR}/geo )
include_directories( ${OpenCamLib_SOURCE_DIR}/algo )
include_directories( ${OpenCamLib_SOURCE_DIR}/dropcutter )
include_directories( ${OpenCamLib_SOURCE_DIR}/common )
include_directories( ${OpenCamLib_SOURCE_DIR} )

include_directories(${OpenCamLib_SOURCE_DIR}/emscriptenlib)

add_executable(opencamlib
	# SHARED
	${OCL_GEO_SRC}
	${OCL_CUTTER_SRC}
	${OCL_DROPCUTTER_SRC}
	${OCL_ALGO_SRC}
	${OCL_COMMON_SRC}
	${OpenCamLib_SOURCE_DIR}/emscriptenlib/emscriptenlib.cpp
)

target_link_libraries(
  opencamlib
  ${Boost_LIBRARIES}
)
