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

template <typename T>
std::vector<T *> get_slice(std::list<T> &l, size_t start, size_t end) {
  std::vector<T *> result;
  auto it = l.begin();
  std::advance(it, start);
  for (size_t i = start; i < end && it != l.end(); i++, it++)
    result.push_back(&(*it));
  return result;
}

template <typename T>
auto error(const T &estimated, const T &actual)
    -> decltype(estimated - actual) {
  const auto diff = estimated - actual;
  return std::abs(diff / actual);
}

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
