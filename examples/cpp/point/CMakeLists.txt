project(OCL_POINT_EXAMPLE)

cmake_minimum_required(VERSION 3.12...3.15)
set(CMAKE_CXX_STANDARD 14)

if(CMAKE_BUILD_TOOL MATCHES "make")
    add_definitions(-Wall -Werror -Wno-deprecated -pedantic-errors)
endif(CMAKE_BUILD_TOOL MATCHES "make")

find_package(Boost)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    message(STATUS "found Boost: " ${Boost_LIB_VERSION})
    message(STATUS "boost-incude dirs are: " ${Boost_INCLUDE_DIRS})
endif()

if(APPLE)
  find_program(brew brew)
  if(brew)
    message(STATUS "Found homebrew at ${brew}.")
    execute_process(COMMAND "${brew}" --prefix libomp
      RESULT_VARIABLE brew_libomp_found
      OUTPUT_VARIABLE brew_libomp OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    if(brew_libomp_found EQUAL 0)
      message(STATUS "Found libomp at ${brew_libomp}.")
      set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp")
      set(OpenMP_C_LIB_NAMES "omp")
      set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp")
      set(OpenMP_CXX_LIB_NAMES "omp")
      set(OpenMP_omp_LIBRARY "${brew_libomp}/lib/libomp.dylib")
      include_directories("${brew_libomp}/include")
    endif()
  else()
    message(STATUS "Homebrew not found, please install homebrew and run `brew install libomp` for OpenMP support.")
  endif()
endif(APPLE)
find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  message(STATUS "found OpenMP, compiling with flags: " ${OpenMP_CXX_FLAGS} )
  include_directories(${OpenMP_CXX_INCLUDE_DIRS})
endif()

find_library(OCL_LIBRARY 
  NAMES ocl
  PATHS /usr/local
  PATH_SUFFIXES opencamlib
  DOC "The opencamlib library"
)
# todo: don't hardcode, make opencamlib export cmake targets
set(OCL_INCLUDE_DIR "/usr/local/include")
include_directories(${OCL_INCLUDE_DIR})

set(OCL_TST_SRC
  ${OCL_POINT_EXAMPLE_SOURCE_DIR}/point_example.cpp
)

add_executable(
  point_example
  ${OCL_TST_SRC}
)
target_link_libraries(
  point_example
  ${OCL_LIBRARY}
  ${Boost_LIBRARIES}
)


