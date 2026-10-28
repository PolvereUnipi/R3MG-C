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

#ifndef r3mgc_base_bounding_box_h
#define r3mgc_base_bounding_box_h

#include <r3mgc/base/config.h>

#include <r3mgc/base/exceptions.h>
#include <r3mgc/base/point.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <ostream>
#include <string>
#include <utility>

R3MGC_NAMESPACE_OPEN

/**
 * An axis-aligned box in @p dim dimensions, described by its lower-left and
 * upper-right corners.
 *
 * Bounding boxes are central to the R-tree agglomeration: they are the
 * geometric entities stored in the R-tree (one per fine cell, or one
 * degenerate box per support point), the minimum bounding rectangles of the
 * R-tree nodes, and the reference elements on which the coarse basis
 * functions of every agglomerate are defined. The last use is the reason for
 * the functions unit_to_real() and real_to_unit(), which implement the affine
 * map between the unit box [0,1]^dim and the box itself: they replace the
 * Mapping classes of deal.II, which the library does not need.
 *
 * The interface follows the BoundingBox class of deal.II, restricted to what
 * the library needs, with one addition: is_degenerate(), used to detect
 * agglomerates whose box is (numerically) flat. On such a box the map
 * real_to_unit() is not defined, and the agglomeration hierarchy is not
 * admissible.
 *
 * Boxes are closed sets: points on the boundary are inside.
 */
template <int dim, typename Number = double>
class BoundingBox
{
public:
  /**
   * Default constructor: build the empty box whose corners are both the
   * origin.
   */
  BoundingBox() = default;

  /**
   * Build the degenerate box containing only the point @p p.
   */
  explicit BoundingBox(const Point<dim, Number> &p);

  /**
   * Build the box with lower-left corner @p boundary_points.first and
   * upper-right corner @p boundary_points.second. In debug mode, check that
   * the first corner is indeed lower-left of the second one.
   */
  BoundingBox(
    const std::pair<Point<dim, Number>, Point<dim, Number>> &boundary_points);

  /**
   * Build the smallest box containing all the points in @p points. If the
   * container is empty, the box is the same as the one built by the default
   * constructor.
   *
   * @tparam Container Any container of Point<dim, Number> that can be
   * iterated over with a range-based for loop.
   */
  template <typename Container>
  explicit BoundingBox(const Container &points);

  /**
   * Return a reference to the pair of corners (lower-left, upper-right).
   */
  std::pair<Point<dim, Number>, Point<dim, Number>> &
  get_boundary_points();

  /**
   * Return a constant reference to the pair of corners (lower-left,
   * upper-right).
   */
  const std::pair<Point<dim, Number>, Point<dim, Number>> &
  get_boundary_points() const;

  /**
   * Return true if the corners of this box and of @p box coincide.
   */
  bool
  operator==(const BoundingBox &box) const;

  /**
   * Return true if the corners of this box and of @p box differ.
   */
  bool
  operator!=(const BoundingBox &box) const;

  /**
   * Enlarge this box to the smallest box containing both itself and
   * @p other_box.
   */
  void
  merge_with(const BoundingBox &other_box);

  /**
   * Return true if the point @p p is inside the box. The box is enlarged in
   * every direction by @p tolerance times its side length in that
   * direction, so that points lying on the boundary up to round-off errors
   * are considered to be inside.
   */
  bool
  point_inside(const Point<dim, Number> &p,
               const double              tolerance = 1e-12) const;

  /**
   * Return the lower bound of the box in direction @p direction.
   */
  Number
  lower_bound(const unsigned int direction) const;

  /**
   * Return the upper bound of the box in direction @p direction.
   */
  Number
  upper_bound(const unsigned int direction) const;

  /**
   * Return the length of the box in direction @p direction.
   */
  Number
  side_length(const unsigned int direction) const;

  /**
   * Return the volume (length in 1d, area in 2d) of the box.
   */
  Number
  volume() const;

  /**
   * Return the center of the box.
   */
  Point<dim, Number>
  center() const;

  /**
   * Return the diameter of the box, i.e., the distance between its corners.
   */
  Number
  diameter() const;

  /**
   * Return true if the box is degenerate, i.e., if it is (numerically) a
   * point, a segment in two or three dimensions, or a flat rectangle in three
   * dimensions.
   *
   * The test only uses the aspect ratio of the box itself, so that it is
   * independent of the size of the box and of the domain: the box is
   * degenerate if its longest side has length zero, or if its shortest side
   * is not longer than @p tolerance times its longest side. In one
   * dimension, the box is degenerate only if its length is zero.
   */
  bool
  is_degenerate(const double tolerance = 1e-10) const;

  /**
   * Map the point @p point of the unit box [0,1]^dim to the corresponding
   * point of this box.
   */
  Point<dim, Number>
  unit_to_real(const Point<dim, Number> &point) const;

  /**
   * Map the point @p point of this box to the corresponding point of the
   * unit box [0,1]^dim. This is the inverse of unit_to_real(). Points outside
   * the box are mapped to points outside the unit box.
   *
   * The map is only defined if no side of the box has length zero. In debug
   * mode, an exception is thrown if this is not the case.
   */
  Point<dim, Number>
  real_to_unit(const Point<dim, Number> &point) const;

private:
  /**
   * The lower-left and upper-right corners of the box.
   */
  std::pair<Point<dim, Number>, Point<dim, Number>> boundary_points;
};



/**
 * Write the corners of @p box to @p out, in the format "p_min : p_max".
 */
template <int dim, typename Number>
std::ostream &
operator<<(std::ostream &out, const BoundingBox<dim, Number> &box);



/* ------------------------- Inline functions ------------------------------ */

template <int dim, typename Number>
inline BoundingBox<dim, Number>::BoundingBox(const Point<dim, Number> &p)
  : boundary_points(p, p)
{}



template <int dim, typename Number>
inline BoundingBox<dim, Number>::BoundingBox(
  const std::pair<Point<dim, Number>, Point<dim, Number>> &boundary_points)
  : boundary_points(boundary_points)
{
  for (unsigned int d = 0; d < dim; ++d)
    R3MGC_Assert(boundary_points.first[d] <= boundary_points.second[d],
                 ExcMessage("The bounding box can't be created: the corners "
                            "must be given in the order lower-left, "
                            "upper-right."));
}



template <int dim, typename Number>
template <typename Container>
inline BoundingBox<dim, Number>::BoundingBox(const Container &points)
{
  // Leave the default (origin, origin) box if there are no points, instead
  // of building a box from +infinity to -infinity.
  if (std::begin(points) == std::end(points))
    return;

  Point<dim, Number> &p_min = boundary_points.first;
  Point<dim, Number> &p_max = boundary_points.second;
  for (unsigned int d = 0; d < dim; ++d)
    {
      p_min[d] = std::numeric_limits<Number>::infinity();
      p_max[d] = -std::numeric_limits<Number>::infinity();
    }

  for (const Point<dim, Number> &p : points)
    for (unsigned int d = 0; d < dim; ++d)
      {
        p_min[d] = std::min(p_min[d], p[d]);
        p_max[d] = std::max(p_max[d], p[d]);
      }
}



template <int dim, typename Number>
inline std::pair<Point<dim, Number>, Point<dim, Number>> &
BoundingBox<dim, Number>::get_boundary_points()
{
  return boundary_points;
}



template <int dim, typename Number>
inline const std::pair<Point<dim, Number>, Point<dim, Number>> &
BoundingBox<dim, Number>::get_boundary_points() const
{
  return boundary_points;
}



template <int dim, typename Number>
inline bool
BoundingBox<dim, Number>::operator==(const BoundingBox &box) const
{
  return boundary_points == box.boundary_points;
}



template <int dim, typename Number>
inline bool
BoundingBox<dim, Number>::operator!=(const BoundingBox &box) const
{
  return !(*this == box);
}



template <int dim, typename Number>
inline void
BoundingBox<dim, Number>::merge_with(const BoundingBox &other_box)
{
  for (unsigned int d = 0; d < dim; ++d)
    {
      boundary_points.first[d] =
        std::min(boundary_points.first[d], other_box.boundary_points.first[d]);
      boundary_points.second[d] = std::max(boundary_points.second[d],
                                           other_box.boundary_points.second[d]);
    }
}



template <int dim, typename Number>
inline bool
BoundingBox<dim, Number>::point_inside(const Point<dim, Number> &p,
                                       const double tolerance) const
{
  for (unsigned int d = 0; d < dim; ++d)
    if ((p[d] < boundary_points.first[d] - tolerance * side_length(d)) ||
        (p[d] > boundary_points.second[d] + tolerance * side_length(d)))
      return false;
  return true;
}



template <int dim, typename Number>
inline Number
BoundingBox<dim, Number>::lower_bound(const unsigned int direction) const
{
  R3MGC_AssertIndexRange(direction, dim);
  return boundary_points.first[direction];
}



template <int dim, typename Number>
inline Number
BoundingBox<dim, Number>::upper_bound(const unsigned int direction) const
{
  R3MGC_AssertIndexRange(direction, dim);
  return boundary_points.second[direction];
}



template <int dim, typename Number>
inline Number
BoundingBox<dim, Number>::side_length(const unsigned int direction) const
{
  R3MGC_AssertIndexRange(direction, dim);
  return boundary_points.second[direction] - boundary_points.first[direction];
}



template <int dim, typename Number>
inline Number
BoundingBox<dim, Number>::volume() const
{
  Number vol = 1.;
  for (unsigned int d = 0; d < dim; ++d)
    vol *= side_length(d);
  return vol;
}



template <int dim, typename Number>
inline Point<dim, Number>
BoundingBox<dim, Number>::center() const
{
  return (boundary_points.first + boundary_points.second) * Number(.5);
}



template <int dim, typename Number>
inline Number
BoundingBox<dim, Number>::diameter() const
{
  return boundary_points.first.distance(boundary_points.second);
}



template <int dim, typename Number>
inline bool
BoundingBox<dim, Number>::is_degenerate(const double tolerance) const
{
  Number min_side = side_length(0);
  Number max_side = side_length(0);
  for (unsigned int d = 1; d < dim; ++d)
    {
      min_side = std::min(min_side, side_length(d));
      max_side = std::max(max_side, side_length(d));
    }

  if (max_side == Number(0.))
    return true;

  return min_side <= tolerance * max_side;
}



template <int dim, typename Number>
inline Point<dim, Number>
BoundingBox<dim, Number>::unit_to_real(const Point<dim, Number> &point) const
{
  Point<dim, Number> real = boundary_points.first;
  for (unsigned int d = 0; d < dim; ++d)
    real[d] += side_length(d) * point[d];
  return real;
}



template <int dim, typename Number>
inline Point<dim, Number>
BoundingBox<dim, Number>::real_to_unit(const Point<dim, Number> &point) const
{
  Point<dim, Number> unit = point - boundary_points.first;
  for (unsigned int d = 0; d < dim; ++d)
    {
      R3MGC_Assert(side_length(d) > Number(0.),
                   ExcMessage("The map from the real to the unit box is not "
                              "defined, since the box has zero length in "
                              "direction " +
                              std::to_string(d) + "."));
      unit[d] /= side_length(d);
    }
  return unit;
}



template <int dim, typename Number>
inline std::ostream &
operator<<(std::ostream &out, const BoundingBox<dim, Number> &box)
{
  out << box.get_boundary_points().first << " : "
      << box.get_boundary_points().second;
  return out;
}

R3MGC_NAMESPACE_CLOSE

#endif
