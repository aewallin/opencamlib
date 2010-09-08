# CmakeLists.txt for OpenCAMLib src/cutsim directory

MESSAGE(STATUS " configuring src/cutsim")

if (CMAKE_BUILD_TOOL MATCHES "make")
    add_definitions(-fPIC)
endif (CMAKE_BUILD_TOOL MATCHES "make")

INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR} )
INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR}/geo )
INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR}/cutters )
INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR}/algo )

#
# this makes the ocl Python module
#

add_library(
    ocl_cutsim
    STATIC
    ${OCL_CUTSIM_SRC}
)




