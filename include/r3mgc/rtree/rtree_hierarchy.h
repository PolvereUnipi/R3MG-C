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

#ifndef r3mgc_rtree_rtree_hierarchy_h
#define r3mgc_rtree_rtree_hierarchy_h

#include <r3mgc/base/config.h>

#include <r3mgc/base/array_view.h>
#include <r3mgc/base/bounding_box.h>
#include <r3mgc/base/exceptions.h>
#include <r3mgc/base/types.h>

#include <r3mgc/rtree/rtree.h>

#include <boost/geometry/algorithms/convert.hpp>

#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

R3MGC_NAMESPACE_OPEN

namespace internal
{
  namespace RTreeHierarchyImplementation
  {
    /**
     * The default way of obtaining the id of a leaf of an R-tree, i.e., the
     * integer that identifies the fine entity stored in the leaf:
     * - if the leaf is an integer (a tree built with pack_rtree_of_indices(),
     *   as in the point-based agglomeration), the id is the leaf itself;
     * - if the leaf is a pair (e.g., a BoxAndIndex, as in the cell-based
     *   agglomeration), the id is its second entry.
     */
    struct DefaultIdExtractor
    {
      template <typename LeafType>
      types::global_cell_index
      operator()(const LeafType &leaf) const
      {
        if constexpr (std::is_integral_v<LeafType>)
          return static_cast<types::global_cell_index>(leaf);
        else
          return static_cast<types::global_cell_index>(leaf.second);
      }
    };
  } // namespace RTreeHierarchyImplementation
} // namespace internal



/**
 * The complete hierarchy of nested agglomerates described by an R-tree.
 *
 * Every node of an R-tree bounds a group of fine entities, i.e., the leaves
 * of the tree that are below it: the support points of the fine DoFs in the
 * point-based agglomeration, the fine cells in the cell-based one. Grouping
 * the leaves under all the nodes of one level of the tree gives a partition
 * of the fine entities into agglomerates, and the partitions of consecutive
 * levels are nested: every agglomerate of a level is the union of the
 * agglomerates of its children. This is the sequence of nested agglomerated
 * grids on which the multilevel hierarchy is built.
 *
 * This class traverses the tree once, and stores for every level:
 * - the agglomerates, as lists of leaf ids. The id of a leaf is the integer
 *   that identifies the fine entity it stores, i.e., an index into the arrays
 *   that the user passed to describe the fine discretization (the index of a
 *   support point, or of a cell). It is obtained from the leaf by an
 *   @p IdExtractor, see internal::RTreeHierarchyImplementation::DefaultIdExtractor;
 * - the parent of every node, and the children of every node;
 * - the bounding box of every node, as stored in the tree (i.e., the minimum
 *   bounding rectangle of the leaves in the agglomerate).
 *
 * <h3>Levels and nodes</h3>
 *
 * Levels are counted from the root, which is the only node on level 0: its
 * agglomerate contains all the leaves. The last level, n_levels()-1, is the
 * one of the leaf nodes of the tree, whose agglomerates are the contents of
 * the single leaf nodes. Hence n_levels() is equal to n_levels(tree)+1. This
 * is the convention used by the agglomeration of polyDEAL, but it differs by
 * one from the one of extract_rtree_level(), whose level 0 is made of the
 * children of the root.
 *
 * The nodes of each level are numbered from zero in the order in which a
 * depth-first traversal of the tree visits them. The leaf ids inside each
 * agglomerate are in the same order, i.e., the one in which they are stored
 * in the tree.
 *
 * <h3>Data structure</h3>
 *
 * The depth-first numbering has a useful property: all the nodes and leaves
 * below a given node are visited consecutively. As a consequence
 * - all the agglomerates of all the levels are contiguous slices of a single
 *   array storing all the leaf ids in depth-first order;
 * - the children of a node are a contiguous range of node indices on the
 *   next level.
 *
 * So we never need to store lists: for every level, an array of offsets
 * (similar to the row pointers of a matrix in CSR format) tells where each
 * slice starts and ends. Consider, for example, a tree whose root has two
 * children A and B, each with two leaf nodes, storing the ids 0,...,8:
 * @code
 *                  root                      level 0
 *            /             \
 *           A               B                level 1
 *        /     \         /     \
 *    {4,7,1}  {0,2}   {5,3}   {6,8}          level 2 (leaf nodes)
 *
 * leaf ids (depth-first) = [4 7 1 0 2 5 3 6 8]
 *
 * agglomerate offsets  level 0: [0 9]          -> root = leaf_ids[0,9)
 *                      level 1: [0 5 9]        -> A = [4 7 1 0 2],  B = [5 3 6
 * 8] level 2: [0 3 5 7 9]    -> {4 7 1}, {0 2}, {5 3}, {6 8}
 *
 * children offsets     level 0: [0 2]          -> root: nodes [0,2) of level 1
 *                      level 1: [0 2 4]        -> A: nodes [0,2) of level 2,
 *                                                 B: nodes [2,4) of level 2
 *
 * parents              level 1: [0 0]          -> A and B are children of the
 * root level 2: [0 0 1 1]      -> the first two of A, the others of B
 * @endcode
 * that is, agglomerate(l, k) is the slice of the leaf ids between
 * agglomerate_offsets[l][k] and agglomerate_offsets[l][k+1], and the
 * children of node k on level l are the nodes of level l+1 with indices in
 * [children_offsets[l][k], children_offsets[l][k+1]).
 *
 * Compared with the map from (node, level) to the list of children used in
 * polyDEAL, this layout needs no memory allocation per node, gives constant
 * time access to children and agglomerates, never copies the agglomerates,
 * and describes all the levels with a single traversal of the tree. It also
 * makes evident that every child has exactly one parent, and that every leaf
 * belongs to exactly one agglomerate per level: the property on which the
 * construction of the transfer operators relies.
 *
 * <h3>Usage</h3>
 *
 * @code
 * namespace bgi = boost::geometry::index;
 *
 * // Point-based agglomeration: the ids are the indices of the points.
 * const auto           tree = pack_rtree_of_indices<bgi::rstar<8, 4>>(points);
 * const RTreeHierarchy hierarchy(tree);
 *
 * for (unsigned int l = 0; l < hierarchy.n_levels(); ++l)
 *   for (types::global_cell_index k = 0; k < hierarchy.n_nodes(l); ++k)
 *     for (const auto i : hierarchy.agglomerate(l, k))
 *       {
 *         // points[i] belongs to the k-th agglomerate of level l, whose
 *         // bounding box is hierarchy.bounding_box(l, k).
 *       }
 * @endcode
 *
 * The object does not keep a reference to the tree, which can be destroyed
 * after the construction of the hierarchy.
 *
 * @tparam RTreeType The type of the R-tree, e.g., the one returned by
 * pack_rtree() or pack_rtree_of_indices().
 * @tparam IdExtractor A function object returning the id (convertible to
 * types::global_cell_index) of a leaf of the tree. The default one handles
 * the trees used by the library, see
 * internal::RTreeHierarchyImplementation::DefaultIdExtractor.
 */
template <typename RTreeType,
          typename IdExtractor =
            internal::RTreeHierarchyImplementation::DefaultIdExtractor>
class RTreeHierarchy
{
public:
  /**
   * The space dimension of the geometries stored in the tree.
   */
  static constexpr unsigned int dim =
    boost::geometry::dimension<typename RTreeType::indexable_type>::value;

  /**
   * Constructor. Traverse @p tree, and store its hierarchy. The tree must not
   * be empty.
   */
  explicit RTreeHierarchy(const RTreeType  &tree,
                          const IdExtractor id_extractor = IdExtractor());

  /**
   * Return the number of levels, including the one of the root. This is
   * n_levels(tree)+1.
   */
  unsigned int
  n_levels() const;

  /**
   * Return the number of nodes (i.e., of agglomerates) on level @p level.
   */
  types::global_cell_index
  n_nodes(const unsigned int level) const;

  /**
   * Return the number of leaves of the tree, i.e., of fine entities.
   */
  types::global_cell_index
  n_leaves() const;

  /**
   * Return the ids of all the leaves, in depth-first order. This is the
   * agglomerate of the root.
   */
  ArrayView<const types::global_cell_index>
  leaf_ids() const;

  /**
   * Return the ids of the leaves in the agglomerate of node @p node on level
   * @p level.
   */
  ArrayView<const types::global_cell_index>
  agglomerate(const unsigned int             level,
              const types::global_cell_index node) const;

  /**
   * Return all the agglomerates of level @p level, as a vector of vectors of
   * leaf ids. This function copies the data: prefer agglomerate() in loops.
   */
  std::vector<std::vector<types::global_cell_index>>
  agglomerates(const unsigned int level) const;

  /**
   * Return the number of children of node @p node on level @p level. Nodes
   * on the last level have no children.
   */
  types::global_cell_index
  n_children(const unsigned int             level,
             const types::global_cell_index node) const;

  /**
   * Return the half-open range [first, last) of the indices, on level
   * @p level+1, of the children of node @p node on level @p level. The range
   * is empty for the nodes of the last level.
   */
  std::pair<types::global_cell_index, types::global_cell_index>
  children(const unsigned int level, const types::global_cell_index node) const;

  /**
   * Return the index, on level @p level-1, of the parent of node @p node on
   * level @p level. The root (the only node on level 0) has no parent, and
   * numbers::invalid_cell_index is returned for it.
   */
  types::global_cell_index
  parent(const unsigned int level, const types::global_cell_index node) const;

  /**
   * Return the bounding box of node @p node on level @p level, as stored in
   * the tree. It is the smallest box containing the geometries of all the
   * leaves in the agglomerate of the node.
   */
  const BoundingBox<dim> &
  bounding_box(const unsigned int             level,
               const types::global_cell_index node) const;

private:
  /**
   * The ids of all the leaves, in depth-first order.
   */
  std::vector<types::global_cell_index> all_leaf_ids;

  /**
   * For every level, an array with n_nodes(level)+1 entries: the agglomerate
   * of node k is the slice [agglomerate_offsets[level][k],
   * agglomerate_offsets[level][k+1]) of all_leaf_ids.
   */
  std::vector<std::vector<types::global_cell_index>> agglomerate_offsets;

  /**
   * For every level but the last one, an array with n_nodes(level)+1
   * entries: the children of node k are the nodes of the next level with
   * indices in [children_offsets[level][k], children_offsets[level][k+1]).
   */
  std::vector<std::vector<types::global_cell_index>> children_offsets;

  /**
   * For every level, the index of the parent (on the previous level) of every
   * node. The only entry of level 0 is numbers::invalid_cell_index.
   */
  std::vector<std::vector<types::global_cell_index>> parents;

  /**
   * For every level, the bounding box of every node.
   */
  std::vector<std::vector<BoundingBox<dim>>> boxes;
};



/* ------------------- Inline and template functions ----------------------- */

template <typename RTreeType, typename IdExtractor>
RTreeHierarchy<RTreeType, IdExtractor>::RTreeHierarchy(
  const RTreeType  &tree,
  const IdExtractor id_extractor)
{
  R3MGC_AssertThrow(tree.size() > 0,
                    ExcMessage("The hierarchy of an empty R-tree is not "
                               "defined."));

  const unsigned int n_levels_total = ::r3mgc::n_levels(tree) + 1;

  agglomerate_offsets.resize(n_levels_total);
  children_offsets.resize(n_levels_total - 1);
  parents.resize(n_levels_total);
  boxes.resize(n_levels_total);
  all_leaf_ids.reserve(tree.size());

  // The root is not visited by the callbacks of visit_rtree(), which start
  // from its children: register it by hand. Its agglomerate starts at the
  // first leaf.
  BoundingBox<dim> root_box;
  boost::geometry::convert(tree.bounds(), root_box);
  boxes[0].push_back(root_box);
  parents[0].push_back(numbers::invalid_cell_index);
  agglomerate_offsets[0].push_back(0);

  // Every node is registered when the traversal reaches it, before its
  // subtree is visited (pre-order). At that moment:
  // - its parent is the last node registered on the previous level, since
  //   the traversal is depth-first;
  // - its agglomerate starts at the current number of leaf ids, since the
  //   leaves of its subtree are the next ones to be stored.
  // visit_rtree() counts levels from the children of the root, hence the +1.
  const auto register_node = [&](const std::size_t &visitor_level,
                                 const auto & /*parent_node*/,
                                 const BoundingBox<dim> &box,
                                 const auto & /*node*/) {
    const unsigned int level = visitor_level + 1;
    boxes[level].push_back(box);
    parents[level].push_back(parents[level - 1].size() - 1);
    agglomerate_offsets[level].push_back(all_leaf_ids.size());
  };

  const auto store_leaf_id = [&](const auto & /*leaf_node*/,
                                 const auto &value) {
    all_leaf_ids.push_back(id_extractor(value));
  };

  visit_rtree(tree, register_node, register_node, store_leaf_id);

  // Close the offsets: the last agglomerate of every level ends at the last
  // leaf.
  for (auto &offsets : agglomerate_offsets)
    offsets.push_back(all_leaf_ids.size());

  // The children of a node are contiguous on the next level, and the parents
  // are non-decreasing: the first child of node k is the first node of the
  // next level whose parent is not smaller than k.
  for (unsigned int level = 0; level + 1 < n_levels_total; ++level)
    {
      const auto &next_parents = parents[level + 1];
      auto       &offsets      = children_offsets[level];
      offsets.assign(parents[level].size() + 1, 0);

      for (const auto p : next_parents)
        ++offsets[p + 1];
      for (std::size_t k = 0; k + 1 < offsets.size(); ++k)
        offsets[k + 1] += offsets[k];
    }

  R3MGC_Assert(all_leaf_ids.size() == tree.size(), ExcInternalError());
}



template <typename RTreeType, typename IdExtractor>
inline unsigned int
RTreeHierarchy<RTreeType, IdExtractor>::n_levels() const
{
  return agglomerate_offsets.size();
}



template <typename RTreeType, typename IdExtractor>
inline types::global_cell_index
RTreeHierarchy<RTreeType, IdExtractor>::n_nodes(const unsigned int level) const
{
  R3MGC_AssertIndexRange(level, n_levels());
  return parents[level].size();
}



template <typename RTreeType, typename IdExtractor>
inline types::global_cell_index
RTreeHierarchy<RTreeType, IdExtractor>::n_leaves() const
{
  return all_leaf_ids.size();
}



template <typename RTreeType, typename IdExtractor>
inline ArrayView<const types::global_cell_index>
RTreeHierarchy<RTreeType, IdExtractor>::leaf_ids() const
{
  return ArrayView<const types::global_cell_index>(all_leaf_ids);
}



template <typename RTreeType, typename IdExtractor>
inline ArrayView<const types::global_cell_index>
RTreeHierarchy<RTreeType, IdExtractor>::agglomerate(
  const unsigned int             level,
  const types::global_cell_index node) const
{
  R3MGC_AssertIndexRange(level, n_levels());
  R3MGC_AssertIndexRange(node, n_nodes(level));

  const types::global_cell_index begin = agglomerate_offsets[level][node];
  const types::global_cell_index end   = agglomerate_offsets[level][node + 1];
  return ArrayView<const types::global_cell_index>(all_leaf_ids.data() + begin,
                                                   end - begin);
}



template <typename RTreeType, typename IdExtractor>
inline std::vector<std::vector<types::global_cell_index>>
RTreeHierarchy<RTreeType, IdExtractor>::agglomerates(
  const unsigned int level) const
{
  std::vector<std::vector<types::global_cell_index>> result(n_nodes(level));
  for (types::global_cell_index k = 0; k < n_nodes(level); ++k)
    {
      const auto view = agglomerate(level, k);
      result[k].assign(view.begin(), view.end());
    }
  return result;
}



template <typename RTreeType, typename IdExtractor>
inline types::global_cell_index
RTreeHierarchy<RTreeType, IdExtractor>::n_children(
  const unsigned int             level,
  const types::global_cell_index node) const
{
  const auto range = children(level, node);
  return range.second - range.first;
}



template <typename RTreeType, typename IdExtractor>
inline std::pair<types::global_cell_index, types::global_cell_index>
RTreeHierarchy<RTreeType, IdExtractor>::children(
  const unsigned int             level,
  const types::global_cell_index node) const
{
  R3MGC_AssertIndexRange(level, n_levels());
  R3MGC_AssertIndexRange(node, n_nodes(level));

  if (level + 1 == n_levels())
    return {0, 0};
  return {children_offsets[level][node], children_offsets[level][node + 1]};
}



template <typename RTreeType, typename IdExtractor>
inline types::global_cell_index
RTreeHierarchy<RTreeType, IdExtractor>::parent(
  const unsigned int             level,
  const types::global_cell_index node) const
{
  R3MGC_AssertIndexRange(level, n_levels());
  R3MGC_AssertIndexRange(node, n_nodes(level));
  return parents[level][node];
}



template <typename RTreeType, typename IdExtractor>
inline const BoundingBox<RTreeHierarchy<RTreeType, IdExtractor>::dim> &
RTreeHierarchy<RTreeType, IdExtractor>::bounding_box(
  const unsigned int             level,
  const types::global_cell_index node) const
{
  R3MGC_AssertIndexRange(level, n_levels());
  R3MGC_AssertIndexRange(node, n_nodes(level));
  return boxes[level][node];
}

R3MGC_NAMESPACE_CLOSE

#endif
