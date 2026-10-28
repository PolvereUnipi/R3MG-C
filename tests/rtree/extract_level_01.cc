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

// Extract the bounding boxes of every level of two R*-trees with
// extract_rtree_level(): a tree of the indices of an 8x8 grid of points, and
// a tree of the (box, index) pairs of the cells of a 4x4 grid. Also count the
// nodes of every level and the leaf values with visit_rtree().
//
// The expected output was generated with deal.II, by running the same
// calls of deal.II's pack_rtree_of_indices(), pack_rtree() and
// extract_rtree_level() on the same input: the boxes must be the same.

#include <r3mgc/base/bounding_box.h>
#include <r3mgc/base/point.h>
#include <r3mgc/base/types.h>

#include <r3mgc/rtree/rtree.h>

#include <vector>

#include "../tests.h"

using namespace r3mgc;
namespace bgi = boost::geometry::index;



template <typename RTreeType>
void
log_levels(const RTreeType &tree)
{
  const unsigned int n = n_levels(tree);
  r3mglog << "n_levels = " << n << std::endl;

  for (unsigned int level = 0; level < n; ++level)
    {
      const auto boxes = extract_rtree_level(tree, level);
      r3mglog << "level " << level << ": " << boxes.size() << " boxes"
              << std::endl;
      for (const auto &box : boxes)
        r3mglog << "  " << box << std::endl;
    }

  // Count the nodes on every level and the values in the leaves.
  std::vector<unsigned int> n_nodes(n, 0);
  unsigned int              n_values = 0;
  const auto                count =
    [&](const std::size_t &level, const auto &, const auto &, const auto &) {
      ++n_nodes[level];
    };
  visit_rtree(tree, count, count, [&](const auto &, const auto &) {
    ++n_values;
  });
  for (unsigned int level = 0; level < n; ++level)
    r3mglog << "visited nodes on level " << level << ": " << n_nodes[level]
            << std::endl;
  r3mglog << "visited values: " << n_values << std::endl;
}



int
main()
{
  initlog();

  // An 8x8 grid of points in [0,1]^2, numbered lexicographically.
  {
    r3mglog.push("points");
    const unsigned int    n = 8;
    std::vector<Point<2>> points;
    for (unsigned int j = 0; j < n; ++j)
      for (unsigned int i = 0; i < n; ++i)
        points.emplace_back(i / (n - 1.), j / (n - 1.));

    const auto tree = pack_rtree_of_indices<bgi::rstar<4, 2>>(points);
    log_levels(tree);
    r3mglog.pop();
  }

  // The cells of a 4x4 grid of [0,1]^2, numbered lexicographically.
  {
    r3mglog.push("cells");
    const unsigned int          n = 4;
    std::vector<BoxAndIndex<2>> cells;
    for (unsigned int j = 0; j < n; ++j)
      for (unsigned int i = 0; i < n; ++i)
        cells.emplace_back(
          BoundingBox<2>({Point<2>(i / double(n), j / double(n)),
                          Point<2>((i + 1) / double(n), (j + 1) / double(n))}),
          j * n + i);

    const auto tree = pack_rtree<bgi::rstar<4, 2>>(cells);
    log_levels(tree);
    r3mglog.pop();
  }
}
