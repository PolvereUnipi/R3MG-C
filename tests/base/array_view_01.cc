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

// Test ArrayView: views of whole vectors and of slices, read-only and
// writable views, iteration, comparison, and the empty view.

#include <r3mgc/base/array_view.h>

#include <vector>

#include "../tests.h"

using namespace r3mgc;



template <typename T>
void
log_view(const std::string &name, const ArrayView<T> &view)
{
  r3mglog << name << " (size " << view.size() << "):";
  for (const auto &x : view)
    r3mglog << ' ' << x;
  r3mglog << std::endl;
}



int
main()
{
  initlog();

  std::vector<int> v = {0, 1, 2, 3, 4, 5, 6, 7};

  // A read-only view of a whole constant vector.
  const std::vector<int>    &cv = v;
  const ArrayView<const int> all(cv);
  log_view("all", all);

  // A slice in the middle.
  const ArrayView<const int> slice(v.data() + 2, 4);
  log_view("slice", slice);
  r3mglog << "slice[1] = " << slice[1] << std::endl;

  // A writable view: changes are visible in the vector.
  const ArrayView<int> writable(v);
  writable[0] = 10;
  for (auto &x : ArrayView<int>(v.data() + 6, 2))
    x *= 2;
  log_view("vector after writes", ArrayView<const int>(cv));

  // Comparison is by value, not by memory location.
  const std::vector<int>     copy = {2, 3, 4, 5};
  const ArrayView<const int> copy_view(copy);
  r3mglog << "slice == copy: " << (slice == copy_view) << std::endl;
  r3mglog << "slice != all: " << (slice != all) << std::endl;

  // The empty view.
  const ArrayView<const int> empty;
  r3mglog << "empty: " << empty.empty() << ", size " << empty.size()
          << ", begin == end: " << (empty.begin() == empty.end()) << std::endl;
}
