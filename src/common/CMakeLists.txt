# CmakeLists.txt for OpenCAMLib src/common directory

message(STATUS " configuring src/common")

if (UNIX)
    add_definitions(-fPIC)
endif (UNIX)

include_directories( ${OpenCamLib_SOURCE_DIR} )
include_directories( ${OpenCamLib_SOURCE_DIR}/geo )
include_directories( ${OpenCamLib_SOURCE_DIR}/algo )
include_directories( ${OpenCamLib_SOURCE_DIR}/dropcutter )
include_directories( ${OpenCamLib_SOURCE_DIR}/cutters )
include_directories( ${OpenCamLib_SOURCE_DIR}/common )

add_library(
    ocl_common
    STATIC
    ${OCL_COMMON_SRC}
)
