/**
 * @file args.hpp
 * @brief Header file for argument handling.
 */

#ifndef ARGS_HPP
#define ARGS_HPP

#include <cstdlib>
#include <functional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

/**
 * @brief Struct to manage and process command-line arguments.
 */
struct arguments {
  /**
   * @brief Vector of pairs containing argument string views and corresponding
   * callback functions.
   */
  std::vector<std::pair<std::string_view,
                        std::function<bool(const std::string_view &)>>>
      action;

  /**
   * @brief Adds a handler for a specific argument.
   * @param arg The argument to handle.
   * @param callback The callback function to execute when the argument is
   * found.
   */
  void add_handler(std::string_view arg,
                   std::function<bool(const std::string_view &)> callback);

  /**
   * @brief Processes the command-line arguments.
   * @param argc Argument count.
   * @param argv Argument vector.
   * @return A vector of strings containing unused arguments.
   */
  std::vector<std::string> process_args(int argc, char **argv);

  /**
   * @brief Constructor for the arguments struct.
   */
  arguments();
};

/**
 * @brief Adds a handler for a specific argument.
 * @param arg The argument to handle.
 * @param callback The callback function to execute when the argument is found.
 */
void arguments::add_handler(
    std::string_view arg,
    std::function<bool(const std::string_view &)> callback) {
  action.push_back(std::make_pair(arg, callback));
}

/**
 * @brief Processes the command-line arguments.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return A vector of strings containing unused arguments.
 */
std::vector<std::string> arguments::process_args(int argc, char **argv) {
  std::vector<std::string_view> args(argv + 1, argv + argc);
  for (auto &str_func : action) {
    auto arg_it = std::begin(args);
    while (arg_it != std::end(args)) {
      if (*arg_it == str_func.first) {
        const std::string_view next = *std::next(arg_it);
        if (str_func.second(next)) {
          std::exit(EXIT_FAILURE);
        }
        // Remove arg_it and next from args
        arg_it = args.erase(arg_it, std::next(arg_it, 2));
      } else {
        arg_it = std::next(arg_it);
      }
    }
  }

  std::vector<std::string> unused;
  for (auto left : args)
    unused.push_back(left.data());
  return unused;
}

/**
 * @brief Constructor for the arguments struct.
 */
arguments::arguments() {}

#endif // ARGS_HPP
