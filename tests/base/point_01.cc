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

// Test the constructors, the arithmetic operations, and the norms of the
// Point class in one, two, and three dimensions.

#include <r3mgc/base/point.h>

#include "../tests.h"

using namespace r3mgc;



template <int dim>
Point<dim>
make_point(const double offset)
{
  Point<dim> p;
  for (unsigned int d = 0; d < dim; ++d)
    p[d] = offset + d;
  return p;
}



template <int dim>
void
test()
{
  r3mglog.push(std::to_string(dim) + "d");

  const Point<dim> origin;
  const Point<dim> p = make_point<dim>(1.);  // (1, 2, 3)
  const Point<dim> q = make_point<dim>(-2.); // (-2, -1, 0)

  r3mglog << "origin = " << origin << std::endl;
  r3mglog << "p = " << p << std::endl;
  r3mglog << "q = " << q << std::endl;

  r3mglog << "p + q = " << p + q << std::endl;
  r3mglog << "p - q = " << p - q << std::endl;
  r3mglog << "-p = " << -p << std::endl;
  r3mglog << "2 * p = " << 2. * p << std::endl;
  r3mglog << "p * 2 = " << p * 2. << std::endl;
  r3mglog << "p / 4 = " << p / 4. << std::endl;
  r3mglog << "p * q = " << p * q << std::endl;

  Point<dim> r = p;
  r += q;
  r -= p;
  r3mglog << "p + q - p == q: " << (r == q) << std::endl;
  r *= 3.;
  r /= 3.;
  r3mglog << "(3 q) / 3 == q: " << (r == q) << std::endl;
  r3mglog << "p != q: " << (p != q) << std::endl;

  r3mglog << "|p|^2 = " << p.norm_square() << std::endl;
  r3mglog << "|p| = " << p.norm() << std::endl;
  r3mglog << "|p - q|^2 = " << p.distance_square(q) << std::endl;
  r3mglog << "|p - q| = " << p.distance(q) << std::endl;

  r3mglog.pop();
}



int
main()
{
  initlog();

  test<1>();
  test<2>();
  test<3>();

  // The constructors that take the coordinates directly.
  r3mglog << Point<1>(0.5) << std::endl;
  r3mglog << Point<2>(0.5, 1.5) << std::endl;
  r3mglog << Point<3>(0.5, 1.5, 2.5) << std::endl;
}
