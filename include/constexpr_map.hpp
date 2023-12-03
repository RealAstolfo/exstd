#ifndef EXSTD_CONSTEXPR_MAP_HPP
#define EXSTD_CONSTEXPR_MAP_HPP

#include <array>
#include <algorithm>
#include <stdexcept>
#include <utility>

template<typename key, typename value, std::size_t size>
struct constexpr_map {
    std::array<std::pair<key, value>, size> data;
    [[nodiscard]] constexpr value at(const key& k) const {
        const auto it = std::find_if(begin(data), end(data), [&k](const auto &v) { return v.first == k; });
        if (it != end(data))
            return it->second;
        else
            throw std::range_error("value not found for key");
    }
};

#endif
