#
# Figure out how to build and where to install the Python modules
#

if(${CMAKE_VERSION} VERSION_LESS "3.12.0")
  find_package(PythonInterp)
  if (PYTHONINTERP_FOUND)
    if (UNIX AND NOT APPLE)
      if (PYTHON_VERSION_MAJOR EQUAL 3)
          find_package(Boost COMPONENTS python${PYTHON_VERSION_SUFFIX})
          find_package(PythonInterp 3)
          find_package(PythonLibs 3 REQUIRED)
      else()
          find_package(Boost COMPONENTS python)
          find_package(PythonInterp)
          find_package(PythonLibs REQUIRED)
      endif()
    else()  
      if (PYTHON_VERSION_MAJOR EQUAL 3)
          find_package(Boost COMPONENTS python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR})
          find_package(PythonInterp 3)
          find_package(PythonLibs 3 REQUIRED)
      else()
          find_package(Boost COMPONENTS python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR})
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
  if (USE_PYTHONV3)
    if (PYTHON_ROOT_DIR)
      find_package(Python3 COMPONENTS Interpreter Development HINTS ${PYTHON_ROOT_DIR})
    else()
      find_package(Python3 COMPONENTS Interpreter Development)
    endif()
    if (UNIX AND NOT APPLE)
      find_package(Boost COMPONENTS python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR})
    else()  
      find_package(Boost COMPONENTS python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR})
    endif()
    set(PYTHON_INCLUDE_DIRS ${Python3_INCLUDE_DIRS})
    set(PYTHON_LIBRARIES ${Python3_LIBRARIES})
    set(PYTHON_SITE_PACKAGES ${Python3_SITELIB})
    set(PYTHON_ARCH_PACKAGES ${Python3_SITEARCH})
  else()
    if (PYTHON_ROOT_DIR)
      find_package(Python2 COMPONENTS Interpreter Development HINTS ${PYTHON_ROOT_DIR})
    else()
      find_package(Python2 COMPONENTS Interpreter Development)
    endif()
    find_package(Boost COMPONENTS python${Python2_VERSION_MAJOR}${Python2_VERSION_MINOR})
    set(PYTHON_INCLUDE_DIRS ${Python2_INCLUDE_DIRS})
    set(PYTHON_LIBRARIES ${Python2_LIBRARIES})
    set(PYTHON_SITE_PACKAGES ${Python2_SITELIB})
    set(PYTHON_ARCH_PACKAGES ${Python2_SITEARCH})
  endif()
endif()

message(STATUS "Python modules will be installed to: " ${PYTHON_SITE_PACKAGES})
message(STATUS "Python libraries will be installed to: " ${PYTHON_ARCH_PACKAGES})
message(STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}")
message(STATUS "PYTHON_INCLUDE_DIRS = ${PYTHON_INCLUDE_DIRS}")
message(STATUS "PYTHON_LIBRARIES = ${PYTHON_LIBRARIES}")
message(STATUS "PYTHON_SITE_PACKAGES = ${PYTHON_SITE_PACKAGES}")
message(STATUS "PYTHON_ARCH_PACKAGES = ${PYTHON_ARCH_PACKAGES}")

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${PYTHON_INCLUDE_DIRS})

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
set_target_properties(ocl PROPERTIES VERSION ${MY_VERSION}) 
endif (WIN32)

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