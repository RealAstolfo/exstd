#ifndef EXSTD_UTILS_HPP
#define EXSTD_UTILS_HPP

#include <cmath>
#include <cstdint>
#include <cstring>
#include <list>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <vector>

/**
 * @file utils.hpp
 * @brief Contains the utility functions and operators for various purposes.
 */

/**
 * @brief Get a slice of a std::list.
 *
 * This function returns a vector containing pointers to elements in the
 * specified range [start, end) of a std::list.
 *
 * @tparam T The type of elements in the list.
 * @param l the list from which to extract the slice.
 * @param start The starting index of the slice.
 * @param end The ending index (exclusive) of the slice.
 * @return A vector containing the pointers to elements in the specified range.
 */
template <typename T>
std::vector<T *> get_slice(std::list<T> &l, size_t start, size_t end) {
  std::vector<T *> result;
  auto it = l.begin();
  std::advance(it, start);
  for (size_t i = start; i < end && it != l.end(); i++, it++)
    result.push_back(&(*it));
  return result;
}

/**
 * @brief Calculate the relative error between two values.
 *
 * This function calculates the relative error between two values, defined as
 * the absolute difference divided by the actual value.
 *
 * @tparam T The type of values being compared.
 * @param estimated The estimated value.
 * @param actual The actual value.
 * @return The relative error between the estimated and actual values.
 */
template <typename T>
auto error(const T &estimated, const T &actual)
    -> decltype(estimated - actual) {
  const auto diff = estimated - actual;
  return std::abs(diff / actual);
}

/**
 * @brief Calculate the maximum relative error between corresponding elements of
 * two ranges.
 *
 * This function calculates the maximum relative error between corresponding
 * elements of two ranges. It assumes that both ranges have the same type of
 * elements.
 *
 * @tparam Range the type of the ranges.
 * @param estimated The estimated range.
 * @param actual The actual range.
 * @return The maximum relative error between corresponding elements of the
 * ranges.
 */
template <std::ranges::range Range>
  requires std::same_as<std::ranges::range_value_t<Range>,
                        std::ranges::range_value_t<Range>>
std::ranges::range_value_t<Range> error(const Range &estimated,
                                        const Range &actual) {
  std::ranges::range_value_t<Range> max_error =
      std::numeric_limits<std::ranges::range_value_t<Range>>::lowest();
  auto est_it = std::begin(estimated);
  auto act_it = std::begin(actual);
  while (est_it != std::end(estimated) && act_it != std::end(actual)) {
    max_error = std::max(max_error, error(*est_it, *act_it));
    est_it = std::next(est_it);
    act_it = std::next(act_it);
  }

  return max_error;
}

/**
 * @brief Compare two ranges for equality
 *
 * This operator checks if two ranges are equal by comparing their elements.
 *
 * @tparam Range The type of the ranges.
 * @param v1 The first range.
 * @param v2 The second range.
 * @return True if the ranges are equal, false otherwise.
 */
template <std::ranges::range Range>
  requires std::same_as<std::ranges::range_value_t<Range>,
                        std::ranges::range_value_t<Range>>
bool operator==(const Range &v1, const Range &v2) {
  auto v1_it = std::begin(v1);
  auto v2_it = std::begin(v2);
  if (std::distance(v1_it, std::end(v1)) != std::distance(v2_it, std::end(v2)))
    return false;

  while (v1_it != std::end(v1) && v2_it != std::end(v2)) {
    if (*v1_it != *v2_it)
      return false;
    v1_it = std::next(v1_it);
    v2_it = std::next(v2_it);
  }

  return true;
}

#endif
