# CmakeLists.txt for OpenCAMLib src/algo directory

message(STATUS " configuring src/algo")

if (UNIX)
  add_definitions(-fPIC)
endif (UNIX)

include_directories( ${PROJECT_SOURCE_DIR} )
include_directories( ${PROJECT_SOURCE_DIR}/geo )
include_directories( ${PROJECT_SOURCE_DIR}/cutters )
include_directories( ${PROJECT_SOURCE_DIR}/algo )
include_directories( ${PROJECT_SOURCE_DIR}/common )

#
# this builds a static ocl_algo target
#

add_library(
  ocl_algo
  STATIC
  ${OCL_ALGO_SRC}
  )
