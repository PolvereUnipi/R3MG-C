// -----------------------------------------------------------------------------
//
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception OR LGPL-2.1-or-later
// Copyright (C) 2026 by the R3MG-C authors
//
// This file is part of the R3MG-C library.
//
// Detailed license information governing the source code
// can be found in LICENSE.md at the top level directory.
//
// -----------------------------------------------------------------------------

#ifndef r3mgc_base_config_h
#define r3mgc_base_config_h

/**
 * @file
 *
 * Global configuration of the R3MG-C library. This file must be the first
 * one included by every other header of the library, since it defines the
 * macros used to open and close the library namespace, and the version
 * information.
 */



/* -------------------------------------------------------------------------
 * Version information.
 * ------------------------------------------------------------------------- */

#define R3MGC_VERSION_MAJOR 0
#define R3MGC_VERSION_MINOR 1
#define R3MGC_VERSION_SUBMINOR 0

/**
 * Evaluate to true if the version of the library is greater than or equal to
 * major.minor.subminor.
 */
#define R3MGC_VERSION_GTE(major, minor, subminor)             \
  ((R3MGC_VERSION_MAJOR * 10000 + R3MGC_VERSION_MINOR * 100 + \
    R3MGC_VERSION_SUBMINOR) >= ((major) * 10000 + (minor) * 100 + (subminor)))



/* -------------------------------------------------------------------------
 * Namespace macros.
 *
 * Every public symbol of the library lives in the namespace r3mgc. We use
 * macros (as deal.II does) so that the namespace could be renamed or
 * versioned in a single place.
 * ------------------------------------------------------------------------- */

#define R3MGC_NAMESPACE r3mgc
#define R3MGC_NAMESPACE_OPEN \
  namespace R3MGC_NAMESPACE  \
  {
#define R3MGC_NAMESPACE_CLOSE }



/* -------------------------------------------------------------------------
 * Debug mode.
 *
 * The library follows the standard convention of the C++ assert() macro:
 * consistency checks implemented with R3MGC_Assert are active unless NDEBUG
 * is defined. CMake defines NDEBUG for all the non-debug build types.
 * ------------------------------------------------------------------------- */

#ifndef NDEBUG
#  define R3MGC_WITH_DEBUG
#endif

#endif
