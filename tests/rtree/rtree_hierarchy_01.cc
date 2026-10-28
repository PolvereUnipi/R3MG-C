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

// Check the invariants of the RTreeHierarchy of trees of point indices (the
// setting of the point-based agglomeration), for random points in one, two,
// and three dimensions, with numbers of points that do and do not fill the
// nodes evenly:
// - on every level, every leaf id appears in exactly one agglomerate;
// - the agglomerate of a node is the concatenation, in order, of the
//   agglomerates of its children, and the parent of each child is the node;
// - the bounding box of a node is the smallest box containing the points of
//   its agglomerate;
// - the number of levels is n_levels(tree)+1, and the root contains all the
//   leaves.

#include <r3mgc/base/bounding_box.h>
#include <r3mgc/base/point.h>
#include <r3mgc/base/types.h>

#include <r3mgc/rtree/rtree.h>
#include <r3mgc/rtree/rtree_hierarchy.h>

#include <algorithm>
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



template <int dim>
std::vector<Point<dim>>
random_points(const unsigned int n_points)
{
  std::mt19937_64         generator(42);
  std::vector<Point<dim>> points(n_points);
  for (auto &p : points)
    for (unsigned int d = 0; d < dim; ++d)
      p[d] = random_value(generator);
  return points;
}



template <int dim, typename HierarchyType>
void
check_hierarchy(const HierarchyType           &hierarchy,
                const std::vector<Point<dim>> &points)
{
  const types::global_cell_index n_leaves = hierarchy.n_leaves();

  bool partition_ok = true;
  bool children_ok  = true;
  bool boxes_ok     = true;

  for (unsigned int level = 0; level < hierarchy.n_levels(); ++level)
    {
      // Every leaf in exactly one agglomerate.
      std::vector<unsigned int> n_occurrences(n_leaves, 0);
      for (types::global_cell_index k = 0; k < hierarchy.n_nodes(level); ++k)
        for (const auto id : hierarchy.agglomerate(level, k))
          ++n_occurrences[id];
      partition_ok = partition_ok &&
                     std::all_of(n_occurrences.begin(),
                                 n_occurrences.end(),
                                 [](const unsigned int n) { return n == 1; });

      for (types::global_cell_index k = 0; k < hierarchy.n_nodes(level); ++k)
        {
          const auto agglomerate = hierarchy.agglomerate(level, k);

          // The children partition the agglomerate, in order.
          if (level + 1 < hierarchy.n_levels())
            {
              const auto [first, last] = hierarchy.children(level, k);
              std::vector<types::global_cell_index> concatenation;
              for (auto c = first; c < last; ++c)
                {
                  children_ok =
                    children_ok && (hierarchy.parent(level + 1, c) == k);
                  const auto child = hierarchy.agglomerate(level + 1, c);
                  concatenation.insert(concatenation.end(),
                                       child.begin(),
                                       child.end());
                }
              children_ok = children_ok && (first < last) &&
                            std::equal(concatenation.begin(),
                                       concatenation.end(),
                                       agglomerate.begin(),
                                       agglomerate.end());
            }
          else
            children_ok = children_ok && (hierarchy.n_children(level, k) == 0);

          // The box of the node is the box of its points.
          std::vector<Point<dim>> agglomerate_points;
          for (const auto id : agglomerate)
            agglomerate_points.push_back(points[id]);
          boxes_ok = boxes_ok && (hierarchy.bounding_box(level, k) ==
                                  BoundingBox<dim>(agglomerate_points));
        }
    }

  r3mglog << "every leaf in one agglomerate per level: " << partition_ok
          << std::endl;
  r3mglog << "children partition their parent: " << children_ok << std::endl;
  r3mglog << "boxes are the boxes of the points: " << boxes_ok << std::endl;
}



template <int dim>
void
test(const unsigned int n_points)
{
  r3mglog.push(std::to_string(dim) + "d-" + std::to_string(n_points));

  const std::vector<Point<dim>> points = random_points<dim>(n_points);
  const auto           tree = pack_rtree_of_indices<bgi::rstar<8, 4>>(points);
  const RTreeHierarchy hierarchy(tree);

  r3mglog << "n_levels(tree) = " << n_levels(tree)
          << ", hierarchy.n_levels() = " << hierarchy.n_levels() << std::endl;
  for (unsigned int level = 0; level < hierarchy.n_levels(); ++level)
    r3mglog << "level " << level << ": " << hierarchy.n_nodes(level) << " nodes"
            << std::endl;
  r3mglog << "root contains all the leaves: "
          << (hierarchy.agglomerate(0, 0).size() == n_points &&
              hierarchy.leaf_ids() == hierarchy.agglomerate(0, 0))
          << std::endl;
  r3mglog << "parent of the root is invalid: "
          << (hierarchy.parent(0, 0) == numbers::invalid_cell_index)
          << std::endl;

  check_hierarchy(hierarchy, points);

  r3mglog.pop();
}



int
main()
{
  initlog();

  for (const unsigned int n_points : {37u, 200u, 1000u})
    {
      test<1>(n_points);
      test<2>(n_points);
      test<3>(n_points);
    }
}
