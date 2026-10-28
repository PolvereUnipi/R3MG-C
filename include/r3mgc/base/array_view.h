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

#ifndef r3mgc_base_array_view_h
#define r3mgc_base_array_view_h

#include <r3mgc/base/config.h>

#include <r3mgc/base/exceptions.h>

#include <cstddef>
#include <type_traits>
#include <vector>

R3MGC_NAMESPACE_OPEN

/**
 * A non-owning view of a contiguous range of objects of type @p ElementType:
 * a pointer to the first element and the number of elements.
 *
 * This is a minimal version of deal.II's ArrayView (or of C++20's std::span,
 * which is not available in C++17). The library uses it to return slices of
 * larger arrays without copying them. The main example is
 * RTreeHierarchy::agglomerate(): all the agglomerates of all the levels of an
 * R-tree are slices of one array of leaf ids.
 *
 * A view does not own its elements: it becomes invalid when the underlying
 * array is destroyed or reallocated. Views are cheap to copy and should be
 * passed by value.
 *
 * @tparam ElementType The type of the elements. Use a const type, e.g.
 * ArrayView<const double>, for a read-only view.
 */
template <typename ElementType>
class ArrayView
{
public:
  /**
   * The type of the elements, including the const qualifier, if any.
   */
  using value_type = ElementType;

  /**
   * The iterator types. Since the elements are contiguous, iterators are
   * plain pointers.
   */
  using iterator       = value_type *;
  using const_iterator = const value_type *;

  /**
   * Default constructor: an empty view.
   */
  ArrayView() = default;

  /**
   * Build a view of the @p n_elements elements starting at @p starting_element.
   */
  ArrayView(value_type *starting_element, const std::size_t n_elements);

  /**
   * Build a view of all the elements of @p vector. Read-only views can be
   * built from constant vectors.
   */
  ArrayView(const std::vector<std::remove_cv_t<value_type>> &vector);

  /**
   * Build a view of all the elements of @p vector.
   */
  ArrayView(std::vector<std::remove_cv_t<value_type>> &vector);

  /**
   * Return the number of elements in the view.
   */
  std::size_t
  size() const;

  /**
   * Return true if the view has no elements.
   */
  bool
  empty() const;

  /**
   * Return a pointer to the first element.
   */
  value_type *
  data() const;

  /**
   * Return an iterator to the first element.
   */
  iterator
  begin() const;

  /**
   * Return an iterator past the last element.
   */
  iterator
  end() const;

  /**
   * Return a reference to the @p i-th element. In debug mode, check that
   * @p i is a valid index.
   */
  value_type &
  operator[](const std::size_t i) const;

  /**
   * Return true if the two views have the same size and equal elements (not
   * necessarily at the same memory locations).
   */
  bool
  operator==(const ArrayView &other) const;

  /**
   * Return the negation of operator==().
   */
  bool
  operator!=(const ArrayView &other) const;

private:
  /**
   * The first element of the view.
   */
  value_type *starting_element = nullptr;

  /**
   * The number of elements of the view.
   */
  std::size_t n_elements = 0;
};



/* ------------------------- Inline functions ------------------------------ */

template <typename ElementType>
inline ArrayView<ElementType>::ArrayView(value_type       *starting_element,
                                         const std::size_t n_elements)
  : starting_element(starting_element)
  , n_elements(n_elements)
{}



template <typename ElementType>
inline ArrayView<ElementType>::ArrayView(
  const std::vector<std::remove_cv_t<value_type>> &vector)
  : starting_element(vector.data())
  , n_elements(vector.size())
{
  static_assert(std::is_const_v<value_type>,
                "A view of a constant vector must have constant elements.");
}



template <typename ElementType>
inline ArrayView<ElementType>::ArrayView(
  std::vector<std::remove_cv_t<value_type>> &vector)
  : starting_element(vector.data())
  , n_elements(vector.size())
{}



template <typename ElementType>
inline std::size_t
ArrayView<ElementType>::size() const
{
  return n_elements;
}



template <typename ElementType>
inline bool
ArrayView<ElementType>::empty() const
{
  return n_elements == 0;
}



template <typename ElementType>
inline typename ArrayView<ElementType>::value_type *
ArrayView<ElementType>::data() const
{
  return starting_element;
}



template <typename ElementType>
inline typename ArrayView<ElementType>::iterator
ArrayView<ElementType>::begin() const
{
  return starting_element;
}



template <typename ElementType>
inline typename ArrayView<ElementType>::iterator
ArrayView<ElementType>::end() const
{
  return starting_element + n_elements;
}



template <typename ElementType>
inline typename ArrayView<ElementType>::value_type &
ArrayView<ElementType>::operator[](const std::size_t i) const
{
  R3MGC_AssertIndexRange(i, n_elements);
  return starting_element[i];
}



template <typename ElementType>
inline bool
ArrayView<ElementType>::operator==(const ArrayView &other) const
{
  if (n_elements != other.n_elements)
    return false;
  for (std::size_t i = 0; i < n_elements; ++i)
    if (!(starting_element[i] == other.starting_element[i]))
      return false;
  return true;
}



template <typename ElementType>
inline bool
ArrayView<ElementType>::operator!=(const ArrayView &other) const
{
  return !(*this == other);
}

R3MGC_NAMESPACE_CLOSE

#endif
