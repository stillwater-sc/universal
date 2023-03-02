#pragma once
#include <string>

#define make_std_fun(name) template <typename T> sw::universal::enable_if_rational<T, T> name(const T &t) { return sw::universal::name(t); }

namespace std {
    make_std_fun(sin)
    make_std_fun(cos)
    make_std_fun(sqrt)
    // TODO: no abs implementation for rational
    // make_std_fun(abs)
    make_std_fun(floor)
    make_std_fun(ceil)
    make_std_fun(exp)
    make_std_fun(log)
    make_std_fun(atan)

    template <typename T>
    sw::universal::enable_if_rational<T, std::string> to_string(const T &t) {
        return sw::universal::to_string(t);
    }

    template <typename T>
    sw::universal::enable_if_rational<T, T> pow(const T &base, T &exp) {
        return sw::universal::pow(base, exp);
    }

    template <typename T>
    sw::universal::enable_if_rational<T, T> atan2(const T &y, const T &x) {
        return sw::universal::atan2(y, x);
    }
}

#undef make_std_fun