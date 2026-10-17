set(OCL_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0025 NEW)
  cmake_policy(SET CMP0091 NEW) # for multithreaded  https://cmake.org/cmake/help/latest/policy/CMP0091.html
  cmake_policy(SET CMP0094 NEW) # for Python*_FIND_STRATEGY=LOCATION
  cmake_policy(SET CMP0042 NEW)
  cmake_policy(SET CMP0054 NEW)
  if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
  endif()
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

option(BUILD_EMSCRIPTEN_LIB "Build the WebAssembly library" OFF)

option(BUILD_DOC
  "Build/install the ocl documentation? " ON)

option(USE_OPENMP
    "Use OpenMP for parallel computation" ON)

option(OCL_WASM_THREADS "Enable Emscripten pthread parallelism" OFF)
option(OCL_WASM_NATIVE_EXCEPTIONS "Use native WebAssembly exceptions (requires a supporting runtime)" OFF)
if(OCL_WASM_NATIVE_EXCEPTIONS AND NOT EMSCRIPTEN)
  message(FATAL_ERROR "OCL_WASM_NATIVE_EXCEPTIONS requires an Emscripten toolchain")
endif()
set(OCL_WASM_THREAD_POOL_SIZE "3" CACHE STRING "Preloaded WebAssembly workers (in addition to the calling thread)")
if(OCL_WASM_THREADS)
  if(NOT EMSCRIPTEN OR NOT BUILD_EMSCRIPTEN_LIB)
    message(FATAL_ERROR "OCL_WASM_THREADS requires an Emscripten toolchain and BUILD_EMSCRIPTEN_LIB=ON")
  endif()
  if(USE_OPENMP)
    message(FATAL_ERROR "Set USE_OPENMP=OFF when using OCL_WASM_THREADS")
  endif()
  if(NOT OCL_WASM_THREAD_POOL_SIZE MATCHES "^[1-9][0-9]*$" OR OCL_WASM_THREAD_POOL_SIZE GREATER 1024)
    message(FATAL_ERROR "OCL_WASM_THREAD_POOL_SIZE must be an integer between 1 and 1024")
  endif()
endif()

set(VERSION_STRING "" CACHE STRING "Override the generated version string")

option(USE_PY_3
    "Use Python V3" ON)

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

option(OCL_ENABLE_IPO "Enable interprocedural optimization in optimized builds" ON)
option(OCL_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if(OCL_ENABLE_IPO AND NOT OCL_ENABLE_SANITIZERS)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT got_ipo_support OUTPUT ipo_error)
  if(got_ipo_support)
    message(STATUS "Interprocedural optimization enabled for optimized builds")
  else()
    message(WARNING "Interprocedural optimization was requested but its compiler check failed. Optimized builds will omit IPO.\n${ipo_error}")
  endif()
endif()
if(OCL_ENABLE_SANITIZERS AND (MSVC OR EMSCRIPTEN))
  message(FATAL_ERROR "The sanitizer preset requires native GCC or Clang")
endif()

function(ocl_configure_target target)
  target_compile_features(${target} PUBLIC cxx_std_14)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4)
  else()
    target_compile_options(${target} PRIVATE -Wall -pedantic-errors)
    if(NOT USE_OPENMP)
      target_compile_options(${target} PRIVATE -Wno-unknown-pragmas)
    endif()
  endif()
  if(got_ipo_support AND NOT (MSVC AND BUILD_CXX_LIB))
    set_target_properties(${target} PROPERTIES
      INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
      INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)
  endif()
  if(OCL_ENABLE_SANITIZERS)
    target_compile_options(${target} PRIVATE -fsanitize=address,undefined,float-cast-overflow -fno-omit-frame-pointer)
    target_link_options(${target} PRIVATE -fsanitize=address,undefined,float-cast-overflow)
  endif()
endfunction()

if(USE_OPENMP)
  if(APPLE)
    if(DEFINED ENV{OPENMP_PREFIX_MACOS})
      message(STATUS "Will use libomp at $ENV{OPENMP_PREFIX_MACOS}.")
      set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp")
      set(OpenMP_C_LIB_NAMES "omp")
      set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp")
      set(OpenMP_CXX_LIB_NAMES "omp")
      set(OpenMP_omp_LIBRARY "$ENV{OPENMP_PREFIX_MACOS}/lib/libomp.dylib")
      set(OpenMP_CXX_INCLUDE_DIR "$ENV{OPENMP_PREFIX_MACOS}/include")
    else()
      list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/libomp")
      list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/libomp")
    endif()
  endif()
  find_package(OpenMP REQUIRED)

endif()

if(EXISTS ${OCL_SOURCE_DIR}/version_string.hpp)
  file(STRINGS "${OCL_SOURCE_DIR}/version_string.hpp" PROJECT_BUILD_SPECIFICATION REGEX "^[ \t]*#define[ \t]+VERSION_STRING[ \t]+.*$")
  if(PROJECT_BUILD_SPECIFICATION)
    string(REGEX REPLACE ".*#define[ \t]+VERSION_STRING[ \t]+\"(.*)\".*" "\\1" MY_VERSION ${PROJECT_BUILD_SPECIFICATION})
  else()
    message(FATAL_ERROR "Data were not found for the required build specification.")
  endif()
  set(version_string ${OCL_SOURCE_DIR}/version_string.hpp)
else()
  ################ create version_string.hpp, http://stackoverflow.com/questions/3780667
  # include the output directory, where the version_string.hpp file is generated

  if(VERSION_STRING)
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
    string(REGEX REPLACE "([0-9]+).*" "\\1" GIT_MAJOR_VERSION "${GIT_COMMIT_ID}" )
    string(REGEX REPLACE "[0-9]+.([0-9]+)-.*" "\\1" GIT_MINOR_VERSION "${GIT_COMMIT_ID}" )
    string(REGEX REPLACE "[0-9]+.[0-9]+-(.*)-.*" "\\1" GIT_PATCH_VERSION "${GIT_COMMIT_ID}" )
    set(MY_VERSION "${GIT_MAJOR_VERSION}.${GIT_MINOR_VERSION}.${GIT_PATCH_VERSION}" CACHE STRING "name")
    set(version_string ${CMAKE_CURRENT_BINARY_DIR}/version_string.hpp)
  endif()
endif()

message(STATUS "OpenCAMLib version: ${MY_VERSION}")

# this defines the source-files
set(OCL_SRC
  ${OCL_SOURCE_DIR}/ocl.cpp
  )

set(OCL_GEO_SRC
  ${OCL_SOURCE_DIR}/geo/arc.cpp
  ${OCL_SOURCE_DIR}/geo/bbox.cpp
  ${OCL_SOURCE_DIR}/geo/ccpoint.cpp
  ${OCL_SOURCE_DIR}/geo/clpoint.cpp
  ${OCL_SOURCE_DIR}/geo/line.cpp
  ${OCL_SOURCE_DIR}/geo/path.cpp
  ${OCL_SOURCE_DIR}/geo/point.cpp
  ${OCL_SOURCE_DIR}/geo/stlreader.cpp
  ${OCL_SOURCE_DIR}/geo/stlsurf.cpp
  ${OCL_SOURCE_DIR}/geo/triangle.cpp
  )

set(OCL_CUTTER_SRC
  ${OCL_SOURCE_DIR}/cutters/ballcutter.cpp
  ${OCL_SOURCE_DIR}/cutters/bullcutter.cpp
  ${OCL_SOURCE_DIR}/cutters/compositecutter.cpp
  ${OCL_SOURCE_DIR}/cutters/conecutter.cpp
  ${OCL_SOURCE_DIR}/cutters/millingcutter.cpp
  ${OCL_SOURCE_DIR}/cutters/cylcutter.cpp
  ${OCL_SOURCE_DIR}/cutters/ellipse.cpp
  ${OCL_SOURCE_DIR}/cutters/ellipseposition.cpp
  )

set(OCL_DROPCUTTER_SRC
  ${OCL_SOURCE_DIR}/dropcutter/batchdropcutter.cpp
  ${OCL_SOURCE_DIR}/dropcutter/pointdropcutter.cpp
  ${OCL_SOURCE_DIR}/dropcutter/pathdropcutter.cpp
  ${OCL_SOURCE_DIR}/dropcutter/adaptivepathdropcutter.cpp
  )

set(OCL_ALGO_SRC
  ${OCL_SOURCE_DIR}/algo/batchpushcutter.cpp
  ${OCL_SOURCE_DIR}/algo/fiberpushcutter.cpp
  ${OCL_SOURCE_DIR}/algo/interval.cpp
  ${OCL_SOURCE_DIR}/algo/fiber.cpp
  ${OCL_SOURCE_DIR}/algo/waterline.cpp
  ${OCL_SOURCE_DIR}/algo/adaptivewaterline.cpp
  ${OCL_SOURCE_DIR}/algo/weave.cpp
  ${OCL_SOURCE_DIR}/algo/simple_weave.cpp
  ${OCL_SOURCE_DIR}/algo/smart_weave.cpp
  )

set(OCL_COMMON_SRC
  ${OCL_SOURCE_DIR}/common/numeric.cpp
  ${OCL_SOURCE_DIR}/common/lineclfilter.cpp
  )

set(OCL_INCLUDE_FILES
  ${OCL_SOURCE_DIR}/ocl.hpp

  ${OCL_SOURCE_DIR}/geo/arc.hpp
  ${OCL_SOURCE_DIR}/geo/bbox.hpp
  ${OCL_SOURCE_DIR}/geo/ccpoint.hpp
  ${OCL_SOURCE_DIR}/geo/clpoint.hpp
  ${OCL_SOURCE_DIR}/geo/line.hpp
  ${OCL_SOURCE_DIR}/geo/path.hpp
  ${OCL_SOURCE_DIR}/geo/stlreader.hpp
  ${OCL_SOURCE_DIR}/geo/stlsurf.hpp
  ${OCL_SOURCE_DIR}/geo/triangle.hpp
  ${OCL_SOURCE_DIR}/geo/point.hpp

  ${OCL_SOURCE_DIR}/cutters/ballcutter.hpp
  ${OCL_SOURCE_DIR}/cutters/bullcutter.hpp
  ${OCL_SOURCE_DIR}/cutters/compositecutter.hpp
  ${OCL_SOURCE_DIR}/cutters/conecutter.hpp
  ${OCL_SOURCE_DIR}/cutters/cylcutter.hpp
  ${OCL_SOURCE_DIR}/cutters/ellipseposition.hpp
  ${OCL_SOURCE_DIR}/cutters/millingcutter.hpp
  ${OCL_SOURCE_DIR}/cutters/ellipse.hpp

  ${OCL_SOURCE_DIR}/dropcutter/adaptivepathdropcutter.hpp
  ${OCL_SOURCE_DIR}/dropcutter/pathdropcutter.hpp
  ${OCL_SOURCE_DIR}/dropcutter/batchdropcutter.hpp
  ${OCL_SOURCE_DIR}/dropcutter/pointdropcutter.hpp

  ${OCL_SOURCE_DIR}/common/brent_zero.hpp
  ${OCL_SOURCE_DIR}/common/kdnode.hpp
  ${OCL_SOURCE_DIR}/common/kdtree.hpp
  ${OCL_SOURCE_DIR}/common/numeric.hpp
  ${OCL_SOURCE_DIR}/common/lineclfilter.hpp
  ${OCL_SOURCE_DIR}/common/clfilter.hpp
  ${OCL_SOURCE_DIR}/common/halfedgediagram.hpp

  ${OCL_SOURCE_DIR}/algo/operation.hpp
  ${OCL_SOURCE_DIR}/algo/batchpushcutter.hpp
  ${OCL_SOURCE_DIR}/algo/fiberpushcutter.hpp
  ${OCL_SOURCE_DIR}/algo/fiber.hpp
  ${OCL_SOURCE_DIR}/algo/interval.hpp
  ${OCL_SOURCE_DIR}/algo/waterline.hpp
  ${OCL_SOURCE_DIR}/algo/adaptivewaterline.hpp
  ${OCL_SOURCE_DIR}/algo/weave.hpp
  ${OCL_SOURCE_DIR}/algo/simple_weave.hpp
  ${OCL_SOURCE_DIR}/algo/smart_weave.hpp
  ${OCL_SOURCE_DIR}/algo/weave_typedef.hpp
  ${OCL_SOURCE_DIR}/algo/tsp.hpp
  )

# Compile the core once; each selected binding and native test reuses its objects.
add_library(ocl_core OBJECT ${OCL_SRC} ${OCL_GEO_SRC} ${OCL_CUTTER_SRC}
  ${OCL_ALGO_SRC} ${OCL_DROPCUTTER_SRC} ${OCL_COMMON_SRC})
set_target_properties(ocl_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(ocl_core PUBLIC
  ${OCL_SOURCE_DIR} ${OCL_SOURCE_DIR}/common ${OCL_SOURCE_DIR}/geo
  ${OCL_SOURCE_DIR}/algo ${OCL_SOURCE_DIR}/cutters ${OCL_SOURCE_DIR}/dropcutter
  ${CMAKE_CURRENT_BINARY_DIR})
ocl_configure_target(ocl_core)
if(USE_OPENMP)
  target_link_libraries(ocl_core PUBLIC OpenMP::OpenMP_CXX)
endif()
if(EMSCRIPTEN)
  # Compile the core, bindings, and tests with the same exception ABI. Native
  # Wasm EH avoids JS invoke wrappers in the contact-calculation hot paths.
  if(OCL_WASM_NATIVE_EXCEPTIONS)
    set(ocl_exception_flag -fwasm-exceptions)
  else()
    set(ocl_exception_flag -fexceptions)
  endif()
  target_compile_options(ocl_core PUBLIC ${ocl_exception_flag})
  target_link_options(ocl_core INTERFACE ${ocl_exception_flag})
endif()
if(OCL_WASM_THREADS)
  target_compile_definitions(ocl_core PUBLIC OCL_WASM_THREAD_POOL_SIZE=${OCL_WASM_THREAD_POOL_SIZE})
  target_compile_options(ocl_core PUBLIC -pthread)
  target_link_options(ocl_core PUBLIC -pthread
    "-sPTHREAD_POOL_SIZE=${OCL_WASM_THREAD_POOL_SIZE}" -sPTHREAD_POOL_SIZE_STRICT=2)
endif()

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

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
  add_subdirectory(${OCL_SOURCE_DIR}/../tests ${CMAKE_CURRENT_BINARY_DIR}/tests)
endif()

option(BUILD_BENCHMARKS "Build cutter performance benchmarks" OFF)
if(BUILD_BENCHMARKS AND NOT CMAKE_CROSSCOMPILING)
  add_subdirectory(${OCL_SOURCE_DIR}/../benchmarks ${CMAKE_CURRENT_BINARY_DIR}/benchmarks)
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

    configure_file(${OCL_SOURCE_DIR}/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    set(DOXY_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    execute_process(
      COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/doc/html ${CMAKE_CURRENT_BINARY_DIR}/doc/latex
    )

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
