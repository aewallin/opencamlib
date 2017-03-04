# CmakeLists.txt for OpenCAMLib src/cutters directory

MESSAGE(STATUS " configuring src/cutters")

IF (UNIX)
    add_definitions(-fPIC)
ENDIF (UNIX)

INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR} )
INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR}/geo )
INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR}/algo )
INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR}/common )

add_library(
    ocl_cutters 
    STATIC
    ${OCL_CUTTER_SRC}
)




