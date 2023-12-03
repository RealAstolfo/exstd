#ifndef EXSTD_SINGLETON_HPP
#define EXSTD_SINGLETON_HPP

template <typename T> struct singleton {
public:
  singleton(const singleton &) = delete;
  singleton &operator=(const singleton &) = delete;

  static T &instance() {
    static T instance;
    return instance;
  }

protected:
  singleton() = default;
  ~singleton() = default;
};

#endif
