# CmakeLists.txt for OpenCAMLib src/dropcutter directory

MESSAGE(STATUS " configuring src/dropcutter")

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
    ocl_dropcutter
    STATIC
    ${OCL_DROPCUTTER_SRC}
)




