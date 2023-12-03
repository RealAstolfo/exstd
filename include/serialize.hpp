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

template <typename T, typename Lambda, std::size_t I, typename... U>
struct universal_type {
  universal_type(Lambda &&l) : lambda(std::move(l)) {}
  template <typename V> operator V() {
    T t = {U()..., V(),
           universal_type<T, Lambda, I - 1, U..., V>(std::move(lambda))};
    return V();
  }

  Lambda lambda;
};

template <typename T, typename Lambda, typename... U>
struct universal_type<T, Lambda, 1, U...> {
  universal_type(Lambda &&l) : lambda(std::move(l)) {}
  template <typename V> operator V() {
    lambda(std::type_identity<std::tuple<U..., V>>());
    return V();
  }

  Lambda lambda;
};

struct any {
  template <typename V> operator V() const {}
};

template <typename T> consteval auto member_counter(auto... member) {
  if constexpr (requires { T{member...}; } == false)
    return sizeof...(member) - 1;
  else
    return member_counter<T>(member..., any{});
}

template <typename T, typename Lambda> void reflect_struct(Lambda &&lambda) {
  T t = {universal_type<T, Lambda, member_counter<T>()>(std::move(lambda))};
}

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
