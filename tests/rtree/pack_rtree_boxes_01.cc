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

// Build R*-trees of (bounding box, cell index) pairs with pack_rtree() in one,
// two, and three dimensions (the setting of the cell-based agglomeration), for
// the cells of a structured grid, and check that intersection queries with a
// box and with a point return the right cell indices.

#include <r3mgc/base/bounding_box.h>
#include <r3mgc/base/point.h>
#include <r3mgc/base/types.h>

#include <r3mgc/rtree/rtree.h>

#include <algorithm>
#include <vector>

#include "../tests.h"

using namespace r3mgc;
namespace bgi = boost::geometry::index;



// The cells of a grid of n^dim equal boxes covering [0,1]^dim, numbered
// lexicographically with the first coordinate running fastest.
template <int dim>
std::vector<BoxAndIndex<dim>>
grid_cells(const unsigned int n)
{
  unsigned int n_cells = 1;
  for (unsigned int d = 0; d < dim; ++d)
    n_cells *= n;

  const double h = 1. / n;

  std::vector<BoxAndIndex<dim>> cells(n_cells);
  for (unsigned int c = 0; c < n_cells; ++c)
    {
      Point<dim>   p_min, p_max;
      unsigned int index = c;
      for (unsigned int d = 0; d < dim; ++d)
        {
          p_min[d] = (index % n) * h;
          p_max[d] = p_min[d] + h;
          index /= n;
        }
      cells[c] = {BoundingBox<dim>({p_min, p_max}), c};
    }
  return cells;
}



// Return true if the two closed boxes have at least one point in common.
template <int dim>
bool
overlap(const BoundingBox<dim> &a, const BoundingBox<dim> &b)
{
  for (unsigned int d = 0; d < dim; ++d)
    if (a.upper_bound(d) < b.lower_bound(d) ||
        b.upper_bound(d) < a.lower_bound(d))
      return false;
  return true;
}



template <int dim, typename Query>
std::vector<types::global_cell_index>
sorted_query_result(const RTree<BoxAndIndex<dim>, bgi::rstar<4, 2>> &tree,
                    const Query                                     &query)
{
  std::vector<types::global_cell_index> ids;
  for (const auto &[box, id] : tree | bgi::adaptors::queried(query))
    {
      (void)box;
      ids.push_back(id);
    }
  std::sort(ids.begin(), ids.end());
  return ids;
}



template <int dim>
void
test()
{
  r3mglog.push(std::to_string(dim) + "d");

  const unsigned int                  n     = 8;
  const std::vector<BoxAndIndex<dim>> cells = grid_cells<dim>(n);

  const auto tree = pack_rtree<bgi::rstar<4, 2>>(cells);
  r3mglog << "size = " << tree.size() << std::endl;
  r3mglog << "n_levels = " << n_levels(tree) << std::endl;

  // The cells intersecting a box strictly inside the domain, compared with a
  // brute force search. Since boxes are closed, the cells touching the query
  // box are also found.
  Point<dim> q_min, q_max;
  for (unsigned int d = 0; d < dim; ++d)
    {
      q_min[d] = 0.3;
      q_max[d] = 0.5;
    }
  const BoundingBox<dim> query_box({q_min, q_max});

  const auto found = sorted_query_result(tree, bgi::intersects(query_box));

  std::vector<types::global_cell_index> expected;
  for (const auto &[box, id] : cells)
    if (overlap(box, query_box))
      expected.push_back(id);

  r3mglog << "cells intersecting " << query_box << ":";
  for (const auto id : found)
    r3mglog << ' ' << id;
  r3mglog << std::endl;
  r3mglog << "same as brute force: " << (found == expected) << std::endl;

  // The cells containing a point inside a cell, and a vertex of the grid.
  Point<dim> inner_point, vertex;
  for (unsigned int d = 0; d < dim; ++d)
    {
      inner_point[d] = 0.6;
      vertex[d]      = 0.5;
    }
  for (const auto &p : {inner_point, vertex})
    {
      r3mglog << "cells containing " << p << ":";
      for (const auto id : sorted_query_result(tree, bgi::intersects(p)))
        r3mglog << ' ' << id;
      r3mglog << std::endl;
    }

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
