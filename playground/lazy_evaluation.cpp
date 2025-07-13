// lazy_evaluation.cpp: experiments in lazy evaluation and state management
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <type_traits>
#include <utility>
#include <memory>
#include <functional>
#include <stdexcept>

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
        Expression<decltype(value * U::value)> operator*(const Expression<U>& other) const {
            return Expression<decltype(value * U::value)>(value * other.value);
        }

        operator T() const { return value; }

    };
}

namespace test2 {

    // using a C++11 trailing return type declaration
    // for some reason, this does not cover the signature Expression * T

    template <typename T>
    struct Expression {

        T value;

        Expression(T value) : value(value) {}

        template <typename U>
        auto operator+(const Expression<U>& other) const -> decltype(value + other.value) {
            return Expression<decltype(value + other.value)>(value + other.value);
        }

        template <typename U>
        auto operator*(const Expression<U>& other) const -> decltype(value * other.value) {
            return Expression<decltype(value * other.value)>(value * other.value);
        }

        operator T() const { return value; }

    };
}

namespace test3 {

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

        operator T() const { return value; }

    };
}

namespace test4 {

    // polymorphic lazy evaluation

    template<typename T>
    class Expression {
    public:
        virtual T evaluate() const = 0;
        virtual ~Expression() = default;
    };

    template<typename T>
    class Constant : public Expression<T> {
        T value;
    public:
        explicit Constant(T val) : value(val) {}
        T evaluate() const override { return value; }
    };

    template<typename T>
    class BinaryOperation : public Expression<T> {
        std::shared_ptr<Expression<T>> left, right;
        std::function<T(T, T)> op;
    public:
        BinaryOperation(std::shared_ptr<Expression<T>> l, std::shared_ptr<Expression<T>> r, std::function<T(T, T)> operation)
            : left(l), right(r), op(operation) {}
        T evaluate() const override {
            return op(left->evaluate(), right->evaluate());
        }
    };

    template<typename T>
    class LazyExpression {
        std::shared_ptr<Expression<T>> expr;
    public:
        LazyExpression(T value) : expr(std::make_shared<Constant<T>>(value)) {}

        LazyExpression(std::shared_ptr<Expression<T>> e) : expr(e) {}

        LazyExpression operator+(const LazyExpression& other) const {
            return LazyExpression(std::make_shared<BinaryOperation<T>>(expr, other.expr, std::plus<T>()));
        }

        LazyExpression operator-(const LazyExpression& other) const {
            return LazyExpression(std::make_shared<BinaryOperation<T>>(expr, other.expr, std::minus<T>()));
        }

        LazyExpression operator*(const LazyExpression& other) const {
            return LazyExpression(std::make_shared<BinaryOperation<T>>(expr, other.expr, std::multiplies<T>()));
        }

        LazyExpression operator/(const LazyExpression& other) const {
            return LazyExpression(std::make_shared<BinaryOperation<T>>(expr, other.expr, [](T a, T b) {
                    if (b == T(0)) throw std::runtime_error("Division by zero");
                    return a / b;
                }));
        }

        T evaluate() const {
            return expr->evaluate();
        }
    };
}

int main(int argc, char** argv)
try {
    using namespace sw::universal;

    // Expression as defined above only works on native types as some implicit conversions make the class definition simpler
    // precondition is definition of operator+(), operator*(), and conversion operator

    {
        std::cout << "Test1: expression template lazy evaluation\n";
        using Real = cfloat<24, 8, uint32_t, true, false, false>;

        Real ra(2.0), rb(3.0), rc(4.0);

        {
            decltype(ra * rb) mulType = (ra * rb);  // mulType is a Real
            std::cout << "should be a cfloat      : " << typeid(mulType).name() << '\n';
            decltype(rb + rc) addType = (rb + rc);  // addType is a Real
            std::cout << "should be a cfloat      : " << typeid(addType).name() << '\n';
        }

        test1::Expression<Real> a(2.0), b(3.0), c(4.0);
        std::cout << "should be an Expression : " << typeid(a).name() << '\n';

        // these statements fail during argument deduction
        // std::cout << "should be an Expression : " << typeid(decltype(a + b)).name() << '\n';
        // std::cout << "should be an Expression : " << typeid(decltype(a * b)).name() << '\n';

        /*
        {
            test1::Expression<Real> mulType = a * b;  // mulType should become an Expression
            std::cout << typeid(mulType).name() << '\n';
            decltype(b + c) addType = (b + c);  // addType should become an Expression
            std::cout << typeid(addType).name() << '\n';
        }
        */
    }

    {  
        std::cout << "Test2: expression template lazy evaluation\n";
        // using Real = cfloat<24,8, uint32_t, true, false, false>;
        using Real = float;
        test1::Expression<Real> a(2.0), b(3.0), c(4.0);

        decltype(a * b) mulType = (a * b);  // mulType is a Real
        std::cout << typeid(mulType).name() << '\n';
        decltype(b + c) addType = (b + c);  // addType is a Real
        std::cout << typeid(addType).name() << '\n';

        test1::Expression<Real> result = a * (b + c);

        std::cout << result << '\n';
    }

    {
        std::cout << "Test3: expression template lazy evaluation\n";
        // using Real = cfloat<24,8, uint32_t, true, false, false>;
        using Real = float;

        test2::Expression<Real> a(2.0), b(3.0), c(4.0);

        //test3::Expression<Real> result = a * (b + c);
        // required: binary operator that supports: Expression * cfloat

        decltype(b + c) bla = (b + c);  // bla is a Real
        std::cout << typeid(bla).name() << '\n';

        test2::Expression<Real> tmpSum = (b + c);

        test2::Expression<Real> result = a * tmpSum;

        Real value = Real(result);

        std::cout << value << '\n';
    }

    {
        std::cout << "Test4a: polymorphic lazy evaluation with native types\n";
        using Real = double;
        test4::LazyExpression<Real> a(2.0), b(3.0), c(4.0);
        auto result = a * (b + c);
        std::cout << result.evaluate() << '\n';

    }

    {
        std::cout << "Test4b: polymorphic lazy evaluation with custom types\n";
        using Real = cfloat<24, 8, uint32_t, true, false, false>;
        test4::LazyExpression<Real> a(2.0), b(3.0), c(4.0);
        auto result = a * (b + c);
        std::cout << result.evaluate() << '\n';
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
