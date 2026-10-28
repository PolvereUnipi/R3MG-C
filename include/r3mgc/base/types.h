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

#ifndef r3mgc_base_types_h
#define r3mgc_base_types_h

#include <r3mgc/base/config.h>

#include <cstdint>
#include <limits>

R3MGC_NAMESPACE_OPEN

/**
 * A namespace collecting the integer types used throughout the library.
 *
 * Global indices (degrees of freedom, cells, agglomerates) are stored as 64
 * bit unsigned integers, so that the transfer matrices produced by the
 * library can be handed over without conversions to external solvers built
 * with 64 bit indices (e.g., PETSc configured with --with-64-bit-indices).
 * Small local quantities (number of DoFs per agglomerate, polynomial degrees,
 * levels of the hierarchy) use plain unsigned int.
 */
namespace types
{
  /**
   * The type used to index degrees of freedom, both on the finest level (in
   * the numbering provided by the user) and on the agglomerated levels.
   */
  using global_dof_index = std::uint64_t;

  /**
   * The type used to index cells of the fine mesh, and agglomerates (i.e.,
   * R-tree nodes) of the coarse levels.
   */
  using global_cell_index = std::uint64_t;
} // namespace types



namespace numbers
{
  /**
   * An invalid value for a degree of freedom index. Used, e.g., to mark fine
   * DoFs that have not been assigned to any agglomerate yet.
   */
  inline constexpr types::global_dof_index invalid_dof_index =
    std::numeric_limits<types::global_dof_index>::max();

  /**
   * An invalid value for a cell (or agglomerate) index.
   */
  inline constexpr types::global_cell_index invalid_cell_index =
    std::numeric_limits<types::global_cell_index>::max();

  /**
   * An invalid value for an unsigned int.
   */
  inline constexpr unsigned int invalid_unsigned_int =
    std::numeric_limits<unsigned int>::max();
} // namespace numbers

R3MGC_NAMESPACE_CLOSE

#endif
