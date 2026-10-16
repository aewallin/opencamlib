/**
 * Converts an object to its string representation using stream insertion.
 *
 * This function creates a std::ostringstream, inserts the object into the stream,
 * and returns the accumulated string. It requires that the type T implements the
 * stream insertion operator (<<). Used for pybind11 helper.
 *
 */

#ifndef OCL_OSTREAM_STR_HPP
#define OCL_OSTREAM_STR_HPP

#include <sstream>
#include <string>

namespace ocl {

template <typename T>
std::string ostream_str(const T& obj)
{
    std::ostringstream oss;
    oss << obj;
    return oss.str();
}

} // namespace ocl

#endif // OCL_OSTREAM_STR_HPP
