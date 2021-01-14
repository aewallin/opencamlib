project(OCL_POINT_EXAMPLE)

cmake_minimum_required(VERSION 2.4)

if (CMAKE_BUILD_TOOL MATCHES "make")
    add_definitions(-Wall -Werror -Wno-deprecated -pedantic-errors)
endif (CMAKE_BUILD_TOOL MATCHES "make")

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


