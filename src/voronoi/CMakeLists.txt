# CmakeLists.txt for OpenCAMLib src/algo/voronoi directory

MESSAGE(STATUS " configuring src/algo/voronoi")

if (CMAKE_BUILD_TOOL MATCHES "make")
    add_definitions(-fPIC)
endif (CMAKE_BUILD_TOOL MATCHES "make")

INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR} )
INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR}/geo )
INCLUDE_DIRECTORIES( ${OpenCamLib_SOURCE_DIR}/algo )


add_library(
    ocl_voronoi 
    STATIC
    ${OCL_VORONOI_SRC}
)




