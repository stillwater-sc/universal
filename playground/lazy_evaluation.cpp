// lazy_evaluation.cpp: experiments in lazy evaluation and state management
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <type_traits>

#include <universal/number/cfloat/cfloat.hpp>

namespace test1 {
    template<typename T>
    struct Expression {
        T value;

        Expression(T v) : value(v) {}

        template<typename U>
        Expression<decltype(value + U::value)> operator+(const Expression<U>& other) const {
            return Expression<decltype(value + U::value)>(value + other.value);
        }

        template<typename U>
        Expression<decltype(value* U::value)> operator*(const Expression<U>& other) const {
            return Expression<decltype(value * U::value)>(value * other.value);
        }

        operator T() const { return value; }

    };
}

namespace test2 {

    template <typename T>
    class Expression {
    private:
        T value;

    public:
        Expression(T value) : value(value) {}

        template <typename U>
        auto operator+(const Expression<U>& other) const -> decltype(value + other.value) {
            return Expression<decltype(value + other.value)>(value + other.value);
        }

        template <typename U>
        auto operator*(const Expression<U>& other) const -> decltype(value * other.value) {
            return Expression<decltype(value * other.value)>(value * other.value);
        }

        operator T() const { return T(value); }

    };
}

int main(int argc, char** argv)
try {
    using namespace sw::universal;

    // Expression as defined above only works on native types as some implicit conversions make the class definition simpler
    // precondition is definition of operator+(), operator*(), and conversion operator


    {  
        using Real = float;
        test1::Expression<Real> a(2.0), b(3.0), c(4.0);

        test1::Expression<Real> result = a * (b + c);

        std::cout << result << '\n';
    }

    {
        using Real = cfloat<24,8, uint32_t, true, false, false>;

        test2::Expression<Real> a(2.5), b(3.0), c(4.0);

        // test2::Expression<Real> result = a * (b + c);
        // required: binary operator that supports: Expression * cfloat

        test2::Expression<Real> tmpSum = (b + c);

        test2::Expression<Real> result = a * tmpSum;

        Real value = Real(result);

        std::cout << value << '\n';
    }

    return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
