#ifndef OCL_TEST_CHECK_HPP
#define OCL_TEST_CHECK_HPP

#include <cstdlib>
#include <iostream>

// Keep checks active in release builds as well as debug builds.
#define CHECK(condition) do { \
    if (!(condition)) { \
        std::cerr << __FILE__ << ':' << __LINE__ << ": " #condition "\n"; \
        std::exit(EXIT_FAILURE); \
    } \
} while (false)

#endif
