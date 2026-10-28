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

#ifndef r3mgc_boost_adaptors_bounding_box_h
#define r3mgc_boost_adaptors_bounding_box_h

#include <r3mgc/base/config.h>

#include <r3mgc/base/bounding_box.h>

#include <r3mgc/boost_adaptors/point.h>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>

#include <cstddef>

/**
 * @file
 *
 * Register r3mgc::BoundingBox as a Boost.Geometry box type, so that it can be
 * stored in a Boost R-tree and used in its queries (e.g., intersects()), and
 * so that the bounds of R-tree nodes can be converted into BoundingBox
 * objects with boost::geometry::convert(). This is a port of deal.II's
 * boost_adaptors/bounding_box.h.
 *
 * The Boost.Geometry Box concept requires a tag, the type of the corner
 * points (an r3mgc::Point, registered in boost_adaptors/point.h), and the
 * access to each coordinate of the two corners.
 */

namespace boost
{
  namespace geometry
  {
    namespace traits
    {
      /**
       * Tag adaptor for r3mgc::BoundingBox.
       */
      template <int dim, class Number>
      struct tag<r3mgc::BoundingBox<dim, Number>>
      {
        using type = box_tag;
      };

      /**
       * Point type adaptor for r3mgc::BoundingBox.
       */
      template <int dim, class Number>
      struct point_type<r3mgc::BoundingBox<dim, Number>>
      {
        using type = r3mgc::Point<dim, Number>;
      };

      /**
       * Access to the D-th coordinate of the lower-left corner of an
       * r3mgc::BoundingBox.
       */
      template <int dim, class Number, std::size_t D>
      struct indexed_access<r3mgc::BoundingBox<dim, Number>, min_corner, D>
      {
        /**
         * Return the D-th coordinate of the lower-left corner of @p box.
         */
        static inline Number
        get(const r3mgc::BoundingBox<dim, Number> &box)
        {
          return box.get_boundary_points().first[D];
        }

        /**
         * Set the D-th coordinate of the lower-left corner of @p box to
         * @p value.
         */
        static inline void
        set(r3mgc::BoundingBox<dim, Number> &box, Number value)
        {
          box.get_boundary_points().first[D] = value;
        }
      };

      /**
       * Access to the D-th coordinate of the upper-right corner of an
       * r3mgc::BoundingBox.
       */
      template <int dim, class Number, std::size_t D>
      struct indexed_access<r3mgc::BoundingBox<dim, Number>, max_corner, D>
      {
        /**
         * Return the D-th coordinate of the upper-right corner of @p box.
         */
        static inline Number
        get(const r3mgc::BoundingBox<dim, Number> &box)
        {
          return box.get_boundary_points().second[D];
        }

        /**
         * Set the D-th coordinate of the upper-right corner of @p box to
         * @p value.
         */
        static inline void
        set(r3mgc::BoundingBox<dim, Number> &box, Number value)
        {
          box.get_boundary_points().second[D] = value;
        }
      };
    } // namespace traits
  }   // namespace geometry
} // namespace boost

#endif
