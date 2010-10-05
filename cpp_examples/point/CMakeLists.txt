project(OCL_POINT_EXAMPLE)

cmake_minimum_required(VERSION 2.4)

if (CMAKE_BUILD_TOOL MATCHES "make")
    add_definitions(-Wall -Werror -Wno-deprecated -pedantic-errors)
endif (CMAKE_BUILD_TOOL MATCHES "make")

# find BOOST and boost-python
find_package( Boost )
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    MESSAGE(STATUS "found Boost: " ${Boost_LIB_VERSION})
    MESSAGE(STATUS "boost-incude dirs are: " ${Boost_INCLUDE_DIRS})
endif()

find_package( OpenMP REQUIRED )
IF (OPENMP_FOUND)
    MESSAGE(STATUS "found OpenMP, compiling with flags: " ${OpenMP_CXX_FLAGS} )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
ENDIF(OPENMP_FOUND)

find_library(OCL_LIBRARY 
            NAMES ocl
            PATHS /usr/local/lib/opencamlib
            DOC "The opencamlib library"
)
#find_package(ocl REQUIRED)
MESSAGE(STATUS "OCL_LIBRARY is now: " ${OCL_LIBRARY})


set(OCL_TST_SRC
    ${OCL_POINT_EXAMPLE_SOURCE_DIR}/point_example.cpp
)

add_executable(
    point_example
    ${OCL_TST_SRC}
)
target_link_libraries(point_example ${OCL_LIBRARY} ${Boost_LIBRARIES})


