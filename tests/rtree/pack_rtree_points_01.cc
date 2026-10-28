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

// Build R*-trees of point indices with pack_rtree_of_indices() in one, two,
// and three dimensions (the setting of the point-based agglomeration), and
// check the results of spatial queries against a brute force search. Also
// check that a tree of copies of the points, built with pack_rtree(), answers
// the same way.

#include <r3mgc/base/bounding_box.h>
#include <r3mgc/base/point.h>

#include <r3mgc/rtree/rtree.h>

#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>

#include "../tests.h"

using namespace r3mgc;
namespace bgi = boost::geometry::index;



// Return a uniformly distributed number in [0,1). We do not use
// std::uniform_real_distribution, whose output is not the same for all the
// implementations of the standard library, but the raw output of
// std::mt19937_64, which is fully specified by the standard.
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



template <int dim>
void
test()
{
  r3mglog.push(std::to_string(dim) + "d");

  const std::vector<Point<dim>> points = random_points<dim>(200);

  const auto tree = pack_rtree_of_indices<bgi::rstar<8, 4>>(points);
  r3mglog << "size = " << tree.size() << std::endl;
  r3mglog << "n_levels = " << n_levels(tree) << std::endl;

  // All the points inside a box, compared with a brute force search.
  Point<dim> q_min, q_max;
  for (unsigned int d = 0; d < dim; ++d)
    {
      q_min[d] = 0.2;
      q_max[d] = 0.7;
    }
  const BoundingBox<dim> query_box({q_min, q_max});

  std::vector<std::size_t> found;
  for (const auto i : tree | bgi::adaptors::queried(bgi::intersects(query_box)))
    found.push_back(i);
  std::sort(found.begin(), found.end());

  std::vector<std::size_t> expected;
  for (std::size_t i = 0; i < points.size(); ++i)
    if (query_box.point_inside(points[i], 0.))
      expected.push_back(i);

  r3mglog << "points in " << query_box << ": " << found.size() << std::endl;
  r3mglog << "same as brute force: " << (found == expected) << std::endl;

  // The 5 nearest neighbors of the center of the box.
  const Point<dim>         center = query_box.center();
  std::vector<std::size_t> nearest;
  for (const auto i : tree | bgi::adaptors::queried(bgi::nearest(center, 5)))
    nearest.push_back(i);
  std::sort(nearest.begin(), nearest.end(), [&](const auto a, const auto b) {
    return points[a].distance(center) < points[b].distance(center);
  });

  std::vector<std::size_t> all_indices(points.size());
  for (std::size_t i = 0; i < points.size(); ++i)
    all_indices[i] = i;
  std::sort(all_indices.begin(),
            all_indices.end(),
            [&](const auto a, const auto b) {
              return points[a].distance(center) < points[b].distance(center);
            });
  all_indices.resize(5);

  r3mglog << "nearest to " << center << ":";
  for (const auto i : nearest)
    r3mglog << ' ' << i;
  r3mglog << std::endl;
  r3mglog << "same as brute force: " << (nearest == all_indices) << std::endl;

  // A tree storing copies of the points gives the same answer.
  const auto   copy_tree = pack_rtree<bgi::rstar<8, 4>>(points);
  unsigned int n_found   = 0;
  for (const auto &p :
       copy_tree | bgi::adaptors::queried(bgi::intersects(query_box)))
    {
      (void)p;
      ++n_found;
    }
  r3mglog << "points in box (tree of copies): " << n_found << std::endl;

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
