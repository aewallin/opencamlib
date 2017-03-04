# CmakeLists.txt for OpenCAMLib src/geo directory

MESSAGE(STATUS " configuring src/geo")

IF (UNIX)
    add_definitions(-fPIC)
ENDIF (UNIX)


INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR} )
INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR}/cutters )
INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR}/algo )
INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR}/geo )
INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR}/common )

#
# this makes the ocl Python module
#

add_library(
    ocl_geo
    STATIC
    ${OCL_GEO_SRC}
)
