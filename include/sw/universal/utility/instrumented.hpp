#pragma once
// instrumented.hpp: wrapper type that tracks arithmetic operations for energy analysis
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// The instrumented<T> wrapper transparently wraps any Universal number type
// and tracks all arithmetic operations performed on it. Combined with the
// energy cost models, this enables accurate energy estimation for algorithms.
//
// Usage:
//   #include <universal/utility/instrumented.hpp>
//   #include <universal/energy/occurrence_energy.hpp>
//
//   using namespace sw::universal;
//
//   // Reset global counters
//   instrumented_stats::reset();
//
//   // Use instrumented type in your algorithm
//   using Real = instrumented<cfloat<32,8>>;
//   Real a = 1.5, b = 2.5;
//   Real c = a + b;    // add tracked
//   Real d = a * b;    // mul tracked
//   Real e = sqrt(c);  // sqrt tracked
//
//   // Get operation counts
//   auto stats = instrumented_stats::snapshot();
//   std::cout << "Adds: " << stats.add << ", Muls: " << stats.mul << "\n";
//
//   // Calculate energy
//   double energy = calculateEnergy(stats, getIntelSkylakeModel(), BitWidth::bits_32);

#include <iostream>
#include <iomanip>
#include <atomic>
#include <cmath>
#include <type_traits>

#include "occurrence.hpp"

namespace sw { namespace universal {

// Forward declaration
template<typename T> class instrumented;

/// Global statistics tracker for instrumented operations
/// Thread-safe using atomic counters
class instrumented_stats {
public:
    // Operation counters (atomic for thread safety)
    static std::atomic<uint64_t> loads;
    static std::atomic<uint64_t> stores;
    static std::atomic<uint64_t> adds;
    static std::atomic<uint64_t> subs;
    static std::atomic<uint64_t> muls;
    static std::atomic<uint64_t> divs;
    static std::atomic<uint64_t> rems;
    static std::atomic<uint64_t> sqrts;
    static std::atomic<uint64_t> comparisons;
    static std::atomic<uint64_t> conversions;

    /// Reset all counters to zero
    static void reset() {
        loads.store(0, std::memory_order_relaxed);
        stores.store(0, std::memory_order_relaxed);
        adds.store(0, std::memory_order_relaxed);
        subs.store(0, std::memory_order_relaxed);
        muls.store(0, std::memory_order_relaxed);
        divs.store(0, std::memory_order_relaxed);
        rems.store(0, std::memory_order_relaxed);
        sqrts.store(0, std::memory_order_relaxed);
        comparisons.store(0, std::memory_order_relaxed);
        conversions.store(0, std::memory_order_relaxed);
    }

    /// Get a snapshot of current counts as occurrence struct
    template<typename NumberSystem = void>
    static occurrence<NumberSystem> snapshot() {
        occurrence<NumberSystem> result;
        result.load = loads.load(std::memory_order_relaxed);
        result.store = stores.load(std::memory_order_relaxed);
        result.add = adds.load(std::memory_order_relaxed);
        result.sub = subs.load(std::memory_order_relaxed);
        result.mul = muls.load(std::memory_order_relaxed);
        result.div = divs.load(std::memory_order_relaxed);
        result.rem = rems.load(std::memory_order_relaxed);
        result.sqrt = sqrts.load(std::memory_order_relaxed);
        return result;
    }

    /// Get total arithmetic operations
    static uint64_t totalArithmeticOps() {
        return adds.load(std::memory_order_relaxed) +
               subs.load(std::memory_order_relaxed) +
               muls.load(std::memory_order_relaxed) +
               divs.load(std::memory_order_relaxed) +
               rems.load(std::memory_order_relaxed) +
               sqrts.load(std::memory_order_relaxed);
    }

    /// Get total memory operations
    static uint64_t totalMemoryOps() {
        return loads.load(std::memory_order_relaxed) +
               stores.load(std::memory_order_relaxed);
    }

    /// Print statistics report
    static void report(std::ostream& ostr = std::cout) {
        ostr << "Instrumented Operation Statistics\n";
        ostr << std::string(40, '-') << "\n";
        ostr << std::left << std::setw(15) << "Operation"
             << std::right << std::setw(15) << "Count" << "\n";
        ostr << std::string(40, '-') << "\n";
        ostr << std::left << std::setw(15) << "Load"
             << std::right << std::setw(15) << loads.load() << "\n";
        ostr << std::left << std::setw(15) << "Store"
             << std::right << std::setw(15) << stores.load() << "\n";
        ostr << std::left << std::setw(15) << "Add"
             << std::right << std::setw(15) << adds.load() << "\n";
        ostr << std::left << std::setw(15) << "Sub"
             << std::right << std::setw(15) << subs.load() << "\n";
        ostr << std::left << std::setw(15) << "Mul"
             << std::right << std::setw(15) << muls.load() << "\n";
        ostr << std::left << std::setw(15) << "Div"
             << std::right << std::setw(15) << divs.load() << "\n";
        ostr << std::left << std::setw(15) << "Rem"
             << std::right << std::setw(15) << rems.load() << "\n";
        ostr << std::left << std::setw(15) << "Sqrt"
             << std::right << std::setw(15) << sqrts.load() << "\n";
        ostr << std::left << std::setw(15) << "Comparison"
             << std::right << std::setw(15) << comparisons.load() << "\n";
        ostr << std::left << std::setw(15) << "Conversion"
             << std::right << std::setw(15) << conversions.load() << "\n";
        ostr << std::string(40, '-') << "\n";
        ostr << std::left << std::setw(15) << "Total Arith"
             << std::right << std::setw(15) << totalArithmeticOps() << "\n";
        ostr << std::left << std::setw(15) << "Total Memory"
             << std::right << std::setw(15) << totalMemoryOps() << "\n";
    }

    // Recording functions (inlined for performance)
    static void recordLoad()       { loads.fetch_add(1, std::memory_order_relaxed); }
    static void recordStore()      { stores.fetch_add(1, std::memory_order_relaxed); }
    static void recordAdd()        { adds.fetch_add(1, std::memory_order_relaxed); }
    static void recordSub()        { subs.fetch_add(1, std::memory_order_relaxed); }
    static void recordMul()        { muls.fetch_add(1, std::memory_order_relaxed); }
    static void recordDiv()        { divs.fetch_add(1, std::memory_order_relaxed); }
    static void recordRem()        { rems.fetch_add(1, std::memory_order_relaxed); }
    static void recordSqrt()       { sqrts.fetch_add(1, std::memory_order_relaxed); }
    static void recordComparison() { comparisons.fetch_add(1, std::memory_order_relaxed); }
    static void recordConversion() { conversions.fetch_add(1, std::memory_order_relaxed); }
};

// Static member definitions (in header for header-only library)
inline std::atomic<uint64_t> instrumented_stats::loads{0};
inline std::atomic<uint64_t> instrumented_stats::stores{0};
inline std::atomic<uint64_t> instrumented_stats::adds{0};
inline std::atomic<uint64_t> instrumented_stats::subs{0};
inline std::atomic<uint64_t> instrumented_stats::muls{0};
inline std::atomic<uint64_t> instrumented_stats::divs{0};
inline std::atomic<uint64_t> instrumented_stats::rems{0};
inline std::atomic<uint64_t> instrumented_stats::sqrts{0};
inline std::atomic<uint64_t> instrumented_stats::comparisons{0};
inline std::atomic<uint64_t> instrumented_stats::conversions{0};

/// RAII guard for scoped statistics collection
class instrumented_scope {
public:
    instrumented_scope() { instrumented_stats::reset(); }
    ~instrumented_scope() = default;

    template<typename NumberSystem = void>
    occurrence<NumberSystem> stats() const {
        return instrumented_stats::snapshot<NumberSystem>();
    }

    void report(std::ostream& ostr = std::cout) const {
        instrumented_stats::report(ostr);
    }
};

/// Instrumented wrapper for any arithmetic type
///
/// This wrapper intercepts all arithmetic operations and records them
/// in the global instrumented_stats counters. The underlying type T
/// must support standard arithmetic operators.
///
/// @tparam T The underlying number type (e.g., cfloat<32,8>, posit<32,2>, float)
template<typename T>
class instrumented {
public:
    using value_type = T;

    // Default constructor
    instrumented() : value_{} {
        instrumented_stats::recordStore();
    }

    // Value constructor
    instrumented(const T& v) : value_(v) {
        instrumented_stats::recordStore();
    }

    // Converting constructor from arithmetic types
    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    instrumented(U v) : value_(static_cast<T>(v)) {
        instrumented_stats::recordConversion();
        instrumented_stats::recordStore();
    }

    // Copy constructor
    instrumented(const instrumented& other) : value_(other.value_) {
        instrumented_stats::recordLoad();
        instrumented_stats::recordStore();
    }

    // Move constructor
    instrumented(instrumented&& other) noexcept : value_(std::move(other.value_)) {
        instrumented_stats::recordLoad();
        instrumented_stats::recordStore();
    }

    // Copy assignment
    instrumented& operator=(const instrumented& other) {
        if (this != &other) {
            instrumented_stats::recordLoad();
            value_ = other.value_;
            instrumented_stats::recordStore();
        }
        return *this;
    }

    // Move assignment
    instrumented& operator=(instrumented&& other) noexcept {
        if (this != &other) {
            instrumented_stats::recordLoad();
            value_ = std::move(other.value_);
            instrumented_stats::recordStore();
        }
        return *this;
    }

    // Assignment from underlying type
    instrumented& operator=(const T& v) {
        value_ = v;
        instrumented_stats::recordStore();
        return *this;
    }

    // Assignment from arithmetic types
    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    instrumented& operator=(U v) {
        value_ = static_cast<T>(v);
        instrumented_stats::recordConversion();
        instrumented_stats::recordStore();
        return *this;
    }

    // Accessor for underlying value
    const T& value() const {
        instrumented_stats::recordLoad();
        return value_;
    }

    T& value() {
        instrumented_stats::recordLoad();
        return value_;
    }

    // Implicit conversion to underlying type
    operator T() const {
        instrumented_stats::recordLoad();
        return value_;
    }

    // Explicit conversion to other types
    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    explicit operator U() const {
        instrumented_stats::recordLoad();
        instrumented_stats::recordConversion();
        return static_cast<U>(value_);
    }

    // Unary operators
    instrumented operator+() const {
        instrumented_stats::recordLoad();
        return instrumented(+value_);
    }

    instrumented operator-() const {
        instrumented_stats::recordLoad();
        instrumented_stats::recordSub();  // Negation is a subtraction from zero
        return instrumented(-value_);
    }

    // Compound assignment operators
    instrumented& operator+=(const instrumented& rhs) {
        instrumented_stats::recordLoad();  // load lhs
        instrumented_stats::recordLoad();  // load rhs
        instrumented_stats::recordAdd();
        value_ += rhs.value_;
        instrumented_stats::recordStore();
        return *this;
    }

    instrumented& operator-=(const instrumented& rhs) {
        instrumented_stats::recordLoad();
        instrumented_stats::recordLoad();
        instrumented_stats::recordSub();
        value_ -= rhs.value_;
        instrumented_stats::recordStore();
        return *this;
    }

    instrumented& operator*=(const instrumented& rhs) {
        instrumented_stats::recordLoad();
        instrumented_stats::recordLoad();
        instrumented_stats::recordMul();
        value_ *= rhs.value_;
        instrumented_stats::recordStore();
        return *this;
    }

    instrumented& operator/=(const instrumented& rhs) {
        instrumented_stats::recordLoad();
        instrumented_stats::recordLoad();
        instrumented_stats::recordDiv();
        value_ /= rhs.value_;
        instrumented_stats::recordStore();
        return *this;
    }

    // Compound assignment with underlying type
    instrumented& operator+=(const T& rhs) {
        instrumented_stats::recordLoad();
        instrumented_stats::recordAdd();
        value_ += rhs;
        instrumented_stats::recordStore();
        return *this;
    }

    instrumented& operator-=(const T& rhs) {
        instrumented_stats::recordLoad();
        instrumented_stats::recordSub();
        value_ -= rhs;
        instrumented_stats::recordStore();
        return *this;
    }

    instrumented& operator*=(const T& rhs) {
        instrumented_stats::recordLoad();
        instrumented_stats::recordMul();
        value_ *= rhs;
        instrumented_stats::recordStore();
        return *this;
    }

    instrumented& operator/=(const T& rhs) {
        instrumented_stats::recordLoad();
        instrumented_stats::recordDiv();
        value_ /= rhs;
        instrumented_stats::recordStore();
        return *this;
    }

    // Pre-increment/decrement
    instrumented& operator++() {
        instrumented_stats::recordLoad();
        instrumented_stats::recordAdd();
        ++value_;
        instrumented_stats::recordStore();
        return *this;
    }

    instrumented& operator--() {
        instrumented_stats::recordLoad();
        instrumented_stats::recordSub();
        --value_;
        instrumented_stats::recordStore();
        return *this;
    }

    // Post-increment/decrement
    instrumented operator++(int) {
        instrumented_stats::recordLoad();
        instrumented temp = *this;
        instrumented_stats::recordAdd();
        ++value_;
        instrumented_stats::recordStore();
        return temp;
    }

    instrumented operator--(int) {
        instrumented_stats::recordLoad();
        instrumented temp = *this;
        instrumented_stats::recordSub();
        --value_;
        instrumented_stats::recordStore();
        return temp;
    }

private:
    T value_;

    // Allow direct access for friend functions
    template<typename U>
    friend class instrumented;

    template<typename U>
    friend instrumented<U> operator+(const instrumented<U>&, const instrumented<U>&);
    template<typename U>
    friend instrumented<U> operator-(const instrumented<U>&, const instrumented<U>&);
    template<typename U>
    friend instrumented<U> operator*(const instrumented<U>&, const instrumented<U>&);
    template<typename U>
    friend instrumented<U> operator/(const instrumented<U>&, const instrumented<U>&);
    template<typename U>
    friend bool operator==(const instrumented<U>&, const instrumented<U>&);
    template<typename U>
    friend bool operator!=(const instrumented<U>&, const instrumented<U>&);
    template<typename U>
    friend bool operator<(const instrumented<U>&, const instrumented<U>&);
    template<typename U>
    friend bool operator>(const instrumented<U>&, const instrumented<U>&);
    template<typename U>
    friend bool operator<=(const instrumented<U>&, const instrumented<U>&);
    template<typename U>
    friend bool operator>=(const instrumented<U>&, const instrumented<U>&);
    template<typename U>
    friend instrumented<U> sqrt(const instrumented<U>&);
    template<typename U>
    friend instrumented<U> abs(const instrumented<U>&);
    template<typename U>
    friend instrumented<U> fabs(const instrumented<U>&);
    template<typename U>
    friend std::ostream& operator<<(std::ostream&, const instrumented<U>&);
    template<typename U>
    friend std::istream& operator>>(std::istream&, instrumented<U>&);
};

// Binary arithmetic operators
template<typename T>
inline instrumented<T> operator+(const instrumented<T>& lhs, const instrumented<T>& rhs) {
    instrumented_stats::recordLoad();  // load lhs
    instrumented_stats::recordLoad();  // load rhs
    instrumented_stats::recordAdd();
    return instrumented<T>(lhs.value_ + rhs.value_);
}

template<typename T>
inline instrumented<T> operator-(const instrumented<T>& lhs, const instrumented<T>& rhs) {
    instrumented_stats::recordLoad();
    instrumented_stats::recordLoad();
    instrumented_stats::recordSub();
    return instrumented<T>(lhs.value_ - rhs.value_);
}

template<typename T>
inline instrumented<T> operator*(const instrumented<T>& lhs, const instrumented<T>& rhs) {
    instrumented_stats::recordLoad();
    instrumented_stats::recordLoad();
    instrumented_stats::recordMul();
    return instrumented<T>(lhs.value_ * rhs.value_);
}

template<typename T>
inline instrumented<T> operator/(const instrumented<T>& lhs, const instrumented<T>& rhs) {
    instrumented_stats::recordLoad();
    instrumented_stats::recordLoad();
    instrumented_stats::recordDiv();
    return instrumented<T>(lhs.value_ / rhs.value_);
}

// Mixed-type arithmetic (instrumented op T)
template<typename T>
inline instrumented<T> operator+(const instrumented<T>& lhs, const T& rhs) {
    instrumented_stats::recordLoad();
    instrumented_stats::recordAdd();
    return instrumented<T>(lhs.value_ + rhs);
}

template<typename T>
inline instrumented<T> operator-(const instrumented<T>& lhs, const T& rhs) {
    instrumented_stats::recordLoad();
    instrumented_stats::recordSub();
    return instrumented<T>(lhs.value_ - rhs);
}

template<typename T>
inline instrumented<T> operator*(const instrumented<T>& lhs, const T& rhs) {
    instrumented_stats::recordLoad();
    instrumented_stats::recordMul();
    return instrumented<T>(lhs.value_ * rhs);
}

template<typename T>
inline instrumented<T> operator/(const instrumented<T>& lhs, const T& rhs) {
    instrumented_stats::recordLoad();
    instrumented_stats::recordDiv();
    return instrumented<T>(lhs.value_ / rhs);
}

// Mixed-type arithmetic (T op instrumented)
template<typename T>
inline instrumented<T> operator+(const T& lhs, const instrumented<T>& rhs) {
    instrumented_stats::recordLoad();
    instrumented_stats::recordAdd();
    return instrumented<T>(lhs + rhs.value_);
}

template<typename T>
inline instrumented<T> operator-(const T& lhs, const instrumented<T>& rhs) {
    instrumented_stats::recordLoad();
    instrumented_stats::recordSub();
    return instrumented<T>(lhs - rhs.value_);
}

template<typename T>
inline instrumented<T> operator*(const T& lhs, const instrumented<T>& rhs) {
    instrumented_stats::recordLoad();
    instrumented_stats::recordMul();
    return instrumented<T>(lhs * rhs.value_);
}

template<typename T>
inline instrumented<T> operator/(const T& lhs, const instrumented<T>& rhs) {
    instrumented_stats::recordLoad();
    instrumented_stats::recordDiv();
    return instrumented<T>(lhs / rhs.value_);
}

// Comparison operators
template<typename T>
inline bool operator==(const instrumented<T>& lhs, const instrumented<T>& rhs) {
    instrumented_stats::recordLoad();
    instrumented_stats::recordLoad();
    instrumented_stats::recordComparison();
    return lhs.value_ == rhs.value_;
}

template<typename T>
inline bool operator!=(const instrumented<T>& lhs, const instrumented<T>& rhs) {
    instrumented_stats::recordLoad();
    instrumented_stats::recordLoad();
    instrumented_stats::recordComparison();
    return lhs.value_ != rhs.value_;
}

template<typename T>
inline bool operator<(const instrumented<T>& lhs, const instrumented<T>& rhs) {
    instrumented_stats::recordLoad();
    instrumented_stats::recordLoad();
    instrumented_stats::recordComparison();
    return lhs.value_ < rhs.value_;
}

template<typename T>
inline bool operator>(const instrumented<T>& lhs, const instrumented<T>& rhs) {
    instrumented_stats::recordLoad();
    instrumented_stats::recordLoad();
    instrumented_stats::recordComparison();
    return lhs.value_ > rhs.value_;
}

template<typename T>
inline bool operator<=(const instrumented<T>& lhs, const instrumented<T>& rhs) {
    instrumented_stats::recordLoad();
    instrumented_stats::recordLoad();
    instrumented_stats::recordComparison();
    return lhs.value_ <= rhs.value_;
}

template<typename T>
inline bool operator>=(const instrumented<T>& lhs, const instrumented<T>& rhs) {
    instrumented_stats::recordLoad();
    instrumented_stats::recordLoad();
    instrumented_stats::recordComparison();
    return lhs.value_ >= rhs.value_;
}

// Math functions
template<typename T>
inline instrumented<T> sqrt(const instrumented<T>& x) {
    instrumented_stats::recordLoad();
    instrumented_stats::recordSqrt();
    using std::sqrt;
    return instrumented<T>(sqrt(x.value_));
}

template<typename T>
inline instrumented<T> abs(const instrumented<T>& x) {
    instrumented_stats::recordLoad();
    using std::abs;
    return instrumented<T>(abs(x.value_));
}

template<typename T>
inline instrumented<T> fabs(const instrumented<T>& x) {
    instrumented_stats::recordLoad();
    using std::fabs;
    return instrumented<T>(fabs(x.value_));
}

// Stream operators
template<typename T>
inline std::ostream& operator<<(std::ostream& ostr, const instrumented<T>& x) {
    instrumented_stats::recordLoad();
    return ostr << x.value_;
}

template<typename T>
inline std::istream& operator>>(std::istream& istr, instrumented<T>& x) {
    istr >> x.value_;
    instrumented_stats::recordStore();
    return istr;
}

// Type traits for instrumented types
template<typename T>
struct is_instrumented : std::false_type {};

template<typename T>
struct is_instrumented<instrumented<T>> : std::true_type {};

template<typename T>
inline constexpr bool is_instrumented_v = is_instrumented<T>::value;

// Extract underlying type from instrumented
template<typename T>
struct underlying_type { using type = T; };

template<typename T>
struct underlying_type<instrumented<T>> { using type = T; };

template<typename T>
using underlying_type_t = typename underlying_type<T>::type;

}} // namespace sw::universal
