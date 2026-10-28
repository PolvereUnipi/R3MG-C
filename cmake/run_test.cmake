# -----------------------------------------------------------------------------
#
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception OR LGPL-2.1-or-later
# Copyright (C) 2026 by the R3MG-C authors
#
# This file is part of the R3MG-C library.
#
# -----------------------------------------------------------------------------

#
# Driver for a single test, invoked by ctest in script mode as
#
#   cmake -DTEST_EXECUTABLE=<exe> -DTEST_DIRECTORY=<dir>
#         -DEXPECTED_OUTPUT=<file> -P run_test.cmake
#
# It runs the executable in TEST_DIRECTORY, and then compares the file
# "output" written by the test with EXPECTED_OUTPUT. The test fails if the
# executable returns a nonzero exit code, if it does not write any output, or
# if the output differs from the expected one. In the last case, both files are
# printed (and their diff, if the diff program is available), so that failures
# can be understood from the ctest log alone, e.g., in continuous integration.
#

foreach(var TEST_EXECUTABLE TEST_DIRECTORY EXPECTED_OUTPUT)
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "run_test.cmake: ${var} is not defined.")
  endif()
endforeach()

set(output_file "${TEST_DIRECTORY}/output")
file(REMOVE "${output_file}")

#
# Run the test.
#
execute_process(
  COMMAND "${TEST_EXECUTABLE}"
  WORKING_DIRECTORY "${TEST_DIRECTORY}"
  RESULT_VARIABLE run_result
  OUTPUT_VARIABLE run_stdout
  ERROR_VARIABLE run_stderr)

if(NOT run_result EQUAL 0)
  message(FATAL_ERROR
    "The test executable failed with exit code '${run_result}'.\n"
    "stdout:\n${run_stdout}\n"
    "stderr:\n${run_stderr}")
endif()

if(NOT EXISTS "${output_file}")
  message(FATAL_ERROR
    "The test did not write the file '${output_file}'. "
    "Did you forget to call initlog()?")
endif()

#
# Compare the output with the expected one.
#
execute_process(
  COMMAND ${CMAKE_COMMAND} -E compare_files
    "${output_file}" "${EXPECTED_OUTPUT}"
  RESULT_VARIABLE compare_result)

if(NOT compare_result EQUAL 0)
  file(READ "${output_file}" actual)
  file(READ "${EXPECTED_OUTPUT}" expected)

  set(diff_text "")
  find_program(DIFF_EXECUTABLE diff)
  if(DIFF_EXECUTABLE)
    execute_process(
      COMMAND "${DIFF_EXECUTABLE}" -u "${EXPECTED_OUTPUT}" "${output_file}"
      OUTPUT_VARIABLE diff_text)
  endif()

  message(FATAL_ERROR
    "The output of the test differs from the expected one.\n"
    "Expected output: ${EXPECTED_OUTPUT}\n"
    "Actual output:   ${output_file}\n"
    "---------------------------- diff -----------------------------\n"
    "${diff_text}\n"
    "------------------------ expected output ----------------------\n"
    "${expected}\n"
    "------------------------- actual output -----------------------\n"
    "${actual}")
endif()
