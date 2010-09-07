# CmakeLists.txt for OpenCAMLib src/cutters directory

MESSAGE(STATUS " configuring src/cutters")

if (CMAKE_BUILD_TOOL MATCHES "make")
    add_definitions(-fPIC)
endif (CMAKE_BUILD_TOOL MATCHES "make")

INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR} )
INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR}/geo )
INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR}/algo )


add_library(
    ocl_cutters 
    STATIC
    ${OCL_CUTTER_SRC}
)




