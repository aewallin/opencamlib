
# CmakeLists.txt for OpenCAMLib src/cutsim directory
# project(OpenCamLib)

# cmake_minimum_required(VERSION 2.4)

MESSAGE(STATUS " configuring src/cutsim")

if (CMAKE_BUILD_TOOL MATCHES "make")
    add_definitions(-fPIC)
endif (CMAKE_BUILD_TOOL MATCHES "make")

# MESSAGE(STATUS " configuring src/cutters",${OpenCamLib_SOURCE_DIR} )

# this branches into the cutters dir and compiles stuff there
# ADD_SUBDIRECTORY( cutters )

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
    ocode.cpp
    octree.cpp
    volume.cpp
)




