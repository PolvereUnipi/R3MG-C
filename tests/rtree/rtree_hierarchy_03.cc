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

// Print the agglomerates and the parent-to-children map of every level of
// the RTreeHierarchy of two R*-trees: a tree of the indices of 60 random
// points, and a tree of the (box, index) pairs of the cells of an 8x6 grid.
//
// The expected output was generated with polyDEAL, by extracting every level
// of the same trees with its CellsAgglomerator class (with use_points = true
// for the points, and with the deal.II cells of a subdivided_hyper_rectangle
// for the grid), and printing the agglomerates and get_hierarchy() in the
// same format: the agglomerates and the hierarchy must be the same.

#include <r3mgc/base/bounding_box.h>
#include <r3mgc/base/point.h>
#include <r3mgc/base/types.h>

#include <r3mgc/rtree/rtree.h>
#include <r3mgc/rtree/rtree_hierarchy.h>

#include <random>
#include <vector>

#include "../tests.h"

using namespace r3mgc;
namespace bgi = boost::geometry::index;



// Return a uniformly distributed number in [0,1), using only the raw output of
// std::mt19937_64, which is the same for all the standard libraries.
double
random_value(std::mt19937_64 &generator)
{
  return (generator() >> 11) * 0x1.0p-53;
}



template <typename HierarchyType>
void
log_hierarchy(const HierarchyType &hierarchy)
{
  r3mglog << "n_levels = " << hierarchy.n_levels() << std::endl;
  for (unsigned int level = 0; level < hierarchy.n_levels(); ++level)
    {
      r3mglog << "level " << level << ": " << hierarchy.n_nodes(level)
              << " agglomerates" << std::endl;
      for (types::global_cell_index k = 0; k < hierarchy.n_nodes(level); ++k)
        {
          r3mglog << "  agglomerate " << k << ":";
          for (const auto id : hierarchy.agglomerate(level, k))
            r3mglog << ' ' << id;
          r3mglog << std::endl;
        }
      if (level + 1 < hierarchy.n_levels())
        for (types::global_cell_index k = 0; k < hierarchy.n_nodes(level); ++k)
          {
            r3mglog << "  children of " << k << ":";
            const auto [first, last] = hierarchy.children(level, k);
            for (auto c = first; c < last; ++c)
              r3mglog << ' ' << c;
            r3mglog << std::endl;
          }
    }
}



int
main()
{
  initlog();

  {
    r3mglog.push("points");
    std::mt19937_64       generator(7);
    std::vector<Point<2>> points(60);
    for (auto &p : points)
      {
        p[0] = random_value(generator);
        p[1] = random_value(generator);
      }

    const auto tree = pack_rtree_of_indices<bgi::rstar<4, 2>>(points);
    log_hierarchy(RTreeHierarchy(tree));
    r3mglog.pop();
  }

  {
    r3mglog.push("cells");
    const unsigned int          nx = 8, ny = 6;
    std::vector<BoxAndIndex<2>> cells;
    for (unsigned int j = 0; j < ny; ++j)
      for (unsigned int i = 0; i < nx; ++i)
        cells.emplace_back(BoundingBox<2>(
                             {Point<2>(i / double(nx), j / double(ny)),
                              Point<2>((i + 1) / double(nx),
                                       (j + 1) / double(ny))}),
                           j * nx + i);

    const auto tree = pack_rtree<bgi::rstar<4, 2>>(cells);
    log_hierarchy(RTreeHierarchy(tree));
    r3mglog.pop();
  }
}
