#pragma once
// tracked.hpp: unified error tracking interface with automatic strategy selection
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// Tracked<T> provides a unified interface for error tracking across all
// Universal number types. The tracking strategy is automatically selected
// based on the type's capabilities via error_tracking_traits<T>.
//
// Strategy Selection:
//   - Exact:    IEEE floats, cfloat - uses two_sum/two_prod for perfect tracking
//   - Shadow:   posit, lns - uses higher-precision shadow computation
//   - Inherent: areal, interval - type natively tracks uncertainty
//
// Usage:
//   #include <universal/utility/tracked.hpp>
//
//   // Automatic strategy selection
//   Tracked<float> a = 1.0f;           // Uses TrackedExact (two_sum)
//   Tracked<posit<32,2>> b = 1.0;      // Uses TrackedShadow
//   Tracked<areal<32,8>> c = 1.0;      // Uses TrackedAreal (native ubit)
//   Tracked<interval<double>> d(1,2);  // Uses TrackedInterval (native bounds)
//
//   // Explicit strategy override
//   Tracked<float, ErrorStrategy::Shadow> e = 1.0f;  // Force shadow for float
//
//   // Common interface for all strategies
//   auto result = a + b;
//   std::cout << result.value() << '\n';
//   std::cout << result.error() << '\n';
//   std::cout << result.valid_bits() << '\n';

#include <cmath>
#include <limits>
#include <type_traits>
#include <ostream>
#include <iomanip>

#include "error_tracking_traits.hpp"
#include "tracked_exact.hpp"
#include "tracked_shadow.hpp"

namespace sw { namespace universal {

// ============================================================================
// Forward declarations for Universal types
// ============================================================================

template<unsigned nbits, unsigned es, typename bt> class areal;
template<typename Scalar> class interval;

// ============================================================================
// TrackedAreal: Wrapper for areal's native uncertainty tracking
// ============================================================================

/// TrackedAreal exposes areal's native ubit uncertainty tracking
/// with a consistent interface matching other tracked types
template<typename ArealType>
class TrackedAreal {
public:
	using value_type = ArealType;

private:
	ArealType value_;
	uint64_t op_count_;

public:
	// ========================================================================
	// Constructors
	// ========================================================================

	constexpr TrackedAreal() noexcept : value_(), op_count_(0) {}

	template<typename U>
	constexpr TrackedAreal(U v) noexcept : value_(v), op_count_(0) {}

	constexpr TrackedAreal(ArealType v, uint64_t ops) noexcept
		: value_(v), op_count_(ops) {}

	// Copy/move
	constexpr TrackedAreal(const TrackedAreal&) noexcept = default;
	constexpr TrackedAreal(TrackedAreal&&) noexcept = default;
	constexpr TrackedAreal& operator=(const TrackedAreal&) noexcept = default;
	constexpr TrackedAreal& operator=(TrackedAreal&&) noexcept = default;

	template<typename U>
	constexpr TrackedAreal& operator=(U v) noexcept {
		value_ = v;
		op_count_ = 0;
		return *this;
	}

	// ========================================================================
	// Accessors
	// ========================================================================

	constexpr ArealType value() const noexcept { return value_; }
	constexpr uint64_t operations() const noexcept { return op_count_; }
	constexpr operator ArealType() const noexcept { return value_; }

	// ========================================================================
	// Error Metrics (using areal's native ubit)
	// ========================================================================

	/// Check if value is exact (ubit = 0)
	bool is_exact() const noexcept {
		return !value_.ubit();
	}

	/// Error bound based on ubit
	/// When ubit=1, true value is in (value, next(value))
	double error() const noexcept {
		if (is_exact()) return 0.0;
		// Width to next encoding
		ArealType next_val = value_;
		++next_val;
		return std::abs(double(next_val) - double(value_));
	}

	double relative_error() const noexcept {
		double v = double(value_);
		if (std::abs(v) < std::numeric_limits<double>::min()) return 0.0;
		return error() / std::abs(v);
	}

	double valid_bits() const noexcept {
		if (is_exact()) return 53.0;
		double rel_err = relative_error();
		if (rel_err <= 0.0) return 53.0;
		return std::max(0.0, -std::log2(rel_err));
	}

	// ========================================================================
	// Arithmetic Operators
	// ========================================================================

	TrackedAreal operator+(const TrackedAreal& rhs) const {
		return TrackedAreal(value_ + rhs.value_, op_count_ + rhs.op_count_ + 1);
	}

	TrackedAreal operator-(const TrackedAreal& rhs) const {
		return TrackedAreal(value_ - rhs.value_, op_count_ + rhs.op_count_ + 1);
	}

	TrackedAreal operator-() const {
		return TrackedAreal(-value_, op_count_);
	}

	TrackedAreal operator*(const TrackedAreal& rhs) const {
		return TrackedAreal(value_ * rhs.value_, op_count_ + rhs.op_count_ + 1);
	}

	TrackedAreal operator/(const TrackedAreal& rhs) const {
		return TrackedAreal(value_ / rhs.value_, op_count_ + rhs.op_count_ + 1);
	}

	// Compound assignment
	TrackedAreal& operator+=(const TrackedAreal& rhs) { *this = *this + rhs; return *this; }
	TrackedAreal& operator-=(const TrackedAreal& rhs) { *this = *this - rhs; return *this; }
	TrackedAreal& operator*=(const TrackedAreal& rhs) { *this = *this * rhs; return *this; }
	TrackedAreal& operator/=(const TrackedAreal& rhs) { *this = *this / rhs; return *this; }

	// ========================================================================
	// Comparison
	// ========================================================================

	bool operator==(const TrackedAreal& rhs) const noexcept { return value_ == rhs.value_; }
	bool operator!=(const TrackedAreal& rhs) const noexcept { return value_ != rhs.value_; }
	bool operator<(const TrackedAreal& rhs) const noexcept { return value_ < rhs.value_; }
	bool operator<=(const TrackedAreal& rhs) const noexcept { return value_ <= rhs.value_; }
	bool operator>(const TrackedAreal& rhs) const noexcept { return value_ > rhs.value_; }
	bool operator>=(const TrackedAreal& rhs) const noexcept { return value_ >= rhs.value_; }

	// ========================================================================
	// Reporting
	// ========================================================================

	void report(std::ostream& os) const {
		os << "TrackedAreal Report:\n";
		os << "  Value:      " << double(value_) << '\n';
		os << "  Ubit:       " << (value_.ubit() ? "1 (uncertain)" : "0 (exact)") << '\n';
		os << "  Error:      " << std::scientific << error() << '\n';
		os << "  Valid bits: " << std::fixed << valid_bits() << '\n';
		os << "  Operations: " << op_count_ << '\n';
	}
};

// ============================================================================
// TrackedInterval: Wrapper for interval's native bounds tracking
// ============================================================================

/// TrackedInterval exposes interval's native bound tracking
/// with a consistent interface matching other tracked types
template<typename Scalar>
class TrackedInterval {
public:
	using value_type = interval<Scalar>;

private:
	interval<Scalar> value_;
	uint64_t op_count_;

public:
	// ========================================================================
	// Constructors
	// ========================================================================

	constexpr TrackedInterval() noexcept : value_(), op_count_(0) {}

	template<typename U>
	constexpr TrackedInterval(U v) noexcept : value_(v), op_count_(0) {}

	constexpr TrackedInterval(Scalar lo, Scalar hi) noexcept
		: value_(lo, hi), op_count_(0) {}

	constexpr TrackedInterval(interval<Scalar> v) noexcept
		: value_(v), op_count_(0) {}

	constexpr TrackedInterval(interval<Scalar> v, uint64_t ops) noexcept
		: value_(v), op_count_(ops) {}

	// Copy/move
	constexpr TrackedInterval(const TrackedInterval&) noexcept = default;
	constexpr TrackedInterval(TrackedInterval&&) noexcept = default;
	constexpr TrackedInterval& operator=(const TrackedInterval&) noexcept = default;
	constexpr TrackedInterval& operator=(TrackedInterval&&) noexcept = default;

	// ========================================================================
	// Accessors
	// ========================================================================

	constexpr interval<Scalar> value() const noexcept { return value_; }
	constexpr uint64_t operations() const noexcept { return op_count_; }
	constexpr operator interval<Scalar>() const noexcept { return value_; }

	/// Get the midpoint as the "representative" value
	Scalar midpoint() const noexcept { return value_.mid(); }

	// ========================================================================
	// Error Metrics (using interval width)
	// ========================================================================

	/// Is this a degenerate (exact) interval?
	bool is_exact() const noexcept {
		return value_.isdegenerate();
	}

	/// Error is the interval width (enclosure of all possible values)
	double error() const noexcept {
		return double(value_.width());
	}

	double relative_error() const noexcept {
		double mid = double(value_.mid());
		if (std::abs(mid) < std::numeric_limits<double>::min()) return 0.0;
		return error() / std::abs(mid);
	}

	double valid_bits() const noexcept {
		if (is_exact()) return 53.0;
		double rel_err = relative_error();
		if (rel_err <= 0.0) return 53.0;
		return std::max(0.0, -std::log2(rel_err));
	}

	// ========================================================================
	// Arithmetic Operators
	// ========================================================================

	TrackedInterval operator+(const TrackedInterval& rhs) const {
		return TrackedInterval(value_ + rhs.value_, op_count_ + rhs.op_count_ + 1);
	}

	TrackedInterval operator-(const TrackedInterval& rhs) const {
		return TrackedInterval(value_ - rhs.value_, op_count_ + rhs.op_count_ + 1);
	}

	TrackedInterval operator-() const {
		return TrackedInterval(-value_, op_count_);
	}

	TrackedInterval operator*(const TrackedInterval& rhs) const {
		return TrackedInterval(value_ * rhs.value_, op_count_ + rhs.op_count_ + 1);
	}

	TrackedInterval operator/(const TrackedInterval& rhs) const {
		return TrackedInterval(value_ / rhs.value_, op_count_ + rhs.op_count_ + 1);
	}

	// Compound assignment
	TrackedInterval& operator+=(const TrackedInterval& rhs) { *this = *this + rhs; return *this; }
	TrackedInterval& operator-=(const TrackedInterval& rhs) { *this = *this - rhs; return *this; }
	TrackedInterval& operator*=(const TrackedInterval& rhs) { *this = *this * rhs; return *this; }
	TrackedInterval& operator/=(const TrackedInterval& rhs) { *this = *this / rhs; return *this; }

	// ========================================================================
	// Comparison
	// ========================================================================

	bool operator==(const TrackedInterval& rhs) const noexcept { return value_ == rhs.value_; }
	bool operator!=(const TrackedInterval& rhs) const noexcept { return value_ != rhs.value_; }

	// ========================================================================
	// Reporting
	// ========================================================================

	void report(std::ostream& os) const {
		os << "TrackedInterval Report:\n";
		os << "  Interval:   " << value_ << '\n';
		os << "  Midpoint:   " << double(value_.mid()) << '\n';
		os << "  Width:      " << double(value_.width()) << '\n';
		os << "  Error:      " << std::scientific << error() << '\n';
		os << "  Valid bits: " << std::fixed << valid_bits() << '\n';
		os << "  Operations: " << op_count_ << '\n';
		os << "  Is exact:   " << (is_exact() ? "yes" : "no") << '\n';
	}
};

// ============================================================================
// Strategy Selection Helpers
// ============================================================================

namespace detail {

// Primary template: use Shadow strategy
template<typename T, ErrorStrategy Strategy>
struct tracked_impl {
	using type = TrackedShadow<T>;
};

// Exact strategy specialization
template<typename T>
struct tracked_impl<T, ErrorStrategy::Exact> {
	using type = TrackedExact<T>;
};

// Shadow strategy specialization (explicit)
template<typename T>
struct tracked_impl<T, ErrorStrategy::Shadow> {
	using type = TrackedShadow<T>;
};

// Bounded strategy uses TrackedInterval
template<typename T>
struct tracked_impl<T, ErrorStrategy::Bounded> {
	using type = TrackedInterval<T>;
};

// Statistical uses Shadow (could add dedicated impl later)
template<typename T>
struct tracked_impl<T, ErrorStrategy::Statistical> {
	using type = TrackedShadow<T>;
};

// Check if type is an areal
template<typename T>
struct is_areal : std::false_type {};

template<unsigned nbits, unsigned es, typename bt>
struct is_areal<areal<nbits, es, bt>> : std::true_type {};

// Check if type is an interval
template<typename T>
struct is_interval : std::false_type {};

template<typename Scalar>
struct is_interval<interval<Scalar>> : std::true_type {};

} // namespace detail

// ============================================================================
// Tracked<T> - Unified Interface
// ============================================================================

namespace detail {

// Helper to get the base type for Tracked<T>
template<typename T, ErrorStrategy Strategy, typename = void>
struct tracked_base_selector {
	// Default: use strategy-based selection
	using type = typename tracked_impl<T, Strategy>::type;
};

// Specialization for areal types
template<typename T, ErrorStrategy Strategy>
struct tracked_base_selector<T, Strategy, std::enable_if_t<is_areal<T>::value>> {
	using type = TrackedAreal<T>;
};

// Specialization for interval types
template<typename T, ErrorStrategy Strategy>
struct tracked_base_selector<T, Strategy, std::enable_if_t<is_interval<T>::value>> {
	using type = TrackedInterval<typename T::value_type>;
};

template<typename T, ErrorStrategy Strategy>
using tracked_base_t = typename tracked_base_selector<T, Strategy>::type;

} // namespace detail

/// Tracked<T> automatically selects the appropriate error tracking strategy
/// based on the type's capabilities via error_tracking_traits<T>
///
/// The strategy can be explicitly overridden via the second template parameter.
template<typename T, ErrorStrategy Strategy = error_tracking_traits<T>::default_strategy>
class Tracked : public detail::tracked_base_t<T, Strategy> {
	using Base = detail::tracked_base_t<T, Strategy>;

public:
	using Base::Base;  // Inherit constructors

	/// Get the strategy being used
	static constexpr ErrorStrategy strategy() noexcept { return Strategy; }

	/// Get strategy name as string
	static constexpr const char* strategy_name() noexcept {
		return sw::universal::strategy_name(Strategy);
	}
};

// ============================================================================
// Specialization for interval<Scalar> - use TrackedInterval directly
// ============================================================================

template<typename Scalar, ErrorStrategy Strategy>
class Tracked<interval<Scalar>, Strategy> : public TrackedInterval<Scalar> {
	using Base = TrackedInterval<Scalar>;

public:
	using Base::Base;

	// Additional constructors for interval bounds
	Tracked(Scalar lo, Scalar hi) : Base(lo, hi) {}

	static constexpr ErrorStrategy strategy() noexcept { return ErrorStrategy::Inherent; }
	static constexpr const char* strategy_name() noexcept { return "Inherent"; }
};

// ============================================================================
// Stream Operators for TrackedAreal and TrackedInterval
// ============================================================================

template<typename ArealType>
inline std::ostream& operator<<(std::ostream& os, const TrackedAreal<ArealType>& v) {
	return os << double(v.value());
}

template<typename Scalar>
inline std::ostream& operator<<(std::ostream& os, const TrackedInterval<Scalar>& v) {
	return os << v.value();
}

// ============================================================================
// Mathematical Functions for TrackedAreal
// ============================================================================

template<typename ArealType>
TrackedAreal<ArealType> abs(const TrackedAreal<ArealType>& v) {
	using std::abs;
	return TrackedAreal<ArealType>(abs(v.value()), v.operations());
}

template<typename ArealType>
TrackedAreal<ArealType> sqrt(const TrackedAreal<ArealType>& v) {
	using std::sqrt;
	return TrackedAreal<ArealType>(sqrt(v.value()), v.operations() + 1);
}

// ============================================================================
// Mathematical Functions for TrackedInterval
// ============================================================================

template<typename Scalar>
TrackedInterval<Scalar> abs(const TrackedInterval<Scalar>& v) {
	return TrackedInterval<Scalar>(abs(v.value()), v.operations());
}

template<typename Scalar>
TrackedInterval<Scalar> sqrt(const TrackedInterval<Scalar>& v) {
	return TrackedInterval<Scalar>(sqrt(v.value()), v.operations() + 1);
}

// ============================================================================
// Type Tags
// ============================================================================

template<typename T, ErrorStrategy S>
inline std::string type_tag(const Tracked<T, S>& = {}) {
	return "Tracked<" + std::string(typeid(T).name()) + ", " + strategy_name(S) + ">";
}

template<typename ArealType>
inline std::string type_tag(const TrackedAreal<ArealType>& = {}) {
	return "TrackedAreal<" + std::string(typeid(ArealType).name()) + ">";
}

template<typename Scalar>
inline std::string type_tag(const TrackedInterval<Scalar>& = {}) {
	return "TrackedInterval<" + std::string(typeid(Scalar).name()) + ">";
}

}} // namespace sw::universal
