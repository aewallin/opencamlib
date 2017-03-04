# CmakeLists.txt for OpenCAMLib src/common directory

MESSAGE(STATUS " configuring src/common")

IF (UNIX)
    add_definitions(-fPIC)
ENDIF (UNIX)

INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR} )
INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR}/geo )
INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR}/algo )
INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR}/dropcutter )
INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR}/cutters )
INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR}/common )

add_library(
    ocl_common
    STATIC
    ${OCL_COMMON_SRC}
)
