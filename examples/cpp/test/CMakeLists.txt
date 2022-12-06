cmake_minimum_required(VERSION 3.12...3.15)

cmake_policy(SET CMP0025 NEW)
set(CMAKE_CXX_STANDARD 20)

project(OCL_TEST_EXAMPLE)

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

find_package(OpenCamLib REQUIRED)

set(EXAMPLE_SRC
  ${PROJECT_SOURCE_DIR}/test_example.cpp
)

add_executable(
  test_example
  ${EXAMPLE_SRC}
)

target_link_libraries(
  test_example
  OpenCamLib::ocl
)

if(WIN32)
  add_custom_command(TARGET test_example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:test_example> $<TARGET_FILE_DIR:test_example>
    COMMAND_EXPAND_LISTS
  )
endif()