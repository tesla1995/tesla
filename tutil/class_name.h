#ifndef DESIGN_PATTERN_UTIL_CLASS_NAME_H_
#define DESIGN_PATTERN_UTIL_CLASS_NAME_H_

#include <typeinfo>
#include <string>

namespace tesla {
namespace tutil {

std::string demangle(const char* name);

// Get typename of |obj|, in std::string
template <typename T>
std::string class_name_str(T const& obj) {
    return demangle(typeid(obj).name());
}

}  // namespace tutil
}  // namespace tesla

#endif // DESIGN_PATTERN_UTIL_CLASS_NAME_H_
