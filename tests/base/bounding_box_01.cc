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

// Test the BoundingBox class in one, two, and three dimensions: construction
// from corners and from a set of points, geometric quantities, merging, point
// location, the maps between the unit box and the real box, and the detection
// of degenerate boxes.

#include <r3mgc/base/bounding_box.h>

#include <vector>

#include "../tests.h"

using namespace r3mgc;



template <int dim>
void
test()
{
  r3mglog.push(std::to_string(dim) + "d");

  // The box [0,1] x [0,2] x [0,3].
  Point<dim> p_min, p_max;
  for (unsigned int d = 0; d < dim; ++d)
    p_max[d] = d + 1.;
  const BoundingBox<dim> box({p_min, p_max});

  r3mglog << "box = " << box << std::endl;
  for (unsigned int d = 0; d < dim; ++d)
    r3mglog << "direction " << d << ": [" << box.lower_bound(d) << ", "
            << box.upper_bound(d) << "], length " << box.side_length(d)
            << std::endl;
  r3mglog << "volume = " << box.volume() << std::endl;
  r3mglog << "center = " << box.center() << std::endl;
  r3mglog << "diameter = " << box.diameter() << std::endl;

  // The smallest box containing a set of points.
  std::vector<Point<dim>> points(3);
  for (unsigned int d = 0; d < dim; ++d)
    {
      points[0][d] = 0.5;
      points[1][d] = -1. * d;
      points[2][d] = 2. + d;
    }
  const BoundingBox<dim> points_box(points);
  r3mglog << "box of points = " << points_box << std::endl;
  r3mglog << "box of no points = "
          << BoundingBox<dim>(std::vector<Point<dim>>()) << std::endl;

  // Merging.
  BoundingBox<dim> merged = box;
  merged.merge_with(points_box);
  r3mglog << "merged = " << merged << std::endl;

  // Point location: the box is closed, and the tolerance is relative to the
  // side lengths.
  r3mglog << "center inside: " << box.point_inside(box.center()) << std::endl;
  r3mglog << "corner inside: " << box.point_inside(p_max) << std::endl;
  Point<dim> slightly_outside = p_max;
  slightly_outside[0] += 1e-14;
  r3mglog << "corner + 1e-14 inside: " << box.point_inside(slightly_outside)
          << std::endl;
  slightly_outside[0] += 1e-6;
  r3mglog << "corner + 1e-6 inside: " << box.point_inside(slightly_outside)
          << std::endl;
  r3mglog << "corner + 1e-6 inside with tolerance 1e-5: "
          << box.point_inside(slightly_outside, 1e-5) << std::endl;

  // The maps between the unit box and the real box are inverse of each other.
  Point<dim> unit_point;
  for (unsigned int d = 0; d < dim; ++d)
    unit_point[d] = 0.25 * (d + 1);
  const Point<dim> real_point = box.unit_to_real(unit_point);
  r3mglog << "unit_to_real(" << unit_point << ") = " << real_point << std::endl;
  r3mglog << "real_to_unit(" << real_point
          << ") = " << box.real_to_unit(real_point) << std::endl;
  r3mglog << "unit_to_real(0) = " << box.unit_to_real(Point<dim>())
          << std::endl;
  r3mglog << "real_to_unit(p_max) = " << box.real_to_unit(p_max) << std::endl;
  r3mglog << "round trip error = "
          << unit_point.distance(
               points_box.real_to_unit(points_box.unit_to_real(unit_point)))
          << std::endl;

  // Degenerate boxes.
  r3mglog << "box degenerate: " << box.is_degenerate() << std::endl;
  r3mglog << "point box degenerate: "
          << BoundingBox<dim>(Point<dim>()).is_degenerate() << std::endl;
  if (dim > 1)
    {
      // A flat box in the last direction, with a thickness small with
      // respect to the other sides: degenerate for the default tolerance,
      // not degenerate for a smaller one.
      Point<dim> flat_max = p_max;
      flat_max[dim - 1]   = 1e-11;
      const BoundingBox<dim> flat({p_min, flat_max});
      r3mglog << "flat box degenerate: " << flat.is_degenerate() << std::endl;
      r3mglog << "flat box degenerate with tolerance 1e-12: "
              << flat.is_degenerate(1e-12) << std::endl;

      // A tiny but well shaped box is not degenerate: the check does not
      // depend on the size of the box.
      const BoundingBox<dim> tiny({p_min, p_max * 1e-20});
      r3mglog << "tiny box degenerate: " << tiny.is_degenerate() << std::endl;
    }

  r3mglog << "box == box: " << (box == box) << std::endl;
  r3mglog << "box != merged: " << (box != merged) << std::endl;

  r3mglog.pop();
}



int
main()
{
  initlog();

  test<1>();
  test<2>();
  test<3>();
}
