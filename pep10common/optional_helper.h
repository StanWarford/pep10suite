#ifndef OPTIONAL_HELPER_H
#define OPTIONAL_HELPER_H

#include <optional>
template<class T>
inline T optional_helper(std::optional<T> opt)
{
    // Apple is dumb. They do not support C++17 optionals in a standards-compliant way.
    // I am unwilling to sacrifice safe access using value() on platforms
    // that (CORRECTLY) support the C++17 standard.
    #ifdef __APPLE__
    return *opt;
    #else
    return opt.value();
    #endif
}

template<class T>
inline const T optional_helper(std::optional<const T> opt)
{
    #ifdef __APPLE__
    return *opt;
    #else
    return opt.value();
    #endif
}

#endif // OPTIONAL_HELPER_H
