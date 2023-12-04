#ifndef EXSTD_CONSTEXPR_MAP_HPP
#define EXSTD_CONSTEXPR_MAP_HPP

#include <algorithm>
#include <array>
#include <stdexcept>
#include <utility>

/**
 * @file constexpr_map.hpp
 * @brief Definition of the constexpr_map template struct.
 */

/**
 * @brief A constexpr map with a fixed size.
 *
 * The `constexpr_map` template struct represents a simple map with a fixed size
 * implemented using a std::array. It allows constant-time lookup of values by
 * key.
 *
 * @tparam key The type of keys in the map.
 * @tparam value The type of values associated with the keys.
 * @tparam size The fixed size of the map.
 */
template <typename key, typename value, std::size_t size> struct constexpr_map {
  std::array<std::pair<key, value>, size> data;

  /**
   * @brief Get the value associated with a key.
   *
   * This constexpr method performs a constant-time lookup of the value
   * associated with the given key. I fthe key is not found, a std::range_error
   * is thrown.
   *
   * @param k The key for which to retrieve the value.
   * @return The value associated with the key.
   * @throws std::range_error if the key is not found in the map.
   */
  [[nodiscard]] constexpr value at(const key &k) const {
    const auto it = std::find_if(begin(data), end(data),
                                 [&k](const auto &v) { return v.first == k; });
    if (it != end(data))
      return it->second;
    else
      throw std::range_error("value not found for key");
  }
};

#endif
