# CmakeLists.txt for OpenCAMLib src/cutters directory

message(STATUS " configuring src/cutters")

if (UNIX)
    add_definitions(-fPIC)
endif (UNIX)

include_directories( ${OpenCamLib_SOURCE_DIR} )
include_directories( ${OpenCamLib_SOURCE_DIR}/geo )
include_directories( ${OpenCamLib_SOURCE_DIR}/algo )
include_directories( ${OpenCamLib_SOURCE_DIR}/common )

add_library(
    ocl_cutters 
    STATIC
    ${OCL_CUTTER_SRC}
)
