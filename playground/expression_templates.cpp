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

namespace test4 {

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

/*
namespace test5 {

this incarnation has a problem with
1>F:\Users\tomtz\dev\clones\universal\playground\lazy_evaluation.cpp(243,60): error C2665: 'test5::Constant<T>::Constant': no overloaded function could convert all the argument types
1>F:\Users\tomtz\dev\clones\universal\playground\lazy_evaluation.cpp(243,60): error C2665:         with
1>F:\Users\tomtz\dev\clones\universal\playground\lazy_evaluation.cpp(243,60): error C2665:         [
1>F:\Users\tomtz\dev\clones\universal\playground\lazy_evaluation.cpp(243,60): error C2665:             T=double
1>F:\Users\tomtz\dev\clones\universal\playground\lazy_evaluation.cpp(243,60): error C2665:         ]

    // Forward declarations
    template<typename T>
    struct Constant;

    template<typename Left, typename Right, typename Op>
    struct BinaryExpression;

    // CRTP base class for all expressions
    template<typename Derived>
    struct Expression {
        constexpr const Derived& self() const { return static_cast<const Derived&>(*this); }

        // This function will be implemented by derived classes
        constexpr auto evaluate() const { return self().eval_impl(); }
    };

    // Constant expression
    template<typename T>
    struct Constant : Expression<Constant<T>> {
        const T value;
        explicit constexpr Constant(const T& v) : value(v) {}
        constexpr T eval_impl() const { return value; }
    };

    // Binary expression
    template<typename Left, typename Right, typename Op>
    struct BinaryExpression : Expression<BinaryExpression<Left, Right, Op>> {
        const Left left;
        const Right right;
        constexpr BinaryExpression(const Left& l, const Right& r) : left(l), right(r) {}
        constexpr auto eval_impl() const { return Op::apply(left.evaluate(), right.evaluate()); }
    };

    // Operator structs
    struct Add {
        template<typename T, typename U>
        static constexpr auto apply(const T& a, const U& b) { return a + b; }
    };

    struct Subtract {
        template<typename T, typename U>
        static constexpr auto apply(const T& a, const U& b) { return a - b; }
    };

    struct Multiply {
        template<typename T, typename U>
        static constexpr auto apply(const T& a, const U& b) { return a * b; }
    };

    struct Divide {
        template<typename T, typename U>
        static constexpr auto apply(const T& a, const U& b) { return a / b; }
    };

    // Helper function to create constant expressions
    template<typename T>
    constexpr auto make_constant(const T& value) {
        return Constant<T>(value);
    }

    // Operator overloads
    template<typename Left, typename Right>
    constexpr auto operator+(const Expression<Left>& lhs, const Expression<Right>& rhs) {
        return BinaryExpression<Left, Right, Add>(lhs.self(), rhs.self());
    }

    template<typename Left, typename Right>
    constexpr auto operator-(const Expression<Left>& lhs, const Expression<Right>& rhs) {
        return BinaryExpression<Left, Right, Subtract>(lhs.self(), rhs.self());
    }

    template<typename Left, typename Right>
    constexpr auto operator*(const Expression<Left>& lhs, const Expression<Right>& rhs) {
        return BinaryExpression<Left, Right, Multiply>(lhs.self(), rhs.self());
    }

    template<typename Left, typename Right>
    constexpr auto operator/(const Expression<Left>& lhs, const Expression<Right>& rhs) {
        return BinaryExpression<Left, Right, Divide>(lhs.self(), rhs.self());
    }

    // LazyExpression wrapper for easy use
    template<typename Expr>
    class LazyExpression {
        Expr expr;
    public:
        constexpr LazyExpression(const Expr& e) : expr(e) {}

        // Allow implicit conversion from any Expression to LazyExpression
        template<typename OtherExpr, typename = std::enable_if_t<std::is_base_of_v<Expression<OtherExpr>, OtherExpr>>>
        constexpr LazyExpression(const OtherExpr& e) : expr(e) {}

        constexpr auto evaluate() const { return expr.evaluate(); }

        template<typename Other>
        constexpr auto operator+(const LazyExpression<Other>& other) const {
            return LazyExpression(expr + other.expr);
        }

        template<typename Other>
        constexpr auto operator-(const LazyExpression<Other>& other) const {
            return LazyExpression(expr - other.expr);
        }

        template<typename Other>
        constexpr auto operator*(const LazyExpression<Other>& other) const {
            return LazyExpression(expr * other.expr);
        }

        template<typename Other>
        constexpr auto operator/(const LazyExpression<Other>& other) const {
            return LazyExpression(expr / other.expr);
        }
    };

    // Helper function to create LazyExpressions
    template<typename T>
    constexpr auto make_lazy(const T& value) {
        return LazyExpression(make_constant(value));
    }
}
*/

/*
namespace test5 {

      1. Introduced an expression_constructor_tag to disambiguate constructors.
      2. Modified the Constant<T> class to have two distinct constructors:
             One for non-Expression types (direct values)
             One for Expression types (which evaluates the expression)
      3. Updated the LazyExpression class to have separate constructors for Expression and non-Expression types.
      4. Removed the make_constant function as it's no longer necessary with the new LazyExpression design.
  
  but this fails as the Constant<T> constructors have the same signature and fail to compile

    // Forward declarations
    template<typename T>
    struct Constant;

    template<typename Left, typename Right, typename Op>
    struct BinaryExpression;

    // Tag type for disambiguating constructors
    struct expression_constructor_tag {};

    // CRTP base class for all expressions
    template<typename Derived>
    struct Expression {
        constexpr const Derived& self() const { return static_cast<const Derived&>(*this); }
        constexpr auto evaluate() const { return self().eval_impl(); }
    };

    // Constant expression
    template<typename T>
    struct Constant : Expression<Constant<T>> {
        const T value;

        // Constructor for non-Expression types
        template<typename U, typename = std::enable_if_t<!std::is_base_of_v<Expression<U>, U>>>
        explicit constexpr Constant(const U& v) : value(v) {}

        // Constructor for Expression types
        template<typename U, typename = std::enable_if_t<std::is_base_of_v<Expression<U>, U>>>
        explicit constexpr Constant(const U& expr, expression_constructor_tag)
            : value(expr.evaluate()) {}

        constexpr T eval_impl() const { return value; }
    };

    // Binary expression
    template<typename Left, typename Right, typename Op>
    struct BinaryExpression : Expression<BinaryExpression<Left, Right, Op>> {
        const Left left;
        const Right right;
        constexpr BinaryExpression(const Left& l, const Right& r) : left(l), right(r) {}
        constexpr auto eval_impl() const { return Op::apply(left.evaluate(), right.evaluate()); }
    };

    // Operator structs (unchanged)
    struct Add {
        template<typename T, typename U>
        static constexpr auto apply(const T& a, const U& b) { return a + b; }
    };

    struct Subtract {
        template<typename T, typename U>
        static constexpr auto apply(const T& a, const U& b) { return a - b; }
    };

    struct Multiply {
        template<typename T, typename U>
        static constexpr auto apply(const T& a, const U& b) { return a * b; }
    };

    struct Divide {
        template<typename T, typename U>
        static constexpr auto apply(const T& a, const U& b) { return a / b; }
    };

    // Helper function to create constant expressions
    template<typename T>
    constexpr auto make_constant(const T& value) {
        return Constant<T>(value);
    }

    // Operator overloads (unchanged)
    template<typename Left, typename Right>
    constexpr auto operator+(const Expression<Left>& lhs, const Expression<Right>& rhs) {
        return BinaryExpression<Left, Right, Add>(lhs.self(), rhs.self());
    }

    template<typename Left, typename Right>
    constexpr auto operator-(const Expression<Left>& lhs, const Expression<Right>& rhs) {
        return BinaryExpression<Left, Right, Subtract>(lhs.self(), rhs.self());
    }

    template<typename Left, typename Right>
    constexpr auto operator*(const Expression<Left>& lhs, const Expression<Right>& rhs) {
        return BinaryExpression<Left, Right, Multiply>(lhs.self(), rhs.self());
    }

    template<typename Left, typename Right>
    constexpr auto operator/(const Expression<Left>& lhs, const Expression<Right>& rhs) {
        return BinaryExpression<Left, Right, Divide>(lhs.self(), rhs.self());
    }

    // LazyExpression wrapper for easy use
    template<typename Expr>
    class LazyExpression {
        Expr expr;
    public:
        // Constructor for Expression types
        template<typename E, typename = std::enable_if_t<std::is_base_of_v<Expression<E>, E>>>
        constexpr LazyExpression(const E& e) : expr(e) {}

        // Constructor for non-Expression types
        template<typename T, typename = std::enable_if_t<!std::is_base_of_v<Expression<T>, T>>>
        constexpr LazyExpression(const T& value) : expr(Constant<T>(value)) {}

        constexpr auto evaluate() const { return expr.evaluate(); }

        // Operator overloads (unchanged)
        template<typename Other>
        constexpr auto operator+(const LazyExpression<Other>& other) const {
            return LazyExpression(expr + other.expr);
        }

        template<typename Other>
        constexpr auto operator-(const LazyExpression<Other>& other) const {
            return LazyExpression(expr - other.expr);
        }

        template<typename Other>
        constexpr auto operator*(const LazyExpression<Other>& other) const {
            return LazyExpression(expr * other.expr);
        }

        template<typename Other>
        constexpr auto operator/(const LazyExpression<Other>& other) const {
            return LazyExpression(expr / other.expr);
        }
    };

    // Helper function to create LazyExpressions
    template<typename T>
    constexpr auto make_lazy(const T& value) {
        return LazyExpression<Constant<T>>(value);
    }
}
*/

/*
namespace test5 {

        1. The Constant<T> class now has a single, unambiguous constructor.
        2. We've introduced a type trait is_expression to differentiate between Expression and non-Expression types.
        3. The LazyExpression class now uses SFINAE to properly distinguish between Expression and non-Expression types in its constructors.
        4. We've simplified the overall design, removing unnecessary complexity.
        5. The make_lazy function has been updated to use perfect forwarding, allowing it to handle both lvalues and rvalues correctly.

    // Forward declarations
    template<typename T>
    struct Constant;

    template<typename Left, typename Right, typename Op>
    struct BinaryExpression;

    // CRTP base class for all expressions
    template<typename Derived>
    struct Expression {
        constexpr const Derived& self() const { return static_cast<const Derived&>(*this); }
        constexpr auto evaluate() const { return self().eval_impl(); }
    };

    // Type trait to check if a type is an Expression
    template<typename T>
    struct is_expression : std::false_type {};

    template<typename T>
    struct is_expression<Expression<T>> : std::true_type {};

    template<typename T>
    inline constexpr bool is_expression_v = is_expression<T>::value;

    // Constant expression
    template<typename T>
    struct Constant : Expression<Constant<T>> {
        const T value;

        // Single constructor for Constant
        explicit constexpr Constant(const T& v) : value(v) {}

        constexpr T eval_impl() const { return value; }
    };

    // Binary expression
    template<typename Left, typename Right, typename Op>
    struct BinaryExpression : Expression<BinaryExpression<Left, Right, Op>> {
        const Left left;
        const Right right;
        constexpr BinaryExpression(const Left& l, const Right& r) : left(l), right(r) {}
        constexpr auto eval_impl() const { return Op::apply(left.evaluate(), right.evaluate()); }
    };

    // Operator structs
    struct Add {
        template<typename T, typename U>
        static constexpr auto apply(const T& a, const U& b) { return a + b; }
    };

    struct Subtract {
        template<typename T, typename U>
        static constexpr auto apply(const T& a, const U& b) { return a - b; }
    };

    struct Multiply {
        template<typename T, typename U>
        static constexpr auto apply(const T& a, const U& b) { return a * b; }
    };

    struct Divide {
        template<typename T, typename U>
        static constexpr auto apply(const T& a, const U& b) { return a / b; }
    };

    // Operator overloads
    template<typename Left, typename Right>
    constexpr auto operator+(const Expression<Left>& lhs, const Expression<Right>& rhs) {
        return BinaryExpression<Left, Right, Add>(lhs.self(), rhs.self());
    }

    template<typename Left, typename Right>
    constexpr auto operator-(const Expression<Left>& lhs, const Expression<Right>& rhs) {
        return BinaryExpression<Left, Right, Subtract>(lhs.self(), rhs.self());
    }

    template<typename Left, typename Right>
    constexpr auto operator*(const Expression<Left>& lhs, const Expression<Right>& rhs) {
        return BinaryExpression<Left, Right, Multiply>(lhs.self(), rhs.self());
    }

    template<typename Left, typename Right>
    constexpr auto operator/(const Expression<Left>& lhs, const Expression<Right>& rhs) {
        return BinaryExpression<Left, Right, Divide>(lhs.self(), rhs.self());
    }

    // LazyExpression wrapper for easy use
    template<typename Expr>
    class LazyExpression {
        Expr expr;
    public:
        // Constructor for Expression types
        template<typename E, typename = std::enable_if_t<is_expression_v<std::remove_cv_t<std::remove_reference_t<E>>>>>
        constexpr LazyExpression(E&& e) : expr(std::forward<E>(e)) {}

        // Constructor for non-Expression types
        template<typename T, typename = std::enable_if_t<!is_expression_v<std::remove_cv_t<std::remove_reference_t<T>>>>>
        constexpr LazyExpression(T&& value) : expr(Constant<std::remove_cv_t<std::remove_reference_t<T>>>(std::forward<T>(value))) {}

        constexpr auto evaluate() const { return expr.evaluate(); }

        // Operator overloads
        template<typename Other>
        constexpr auto operator+(const LazyExpression<Other>& other) const {
            return LazyExpression(expr + other.expr);
        }

        template<typename Other>
        constexpr auto operator-(const LazyExpression<Other>& other) const {
            return LazyExpression(expr - other.expr);
        }

        template<typename Other>
        constexpr auto operator*(const LazyExpression<Other>& other) const {
            return LazyExpression(expr * other.expr);
        }

        template<typename Other>
        constexpr auto operator/(const LazyExpression<Other>& other) const {
            return LazyExpression(expr / other.expr);
        }
    };

    // Helper function to create LazyExpressions
    template<typename T>
    constexpr auto make_lazy(T&& value) {
        return LazyExpression<std::remove_cv_t<std::remove_reference_t<T>>>(std::forward<T>(value));
    }

}
*/

/*
namespace test5 {

       1. The LazyExpression class now has a single, templated constructor. This avoids the ambiguity problem we were facing with multiple constructors.
       2. We've moved the logic for differentiating between Expression and non-Expression types to the make_lazy function. This function uses if constexpr to determine at compile-time whether to wrap the value in a Constant or use it directly.
       3. The Constant and BinaryExpression classes remain largely unchanged, maintaining their simplicity.
       4. We've removed the separate is_expression type trait, as it's no longer needed with this approach.

       fails on Constant<> constructor not being disambiguated between &&, const &, and const T&
    
    // Forward declarations
    template<typename T>
    struct Constant;

    template<typename Left, typename Right, typename Op>
    struct BinaryExpression;

    // CRTP base class for all expressions
    template<typename Derived>
    struct Expression {
        constexpr const Derived& self() const { return static_cast<const Derived&>(*this); }
        constexpr auto evaluate() const { return self().eval_impl(); }
    };

    // Constant expression
    template<typename T>
    struct Constant : Expression<Constant<T>> {
        const T value;
        explicit constexpr Constant(const T& v) : value(v) {}
        constexpr T eval_impl() const { return value; }
    };

    // Binary expression
    template<typename Left, typename Right, typename Op>
    struct BinaryExpression : Expression<BinaryExpression<Left, Right, Op>> {
        const Left left;
        const Right right;
        constexpr BinaryExpression(const Left& l, const Right& r) : left(l), right(r) {}
        constexpr auto eval_impl() const { return Op::apply(left.evaluate(), right.evaluate()); }
    };

    // Operator structs
    struct Add {
        template<typename T, typename U>
        static constexpr auto apply(const T& a, const U& b) { return a + b; }
    };

    struct Subtract {
        template<typename T, typename U>
        static constexpr auto apply(const T& a, const U& b) { return a - b; }
    };

    struct Multiply {
        template<typename T, typename U>
        static constexpr auto apply(const T& a, const U& b) { return a * b; }
    };

    struct Divide {
        template<typename T, typename U>
        static constexpr auto apply(const T& a, const U& b) { return a / b; }
    };

    // Operator overloads
    template<typename Left, typename Right>
    constexpr auto operator+(const Expression<Left>& lhs, const Expression<Right>& rhs) {
        return BinaryExpression<Left, Right, Add>(lhs.self(), rhs.self());
    }

    template<typename Left, typename Right>
    constexpr auto operator-(const Expression<Left>& lhs, const Expression<Right>& rhs) {
        return BinaryExpression<Left, Right, Subtract>(lhs.self(), rhs.self());
    }

    template<typename Left, typename Right>
    constexpr auto operator*(const Expression<Left>& lhs, const Expression<Right>& rhs) {
        return BinaryExpression<Left, Right, Multiply>(lhs.self(), rhs.self());
    }

    template<typename Left, typename Right>
    constexpr auto operator/(const Expression<Left>& lhs, const Expression<Right>& rhs) {
        return BinaryExpression<Left, Right, Divide>(lhs.self(), rhs.self());
    }

    // LazyExpression wrapper
    template<typename Expr>
    class LazyExpression {
        Expr expr;

    public:
        // Single constructor
        template<typename E>
        constexpr explicit LazyExpression(E&& e) : expr(std::forward<E>(e)) {}

        constexpr auto evaluate() const { return expr.evaluate(); }

        // Operator overloads
        template<typename Other>
        constexpr auto operator+(const LazyExpression<Other>& other) const {
            return LazyExpression(expr + other.expr);
        }

        template<typename Other>
        constexpr auto operator-(const LazyExpression<Other>& other) const {
            return LazyExpression(expr - other.expr);
        }

        template<typename Other>
        constexpr auto operator*(const LazyExpression<Other>& other) const {
            return LazyExpression(expr * other.expr);
        }

        template<typename Other>
        constexpr auto operator/(const LazyExpression<Other>& other) const {
            return LazyExpression(expr / other.expr);
        }
    };

    // Helper functions to create LazyExpressions
    template<typename T>
    constexpr auto make_lazy(T&& value) {
        if constexpr (std::is_base_of_v<Expression<std::remove_cv_t<std::remove_reference_t<T>>>,
            std::remove_cv_t<std::remove_reference_t<T>>>) {
            return LazyExpression<std::remove_cv_t<std::remove_reference_t<T>>>(std::forward<T>(value));
        }
        else {
            return LazyExpression<Constant<std::remove_cv_t<std::remove_reference_t<T>>>>(
                Constant<std::remove_cv_t<std::remove_reference_t<T>>>(std::forward<T>(value))
            );
        }
    }
}
*/

/*
namespace test5 {

       1. The Constant<T> class now has:
            A single constructor template that only accepts non-Expression types.
            Deleted copy and move constructors to prevent ambiguity.
       2. The BinaryExpression now takes its arguments by value and moves them into place, which avoids the reference ambiguity.
       3. The operator structs (Add, Subtract, etc.) now take their arguments by value to match the changes in BinaryExpression.
       4. The make_lazy function remains unchanged, as it was already correctly handling the creation of Constant<T> objects.

       These changes should resolve the ambiguity issues we were facing. 
       The Constant<T> class can now only be constructed from non-Expression types, 
       and the BinaryExpression avoids reference ambiguity by taking its arguments by value.

    // Forward declarations
    template<typename T>
    struct Constant;

    template<typename Left, typename Right, typename Op>
    struct BinaryExpression;

    // CRTP base class for all expressions
    template<typename Derived>
    struct Expression {
        constexpr const Derived& self() const { return static_cast<const Derived&>(*this); }
        constexpr auto evaluate() const { return self().eval_impl(); }
    };

    // Constant expression
    template<typename T>
    struct Constant : Expression<Constant<T>> {
        T value;

        // Constructor for non-Expression types
        template<typename U, typename = std::enable_if_t<!std::is_base_of_v<Expression<std::remove_cv_t<std::remove_reference_t<U>>>, std::remove_cv_t<std::remove_reference_t<U>>>>>
        explicit constexpr Constant(U&& v) : value(std::forward<U>(v)) {}

        // Deleted copy and move constructors to avoid ambiguity
        Constant(const Constant&) = delete;
        Constant(Constant&&) = delete;

        constexpr T eval_impl() const { return value; }
    };

    // Binary expression
    template<typename Left, typename Right, typename Op>
    struct BinaryExpression : Expression<BinaryExpression<Left, Right, Op>> {
        Left left;
        Right right;

        constexpr BinaryExpression(Left l, Right r) : left(std::move(l)), right(std::move(r)) {}

        constexpr auto eval_impl() const { return Op::apply(left.evaluate(), right.evaluate()); }
    };

    // Operator structs
    struct Add {
        template<typename T, typename U>
        static constexpr auto apply(T a, U b) { return a + b; }
    };

    struct Subtract {
        template<typename T, typename U>
        static constexpr auto apply(T a, U b) { return a - b; }
    };

    struct Multiply {
        template<typename T, typename U>
        static constexpr auto apply(T a, U b) { return a * b; }
    };

    struct Divide {
        template<typename T, typename U>
        static constexpr auto apply(T a, U b) { return a / b; }
    };

    // Operator overloads
    template<typename Left, typename Right>
    constexpr auto operator+(const Expression<Left>& lhs, const Expression<Right>& rhs) {
        return BinaryExpression<Left, Right, Add>(lhs.self(), rhs.self());
    }

    template<typename Left, typename Right>
    constexpr auto operator-(const Expression<Left>& lhs, const Expression<Right>& rhs) {
        return BinaryExpression<Left, Right, Subtract>(lhs.self(), rhs.self());
    }

    template<typename Left, typename Right>
    constexpr auto operator*(const Expression<Left>& lhs, const Expression<Right>& rhs) {
        return BinaryExpression<Left, Right, Multiply>(lhs.self(), rhs.self());
    }

    template<typename Left, typename Right>
    constexpr auto operator/(const Expression<Left>& lhs, const Expression<Right>& rhs) {
        return BinaryExpression<Left, Right, Divide>(lhs.self(), rhs.self());
    }

    // LazyExpression wrapper
    template<typename Expr>
    class LazyExpression {
        Expr expr;

    public:
        template<typename E>
        constexpr explicit LazyExpression(E&& e) : expr(std::forward<E>(e)) {}

        constexpr auto evaluate() const { return expr.evaluate(); }

        // Operator overloads
        template<typename Other>
        constexpr auto operator+(const LazyExpression<Other>& other) const {
            return LazyExpression(expr + other.expr);
        }

        template<typename Other>
        constexpr auto operator-(const LazyExpression<Other>& other) const {
            return LazyExpression(expr - other.expr);
        }

        template<typename Other>
        constexpr auto operator*(const LazyExpression<Other>& other) const {
            return LazyExpression(expr * other.expr);
        }

        template<typename Other>
        constexpr auto operator/(const LazyExpression<Other>& other) const {
            return LazyExpression(expr / other.expr);
        }
    };

    // Helper function to create LazyExpressions
    template<typename T>
    constexpr auto make_lazy(T&& value) {
        if constexpr (std::is_base_of_v<Expression<std::remove_cv_t<std::remove_reference_t<T>>>,
            std::remove_cv_t<std::remove_reference_t<T>>>) {
            return LazyExpression<std::remove_cv_t<std::remove_reference_t<T>>>(std::forward<T>(value));
        }
        else {
            return LazyExpression<Constant<std::remove_cv_t<std::remove_reference_t<T>>>>(
                Constant<std::remove_cv_t<std::remove_reference_t<T>>>(std::forward<T>(value))
            );
        }
    }

}
*/

#undef ET_EXPERIMENT
#ifdef ET_EXPERIMENT
namespace test5 {

    /*
       1. Reintroduced copy constructors for Constant<T> and BinaryExpression.
       2. Added move constructors for Constant<T> and BinaryExpression to support efficient moves when possible.
       3. Modified BinaryExpression to take its arguments by const reference, allowing for proper copying.
       4. Added copy and move constructors to LazyExpression to ensure it can be properly copied or moved.
       5. Reverted the operator structs to take their arguments by const reference, matching the BinaryExpression changes.

      These changes allow for proper copying of expressions while maintaining the benefits of lazy evaluation and avoiding ambiguity. 
      The LazyExpression operators can now correctly create BinaryExpression instances without running into deleted copy constructor issues.

    */

    // Forward declarations
    template<typename T>
    struct Constant;

    template<typename Left, typename Right, typename Op>
    struct BinaryExpression;

    // CRTP base class for all expressions
    template<typename Derived>
    struct Expression {
        constexpr const Derived& self() const { return static_cast<const Derived&>(*this); }
        constexpr auto evaluate() const { return self().eval_impl(); }
    };

    // Constant expression
    template<typename T>
    struct Constant : Expression<Constant<T>> {
        T value;

        // Constructor for non-Expression types
        template<typename U, typename = std::enable_if_t<!std::is_base_of_v<Expression<std::remove_cv_t<std::remove_reference_t<U>>>, std::remove_cv_t<std::remove_reference_t<U>>>>>
        explicit constexpr Constant(U&& v) : value(std::forward<U>(v)) {}

        // Copy constructor
        constexpr Constant(const Constant& other) : value(other.value) {}

        // Move constructor
        constexpr Constant(Constant&& other) noexcept : value(std::move(other.value)) {}

        constexpr T eval_impl() const { return value; }
    };

    // Binary expression
    template<typename Left, typename Right, typename Op>
    struct BinaryExpression : Expression<BinaryExpression<Left, Right, Op>> {
        Left left;
        Right right;

        constexpr BinaryExpression(const Left& l, const Right& r) : left(l), right(r) {}

        // Copy constructor
        constexpr BinaryExpression(const BinaryExpression& other) : left(other.left), right(other.right) {}

        // Move constructor
        constexpr BinaryExpression(BinaryExpression&& other) noexcept
            : left(std::move(other.left)), right(std::move(other.right)) {}

        constexpr auto eval_impl() const { return Op::apply(left.evaluate(), right.evaluate()); }
    };

    // Operator structs
    struct Add {
        template<typename T, typename U>
        static constexpr auto apply(const T& a, const U& b) { return a + b; }
    };

    struct Subtract {
        template<typename T, typename U>
        static constexpr auto apply(const T& a, const U& b) { return a - b; }
    };

    struct Multiply {
        template<typename T, typename U>
        static constexpr auto apply(const T& a, const U& b) { return a * b; }
    };

    struct Divide {
        template<typename T, typename U>
        static constexpr auto apply(const T& a, const U& b) { return a / b; }
    };

    // Operator overloads
    template<typename Left, typename Right>
    constexpr auto operator+(const Expression<Left>& lhs, const Expression<Right>& rhs) {
        return BinaryExpression<Left, Right, Add>(lhs.self(), rhs.self());
    }

    template<typename Left, typename Right>
    constexpr auto operator-(const Expression<Left>& lhs, const Expression<Right>& rhs) {
        return BinaryExpression<Left, Right, Subtract>(lhs.self(), rhs.self());
    }

    template<typename Left, typename Right>
    constexpr auto operator*(const Expression<Left>& lhs, const Expression<Right>& rhs) {
        return BinaryExpression<Left, Right, Multiply>(lhs.self(), rhs.self());
    }

    template<typename Left, typename Right>
    constexpr auto operator/(const Expression<Left>& lhs, const Expression<Right>& rhs) {
        return BinaryExpression<Left, Right, Divide>(lhs.self(), rhs.self());
    }

    // LazyExpression wrapper
    template<typename Expr>
    class LazyExpression {
        Expr expr;

    public:
        template<typename E>
        constexpr explicit LazyExpression(E&& e) : expr(std::forward<E>(e)) {}

        // Copy constructor
        constexpr LazyExpression(const LazyExpression& other) : expr(other.expr) {}

        // Move constructor
        constexpr LazyExpression(LazyExpression&& other) noexcept : expr(std::move(other.expr)) {}

        constexpr auto evaluate() const { return expr.evaluate(); }

        // Operator overloads
        template<typename Other>
        constexpr auto operator+(const LazyExpression<Other>& other) const {
            return LazyExpression(expr + other.expr);
        }

        template<typename Other>
        constexpr auto operator-(const LazyExpression<Other>& other) const {
            return LazyExpression(expr - other.expr);
        }

        template<typename Other>
        constexpr auto operator*(const LazyExpression<Other>& other) const {
            return LazyExpression(expr * other.expr);
        }

        template<typename Other>
        constexpr auto operator/(const LazyExpression<Other>& other) const {
            return LazyExpression(expr / other.expr);
        }
    };

    // Helper function to create LazyExpressions
    template<typename T>
    constexpr auto make_lazy(T&& value) {
        if constexpr (std::is_base_of_v<Expression<std::remove_cv_t<std::remove_reference_t<T>>>,
            std::remove_cv_t<std::remove_reference_t<T>>>) {
            return LazyExpression<std::remove_cv_t<std::remove_reference_t<T>>>(std::forward<T>(value));
        }
        else {
            return LazyExpression<Constant<std::remove_cv_t<std::remove_reference_t<T>>>>(
                Constant<std::remove_cv_t<std::remove_reference_t<T>>>(std::forward<T>(value))
            );
        }
    }
}
#endif

int main(int argc, char** argv)
try {
    using namespace sw::universal;

    {
        std::cout << "Test4: polymorphic lazy evaluation\n";
        test4::LazyExpression<double> a(5.0);
        test4::LazyExpression<double> b(3.0);
        auto result = (a + b) * (a - b);
        std::cout << result.evaluate();
    }

#ifdef ET_EXPERIMENT
    {
        std::cout << "Test5: expression template lazy evaluation\n";
        auto a = test5::make_lazy(5.0);
        auto b = test5::make_lazy(3.0);
        auto result = (a + b) * (a - b);
        std::cout << result.evaluate();
    }
#endif

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
