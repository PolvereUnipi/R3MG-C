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

#ifndef r3mgc_boost_adaptors_point_h
#define r3mgc_boost_adaptors_point_h

#include <r3mgc/base/config.h>

#include <r3mgc/base/point.h>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/mpl/int.hpp>

#include <cstddef>

/**
 * @file
 *
 * Register r3mgc::Point as a Boost.Geometry point type, so that it can be
 * stored in a Boost R-tree and used in its queries. This is a port of
 * deal.II's boost_adaptors/point.h.
 *
 * The traits below are the ones required by the Boost.Geometry Point
 * concept: a tag, the coordinate type, the coordinate system (always
 * cartesian), the dimension, and the access to each coordinate.
 */

namespace boost
{
  namespace geometry
  {
    namespace traits
    {
      /**
       * Tag adaptor for r3mgc::Point.
       */
      template <int dim, class Number>
      struct tag<r3mgc::Point<dim, Number>>
      {
        using type = point_tag;
      };

      /**
       * Coordinate type adaptor for r3mgc::Point.
       */
      template <int dim, class Number>
      struct coordinate_type<r3mgc::Point<dim, Number>>
      {
        using type = Number;
      };

      /**
       * Coordinate system adaptor for r3mgc::Point. Points are always
       * cartesian.
       */
      template <int dim, class Number>
      struct coordinate_system<r3mgc::Point<dim, Number>>
      {
        using type = cs::cartesian;
      };

      /**
       * Dimension adaptor for r3mgc::Point.
       */
      template <int dim, class Number>
      struct dimension<r3mgc::Point<dim, Number>> : boost::mpl::int_<dim>
      {};

      /**
       * Getter and setter for the D-th coordinate of an r3mgc::Point.
       */
      template <std::size_t D, int dim, class Number>
      struct access<r3mgc::Point<dim, Number>, D>
      {
        /**
         * Return the D-th coordinate of @p p.
         */
        static inline Number
        get(const r3mgc::Point<dim, Number> &p)
        {
          return p[D];
        }

        /**
         * Set the D-th coordinate of @p p to @p value.
         */
        static inline void
        set(r3mgc::Point<dim, Number> &p, Number value)
        {
          p[D] = value;
        }
      };
    } // namespace traits
  }   // namespace geometry
} // namespace boost

#endif
