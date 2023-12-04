#ifndef EXSTD_SERIALIZE_HPP
#define EXSTD_SERIALIZE_HPP

#include <bits/utility.h>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <tuple>
#include <type_traits>
#include <vector>

using std::tuple;
using std::vector;

using std::is_class;
using std::is_class_v;
using std::is_function;
using std::is_function_v;
using std::is_polymorphic;
using std::is_polymorphic_v;

using std::is_pointer;
using std::is_pointer_v;

using std::is_array;
using std::is_array_v;

/**
 * @file serialize.hpp
 * @brief Definition of the serialize utility functions and structs.
 */

/**
 * @brief Struct representing a universal type for serialization purposes.
 *
 * The `universal_type` struct is used for serialization. It represents a
 * universal type that can be converted to any desired type during
 * serialization.
 *
 * @tparam T The type to be serialized.
 * @tparam Lambda The lambda function to perform the serialization.
 * @tparam I The index representing the current level of recursion during
 * serialization.
 * @tparam U The types accumulated during recursion for serialization.
 */
template <typename T, typename Lambda, std::size_t I, typename... U>
struct universal_type {
  /**
   * @brief Constructor for the universal_type.
   *
   * @param l The lambda function for serialization.
   */
  universal_type(Lambda &&l) : lambda(std::move(l)) {}

  /**
   * @brief Conversion operator for the universal_type.
   *
   * This operator performs the serialization by constructing a nested structure
   * of universal_type with accumulated types and invoking the lambda function.
   *
   * @tparam V The type to convert to during serialization.
   * @return An instance of V.
   */
  template <typename V> operator V() {
    T t = {U()..., V(),
           universal_type<T, Lambda, I - 1, U..., V>(std::move(lambda))};
    return V();
  }

  Lambda lambda;
};

/**
 * @brief Specialization of universal_type for the final level of recursion.
 */
template <typename T, typename Lambda, typename... U>
struct universal_type<T, Lambda, 1, U...> {
  universal_type(Lambda &&l) : lambda(std::move(l)) {}
  template <typename V> operator V() {
    lambda(std::type_identity<std::tuple<U..., V>>());
    return V();
  }

  Lambda lambda;
};

/**
 * @brief Struct representing a placeholder type for serialization.
 */
struct any {
  /**
   * @brief Conversion operator for any type.
   *
   * This operator allows any type to be converted.
   *
   * @tparam V The type to convert to during serialization.
   * @return An instance of V.
   */
  template <typename V> operator V() const {}
};

/**
 * @brief Compile-time function to count the number of members in a struct.
 *
 * This constexpr function counts the number of members in a struct using
 * the presence of specific members and the any type as placeholders.
 *
 * @tparam T The type for which to count members.
 * @param member... Members used as counters.
 * @return The number of members in the struct.
 */
template <typename T> consteval auto member_counter(auto... member) {
  if constexpr (requires { T{member...}; } == false)
    return sizeof...(member) - 1;
  else
    return member_counter<T>(member..., any{});
}

/**
 * @brief Reflects on the structure of a class for serialization.
 *
 * This function generates a structure that reflects the members of a class
 * for serialization. It uses a lambda function to serialize each member.
 *
 * @tparam T The type of the struct or class.
 * @tparam Lambda The lambda function to perform serialization for each member.
 * @param lambda The lambda function.
 */
template <typename T, typename Lambda> void reflect_struct(Lambda &&lambda) {
  T t = {universal_type<T, Lambda, member_counter<T>()>(std::move(lambda))};
}

/**
 * @brief Reflects on the structure of a class and returns a tuple of serialized
 * values.
 *
 * This function reflects on the members of a class, serializes each member, and
 * returns a tuple of serialized values.
 *
 * @tparam T The type of the struct or class.
 * @param t The instance of the struct or class.
 * @return A tuple of serialized values.
 */
template <typename T> auto reflect_lambda(T &t) {
  tuple<> tmp;
  reflect_struct<T>([&t, &tmp](auto identity) {
    using identity_type = decltype(identity);
    using tuple_type = identity_type::type;
    tuple_type tt;
    tmp = std::tuple_cat(tmp, tt);
  });

  return tmp;
}

#endif
