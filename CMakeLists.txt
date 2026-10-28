# -----------------------------------------------------------------------------
#
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception OR LGPL-2.1-or-later
# Copyright (C) 2026 by the R3MG-C authors
#
# This file is part of the R3MG-C library.
#
# -----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.16)

project(R3MGC
  VERSION 0.1.0
  DESCRIPTION "R-tree based agglomeration and transfer operators for algebraic multigrid"
  LANGUAGES CXX)

#
# Options.
#
option(R3MGC_BUILD_TESTS "Build the test suite of the library." ON)

#
# Default to a debug build, as deal.II does for development: the consistency
# checks implemented with R3MGC_Assert are only active in debug mode. Every
# build type other than Debug defines NDEBUG, and therefore disables them.
#
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()
message(STATUS "R3MG-C build type: ${CMAKE_BUILD_TYPE}")

#
# Dependencies. Boost is the only dependency of the library, and only its
# header-only components (Geometry) are used.
#
if(POLICY CMP0167)
  # Use the BoostConfig.cmake file shipped with Boost, rather than the
  # FindBoost module that was removed from CMake.
  cmake_policy(SET CMP0167 NEW)
endif()
find_package(Boost 1.70 REQUIRED)
message(STATUS "Found Boost ${Boost_VERSION} in ${Boost_INCLUDE_DIRS}")

#
# The library target. For now, all of R3MG-C is header-only, so the target is
# an INTERFACE library carrying the include directories, the C++ standard,
# and the dependency on Boost. It will become a compiled library (with
# explicit instantiations for dim = 1, 2, 3) once non-template code is added.
#
add_library(r3mgc INTERFACE)
add_library(R3MGC::r3mgc ALIAS r3mgc)

target_include_directories(r3mgc INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(r3mgc INTERFACE cxx_std_17)
target_link_libraries(r3mgc INTERFACE Boost::headers)

#
# Tests.
#
if(R3MGC_BUILD_TESTS)
  enable_testing()
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/r3mgc_pickup_tests.cmake)
  add_subdirectory(tests)
endif()
