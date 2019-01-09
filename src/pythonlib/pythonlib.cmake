#
# Figure out how to build and where to install the Python modules
#

if (USE_PYTHONV3)
  find_package(Python3 COMPONENTS Interpreter Development)
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
  find_package(Python2 COMPONENTS Interpreter Development)
  find_package(Boost COMPONENTS python${Python2_VERSION_MAJOR}${Python2_VERSION_MINOR})
  set(PYTHON_INCLUDE_DIRS ${Python2_INCLUDE_DIRS})
  set(PYTHON_LIBRARIES ${Python2_LIBRARIES})
  set(PYTHON_SITE_PACKAGES ${Python2_SITELIB})
  set(PYTHON_ARCH_PACKAGES ${Python2_SITEARCH})
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