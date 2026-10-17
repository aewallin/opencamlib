# Register the same cases for native binaries and Node-hosted WASM binaries.
# Optional trailing arguments specify the runner (for example, the Node path).
function(ocl_add_cutter_tests target prefix)
  set(processors 1)
  if(USE_OPENMP OR OCL_WASM_THREADS)
    # The geometry tests request up to four threads per process.
    set(processors 4)
  endif()

  foreach(cutter cylinder ball bull cone composite_cylinder composite_ball)
    foreach(algorithm batch_drop batch_push adaptive_path adaptive_waterline)
      set(name ${prefix}.${algorithm}.${cutter})
      add_test(NAME ${name} COMMAND ${ARGN} $<TARGET_FILE:${target}> ${algorithm} ${cutter})
      set_tests_properties(${name} PROPERTIES
        TIMEOUT 120 PROCESSORS ${processors} LABELS "cutter;${algorithm}")
    endforeach()
  endforeach()

  add_test(NAME ${prefix}.composite_contacts
    COMMAND ${ARGN} $<TARGET_FILE:${target}> composite_contacts)
  set_tests_properties(${prefix}.composite_contacts PROPERTIES
    TIMEOUT 120 LABELS "cutter;ownership")

  if(USE_OPENMP OR OCL_WASM_THREADS)
    add_test(NAME ${prefix}.concurrent_waterline
      COMMAND ${ARGN} $<TARGET_FILE:${target}> concurrent_waterline)
    set_tests_properties(${prefix}.concurrent_waterline PROPERTIES
      TIMEOUT 120 PROCESSORS ${processors} LABELS "cutter;concurrency")
  endif()
endfunction()
