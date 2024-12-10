project(OpenCAMLib LANGUAGES CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0025 NEW)
  cmake_policy(SET CMP0091 NEW) # for multithreaded  https://cmake.org/cmake/help/latest/policy/CMP0091.html
  cmake_policy(SET CMP0094 NEW) # for Python*_FIND_STRATEGY=LOCATION
  cmake_policy(SET CMP0042 NEW)
  cmake_policy(SET CMP0054 NEW)
endif(COMMAND cmake_policy)

# install targets in root of the build dir, using $<0:> generator expression to force it from not being overwritten
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<0:>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<0:>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<0:>)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/$<0:>)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/$<0:>)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/$<0:>)
endforeach()

set(CMAKE_CXX_STANDARD 14)

if(WIN32)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# some options to set what is built:
option(BUILD_CXX_LIB
  "Build/install the pure c++ ocl library? " OFF)

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
    "Use Python V3" ON)

option(USE_STATIC_BOOST
    "Use static boost" OFF)

if(NOT BUILD_CXX_LIB)
  message(STATUS " Note: will NOT build pure c++ library")
endif()

if(NOT BUILD_PY_LIB)
  message(STATUS " Note: will NOT build python library")
endif()

if(NOT BUILD_NODEJS_LIB)
  message(STATUS " Note: will NOT build node.js library")
endif()

if(NOT BUILD_EMSCRIPTEN_LIB)
  message(STATUS " Note: will NOT build emscripten library")
endif()

if(NOT BUILD_DOC)
  message(STATUS " Note: will NOT build ocl documentation")
endif()

if (NOT VERSION_STRING)
  set(VERSION_STRING "unknown-version")
endif()

# figure out the gcc version
include(gcc_version.cmake)

#
# Turn compiler warnings up to 11, at least with gcc.  I don't know how to
# do this with other compilers we might support and I'm leaving it up to
# the relevant platform maintainers...
# #include'ing the boost graph-library creates deprecated warnings
# thus we use now use -Wno-deprecated here.
#
if(UNIX)
  if(GCC_4_6)
    message(STATUS "setting gcc options: -Wall -Werror -Wno-deprecated -pedantic-errors")
    add_definitions(-Wall  -Wno-deprecated -Werror -pedantic-errors)
  else()
    message(STATUS "setting gcc options: -Wall  -Wno-deprecated -pedantic-errors")
    add_definitions(-Wall  -Wno-deprecated -pedantic-errors)
  endif()
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT got_ipo_support)
if(got_ipo_support)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(Boost_DEBUG ON CACHE BOOL "boost-debug")
if(USE_STATIC_BOOST)
  set(Boost_USE_STATIC_LIBS ON CACHE BOOL "boost-use-static-libs")
endif()
if(DEFINED ENV{BOOST_ROOT} OR DEFINED BOOST_ROOT)
  set(Boost_NO_SYSTEM_PATHS ON)
endif()
if(CMAKE_BUILD_TYPE MATCHES Debug)
	set(Boost_USE_DEBUG_LIBS     ON)
	set(Boost_USE_RELEASE_LIBS   OFF)
else()
	set(Boost_USE_DEBUG_LIBS     OFF)
	set(Boost_USE_RELEASE_LIBS   ON)
endif()
if(BUILD_EMSCRIPTEN_LIB)
  set(Boost_USE_MULTITHREADED  OFF)
elseif(USE_OPENMP)
  set(Boost_USE_MULTITHREADED  ON)
endif()
if(WIN32)
  # use static python lib
  if(Boost_USE_STATIC_LIBS)
    add_definitions(-D BOOST_PYTHON_STATIC_LIB)
  endif()
  # disable autolinking in boost
  add_definitions(-D BOOST_ALL_NO_LIB) # avoid LNK1104 on Windows: http://stackoverflow.com/a/28902261/122441
endif()

# when building the emscripten we have to temporarily set
# the root path mode to BOTH so Boost can be found
if(BUILD_EMSCRIPTEN_LIB)
  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
  set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
endif()
find_package(Boost CONFIG REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
if(BUILD_EMSCRIPTEN_LIB)
  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
  set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
endif()

if(USE_OPENMP)
  if(APPLE)
    if(DEFINED ENV{OPENMP_PREFIX_MACOS})
      message(STATUS "Will use libomp at $ENV{OPENMP_PREFIX_MACOS}.")
      set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp")
      set(OpenMP_C_LIB_NAMES "omp")
      set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp")
      set(OpenMP_CXX_LIB_NAMES "omp")
      set(OpenMP_omp_LIBRARY "$ENV{OPENMP_PREFIX_MACOS}/lib/libomp.dylib")
      include_directories("$ENV{OPENMP_PREFIX_MACOS}/include")
    else()
      list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/libomp")
      list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/libomp")
    endif()
  endif()
  if(MSVC)
    set(CMAKE_INSTALL_OPENMP_LIBRARIES ON)
    include(InstallRequiredSystemLibraries)
  endif()
  find_package(OpenMP REQUIRED)
  if(OPENMP_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    message(STATUS "found OpenMP, compiling with flags: " ${OpenMP_CXX_FLAGS} )
    include_directories(${OpenMP_CXX_INCLUDE_DIRS})
  endif()
endif()

# this defines the source-files
set(OCL_SRC
  ${PROJECT_SOURCE_DIR}/ocl.cpp
  )

set(OCL_GEO_SRC
  ${PROJECT_SOURCE_DIR}/geo/arc.cpp
  ${PROJECT_SOURCE_DIR}/geo/bbox.cpp
  ${PROJECT_SOURCE_DIR}/geo/ccpoint.cpp
  ${PROJECT_SOURCE_DIR}/geo/clpoint.cpp
  ${PROJECT_SOURCE_DIR}/geo/line.cpp
  ${PROJECT_SOURCE_DIR}/geo/path.cpp
  ${PROJECT_SOURCE_DIR}/geo/point.cpp
  ${PROJECT_SOURCE_DIR}/geo/stlreader.cpp
  ${PROJECT_SOURCE_DIR}/geo/stlsurf.cpp
  ${PROJECT_SOURCE_DIR}/geo/triangle.cpp
  )

set(OCL_CUTTER_SRC
  ${PROJECT_SOURCE_DIR}/cutters/ballcutter.cpp
  ${PROJECT_SOURCE_DIR}/cutters/bullcutter.cpp
  ${PROJECT_SOURCE_DIR}/cutters/compositecutter.cpp
  ${PROJECT_SOURCE_DIR}/cutters/conecutter.cpp
  ${PROJECT_SOURCE_DIR}/cutters/millingcutter.cpp
  ${PROJECT_SOURCE_DIR}/cutters/cylcutter.cpp
  ${PROJECT_SOURCE_DIR}/cutters/ellipse.cpp
  ${PROJECT_SOURCE_DIR}/cutters/ellipseposition.cpp
  )

set(OCL_DROPCUTTER_SRC
  ${PROJECT_SOURCE_DIR}/dropcutter/batchdropcutter.cpp
  ${PROJECT_SOURCE_DIR}/dropcutter/pointdropcutter.cpp
  ${PROJECT_SOURCE_DIR}/dropcutter/pathdropcutter.cpp
  ${PROJECT_SOURCE_DIR}/dropcutter/adaptivepathdropcutter.cpp
  )

set(OCL_ALGO_SRC
  ${PROJECT_SOURCE_DIR}/algo/batchpushcutter.cpp
  ${PROJECT_SOURCE_DIR}/algo/fiberpushcutter.cpp
  ${PROJECT_SOURCE_DIR}/algo/interval.cpp
  ${PROJECT_SOURCE_DIR}/algo/fiber.cpp
  ${PROJECT_SOURCE_DIR}/algo/waterline.cpp
  ${PROJECT_SOURCE_DIR}/algo/adaptivewaterline.cpp
  ${PROJECT_SOURCE_DIR}/algo/weave.cpp
  ${PROJECT_SOURCE_DIR}/algo/simple_weave.cpp
  ${PROJECT_SOURCE_DIR}/algo/smart_weave.cpp
  )

set(OCL_COMMON_SRC
  ${PROJECT_SOURCE_DIR}/common/numeric.cpp
  ${PROJECT_SOURCE_DIR}/common/lineclfilter.cpp
  )

set(OCL_INCLUDE_FILES
  ${PROJECT_SOURCE_DIR}/ocl.hpp

  ${PROJECT_SOURCE_DIR}/geo/arc.hpp
  ${PROJECT_SOURCE_DIR}/geo/bbox.hpp
  ${PROJECT_SOURCE_DIR}/geo/ccpoint.hpp
  ${PROJECT_SOURCE_DIR}/geo/clpoint.hpp
  ${PROJECT_SOURCE_DIR}/geo/line.hpp
  ${PROJECT_SOURCE_DIR}/geo/path.hpp
  ${PROJECT_SOURCE_DIR}/geo/stlreader.hpp
  ${PROJECT_SOURCE_DIR}/geo/stlsurf.hpp
  ${PROJECT_SOURCE_DIR}/geo/triangle.hpp
  ${PROJECT_SOURCE_DIR}/geo/point.hpp

  ${PROJECT_SOURCE_DIR}/cutters/ballcutter.hpp
  ${PROJECT_SOURCE_DIR}/cutters/bullcutter.hpp
  ${PROJECT_SOURCE_DIR}/cutters/compositecutter.hpp
  ${PROJECT_SOURCE_DIR}/cutters/conecutter.hpp
  ${PROJECT_SOURCE_DIR}/cutters/cylcutter.hpp
  ${PROJECT_SOURCE_DIR}/cutters/ellipseposition.hpp
  ${PROJECT_SOURCE_DIR}/cutters/millingcutter.hpp
  ${PROJECT_SOURCE_DIR}/cutters/ellipse.hpp

  ${PROJECT_SOURCE_DIR}/dropcutter/adaptivepathdropcutter.hpp
  ${PROJECT_SOURCE_DIR}/dropcutter/pathdropcutter.hpp
  ${PROJECT_SOURCE_DIR}/dropcutter/batchdropcutter.hpp
  ${PROJECT_SOURCE_DIR}/dropcutter/pointdropcutter.hpp

  ${PROJECT_SOURCE_DIR}/common/brent_zero.hpp
  ${PROJECT_SOURCE_DIR}/common/kdnode.hpp
  ${PROJECT_SOURCE_DIR}/common/kdtree.hpp
  ${PROJECT_SOURCE_DIR}/common/numeric.hpp
  ${PROJECT_SOURCE_DIR}/common/lineclfilter.hpp
  ${PROJECT_SOURCE_DIR}/common/clfilter.hpp
  ${PROJECT_SOURCE_DIR}/common/halfedgediagram.hpp

  ${PROJECT_SOURCE_DIR}/algo/operation.hpp
  ${PROJECT_SOURCE_DIR}/algo/batchpushcutter.hpp
  ${PROJECT_SOURCE_DIR}/algo/fiberpushcutter.hpp
  ${PROJECT_SOURCE_DIR}/algo/fiber.hpp
  ${PROJECT_SOURCE_DIR}/algo/interval.hpp
  ${PROJECT_SOURCE_DIR}/algo/waterline.hpp
  ${PROJECT_SOURCE_DIR}/algo/adaptivewaterline.hpp
  ${PROJECT_SOURCE_DIR}/algo/weave.hpp
  ${PROJECT_SOURCE_DIR}/algo/simple_weave.hpp
  ${PROJECT_SOURCE_DIR}/algo/smart_weave.hpp
  ${PROJECT_SOURCE_DIR}/algo/weave_typedef.hpp
  ${PROJECT_SOURCE_DIR}/algo/tsp.hpp
  )

# this branches into the dirs and compiles stuff there
add_subdirectory(${PROJECT_SOURCE_DIR}/cutters)
add_subdirectory(${PROJECT_SOURCE_DIR}/geo)
add_subdirectory(${PROJECT_SOURCE_DIR}/algo)
add_subdirectory(${PROJECT_SOURCE_DIR}/dropcutter)
add_subdirectory(${PROJECT_SOURCE_DIR}/common)

if(BUILD_NODEJS_LIB)
  include(${CMAKE_CURRENT_SOURCE_DIR}/nodejslib/nodejslib.cmake)
endif()

if(BUILD_PY_LIB)
  include(${CMAKE_CURRENT_SOURCE_DIR}/pythonlib/pythonlib.cmake)
endif()

if(BUILD_CXX_LIB)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cxxlib/cxxlib.cmake)
endif()

if(BUILD_EMSCRIPTEN_LIB)
  include(${CMAKE_CURRENT_SOURCE_DIR}/emscriptenlib/emscriptenlib.cmake)
endif()

# build & install documentation (if Doxygen is available)
if(BUILD_DOC)
  find_package(Doxygen)
  if(DOXYGEN_FOUND)
    message(STATUS "Found doxygen. Documentation can be built with 'make doc' ")
    find_package(LATEX)
    if(NOT LATEX_COMPILER)
      message(STATUS "latex command LATEX_COMPILER not found but usually required. You will probably get warnings and user inetraction on doxy run.")
    endif()
    if(NOT MAKEINDEX_COMPILER)
      message(STATUS "makeindex command MAKEINDEX_COMPILER not found but usually required.")
    endif()
    if(NOT DVIPS_CONVERTER)
      message(STATUS "dvips command DVIPS_CONVERTER not found but usually required.")
    endif()

    configure_file(${PROJECT_SOURCE_DIR}/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    set(DOXY_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    execute_process(
      COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/doc/html ${CMAKE_CURRENT_BINARY_DIR}/doc/latex
    )

    add_custom_command(
      OUTPUT
      doc/latex/index.tex
      doc/html/index.html
      COMMAND Doxygen::doxygen ${DOXY_CONFIG}
      COMMENT building LaTex & HTML docs
    )

    add_custom_target(
      doc
      DEPENDS doc/latex/index.tex
    )

    if(EXISTS ${PDFLATEX_COMPILER})
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
    else()
      message(STATUS "pdflatex compiler not found, PDF docs will not be built")
    endif()

    add_custom_target(
      doc-latex
      DEPENDS doc/latex/index.tex
    )
  endif()
endif()

# "make spackage"
add_custom_target(spackage 
  ${CMAKE_COMMAND} 
  -D SRC_DIR:STRING=${CMAKE_CURRENT_SOURCE_DIR} 
  -D MY_VERSION:STRING=${MY_VERSION} 
  -C ${CMAKE_CURRENT_SOURCE_DIR}/deb/package_details.cmake
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
