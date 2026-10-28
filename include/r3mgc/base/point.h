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

#ifndef r3mgc_base_point_h
#define r3mgc_base_point_h

#include <r3mgc/base/config.h>

#include <r3mgc/base/exceptions.h>

#include <array>
#include <cmath>
#include <ostream>

R3MGC_NAMESPACE_OPEN

/**
 * A point in @p dim dimensional Euclidean space, with coordinates of type
 * @p Number.
 *
 * This class plays the role of deal.II's Point class, of which it keeps the
 * interface for the operations the library needs: access to the coordinates,
 * vector space operations, scalar product, and distances. Contrary to
 * deal.II, there is no separate Tensor class: a Point is used both for
 * positions and for differences of positions.
 *
 * Points are used for the support points of the degrees of freedom of the
 * fine space, for the vertices of the fine cells, and for the corners of
 * bounding boxes. Through the adaptors in boost_adaptors/point.h, they can
 * also be stored in, or used to query, Boost R-trees.
 *
 * @tparam dim The space dimension. Only 1, 2, and 3 are used in the library,
 * but nothing in this class depends on it.
 * @tparam Number The type of the coordinates.
 */
template <int dim, typename Number = double>
class Point
{
public:
  static_assert(dim > 0, "The dimension of a Point must be positive.");

  /**
   * The space dimension, available as a static member.
   */
  static constexpr unsigned int dimension = dim;

  /**
   * The type of the coordinates.
   */
  using value_type = Number;

  /**
   * Constructor. Initialize all the coordinates to zero, i.e., build the
   * origin.
   */
  constexpr Point();

  /**
   * Constructor for one dimensional points.
   */
  constexpr explicit Point(const Number x);

  /**
   * Constructor for two dimensional points.
   */
  constexpr Point(const Number x, const Number y);

  /**
   * Constructor for three dimensional points.
   */
  constexpr Point(const Number x, const Number y, const Number z);

  /**
   * Read access to the @p i-th coordinate.
   */
  constexpr const Number &
  operator[](const unsigned int i) const;

  /**
   * Read and write access to the @p i-th coordinate.
   */
  constexpr Number &
  operator[](const unsigned int i);

  /**
   * Add @p p to this point, coordinate by coordinate.
   */
  constexpr Point &
  operator+=(const Point &p);

  /**
   * Subtract @p p from this point, coordinate by coordinate.
   */
  constexpr Point &
  operator-=(const Point &p);

  /**
   * Multiply all the coordinates by @p factor.
   */
  constexpr Point &
  operator*=(const Number factor);

  /**
   * Divide all the coordinates by @p factor.
   */
  constexpr Point &
  operator/=(const Number factor);

  /**
   * Return the sum of this point and @p p.
   */
  constexpr Point
  operator+(const Point &p) const;

  /**
   * Return the difference between this point and @p p.
   */
  constexpr Point
  operator-(const Point &p) const;

  /**
   * Return the point with all the coordinates of opposite sign.
   */
  constexpr Point
  operator-() const;

  /**
   * Return this point with all the coordinates multiplied by @p factor.
   */
  constexpr Point
  operator*(const Number factor) const;

  /**
   * Return this point with all the coordinates divided by @p factor.
   */
  constexpr Point
  operator/(const Number factor) const;

  /**
   * Return the scalar product between this point and @p p, both seen as
   * vectors.
   */
  constexpr Number
  operator*(const Point &p) const;

  /**
   * Return true if all the coordinates of this point and of @p p are
   * exactly equal.
   */
  constexpr bool
  operator==(const Point &p) const;

  /**
   * Return true if at least one coordinate of this point differs from the
   * corresponding coordinate of @p p.
   */
  constexpr bool
  operator!=(const Point &p) const;

  /**
   * Return the square of the Euclidean norm of this point, seen as a vector.
   */
  constexpr Number
  norm_square() const;

  /**
   * Return the Euclidean norm of this point, seen as a vector.
   */
  Number
  norm() const;

  /**
   * Return the square of the Euclidean distance between this point and
   * @p p.
   */
  constexpr Number
  distance_square(const Point &p) const;

  /**
   * Return the Euclidean distance between this point and @p p.
   */
  Number
  distance(const Point &p) const;

private:
  /**
   * The coordinates of the point.
   */
  std::array<Number, dim> values;
};



/**
 * Return the point @p p with all the coordinates multiplied by @p factor.
 */
template <int dim, typename Number>
constexpr Point<dim, Number>
operator*(const Number factor, const Point<dim, Number> &p);

/**
 * Write the coordinates of @p p to @p out, separated by a blank space (the
 * same format used by deal.II).
 */
template <int dim, typename Number>
std::ostream &
operator<<(std::ostream &out, const Point<dim, Number> &p);



/* ------------------------- Inline functions ------------------------------ */

template <int dim, typename Number>
constexpr Point<dim, Number>::Point()
  : values{}
{}



template <int dim, typename Number>
constexpr Point<dim, Number>::Point(const Number x)
  : values{}
{
  static_assert(dim == 1,
                "This constructor can only be used for one dimensional "
                "points.");
  values[0] = x;
}



template <int dim, typename Number>
constexpr Point<dim, Number>::Point(const Number x, const Number y)
  : values{}
{
  static_assert(dim == 2,
                "This constructor can only be used for two dimensional "
                "points.");
  values[0] = x;
  values[1] = y;
}



template <int dim, typename Number>
constexpr Point<dim, Number>::Point(const Number x,
                                    const Number y,
                                    const Number z)
  : values{}
{
  static_assert(dim == 3,
                "This constructor can only be used for three dimensional "
                "points.");
  values[0] = x;
  values[1] = y;
  values[2] = z;
}



template <int dim, typename Number>
constexpr const Number &
Point<dim, Number>::operator[](const unsigned int i) const
{
  R3MGC_AssertIndexRange(i, dim);
  return values[i];
}



template <int dim, typename Number>
constexpr Number &
Point<dim, Number>::operator[](const unsigned int i)
{
  R3MGC_AssertIndexRange(i, dim);
  return values[i];
}



template <int dim, typename Number>
constexpr Point<dim, Number> &
Point<dim, Number>::operator+=(const Point &p)
{
  for (unsigned int d = 0; d < dim; ++d)
    values[d] += p.values[d];
  return *this;
}



template <int dim, typename Number>
constexpr Point<dim, Number> &
Point<dim, Number>::operator-=(const Point &p)
{
  for (unsigned int d = 0; d < dim; ++d)
    values[d] -= p.values[d];
  return *this;
}



template <int dim, typename Number>
constexpr Point<dim, Number> &
Point<dim, Number>::operator*=(const Number factor)
{
  for (unsigned int d = 0; d < dim; ++d)
    values[d] *= factor;
  return *this;
}



template <int dim, typename Number>
constexpr Point<dim, Number> &
Point<dim, Number>::operator/=(const Number factor)
{
  for (unsigned int d = 0; d < dim; ++d)
    values[d] /= factor;
  return *this;
}



template <int dim, typename Number>
constexpr Point<dim, Number>
Point<dim, Number>::operator+(const Point &p) const
{
  Point result(*this);
  result += p;
  return result;
}



template <int dim, typename Number>
constexpr Point<dim, Number>
Point<dim, Number>::operator-(const Point &p) const
{
  Point result(*this);
  result -= p;
  return result;
}



template <int dim, typename Number>
constexpr Point<dim, Number>
Point<dim, Number>::operator-() const
{
  Point result;
  for (unsigned int d = 0; d < dim; ++d)
    result.values[d] = -values[d];
  return result;
}



template <int dim, typename Number>
constexpr Point<dim, Number>
Point<dim, Number>::operator*(const Number factor) const
{
  Point result(*this);
  result *= factor;
  return result;
}



template <int dim, typename Number>
constexpr Point<dim, Number>
Point<dim, Number>::operator/(const Number factor) const
{
  Point result(*this);
  result /= factor;
  return result;
}



template <int dim, typename Number>
constexpr Number
Point<dim, Number>::operator*(const Point &p) const
{
  Number result = Number();
  for (unsigned int d = 0; d < dim; ++d)
    result += values[d] * p.values[d];
  return result;
}



template <int dim, typename Number>
constexpr bool
Point<dim, Number>::operator==(const Point &p) const
{
  for (unsigned int d = 0; d < dim; ++d)
    if (values[d] != p.values[d])
      return false;
  return true;
}



template <int dim, typename Number>
constexpr bool
Point<dim, Number>::operator!=(const Point &p) const
{
  return !(*this == p);
}



template <int dim, typename Number>
constexpr Number
Point<dim, Number>::norm_square() const
{
  return (*this) * (*this);
}



template <int dim, typename Number>
inline Number
Point<dim, Number>::norm() const
{
  return std::sqrt(norm_square());
}



template <int dim, typename Number>
constexpr Number
Point<dim, Number>::distance_square(const Point &p) const
{
  return (*this - p).norm_square();
}



template <int dim, typename Number>
inline Number
Point<dim, Number>::distance(const Point &p) const
{
  return std::sqrt(distance_square(p));
}



template <int dim, typename Number>
constexpr Point<dim, Number>
operator*(const Number factor, const Point<dim, Number> &p)
{
  return p * factor;
}



template <int dim, typename Number>
inline std::ostream &
operator<<(std::ostream &out, const Point<dim, Number> &p)
{
  for (unsigned int d = 0; d < dim - 1; ++d)
    out << p[d] << ' ';
  out << p[dim - 1];
  return out;
}

R3MGC_NAMESPACE_CLOSE

#endif
