# CmakeLists.txt for OpenCAMLib src/geo directory

message(STATUS " configuring src/geo")

if (UNIX)
    add_definitions(-fPIC)
endif (UNIX)


include_directories( ${OpenCamLib_SOURCE_DIR} )
include_directories( ${OpenCamLib_SOURCE_DIR}/cutters )
include_directories( ${OpenCamLib_SOURCE_DIR}/algo )
include_directories( ${OpenCamLib_SOURCE_DIR}/geo )
include_directories( ${OpenCamLib_SOURCE_DIR}/common )

#
# this makes the ocl Python module
#

add_library(
    ocl_geo
    STATIC
    ${OCL_GEO_SRC}
)
