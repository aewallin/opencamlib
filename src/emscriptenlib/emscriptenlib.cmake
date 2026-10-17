add_executable(ocl ${OCL_SOURCE_DIR}/emscriptenlib/emscriptenlib.cpp)
target_link_libraries(ocl PRIVATE ocl_core)
ocl_configure_target(ocl)
# Current Embind headers require C++17.
target_compile_features(ocl PRIVATE cxx_std_17)
set(OCL_WASM_ENVIRONMENT "web,worker" CACHE STRING "Supported Emscripten runtimes")
target_link_options(ocl PRIVATE
  -lembind -sMODULARIZE=1 -sEXPORT_NAME=ocl -sFORCE_FILESYSTEM=1
  "-sEXPORTED_RUNTIME_METHODS=['FS']" -sASSERTIONS=1 -sALLOW_MEMORY_GROWTH=1
  -sSINGLE_FILE=1 "-sENVIRONMENT=${OCL_WASM_ENVIRONMENT}"
  $<$<CONFIG:Release>:--closure> $<$<CONFIG:Release>:1>)
install(TARGETS ocl RUNTIME DESTINATION ".")

if(BUILD_TESTING)
  # Run real WASM workers under Node, including the serial/parallel geometry
  # equivalence tests. EXIT_RUNTIME tears down the pool after main returns.
  find_program(OCL_NODE_EXECUTABLE node REQUIRED)
  include("${OCL_SOURCE_DIR}/../tests/cutter_tests.cmake")
  foreach(test_name cutter parallel)
    add_executable(wasm_${test_name}_test ${OCL_SOURCE_DIR}/../tests/${test_name}_test.cpp)
    target_link_libraries(wasm_${test_name}_test PRIVATE ocl_core)
    ocl_configure_target(wasm_${test_name}_test)
    target_link_options(wasm_${test_name}_test PRIVATE -sENVIRONMENT=node,worker
      -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
    if(test_name STREQUAL "cutter")
      ocl_add_cutter_tests(wasm_cutter_test wasm_cutter "${OCL_NODE_EXECUTABLE}")
    else()
      add_test(NAME wasm_${test_name} COMMAND ${OCL_NODE_EXECUTABLE} $<TARGET_FILE:wasm_${test_name}_test>)
      set_tests_properties(wasm_${test_name} PROPERTIES TIMEOUT 120)
    endif()
  endforeach()
endif()
