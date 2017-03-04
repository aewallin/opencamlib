# CmakeLists.txt for OpenCAMLib src/dropcutter directory

message(STATUS " configuring src/dropcutter")

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
    ocl_dropcutter
    STATIC
    ${OCL_DROPCUTTER_SRC}
)




