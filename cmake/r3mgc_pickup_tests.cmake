# -----------------------------------------------------------------------------
#
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception OR LGPL-2.1-or-later
# Copyright (C) 2026 by the R3MG-C authors
#
# This file is part of the R3MG-C library.
#
# -----------------------------------------------------------------------------

#
# r3mgc_pickup_tests(<module>)
#
# Set up all the tests in the directory tests/<module>, in the spirit of
# deal.II's DEAL_II_PICKUP_TESTS(). Every file <name>.cc in that directory is a
# test: it is compiled into an executable, which is run by ctest in its own
# directory. The output it writes (the file "output", see tests/tests.h) is
# compared with the expected output, which is the first existing file among
#
#   <name>.debug.output     (debug builds only)
#   <name>.release.output   (all the other build types)
#   <name>.output           (any build type)
#
# A test with no expected output is not set up, and a warning is printed.
# The ctest name of the test is <module>/<name>, so that, e.g.,
#
#   ctest -R rtree/
#
# runs all the tests of the module "rtree".
#
function(r3mgc_pickup_tests module)
  set(source_dir "${CMAKE_CURRENT_SOURCE_DIR}/${module}")
  file(GLOB test_sources RELATIVE "${source_dir}" "${source_dir}/*.cc")
  list(SORT test_sources)

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(build_suffix "debug")
  else()
    set(build_suffix "release")
  endif()

  foreach(test_source ${test_sources})
    get_filename_component(test_name "${test_source}" NAME_WE)

    # Find the expected output for the current build type.
    set(expected_output "")
    foreach(candidate
        "${source_dir}/${test_name}.${build_suffix}.output"
        "${source_dir}/${test_name}.output")
      if(EXISTS "${candidate}" AND expected_output STREQUAL "")
        set(expected_output "${candidate}")
      endif()
    endforeach()

    if(expected_output STREQUAL "")
      message(WARNING
        "The test ${module}/${test_name} has no expected output for a "
        "${build_suffix} build, and will not be run.")
      continue()
    endif()

    # The executable of the test, built in its own directory.
    set(target "${module}.${test_name}")
    set(test_dir "${CMAKE_CURRENT_BINARY_DIR}/${module}/${test_name}")

    add_executable(${target} "${source_dir}/${test_source}")
    target_link_libraries(${target} PRIVATE R3MGC::r3mgc)
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_compile_options(${target} PRIVATE
      $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
        -Wall -Wextra -Wpedantic>)
    set_target_properties(${target} PROPERTIES
      OUTPUT_NAME "${test_name}"
      RUNTIME_OUTPUT_DIRECTORY "${test_dir}")

    # Run the executable and compare its output with the expected one.
    add_test(NAME "${module}/${test_name}"
      COMMAND ${CMAKE_COMMAND}
        -DTEST_EXECUTABLE=$<TARGET_FILE:${target}>
        -DTEST_DIRECTORY=${test_dir}
        -DEXPECTED_OUTPUT=${expected_output}
        -P "${R3MGC_TEST_DRIVER}")
    set_tests_properties("${module}/${test_name}" PROPERTIES
      LABELS "${module}")
  endforeach()
endfunction()

# The script that runs a single test, see cmake/run_test.cmake.
set(R3MGC_TEST_DRIVER "${CMAKE_CURRENT_LIST_DIR}/run_test.cmake")
