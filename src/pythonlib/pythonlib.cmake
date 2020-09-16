# find boost so we can get it's version
find_package(Boost)

# debugging
set(Boost_DEBUG ON)

# set additional versions, when using a old CMake version this can be handy to find a modern boost
# set(Boost_ADDITIONAL_VERSIONS 1.69.0)

# set boost architecture and namespace, this is needed to make cmake find boost when you didn't compile boost (using b2) with the --layout=system option.
# set(Boost_ARCHITECTURE "-x64")
# set(Boost_NAMESPACE "libboost")

if (WIN32)
  # use static python lib
  add_definitions(-DBOOST_PYTHON_STATIC_LIB) 
  # disable autolinking in boost
  add_definitions( -DBOOST_ALL_NO_LIB ) # avoid LNK1104 on Windows: http://stackoverflow.com/a/28902261/122441
  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_RUNTIME OFF)
endif()

if(${CMAKE_VERSION} VERSION_LESS "3.12.0")
  message(STATUS "CMake version < 3.12.0")
  find_package(PythonInterp)
  if (PYTHONINTERP_FOUND)
    if (UNIX AND NOT APPLE)
      find_package(Boost COMPONENTS python${PYTHON_VERSION_SUFFIX})
      if (PYTHON_VERSION_MAJOR EQUAL 3)
          find_package(PythonInterp 3)
          find_package(PythonLibs 3 REQUIRED)
      else()
          find_package(PythonInterp)
          find_package(PythonLibs REQUIRED)
      endif()
    else()
      find_package(Boost COMPONENTS python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR})
      if (PYTHON_VERSION_MAJOR EQUAL 3)
          find_package(PythonInterp 3)
          find_package(PythonLibs 3 REQUIRED)
      else()
          find_package(PythonInterp)
          find_package(PythonLibs REQUIRED)
      endif()
    endif()
  else()
    message("Python not found")
  endif()
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "from distutils.sysconfig import get_python_lib; print get_python_lib(0,0,\"/usr/local\")"
    OUTPUT_VARIABLE PYTHON_SITE_PACKAGES
    OUTPUT_STRIP_TRAILING_WHITESPACE
  ) # on Ubuntu 11.10 this outputs: /usr/local/lib/python2.7/dist-packages

  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "from distutils.sysconfig import get_python_lib; print get_python_lib(plat_specific=1,standard_lib=0,prefix=\"/usr/local\")"
    OUTPUT_VARIABLE PYTHON_ARCH_PACKAGES
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
else()
  message(STATUS "CMake version >= 3.12.0")
  if (USE_PY_3)
    find_package(Python3 COMPONENTS Interpreter Development)
    set(PYTHON_INCLUDE_DIRS ${Python3_INCLUDE_DIRS})
    set(PYTHON_LIBRARIES ${Python3_LIBRARIES})
    set(PYTHON_SITE_PACKAGES ${Python3_SITELIB})
    set(PYTHON_ARCH_PACKAGES ${Python3_SITEARCH})
    find_package(Boost COMPONENTS python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR})
  else()
    find_package(Python2 COMPONENTS Interpreter Development)
    set(PYTHON_INCLUDE_DIRS ${Python2_INCLUDE_DIRS})
    set(PYTHON_LIBRARIES ${Python2_LIBRARIES})
    set(PYTHON_SITE_PACKAGES ${Python2_SITELIB})
    set(PYTHON_ARCH_PACKAGES ${Python2_SITEARCH})
    find_package(Boost COMPONENTS python${Python2_VERSION_MAJOR}${Python2_VERSION_MINOR})
  endif()
endif()

message(STATUS "Boost_INCLUDE_DIR = ${Boost_INCLUDE_DIR}")
message(STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost_LIBRARY_DIRS = ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}")
message(STATUS "PYTHON_SITE_PACKAGES = " ${PYTHON_SITE_PACKAGES})
message(STATUS "PYTHON_ARCH_PACKAGES = " ${PYTHON_ARCH_PACKAGES})
message(STATUS "PYTHON_INCLUDE_DIRS = ${PYTHON_INCLUDE_DIRS}")
message(STATUS "PYTHON_LIBRARIES = ${PYTHON_LIBRARIES}")
message(STATUS "PYTHON_SITE_PACKAGES = ${PYTHON_SITE_PACKAGES}")
message(STATUS "PYTHON_ARCH_PACKAGES = ${PYTHON_ARCH_PACKAGES}")

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${PYTHON_INCLUDE_DIRS})

# include dirs
include_directories( ${OpenCamLib_SOURCE_DIR}/cutters )
include_directories( ${OpenCamLib_SOURCE_DIR}/geo )
include_directories( ${OpenCamLib_SOURCE_DIR}/algo )
include_directories( ${OpenCamLib_SOURCE_DIR}/dropcutter )
include_directories( ${OpenCamLib_SOURCE_DIR}/common )
include_directories( ${OpenCamLib_SOURCE_DIR} )

# this makes the ocl Python module
add_library(
  ocl 
  MODULE
  pythonlib/ocl_cutters.cpp
  pythonlib/ocl_geometry.cpp
  pythonlib/ocl_algo.cpp
  pythonlib/ocl_dropcutter.cpp
  pythonlib/ocl.cpp
)

target_link_libraries(
  ocl
  ocl_common
  ocl_dropcutter
  ocl_cutters
  ocl_geo
  ocl_algo
  ${Boost_LIBRARIES}
  ${PYTHON_LIBRARIES}
)

message(STATUS "linking python binary ocl.so with boost: " ${Boost_PYTHON_LIBRARY})

# this makes the lib name ocl.so and not libocl.so
set_target_properties(ocl PROPERTIES PREFIX "")
if (WIN32)
  set_target_properties(ocl PROPERTIES SUFFIX ".pyd")
endif (WIN32)
# if (WIN32)
# set_target_properties(ocl PROPERTIES VERSION ${MY_VERSION})
# endif (WIN32)

if (APPLE AND NOT CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
install(
  TARGETS ocl
  LIBRARY DESTINATION lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages/ocl
)
# these are the python helper lib-files such as camvtk.py
install(
  DIRECTORY lib/
  DESTINATION lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages/ocl
  #    PATTERN .svn EXCLUDE
)
else()
install(
  TARGETS ocl
  LIBRARY DESTINATION ${PYTHON_ARCH_PACKAGES}
)
# these are the python helper lib-files such as camvtk.py 
install(
  DIRECTORY lib/
  DESTINATION ${PYTHON_SITE_PACKAGES}
  #    PATTERN .svn EXCLUDE
)
endif()
