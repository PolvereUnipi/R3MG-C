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

// Check that, in debug mode, BoundingBox::real_to_unit() throws an exception
// on a box with a side of length zero, and that a box cannot be built from
// corners given in the wrong order. Both checks are disabled in release mode,
// hence the two expected outputs.

#include <r3mgc/base/bounding_box.h>

#include "../tests.h"

using namespace r3mgc;



int
main()
{
  initlog();

  // A segment in two dimensions.
  const BoundingBox<2> segment({Point<2>(0., 0.), Point<2>(1., 0.)});
  r3mglog << "segment degenerate: " << segment.is_degenerate() << std::endl;
  check_throw(segment.real_to_unit(Point<2>(0.5, 0.)));

  // The unit map of a non-degenerate box works.
  const BoundingBox<2> box({Point<2>(0., 0.), Point<2>(1., 2.)});
  check_throw(box.real_to_unit(Point<2>(0.5, 1.)));

  // Corners in the wrong order. The pair is built outside of the macro, since
  // the comma between the corners would split the macro argument.
  const std::pair<Point<2>, Point<2>> wrong_corners(Point<2>(1., 1.),
                                                    Point<2>(0., 0.));
  check_throw(BoundingBox<2>{wrong_corners});
}
