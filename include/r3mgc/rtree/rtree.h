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

#ifndef r3mgc_rtree_rtree_h
#define r3mgc_rtree_rtree_h

#include <r3mgc/base/config.h>

#include <r3mgc/base/bounding_box.h>
#include <r3mgc/base/point.h>
#include <r3mgc/base/types.h>

#include <r3mgc/boost_adaptors/bounding_box.h>
#include <r3mgc/boost_adaptors/point.h>

#include <boost/geometry/algorithms/comparable_distance.hpp>
#include <boost/geometry/algorithms/convert.hpp>
#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/index/detail/rtree/utilities/view.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/strategies/strategies.hpp>

#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

/**
 * @file
 *
 * Thin wrappers around the R-tree of Boost.Geometry, ported from deal.II's
 * numerics/rtree.h.
 *
 * The R-tree is the data structure that drives the whole agglomeration: its
 * leaves are the fine entities (support points, or bounding boxes of fine
 * cells), and every internal node groups its children into an agglomerate
 * whose minimum bounding rectangle is stored in the node. The library uses
 * the tree in two ways:
 *
 * - <b>Point-based agglomeration.</b> The leaves are the indices of the
 *   support points in the vector provided by the user. The tree is built
 *   with pack_rtree_of_indices(), so that the points are not copied and an
 *   agglomerate can directly be described by the list of indices it
 *   contains:
 *   @code
 *   namespace bgi = boost::geometry::index;
 *   std::vector<Point<dim>> support_points = ...;
 *   const auto tree = pack_rtree_of_indices<bgi::rstar<8, 4>>(support_points);
 *   @endcode
 *
 * - <b>Cell-based agglomeration.</b> The leaves are pairs (bounding box of a
 *   fine cell, index of the cell), built with pack_rtree():
 *   @code
 *   std::vector<BoxAndIndex<dim>> boxes(n_cells);
 *   for (types::global_cell_index c = 0; c < n_cells; ++c)
 *     boxes[c] = {cell_bounding_box(c), c};
 *   const auto tree = pack_rtree<bgi::rstar<8, 4>>(boxes);
 *   @endcode
 *
 * In both cases we recommend (and the agglomeration functions of the library
 * use) the R*-tree variant bgi::rstar<M, m>, where M and m are the maximum and
 * minimum number of children of a node. Packing (i.e., building the tree from
 * a range in one go, instead of inserting one element at a time) is
 * deterministic: the same input in the same order always produces the same
 * tree, and therefore the same agglomerates.
 *
 * On top of the construction functions, this file provides a depth-first
 * visitor driven by user-provided lambdas (visit_rtree()), the number of
 * levels of a tree (n_levels()), and the extraction of the bounding boxes of
 * a given level (extract_rtree_level()).
 *
 * @warning As in deal.II, the leaves of the trees built here are values (or
 * indices), and their bounding boxes are the ones of the input. The function
 * pack_rtree_of_indices() stores a reference to the input container: the
 * container must outlive the tree, and must not be modified while the tree
 * is in use.
 */

R3MGC_NAMESPACE_OPEN

namespace internal
{
  namespace RTreeImplementation
  {
    /**
     * Sentinel type used by RTreeFunctionalVisitor to indicate that no
     * callback was provided.
     */
    struct EmptyVisitor
    {};
  } // namespace RTreeImplementation
} // namespace internal



/**
 * An alias for the Boost R-tree type.
 *
 * @tparam LeafType The type stored in the leaves: anything for which
 * @p IndexableGetter returns a Point, a BoundingBox, or another
 * Boost.Geometry indexable type. The default getter accepts the indexable
 * types themselves, and pairs or tuples whose first entry is an indexable
 * type (e.g., BoxAndIndex).
 * @tparam IndexType The balancing algorithm and the node capacity, e.g.,
 * boost::geometry::index::rstar<M, m>.
 * @tparam IndexableGetter The functor that extracts the indexable geometry
 * from a leaf.
 */
template <typename LeafType,
          typename IndexType = boost::geometry::index::linear<16>,
          typename IndexableGetter =
            boost::geometry::index::indexable<LeafType>>
using RTree =
  boost::geometry::index::rtree<LeafType, IndexType, IndexableGetter>;

/**
 * An alias for a read-only view of the internal structure of an RTree. Boost
 * does not expose the nodes of the tree in its public interface, and this
 * view is the only (detail) way to traverse them.
 */
template <typename RTreeType>
using RTreeView =
  boost::geometry::index::detail::rtree::utilities::view<RTreeType>;

/**
 * The leaf type used for cell-based agglomeration: the bounding box of a fine
 * cell, together with the index of the cell.
 */
template <int dim, typename Number = double>
using BoxAndIndex =
  std::pair<BoundingBox<dim, Number>, types::global_cell_index>;



/**
 * Build an RTree from the iterator range [@p begin, @p end) by packing.
 *
 * The order of the template parameters is the opposite of the one of the
 * RTree class, since the leaf type can be deduced from the arguments, and only
 * the @p IndexType usually needs to be specified.
 */
template <typename IndexType = boost::geometry::index::linear<16>,
          typename LeafTypeIterator,
          typename IndexableGetter = boost::geometry::index::indexable<
            typename LeafTypeIterator::value_type>>
RTree<typename LeafTypeIterator::value_type, IndexType, IndexableGetter>
pack_rtree(const LeafTypeIterator &begin, const LeafTypeIterator &end);

/**
 * Build an RTree from all the elements of @p container by packing. The tree
 * stores a copy of the elements.
 */
template <typename IndexType = boost::geometry::index::linear<16>,
          typename ContainerType,
          typename IndexableGetter = boost::geometry::index::indexable<
            typename ContainerType::value_type>>
RTree<typename ContainerType::value_type, IndexType, IndexableGetter>
pack_rtree(const ContainerType &container);



/**
 * An @p IndexableGetter for RTree objects whose leaves are indices into a
 * container of indexable types (or of pairs/tuples whose first entry is an
 * indexable type).
 *
 * For example:
 * @code
 * std::vector<Point<dim>> points = ...;
 * IndexableGetterFromIndices<std::vector<Point<dim>>> getter(points);
 * const Point<dim> &p = getter(i); // returns points[i]
 * @endcode
 *
 * This class is used by pack_rtree_of_indices().
 */
template <typename Container>
class IndexableGetterFromIndices
{
public:
  /**
   * The Boost getter that extracts the indexable geometry from an element of
   * the container.
   */
  using IndexableGetter =
    typename boost::geometry::index::indexable<typename Container::value_type>;

  /**
   * The geometry type returned by the getter.
   */
  using result_type = typename IndexableGetter::result_type;

  /**
   * The index type.
   */
  using size_t = typename Container::size_type;

  /**
   * Constructor. Store a reference to @p c.
   */
  explicit IndexableGetterFromIndices(const Container &c);

  /**
   * Return the geometry of the @p i-th element of the container. This is
   * the interface required by the Boost R-tree.
   */
  result_type
  operator()(size_t i) const;

private:
  /**
   * A reference to the container.
   */
  const Container &container;

  /**
   * The getter that translates elements of the container into geometries.
   */
  IndexableGetter getter;
};



/**
 * Build an RTree whose leaves are the indices of the elements of
 * @p container, rather than copies of the elements. The container only needs
 * to support operator[] for all the indices between 0 and its size (e.g., a
 * std::vector or a std::array).
 *
 * This is the function used by the point-based agglomeration: the leaves of
 * the tree are the indices of the support points, so that each agglomerate
 * can be returned as a list of indices.
 *
 * @code
 * std::vector<Point<dim>> points = ...;
 * const auto tree = pack_rtree_of_indices<bgi::rstar<8, 4>>(points);
 *
 * for (const auto i : tree | bgi::adaptors::queried(bgi::intersects(box)))
 *   std::cout << "Point " << i << ": " << points[i] << std::endl;
 * @endcode
 *
 * @warning The tree stores a reference to @p container, which must outlive
 * the tree and must not be modified while the tree is in use. If the
 * container changes, the tree must be rebuilt.
 */
template <typename IndexType = boost::geometry::index::linear<16>,
          typename ContainerType>
RTree<typename ContainerType::size_type,
      IndexType,
      IndexableGetterFromIndices<ContainerType>>
pack_rtree_of_indices(const ContainerType &container);



/**
 * A depth-first visitor for RTree objects that forwards every traversal event
 * to (optional) user-provided callables.
 *
 * Every RTree has a root node, internal nodes (whose children are other
 * nodes) and leaf nodes (which store the actual values). During the
 * traversal, the following callbacks are invoked:
 * - the internal node visitor, once for every internal node, before
 *   descending into it;
 * - the leaf visitor, once for every leaf node, before visiting its values;
 * - the element visitor, once for every value stored in a leaf;
 * - the indexable visitor, once for every value stored in a leaf, with the
 *   geometry extracted from the value by the indexable getter of the tree.
 *
 * Children are visited immediately after their callback, so that the
 * callbacks are invoked in depth-first order, and the values of every leaf
 * are delivered in the order in which they are stored in it.
 *
 * Levels are counted without the root: the children of the root are on
 * level 0, and the leaves are on level n_levels(tree)-1.
 *
 * This is a port of the class with the same name in deal.II. Users should
 * call visit_rtree() rather than building this class directly.
 */
template <
  typename RTreeType,
  typename InternalVisitor  = internal::RTreeImplementation::EmptyVisitor,
  typename LeafVisitor      = internal::RTreeImplementation::EmptyVisitor,
  typename ElementVisitor   = internal::RTreeImplementation::EmptyVisitor,
  typename IndexableVisitor = internal::RTreeImplementation::EmptyVisitor>
struct RTreeFunctionalVisitor
  : public boost::geometry::index::detail::rtree::visitor<
      typename RTreeView<RTreeType>::value_type,
      typename RTreeView<RTreeType>::options_type::parameters_type,
      typename RTreeView<RTreeType>::box_type,
      typename RTreeView<RTreeType>::allocators_type,
      typename RTreeView<RTreeType>::options_type::node_tag,
      true>::type
{
  /**
   * The space dimension of the geometries stored in the tree.
   */
  static constexpr unsigned int dim =
    boost::geometry::dimension<typename RTreeType::indexable_type>::value;

  /**
   * The internal node type of the tree.
   */
  using internal_node_type =
    typename boost::geometry::index::detail::rtree::internal_node<
      typename RTreeView<RTreeType>::value_type,
      typename RTreeView<RTreeType>::options_type::parameters_type,
      typename RTreeView<RTreeType>::box_type,
      typename RTreeView<RTreeType>::allocators_type,
      typename RTreeView<RTreeType>::options_type::node_tag>::type;

  /**
   * The leaf node type of the tree.
   */
  using leaf_type = typename boost::geometry::index::detail::rtree::leaf<
    typename RTreeView<RTreeType>::value_type,
    typename RTreeView<RTreeType>::options_type::parameters_type,
    typename RTreeView<RTreeType>::box_type,
    typename RTreeView<RTreeType>::allocators_type,
    typename RTreeView<RTreeType>::options_type::node_tag>::type;

  /**
   * The type of the values stored in the leaves.
   */
  using value_type = typename RTreeType::value_type;

  /**
   * The geometry associated to a value by the indexable getter.
   */
  using indexable_type = typename RTreeType::indexable_type;

  /**
   * The translator (indexable getter) of the tree.
   */
  using translator_type = typename RTreeView<RTreeType>::translator_type;

  /**
   * The sentinel type of the callbacks that were not provided.
   */
  using empty_visitor = internal::RTreeImplementation::EmptyVisitor;

  /**
   * Whether each of the four callbacks was provided.
   */
  static constexpr bool has_internal_visitor =
    !std::is_same_v<InternalVisitor, empty_visitor>;
  static constexpr bool has_leaf_visitor =
    !std::is_same_v<LeafVisitor, empty_visitor>;
  static constexpr bool has_element_visitor =
    !std::is_same_v<ElementVisitor, empty_visitor>;
  static constexpr bool has_indexable_visitor =
    !std::is_same_v<IndexableVisitor, empty_visitor>;

  /**
   * Constructor. Store the callbacks and traverse @p tree.
   */
  RTreeFunctionalVisitor(const RTreeType &tree,
                         InternalVisitor  internal_node_visitor = {},
                         LeafVisitor      leaf_visitor          = {},
                         ElementVisitor   element_visitor       = {},
                         IndexableVisitor indexable_visitor     = {});

  /**
   * Visit an internal node: for every child, call the internal node visitor
   * or the leaf visitor, then descend into the child. This is the
   * internal-node part of the Boost visitor interface.
   */
  void
  operator()(internal_node_type &node);

  /**
   * Visit a leaf: call the element and indexable visitors for every value.
   * This is the leaf part of the Boost visitor interface.
   */
  void
  operator()(leaf_type &leaf);

  /**
   * Callback invoked for every internal node, with signature
   * @code
   * void(const std::size_t        &level,
   *      const internal_node_type &parent,
   *      const BoundingBox<dim>   &node_box,
   *      const internal_node_type &node);
   * @endcode
   */
  InternalVisitor internal_node_visitor;

  /**
   * Callback invoked for every leaf node, with signature
   * @code
   * void(const std::size_t        &level,
   *      const internal_node_type &parent,
   *      const BoundingBox<dim>   &leaf_box,
   *      const leaf_type          &leaf);
   * @endcode
   */
  LeafVisitor leaf_visitor;

  /**
   * Callback invoked for every value stored in a leaf, with signature
   * @code
   * void(const leaf_type &leaf, const value_type &value);
   * @endcode
   */
  ElementVisitor element_visitor;

  /**
   * Callback invoked for every value stored in a leaf, with the geometry of
   * the value, with signature
   * @code
   * void(const leaf_type &leaf, const indexable_type &indexable);
   * @endcode
   */
  IndexableVisitor indexable_visitor;

  /**
   * The level of the children of the node currently visited.
   */
  std::size_t current_level;

  /**
   * The translator used to obtain the geometry of the values.
   */
  translator_type translator;
};



/**
 * Traverse @p tree depth-first, calling the given callbacks. See
 * RTreeFunctionalVisitor for the meaning and the signature of the callbacks.
 * Callbacks that are not needed can be left out (only trailing ones), or
 * replaced by internal::RTreeImplementation::EmptyVisitor{}.
 *
 * For example, the following counts the leaves on every level of a tree:
 * @code
 * std::vector<unsigned int> n_nodes(n_levels(tree), 0);
 * visit_rtree(tree,
 *             [&](const std::size_t &level, const auto &, const auto &,
 *                 const auto &) { ++n_nodes[level]; },
 *             [&](const std::size_t &level, const auto &, const auto &,
 *                 const auto &) { ++n_nodes[level]; });
 * @endcode
 */
template <
  typename RTreeType,
  typename InternalVisitor  = internal::RTreeImplementation::EmptyVisitor,
  typename LeafVisitor      = internal::RTreeImplementation::EmptyVisitor,
  typename ElementVisitor   = internal::RTreeImplementation::EmptyVisitor,
  typename IndexableVisitor = internal::RTreeImplementation::EmptyVisitor>
void
visit_rtree(const RTreeType &tree,
            InternalVisitor  internal_node_visitor = {},
            LeafVisitor      leaf_visitor          = {},
            ElementVisitor   element_visitor       = {},
            IndexableVisitor indexable_visitor     = {});



/**
 * Return the number of levels of @p tree, without counting the root. A tree
 * whose root is a leaf (i.e., a tree with at most M entries) has zero levels;
 * a tree whose root has only leaves as children has one level, and so on.
 *
 * This is the depth of the tree in the Boost sense. It bounds the number of
 * levels of the multilevel hierarchy that can be extracted from the tree.
 */
template <typename RTreeType>
unsigned int
n_levels(const RTreeType &tree);



/**
 * Return the bounding boxes of all the nodes on level @p level of @p tree,
 * in the order in which a depth-first traversal visits them. Levels are
 * counted as in RTreeFunctionalVisitor: the children of the root are on
 * level 0. If @p level is larger than the last level, the boxes of the last
 * level (the leaf nodes) are returned. If the tree has no levels (its root is
 * a leaf), a single box containing the whole tree is returned.
 */
template <typename RTreeType>
std::vector<BoundingBox<
  boost::geometry::dimension<typename RTreeType::indexable_type>::value>>
extract_rtree_level(const RTreeType &tree, const unsigned int level);



/* ------------------- Inline and template functions ----------------------- */

template <typename IndexType,
          typename LeafTypeIterator,
          typename IndexableGetter>
inline RTree<typename LeafTypeIterator::value_type, IndexType, IndexableGetter>
pack_rtree(const LeafTypeIterator &begin, const LeafTypeIterator &end)
{
  return RTree<typename LeafTypeIterator::value_type,
               IndexType,
               IndexableGetter>(begin, end);
}



template <typename IndexType, typename ContainerType, typename IndexableGetter>
inline RTree<typename ContainerType::value_type, IndexType, IndexableGetter>
pack_rtree(const ContainerType &container)
{
  return pack_rtree<IndexType, decltype(container.begin()), IndexableGetter>(
    container.begin(), container.end());
}



template <typename Container>
inline IndexableGetterFromIndices<Container>::IndexableGetterFromIndices(
  const Container &c)
  : container(c)
{}



template <typename Container>
inline typename IndexableGetterFromIndices<Container>::result_type
IndexableGetterFromIndices<Container>::operator()(size_t i) const
{
  return getter(container[i]);
}



template <typename IndexType, typename ContainerType>
inline RTree<typename ContainerType::size_type,
             IndexType,
             IndexableGetterFromIndices<ContainerType>>
pack_rtree_of_indices(const ContainerType &container)
{
  // The packing constructor of the Boost R-tree needs a range whose elements
  // can be referenced, so we cannot pass a range of integers generated on
  // the fly: store the indices in a vector first.
  std::vector<typename ContainerType::size_type> indices(container.size());
  for (typename ContainerType::size_type i = 0; i < container.size(); ++i)
    indices[i] = i;

  return RTree<typename ContainerType::size_type,
               IndexType,
               IndexableGetterFromIndices<ContainerType>>(
    indices.begin(),
    indices.end(),
    IndexType(),
    IndexableGetterFromIndices<ContainerType>(container));
}



template <typename RTreeType,
          typename InternalVisitor,
          typename LeafVisitor,
          typename ElementVisitor,
          typename IndexableVisitor>
RTreeFunctionalVisitor<RTreeType,
                       InternalVisitor,
                       LeafVisitor,
                       ElementVisitor,
                       IndexableVisitor>::
  RTreeFunctionalVisitor(const RTreeType &tree,
                         InternalVisitor  internal_node_visitor,
                         LeafVisitor      leaf_visitor,
                         ElementVisitor   element_visitor,
                         IndexableVisitor indexable_visitor)
  : internal_node_visitor(std::move(internal_node_visitor))
  , leaf_visitor(std::move(leaf_visitor))
  , element_visitor(std::move(element_visitor))
  , indexable_visitor(std::move(indexable_visitor))
  , current_level(0)
  , translator(RTreeView<RTreeType>(tree).translator())
{
  static_assert(!has_internal_visitor ||
                  std::is_invocable_v<InternalVisitor,
                                      const std::size_t &,
                                      const internal_node_type &,
                                      const BoundingBox<dim> &,
                                      const internal_node_type &>,
                "The internal node visitor must be callable with "
                "(std::size_t, internal_node_type, BoundingBox<dim>, "
                "internal_node_type).");

  static_assert(!has_leaf_visitor ||
                  std::is_invocable_v<LeafVisitor,
                                      const std::size_t &,
                                      const internal_node_type &,
                                      const BoundingBox<dim> &,
                                      const leaf_type &>,
                "The leaf visitor must be callable with (std::size_t, "
                "internal_node_type, BoundingBox<dim>, leaf_type).");

  static_assert(!has_element_visitor || std::is_invocable_v<ElementVisitor,
                                                            const leaf_type &,
                                                            const value_type &>,
                "The element visitor must be callable with (leaf_type, "
                "value_type).");

  static_assert(!has_indexable_visitor ||
                  std::is_invocable_v<IndexableVisitor,
                                      const leaf_type &,
                                      const indexable_type &>,
                "The indexable visitor must be callable with (leaf_type, "
                "indexable_type).");

  RTreeView<RTreeType> rtv(tree);
  rtv.apply_visitor(*this);
}



template <typename RTreeType,
          typename InternalVisitor,
          typename LeafVisitor,
          typename ElementVisitor,
          typename IndexableVisitor>
void
RTreeFunctionalVisitor<RTreeType,
                       InternalVisitor,
                       LeafVisitor,
                       ElementVisitor,
                       IndexableVisitor>::operator()(internal_node_type &node)
{
  const std::size_t child_level = current_level;
  for (const auto &element :
       boost::geometry::index::detail::rtree::elements(node))
    {
      // Each element of an internal node is a pair (bounding box of the
      // child, pointer to the child). The child is either another internal
      // node or a leaf: call the right callback before descending into it.
      BoundingBox<dim> box;
      boost::geometry::convert(element.first, box);

      if (const auto *leaf_ptr = boost::get<leaf_type>(&*element.second))
        {
          if constexpr (has_leaf_visitor)
            leaf_visitor(child_level, node, box, *leaf_ptr);
        }
      else if (const auto *internal_ptr =
                 boost::get<internal_node_type>(&*element.second))
        {
          if constexpr (has_internal_visitor)
            internal_node_visitor(child_level, node, box, *internal_ptr);
        }

      current_level = child_level + 1;
      boost::geometry::index::detail::rtree::apply_visitor(*this,
                                                           *element.second);
    }
  current_level = child_level;
}



template <typename RTreeType,
          typename InternalVisitor,
          typename LeafVisitor,
          typename ElementVisitor,
          typename IndexableVisitor>
void
RTreeFunctionalVisitor<RTreeType,
                       InternalVisitor,
                       LeafVisitor,
                       ElementVisitor,
                       IndexableVisitor>::operator()(leaf_type &leaf)
{
  for (const auto &element :
       boost::geometry::index::detail::rtree::elements(leaf))
    {
      if constexpr (has_element_visitor)
        element_visitor(leaf, element);
      if constexpr (has_indexable_visitor)
        indexable_visitor(leaf, translator(element));
    }
}



template <typename RTreeType,
          typename InternalVisitor,
          typename LeafVisitor,
          typename ElementVisitor,
          typename IndexableVisitor>
inline void
visit_rtree(const RTreeType &tree,
            InternalVisitor  internal_node_visitor,
            LeafVisitor      leaf_visitor,
            ElementVisitor   element_visitor,
            IndexableVisitor indexable_visitor)
{
  RTreeFunctionalVisitor<RTreeType,
                         std::decay_t<InternalVisitor>,
                         std::decay_t<LeafVisitor>,
                         std::decay_t<ElementVisitor>,
                         std::decay_t<IndexableVisitor>>
    visitor(tree,
            std::move(internal_node_visitor),
            std::move(leaf_visitor),
            std::move(element_visitor),
            std::move(indexable_visitor));
}



template <typename RTreeType>
inline unsigned int
n_levels(const RTreeType &tree)
{
  RTreeView<RTreeType> rtv(tree);
  return rtv.depth();
}



template <typename RTreeType>
inline std::vector<BoundingBox<
  boost::geometry::dimension<typename RTreeType::indexable_type>::value>>
extract_rtree_level(const RTreeType &tree, const unsigned int level)
{
  constexpr unsigned int dim =
    boost::geometry::dimension<typename RTreeType::indexable_type>::value;

  std::vector<BoundingBox<dim>> boxes;

  const unsigned int n_levels_in_tree = n_levels(tree);

  if (n_levels_in_tree == 0)
    {
      // The root is a leaf, so there are no levels to extract: return the
      // box of the whole tree.
      boxes.resize(1);
      boost::geometry::convert(tree.bounds(), boxes[0]);
    }
  else
    {
      const unsigned int target_level =
        std::min<unsigned int>(level, n_levels_in_tree - 1);

      const auto store_box_if_on_target_level =
        [&](const std::size_t &current_level,
            const auto & /*parent*/,
            const BoundingBox<dim> &box,
            const auto & /*node*/) {
          if (current_level == target_level)
            boxes.push_back(box);
        };

      visit_rtree(tree,
                  store_box_if_on_target_level,
                  store_box_if_on_target_level);
    }

  return boxes;
}

R3MGC_NAMESPACE_CLOSE

#endif
