#ifndef EXSTD_SINGLETON_HPP
#define EXSTD_SINGLETON_HPP

/**
 * @file singleton.hpp
 * @brief Definition of the singleton template struct.
 */

/**
 * @brief A template struct for creating a singleton instance
 *
 * The singleton template struct ensures that only one instance of the specified
 * type is created and provides a static method to access that instance
 *
 * @tparam T The type of the struct for which a singleton instance is desired.
 */
template <typename T> struct singleton {
public:
  /**
   * @brief Deleted copy constructor to prevent cloning of the singleton
   * instance
   */
  singleton(const singleton &) = delete;

  /**
   * @brief Deleted copy assignment operator to prevent cloning of the singleton
   * instance
   */
  singleton &operator=(const singleton &) = delete;

  /**
   * @brief Access the singleton instance.
   *
   * This static method returns a reference to the singleton instance of the
   * specified type.
   *
   * @return A reference to the singleton instance.
   */
  static T &instance() {
    static T instance;
    return instance;
  }

protected:
  /**
   * @brief Default constructor of the singleton struct.
   */
  singleton() = default;

  /**
   * @brief Default destructor of the singleton struct.
   */
  ~singleton() = default;
};

#endif
