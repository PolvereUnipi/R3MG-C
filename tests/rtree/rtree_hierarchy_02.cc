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

// Test RTreeHierarchy on trees of (box, cell index) pairs, the setting of the
// cell-based agglomeration:
// - the cells of an 8x8 grid: print the agglomerates of the first levels,
//   check that the box of every node is the union of the boxes of its cells;
// - a tree whose root is a leaf (fewer cells than the capacity of a node):
//   the hierarchy has a single level, with one agglomerate with all cells;
// - a custom id extractor;
// - the hierarchy of an empty tree is not defined.

#include <r3mgc/base/bounding_box.h>
#include <r3mgc/base/point.h>
#include <r3mgc/base/types.h>

#include <r3mgc/rtree/rtree.h>
#include <r3mgc/rtree/rtree_hierarchy.h>

#include <vector>

#include "../tests.h"

using namespace r3mgc;
namespace bgi = boost::geometry::index;



// The cells of a grid of n x n equal squares covering [0,1]^2, numbered
// lexicographically with the x coordinate running fastest.
std::vector<BoxAndIndex<2>>
grid_cells(const unsigned int n)
{
  std::vector<BoxAndIndex<2>> cells;
  for (unsigned int j = 0; j < n; ++j)
    for (unsigned int i = 0; i < n; ++i)
      cells.emplace_back(BoundingBox<2>({Point<2>(i / double(n), j / double(n)),
                                         Point<2>((i + 1) / double(n),
                                                  (j + 1) / double(n))}),
                         j * n + i);
  return cells;
}



template <typename HierarchyType>
void
log_level(const HierarchyType &hierarchy, const unsigned int level)
{
  r3mglog << "level " << level << ": " << hierarchy.n_nodes(level)
          << " agglomerates" << std::endl;
  for (types::global_cell_index k = 0; k < hierarchy.n_nodes(level); ++k)
    {
      r3mglog << "  " << k << " [box " << hierarchy.bounding_box(level, k)
              << "]:";
      for (const auto id : hierarchy.agglomerate(level, k))
        r3mglog << ' ' << id;
      r3mglog << std::endl;
    }
}



// An id extractor returning the cell index with an offset, as if the cells
// were numbered from 100.
struct ShiftedIdExtractor
{
  types::global_cell_index
  operator()(const BoxAndIndex<2> &leaf) const
  {
    return leaf.second + 100;
  }
};



int
main()
{
  initlog();

  {
    r3mglog.push("grid");
    const std::vector<BoxAndIndex<2>> cells = grid_cells(8);
    const auto           tree = pack_rtree<bgi::rstar<4, 2>>(cells);
    const RTreeHierarchy hierarchy(tree);

    r3mglog << "n_levels = " << hierarchy.n_levels() << std::endl;
    for (unsigned int level = 0; level < 3; ++level)
      log_level(hierarchy, level);

    // The box of every node is the union of the boxes of its cells.
    bool boxes_ok = true;
    for (unsigned int level = 0; level < hierarchy.n_levels(); ++level)
      for (types::global_cell_index k = 0; k < hierarchy.n_nodes(level); ++k)
        {
          const auto     agglomerate = hierarchy.agglomerate(level, k);
          BoundingBox<2> union_box   = cells[agglomerate[0]].first;
          for (const auto id : agglomerate)
            union_box.merge_with(cells[id].first);
          boxes_ok =
            boxes_ok && (union_box == hierarchy.bounding_box(level, k));
        }
    r3mglog << "boxes are the unions of the cell boxes: " << boxes_ok
            << std::endl;

    // The agglomerates() convenience function returns the same data.
    const auto copies = hierarchy.agglomerates(2);
    bool       same   = (copies.size() == hierarchy.n_nodes(2));
    for (types::global_cell_index k = 0; k < copies.size(); ++k)
      same = same && (ArrayView<const types::global_cell_index>(copies[k]) ==
                      hierarchy.agglomerate(2, k));
    r3mglog << "agglomerates() equal agglomerate(): " << same << std::endl;
    r3mglog.pop();
  }

  {
    r3mglog.push("small");
    const std::vector<BoxAndIndex<2>> cells = grid_cells(2);
    const auto           tree = pack_rtree<bgi::rstar<4, 2>>(cells);
    const RTreeHierarchy hierarchy(tree);

    r3mglog << "n_levels(tree) = " << n_levels(tree)
            << ", hierarchy.n_levels() = " << hierarchy.n_levels() << std::endl;
    log_level(hierarchy, 0);
    r3mglog << "root has children: " << hierarchy.n_children(0, 0) << std::endl;
    r3mglog.pop();
  }

  {
    r3mglog.push("extractor");
    const std::vector<BoxAndIndex<2>> cells = grid_cells(2);
    const auto           tree = pack_rtree<bgi::rstar<4, 2>>(cells);
    const RTreeHierarchy hierarchy(tree, ShiftedIdExtractor());
    log_level(hierarchy, 0);
    r3mglog.pop();
  }

  {
    r3mglog.push("empty");
    const std::vector<BoxAndIndex<2>> no_cells;
    const auto tree = pack_rtree<bgi::rstar<4, 2>>(no_cells);
    check_throw(RTreeHierarchy{tree});
    r3mglog.pop();
  }
}
