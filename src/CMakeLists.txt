project(OpenCamLib)

cmake_minimum_required(VERSION 3.9)
#set( CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR} )
set(CMAKE_VERBOSE_MAKEFILE ON)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0025 NEW)
endif(COMMAND cmake_policy)

#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
if (CMAKE_VERSION VERSION_LESS "3.1")
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
  endif ()
else ()
  set (CMAKE_CXX_STANDARD 11)
endif ()

# some options to set what is built:
option(BUILD_CXX_LIB
  "Build/install the pure c++ libocl library? " ON)

option(BUILD_PY_LIB 
  "Build/install the python ocl library? " OFF)

option(BUILD_NODEJS_LIB 
  "Build/install the node.js ocl library? " OFF)

option(BUILD_DOC
  "Build/install the ocl documentation? " ON)

option(USE_OPENMP
    "Use OpenMP for parallel computation" ON)

option(VERSION_STRING
    "Set version string" OFF)

option(USE_PY_3
    "Use Python V3" OFF)

if (NOT BUILD_CXX_LIB)
  message(STATUS " Note: will NOT build pure c++ library")
endif(NOT BUILD_CXX_LIB)

if (NOT BUILD_PY_LIB)
  message(STATUS " Note: will NOT build python library")
endif(NOT BUILD_PY_LIB)

if (NOT BUILD_EMSCRIPTEN_LIB)
  message(STATUS " Note: will NOT build emscripten library")
endif(NOT BUILD_EMSCRIPTEN_LIB)

if (NOT BUILD_DOC)
  message(STATUS " Note: will NOT build ocl documentation")
endif(NOT BUILD_DOC)

# figure out the gcc version
include(gcc_version.cmake)

#
# Turn compiler warnings up to 11, at least with gcc.  I don't know how to
# do this with other compilers we might support and I'm leaving it up to
# the relevant platform maintainers...
# #include'ing the boost graph-library creates deprecated warnings
# thus we use now use -Wno-deprecated here.
#
if (UNIX)
  if(GCC_4_6)
    message(STATUS "setting gcc options: -Wall -Werror -Wno-deprecated -pedantic-errors")
    add_definitions(-Wall  -Wno-deprecated -Werror -pedantic-errors)
  else(GCC_4_6)
    message(STATUS "setting gcc options: -Wall  -Wno-deprecated -pedantic-errors")
    add_definitions(-Wall  -Wno-deprecated -pedantic-errors)
  endif(GCC_4_6)
endif (UNIX)

# option(BUILD_TYPE
#   "Build type: Release=ON/Debug=OFF  " ON)

# if (BUILD_TYPE)
#   message(STATUS " CMAKE_BUILD_TYPE = Release")
#   set(CMAKE_BUILD_TYPE Release)
# endif(BUILD_TYPE)

# if (NOT BUILD_TYPE)
#   message(STATUS " CMAKE_BUILD_TYPE = Debug")
#   set(CMAKE_BUILD_TYPE Debug)
# endif(NOT BUILD_TYPE)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(USE_OPENMP)
  if(APPLE)
    # OpenMP does not work with the default Apple llvm/clang++
    # Install a more recent version using Homebrew:
    # "brew install llvm"
    # set(OpenMP_CXX_FLAGS "-fopenmp=libomp")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  else()
    find_package( OpenMP REQUIRED )
  endif()

  if (OPENMP_FOUND)
    message(STATUS "found OpenMP, compiling with flags: " ${OpenMP_CXX_FLAGS} )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  endif(OPENMP_FOUND)

endif(USE_OPENMP)

IF(EXISTS ${OpenCamLib_SOURCE_DIR}/version_string.hpp)
  file(STRINGS "${OpenCamLib_SOURCE_DIR}/version_string.hpp" OpenCamLib_BUILD_SPECIFICATION REGEX "^[ \t]*#define[ \t]+VERSION_STRING[ \t]+.*$")
  if(OpenCamLib_BUILD_SPECIFICATION)
    string(REGEX REPLACE ".*#define[ \t]+VERSION_STRING[ \t]+\"(.*)\".*" "\\1" MY_VERSION ${OpenCamLib_BUILD_SPECIFICATION})
  else()
    message(FATAL_ERROR "Data were not found for the required build specification.")
  endif()
  SET(version_string ${OpenCamLib_SOURCE_DIR}/version_string.hpp)
ELSE(EXISTS ${OpenCamLib_SOURCE_DIR}/version_string.hpp)
  ################ create version_string.hpp, http://stackoverflow.com/questions/3780667
  # include the output directory, where the version_string.hpp file is generated
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  if (VERSION_STRING)
    set( vstring "//version_string.hpp - written by cmake. changes will be lost!\n"
               "#ifndef VERSION_STRING\n"
               "#define VERSION_STRING \"${VERSION_STRING}\"\n"
               "#endif\n"
    )
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/version_string.hpp ${vstring} )
    set(MY_VERSION ${VERSION_STRING})
    set(version_string ${VERSION_STRING})
  else()
    include(version_string.cmake)
    # now parse the git commit id:
    STRING(REGEX REPLACE "([0-9]+).*" "\\1" GIT_MAJOR_VERSION "${GIT_COMMIT_ID}" )
    STRING(REGEX REPLACE "[0-9]+.([0-9]+)-.*" "\\1" GIT_MINOR_VERSION "${GIT_COMMIT_ID}" )
    STRING(REGEX REPLACE "[0-9]+.[0-9]+-(.*)-.*" "\\1" GIT_PATCH_VERSION "${GIT_COMMIT_ID}" )
    SET(MY_VERSION "${GIT_MAJOR_VERSION}.${GIT_MINOR_VERSION}.${GIT_PATCH_VERSION}" CACHE STRING "name")
    SET(version_string ${CMAKE_CURRENT_BINARY_DIR}/version_string.hpp)
  endif()
ENDIF(EXISTS ${OpenCamLib_SOURCE_DIR}/version_string.hpp)

message(STATUS "OpenCamLib version: ${MY_VERSION}")

# this defines the source-files
set(OCL_GEO_SRC
  ${OpenCamLib_SOURCE_DIR}/geo/arc.cpp
  ${OpenCamLib_SOURCE_DIR}/geo/bbox.cpp
  ${OpenCamLib_SOURCE_DIR}/geo/ccpoint.cpp
  ${OpenCamLib_SOURCE_DIR}/geo/clpoint.cpp
  ${OpenCamLib_SOURCE_DIR}/geo/line.cpp
  ${OpenCamLib_SOURCE_DIR}/geo/path.cpp
  ${OpenCamLib_SOURCE_DIR}/geo/point.cpp
  ${OpenCamLib_SOURCE_DIR}/geo/stlreader.cpp
  ${OpenCamLib_SOURCE_DIR}/geo/stlsurf.cpp
  ${OpenCamLib_SOURCE_DIR}/geo/triangle.cpp
  )

set(OCL_CUTTER_SRC
  ${OpenCamLib_SOURCE_DIR}/cutters/ballcutter.cpp
  ${OpenCamLib_SOURCE_DIR}/cutters/bullcutter.cpp
  ${OpenCamLib_SOURCE_DIR}/cutters/compositecutter.cpp
  ${OpenCamLib_SOURCE_DIR}/cutters/conecutter.cpp
  ${OpenCamLib_SOURCE_DIR}/cutters/millingcutter.cpp
  ${OpenCamLib_SOURCE_DIR}/cutters/cylcutter.cpp
  ${OpenCamLib_SOURCE_DIR}/cutters/ellipse.cpp
  ${OpenCamLib_SOURCE_DIR}/cutters/ellipseposition.cpp
  )

set(OCL_DROPCUTTER_SRC
  ${OpenCamLib_SOURCE_DIR}/dropcutter/batchdropcutter.cpp
  ${OpenCamLib_SOURCE_DIR}/dropcutter/pointdropcutter.cpp
  ${OpenCamLib_SOURCE_DIR}/dropcutter/pathdropcutter.cpp
  ${OpenCamLib_SOURCE_DIR}/dropcutter/adaptivepathdropcutter.cpp
  )

set(OCL_ALGO_SRC
  ${OpenCamLib_SOURCE_DIR}/algo/batchpushcutter.cpp
  ${OpenCamLib_SOURCE_DIR}/algo/fiberpushcutter.cpp
  ${OpenCamLib_SOURCE_DIR}/algo/interval.cpp
  ${OpenCamLib_SOURCE_DIR}/algo/fiber.cpp
  ${OpenCamLib_SOURCE_DIR}/algo/waterline.cpp
  ${OpenCamLib_SOURCE_DIR}/algo/adaptivewaterline.cpp
  ${OpenCamLib_SOURCE_DIR}/algo/weave.cpp
  ${OpenCamLib_SOURCE_DIR}/algo/simple_weave.cpp
  ${OpenCamLib_SOURCE_DIR}/algo/smart_weave.cpp
  )


set(OCL_COMMON_SRC
  ${OpenCamLib_SOURCE_DIR}/common/numeric.cpp
  ${OpenCamLib_SOURCE_DIR}/common/lineclfilter.cpp
  )

set( OCL_INCLUDE_FILES  
  ${OpenCamLib_SOURCE_DIR}/geo/arc.hpp
  ${OpenCamLib_SOURCE_DIR}/geo/bbox.hpp
  ${OpenCamLib_SOURCE_DIR}/geo/ccpoint.hpp
  ${OpenCamLib_SOURCE_DIR}/geo/clpoint.hpp
  ${OpenCamLib_SOURCE_DIR}/geo/line.hpp
  ${OpenCamLib_SOURCE_DIR}/geo/path.hpp
  ${OpenCamLib_SOURCE_DIR}/geo/stlreader.hpp
  ${OpenCamLib_SOURCE_DIR}/geo/stlsurf.hpp
  ${OpenCamLib_SOURCE_DIR}/geo/triangle.hpp
  ${OpenCamLib_SOURCE_DIR}/geo/point.hpp
  
  ${OpenCamLib_SOURCE_DIR}/cutters/ballcutter.hpp
  ${OpenCamLib_SOURCE_DIR}/cutters/bullcutter.hpp
  ${OpenCamLib_SOURCE_DIR}/cutters/compositecutter.hpp
  ${OpenCamLib_SOURCE_DIR}/cutters/conecutter.hpp
  ${OpenCamLib_SOURCE_DIR}/cutters/cylcutter.hpp
  ${OpenCamLib_SOURCE_DIR}/cutters/ellipseposition.hpp
  ${OpenCamLib_SOURCE_DIR}/cutters/millingcutter.hpp
  ${OpenCamLib_SOURCE_DIR}/cutters/ellipse.hpp
  
  ${OpenCamLib_SOURCE_DIR}/dropcutter/adaptivepathdropcutter.hpp
  ${OpenCamLib_SOURCE_DIR}/dropcutter/pathdropcutter.hpp
  ${OpenCamLib_SOURCE_DIR}/dropcutter/batchdropcutter.hpp
  ${OpenCamLib_SOURCE_DIR}/dropcutter/pointdropcutter.hpp
  
  ${OpenCamLib_SOURCE_DIR}/common/brent_zero.hpp
  ${OpenCamLib_SOURCE_DIR}/common/kdnode.hpp
  ${OpenCamLib_SOURCE_DIR}/common/kdtree.hpp
  ${OpenCamLib_SOURCE_DIR}/common/numeric.hpp
  ${OpenCamLib_SOURCE_DIR}/common/lineclfilter.hpp
  ${OpenCamLib_SOURCE_DIR}/common/clfilter.hpp
  ${OpenCamLib_SOURCE_DIR}/common/halfedgediagram.hpp

  
  ${OpenCamLib_SOURCE_DIR}/algo/operation.hpp
  ${OpenCamLib_SOURCE_DIR}/algo/batchpushcutter.hpp
  ${OpenCamLib_SOURCE_DIR}/algo/fiberpushcutter.hpp
  ${OpenCamLib_SOURCE_DIR}/algo/fiber.hpp
  ${OpenCamLib_SOURCE_DIR}/algo/interval.hpp
  ${OpenCamLib_SOURCE_DIR}/algo/waterline.hpp
  ${OpenCamLib_SOURCE_DIR}/algo/adaptivewaterline.hpp
  ${OpenCamLib_SOURCE_DIR}/algo/weave.hpp
  ${OpenCamLib_SOURCE_DIR}/algo/simple_weave.hpp
  ${OpenCamLib_SOURCE_DIR}/algo/smart_weave.hpp
  ${OpenCamLib_SOURCE_DIR}/algo/weave_typedef.hpp
  ${OpenCamLib_SOURCE_DIR}/algo/tsp.hpp
  )

# this branches into the dirs and compiles stuff there
add_subdirectory( ${OpenCamLib_SOURCE_DIR}/cutters  )
add_subdirectory( ${OpenCamLib_SOURCE_DIR}/geo  )
add_subdirectory( ${OpenCamLib_SOURCE_DIR}/algo  ) 
add_subdirectory( ${OpenCamLib_SOURCE_DIR}/dropcutter  ) 
add_subdirectory( ${OpenCamLib_SOURCE_DIR}/common  ) 
  
if (BUILD_NODEJS_LIB)
  include(${CMAKE_CURRENT_SOURCE_DIR}/nodejslib/nodejslib.cmake)
endif (BUILD_NODEJS_LIB)

if (BUILD_PY_LIB)
  include(${CMAKE_CURRENT_SOURCE_DIR}/pythonlib/pythonlib.cmake)
endif (BUILD_PY_LIB)

# this is the C++ shared library
if (BUILD_CXX_LIB)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cxxlib/cxxlib.cmake)
endif (BUILD_CXX_LIB)

if (BUILD_EMSCRIPTEN_LIB)
  include(${CMAKE_CURRENT_SOURCE_DIR}/emscriptenlib/emscriptenlib.cmake)
endif (BUILD_EMSCRIPTEN_LIB)

#
# this installs the examples
#

# install(
#     DIRECTORY ../scripts/
#    DESTINATION share/doc/python-opencam/examples
#    PATTERN .svn EXCLUDE
# )

# the example STL files
# install(
#     DIRECTORY ../stl/
#     DESTINATION share/doc/python-opencam/examples/stl
#     PATTERN .svn EXCLUDE
# )


#
# build & install documentation (if Doxygen is available)
#

if (BUILD_DOC)

  find_package(Doxygen)
  IF (DOXYGEN_FOUND)
    message(STATUS "Found doxygen. Documentation can be built with 'make doc' ")
    
    # this works around a bug in cmake 2.4 (Ubuntu Hardy)
    execute_process(
      COMMAND mkdir -p doc/html doc/latex
      )

    find_package(LATEX)
    IF    (NOT LATEX_COMPILER)
      message(STATUS "latex command LATEX_COMPILER not found but usually required. You will probably get warnings and user inetraction on doxy run.")
    ENDIF (NOT LATEX_COMPILER)
    IF    (NOT MAKEINDEX_COMPILER)
      message(STATUS "makeindex command MAKEINDEX_COMPILER not found but usually required.")
    ENDIF (NOT MAKEINDEX_COMPILER)
    IF    (NOT DVIPS_CONVERTER)
      message(STATUS "dvips command DVIPS_CONVERTER not found but usually required.")
    ENDIF (NOT DVIPS_CONVERTER)
    
    #if (EXISTS Doxyfile)
    set(DOXY_CONFIG ${OpenCamLib_SOURCE_DIR}/Doxyfile)
    #endif (EXISTS Doxyfile)

    add_custom_command(
      OUTPUT
      doc/latex/index.tex
      doc/html/index.html
      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXY_CONFIG}
      COMMENT building LaTex & HTML docs
      )

    add_custom_target(
      doc
      DEPENDS doc/latex/index.tex
      )


    IF (EXISTS ${PDFLATEX_COMPILER})
      add_custom_command(
        OUTPUT doc/latex/refman.pdf
        DEPENDS doc/latex/index.tex
        WORKING_DIRECTORY doc/latex
        COMMAND make pdf
        COMMENT building PDF docs
        COMMAND mv refman.pdf ../ocl-manual.pdf
        )

      add_custom_target(
        doc-pdf
        DEPENDS doc/latex/refman.pdf
        )

      add_dependencies(doc doc-pdf)
    ELSE (EXISTS ${PDFLATEX_COMPILER}) 
      message(STATUS "pdflatex compiler not found, PDF docs will not be built")
    ENDIF (EXISTS ${PDFLATEX_COMPILER})


    add_custom_target(
      doc-latex
      DEPENDS doc/latex/index.tex
      )

    #install(
    #    DIRECTORY doc/latex/
    #    DESTINATION share/doc/python-opencam/pdf
    #    FILES_MATCHING PATTERN *.pdf
    #)

    #install(
    #    FILES doc/ocl-manual.pdf
    #    DESTINATION share/doc/python-opencam/pdf
    #)

    #install(
    #    DIRECTORY doc/html
    #    DESTINATION share/doc/python-opencam/
    #)

  endif(DOXYGEN_FOUND)
endif (BUILD_DOC)

# "make spackage"
add_custom_target(spackage 
  ${CMAKE_COMMAND} 
  -D SRC_DIR:STRING=${CMAKE_SOURCE_DIR} 
  -D MY_VERSION:STRING=${MY_VERSION} 
  -C ${CMAKE_SOURCE_DIR}/deb/package_details.cmake
  -P ${CMAKE_CURRENT_SOURCE_DIR}/deb/DebSourcePPA.cmake 
  ) 
#add_custom_target(spackage-oneiric
#              ${CMAKE_COMMAND} 
#              -D SRC_DIR:STRING=${CMAKE_SOURCE_DIR} 
#              -D MY_VERSION:STRING=${MY_VERSION} 
#              -D MY_DISTRIBUTION_RELEASES:STRING="oneiric"
#              -C ${CMAKE_SOURCE_DIR}/package_details.cmake
#              -P ${CMAKE_CURRENT_SOURCE_DIR}/deb/DebSourcePPA.cmake 
#            )

message(STATUS "type:")
message(STATUS " 'make' for a normal build")
message(STATUS " 'make -j8' to build faster (if you have many cpus)")
message(STATUS " 'make install' to install")
message(STATUS " 'make package' to build a binary deb-packate")
message(STATUS " 'make spackage' to build debian source-packages")
#message(STATUS " 'make spackage-oneiric' to build a debian source-package only for oneiric")
message(STATUS " 'make test' to run the tests")

# "make package"
include(CPack) # this should be last
