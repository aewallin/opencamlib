# CmakeLists.txt for OpenCAMLib src/algo directory
project(OpenCamLib_algorithms)

cmake_minimum_required(VERSION 2.4)

message(STATUS " configuring src/algo")

if (UNIX)
  add_definitions(-fPIC)
endif (UNIX)

include_directories( ${OpenCamLib_SOURCE_DIR} )
include_directories( ${OpenCamLib_SOURCE_DIR}/geo )
include_directories( ${OpenCamLib_SOURCE_DIR}/cutters )
include_directories( ${OpenCamLib_SOURCE_DIR}/algo )
include_directories( ${OpenCamLib_SOURCE_DIR}/common )

#
# this builds a static ocl_algo target
#

add_library(
  ocl_algo
  STATIC
  ${OCL_ALGO_SRC}
  )
