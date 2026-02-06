#pragma once
// tracked_lns.hpp: specialized error tracker for Logarithmic Number System
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// TrackedLNS<T> provides specialized error tracking for the Logarithmic Number
// System (LNS) that exploits a key property: MULTIPLICATION IS EXACT in LNS.
//
// In LNS, numbers are stored as logarithms:
//   x is stored as log(x)
//
// This means:
//   - Multiplication: log(a*b) = log(a) + log(b)  → EXACT (addition in log domain)
//   - Division:       log(a/b) = log(a) - log(b)  → EXACT (subtraction in log domain)
//   - Addition:       log(a+b) requires exp/log   → INTRODUCES ERROR
//   - Subtraction:    log(a-b) requires exp/log   → INTRODUCES ERROR
//
// TrackedLNS exploits this by:
//   1. NOT accumulating error on multiplication/division (they're exact)
//   2. Only tracking error on addition/subtraction
//   3. Detecting catastrophic cancellation when a ≈ -b
//   4. Separately counting mults vs adds for algorithm analysis
//
// Usage:
//   #include <universal/utility/tracked_lns.hpp>
//   #include <universal/number/lns/lns.hpp>
//
//   using namespace sw::universal;
//
//   TrackedLNS<lns<32,8>> a = 2.0;
//   TrackedLNS<lns<32,8>> b = 3.0;
//
//   auto c = a * b;  // EXACT! No error introduced
//   auto d = a + b;  // Error tracked
//
//   c.report(std::cout);
//   // Shows: additions=0, multiplications=1, error from mult = 0

#include <cmath>
#include <limits>
#include <type_traits>
#include <ostream>
#include <iomanip>

#include "error_tracking_traits.hpp"

namespace sw { namespace universal {

// Forward declaration
template<unsigned nbits, unsigned rbits, typename bt, auto...x> class lns;

// ============================================================================
// TrackedLNS Class
// ============================================================================

/// TrackedLNS provides specialized error tracking for LNS types
/// exploiting the fact that multiplication is exact in log representation
template<typename LNSType, typename ShadowType = double>
class TrackedLNS {
public:
	using value_type = LNSType;
	using shadow_type = ShadowType;

private:
	LNSType value_;           ///< The computed value in LNS
	ShadowType shadow_;       ///< Higher-precision shadow for reference
	double add_error_;        ///< Cumulative error from additions only
	uint64_t adds_;           ///< Number of additions (error source)
	uint64_t mults_;          ///< Number of multiplications (exact)
	uint64_t divs_;           ///< Number of divisions (exact)
	uint64_t cancellations_;  ///< Near-cancellation events detected
	uint64_t absorptions_;    ///< Absorption events detected (small operand swallowed)

	/// Detect absorption in shadow space: when smaller operand loses significant bits
	/// Returns the number of bits lost (0 if no significant absorption)
	static double detect_absorption(ShadowType a, ShadowType b, ShadowType result) noexcept {
		if (result == ShadowType(0)) return 0.0;
		ShadowType larger = std::abs(a) > std::abs(b) ? std::abs(a) : std::abs(b);
		ShadowType smaller = std::abs(a) < std::abs(b) ? std::abs(a) : std::abs(b);
		if (smaller == ShadowType(0) || larger == ShadowType(0)) return 0.0;

		double magnitude_ratio = static_cast<double>(larger) / static_cast<double>(smaller);
		if (magnitude_ratio <= 1.0) return 0.0;

		double bits_lost = std::log2(magnitude_ratio);

		// Only count as absorption if more than half the mantissa bits are lost
		constexpr double absorption_threshold = std::numeric_limits<ShadowType>::digits / 2.0;
		if (bits_lost > absorption_threshold) {
			return bits_lost;
		}
		return 0.0;
	}

public:
	// ========================================================================
	// Constructors
	// ========================================================================

	constexpr TrackedLNS() noexcept
		: value_(), shadow_(0), add_error_(0), adds_(0), mults_(0), divs_(0)
		, cancellations_(0), absorptions_(0) {}

	template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
	constexpr TrackedLNS(U v) noexcept
		: value_(v), shadow_(static_cast<ShadowType>(v))
		, add_error_(0), adds_(0), mults_(0), divs_(0), cancellations_(0), absorptions_(0) {}

	constexpr TrackedLNS(LNSType v) noexcept
		: value_(v), shadow_(static_cast<ShadowType>(double(v)))
		, add_error_(0), adds_(0), mults_(0), divs_(0), cancellations_(0), absorptions_(0) {}

	// Internal constructor with all state
	constexpr TrackedLNS(LNSType v, ShadowType s, double err,
	                     uint64_t a, uint64_t m, uint64_t d, uint64_t c, uint64_t ab = 0) noexcept
		: value_(v), shadow_(s), add_error_(err)
		, adds_(a), mults_(m), divs_(d), cancellations_(c), absorptions_(ab) {}

	// Copy/move
	constexpr TrackedLNS(const TrackedLNS&) noexcept = default;
	constexpr TrackedLNS(TrackedLNS&&) noexcept = default;
	constexpr TrackedLNS& operator=(const TrackedLNS&) noexcept = default;
	constexpr TrackedLNS& operator=(TrackedLNS&&) noexcept = default;

	template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
	constexpr TrackedLNS& operator=(U v) noexcept {
		value_ = v;
		shadow_ = static_cast<ShadowType>(v);
		add_error_ = 0;
		adds_ = mults_ = divs_ = cancellations_ = absorptions_ = 0;
		return *this;
	}

	// ========================================================================
	// Accessors
	// ========================================================================

	constexpr LNSType value() const noexcept { return value_; }
	constexpr ShadowType shadow() const noexcept { return shadow_; }
	constexpr operator LNSType() const noexcept { return value_; }

	// ========================================================================
	// Operation Counts (LNS-specific)
	// ========================================================================

	/// Number of additions (the only error source in LNS)
	constexpr uint64_t additions() const noexcept { return adds_; }

	/// Number of multiplications (exact in LNS!)
	constexpr uint64_t multiplications() const noexcept { return mults_; }

	/// Number of divisions (exact in LNS!)
	constexpr uint64_t divisions() const noexcept { return divs_; }

	/// Number of near-cancellation events (a ≈ -b)
	constexpr uint64_t cancellations() const noexcept { return cancellations_; }

	/// Number of absorption events (small operand swallowed)
	constexpr uint64_t absorptions() const noexcept { return absorptions_; }

	/// Did any absorption occur?
	constexpr bool had_absorption() const noexcept { return absorptions_ > 0; }

	/// Total operations
	constexpr uint64_t operations() const noexcept { return adds_ + mults_ + divs_; }

	/// Exact operations (mult + div in LNS)
	constexpr uint64_t exact_operations() const noexcept { return mults_ + divs_; }

	// ========================================================================
	// Error Metrics
	// ========================================================================

	/// Total error (difference from shadow)
	double error() const noexcept {
		return std::abs(shadow_ - static_cast<ShadowType>(double(value_)));
	}

	/// Error accumulated only from additions
	double addition_error() const noexcept {
		return add_error_;
	}

	/// Relative error
	double relative_error() const noexcept {
		if (std::abs(shadow_) < std::numeric_limits<double>::min()) return 0.0;
		return error() / std::abs(static_cast<double>(shadow_));
	}

	/// Valid bits remaining, capped at type precision
	double valid_bits() const noexcept {
		// Use nbits as precision proxy for LNS types
		constexpr double type_precision = static_cast<double>(
			error_tracking_traits<LNSType>::nbits > 0 ? error_tracking_traits<LNSType>::nbits : 53);
		double rel_err = relative_error();
		if (rel_err <= 0.0) return type_precision;
		return std::min(type_precision, std::max(0.0, -std::log2(rel_err)));
	}

	/// Is the result exact? (Only possible if no additions were performed)
	bool is_exact() const noexcept {
		return adds_ == 0 && error() == 0.0;
	}

	/// Was there catastrophic cancellation?
	bool had_cancellation() const noexcept {
		return cancellations_ > 0;
	}

	// ========================================================================
	// Arithmetic Operators
	// ========================================================================

	/// Addition: THE ONLY SOURCE OF ERROR in LNS
	/// Also detects near-cancellation when a ≈ -b and absorption when |a| >> |b|
	TrackedLNS operator+(const TrackedLNS& rhs) const {
		LNSType result = value_ + rhs.value_;
		ShadowType exact = shadow_ + rhs.shadow_;

		// Compute error introduced by this addition
		double this_error = std::abs(exact - static_cast<ShadowType>(double(result)));
		double total_add_error = add_error_ + rhs.add_error_ + this_error;

		// Detect near-cancellation (when result is much smaller than operands)
		uint64_t cancel_count = cancellations_ + rhs.cancellations_;
		if (shadow_ != 0 && rhs.shadow_ != 0) {
			double ratio = shadow_ / rhs.shadow_;
			// If ratio is close to -1, we have near-cancellation
			if (ratio < 0 && std::abs(1.0 + ratio) < 0.1) {
				++cancel_count;
			}
		}

		// Detect absorption (small operand swallowed by large one)
		uint64_t absorb_count = absorptions_ + rhs.absorptions_;
		double bits_lost = detect_absorption(shadow_, rhs.shadow_, exact);
		if (bits_lost > 0.0) {
			++absorb_count;
		}

		return TrackedLNS(result, exact, total_add_error,
		                  adds_ + rhs.adds_ + 1,
		                  mults_ + rhs.mults_,
		                  divs_ + rhs.divs_,
		                  cancel_count,
		                  absorb_count);
	}

	/// Subtraction: Also introduces error (like addition)
	/// Detects cancellation when a ≈ b and absorption when |a| >> |b|
	TrackedLNS operator-(const TrackedLNS& rhs) const {
		LNSType result = value_ - rhs.value_;
		ShadowType exact = shadow_ - rhs.shadow_;

		double this_error = std::abs(exact - static_cast<ShadowType>(double(result)));
		double total_add_error = add_error_ + rhs.add_error_ + this_error;

		// Detect near-cancellation
		uint64_t cancel_count = cancellations_ + rhs.cancellations_;
		if (shadow_ != 0 && rhs.shadow_ != 0) {
			double ratio = shadow_ / rhs.shadow_;
			// If ratio is close to 1, subtraction causes cancellation
			if (ratio > 0 && std::abs(1.0 - ratio) < 0.1) {
				++cancel_count;
			}
		}

		// Detect absorption
		uint64_t absorb_count = absorptions_ + rhs.absorptions_;
		double bits_lost = detect_absorption(shadow_, rhs.shadow_, exact);
		if (bits_lost > 0.0) {
			++absorb_count;
		}

		return TrackedLNS(result, exact, total_add_error,
		                  adds_ + rhs.adds_ + 1,  // Subtraction counts as add
		                  mults_ + rhs.mults_,
		                  divs_ + rhs.divs_,
		                  cancel_count,
		                  absorb_count);
	}

	/// Unary minus: No error (just sign flip, preserves absorptions)
	TrackedLNS operator-() const {
		return TrackedLNS(-value_, -shadow_, add_error_,
		                  adds_, mults_, divs_, cancellations_, absorptions_);
	}

	/// Multiplication: EXACT in LNS! No error introduced.
	/// This is the key advantage of LNS for multiply-heavy algorithms.
	TrackedLNS operator*(const TrackedLNS& rhs) const {
		LNSType result = value_ * rhs.value_;
		ShadowType exact = shadow_ * rhs.shadow_;

		// No new error from multiplication - it's exact in LNS!
		// Only propagate existing addition errors
		return TrackedLNS(result, exact,
		                  add_error_ + rhs.add_error_,  // Propagate, don't add
		                  adds_ + rhs.adds_,
		                  mults_ + rhs.mults_ + 1,      // Count the mult
		                  divs_ + rhs.divs_,
		                  cancellations_ + rhs.cancellations_,
		                  absorptions_ + rhs.absorptions_);
	}

	/// Division: EXACT in LNS! No error introduced.
	TrackedLNS operator/(const TrackedLNS& rhs) const {
		LNSType result = value_ / rhs.value_;
		ShadowType exact = shadow_ / rhs.shadow_;

		// No new error from division - it's exact in LNS!
		return TrackedLNS(result, exact,
		                  add_error_ + rhs.add_error_,
		                  adds_ + rhs.adds_,
		                  mults_ + rhs.mults_,
		                  divs_ + rhs.divs_ + 1,        // Count the div
		                  cancellations_ + rhs.cancellations_,
		                  absorptions_ + rhs.absorptions_);
	}

	// Scalar operations
	template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
	TrackedLNS operator+(U rhs) const { return *this + TrackedLNS(rhs); }

	template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
	TrackedLNS operator-(U rhs) const { return *this - TrackedLNS(rhs); }

	template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
	TrackedLNS operator*(U rhs) const { return *this * TrackedLNS(rhs); }

	template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
	TrackedLNS operator/(U rhs) const { return *this / TrackedLNS(rhs); }

	// ========================================================================
	// Compound Assignment Operators
	// ========================================================================

	TrackedLNS& operator+=(const TrackedLNS& rhs) { *this = *this + rhs; return *this; }
	TrackedLNS& operator-=(const TrackedLNS& rhs) { *this = *this - rhs; return *this; }
	TrackedLNS& operator*=(const TrackedLNS& rhs) { *this = *this * rhs; return *this; }
	TrackedLNS& operator/=(const TrackedLNS& rhs) { *this = *this / rhs; return *this; }

	template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
	TrackedLNS& operator+=(U rhs) { *this = *this + TrackedLNS(rhs); return *this; }

	template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
	TrackedLNS& operator-=(U rhs) { *this = *this - TrackedLNS(rhs); return *this; }

	template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
	TrackedLNS& operator*=(U rhs) { *this = *this * TrackedLNS(rhs); return *this; }

	template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
	TrackedLNS& operator/=(U rhs) { *this = *this / TrackedLNS(rhs); return *this; }

	// ========================================================================
	// Comparison Operators
	// ========================================================================

	bool operator==(const TrackedLNS& rhs) const noexcept { return value_ == rhs.value_; }
	bool operator!=(const TrackedLNS& rhs) const noexcept { return value_ != rhs.value_; }
	bool operator<(const TrackedLNS& rhs) const noexcept { return value_ < rhs.value_; }
	bool operator<=(const TrackedLNS& rhs) const noexcept { return value_ <= rhs.value_; }
	bool operator>(const TrackedLNS& rhs) const noexcept { return value_ > rhs.value_; }
	bool operator>=(const TrackedLNS& rhs) const noexcept { return value_ >= rhs.value_; }

	// ========================================================================
	// Reporting
	// ========================================================================

	void report(std::ostream& os) const {
		os << "TrackedLNS Report:\n";
		os << "  Value:          " << std::setprecision(15) << double(value_) << '\n';
		os << "  Shadow:         " << shadow_ << '\n';
		os << "  Total Error:    " << std::scientific << error() << '\n';
		os << "  Addition Error: " << add_error_ << '\n';
		os << "  Rel Error:      " << relative_error() << '\n';
		os << "  Valid bits:     " << std::fixed << std::setprecision(1) << valid_bits() << '\n';
		os << "  Operations:\n";
		os << "    Additions:       " << adds_ << " (error source)\n";
		os << "    Multiplications: " << mults_ << " (EXACT)\n";
		os << "    Divisions:       " << divs_ << " (EXACT)\n";
		os << "    Total:           " << operations() << '\n';
		os << "  Exact ops ratio:   " << std::setprecision(1)
		   << (operations() > 0 ? 100.0 * exact_operations() / operations() : 100.0) << "%\n";
		os << "  Cancellations:     " << cancellations_ << '\n';
		os << "  Absorptions:       " << absorptions_ << '\n';
		os << "  Is exact:          " << (is_exact() ? "yes" : "no") << '\n';
	}
};

// ============================================================================
// Stream Operators
// ============================================================================

template<typename LNSType, typename S>
inline std::ostream& operator<<(std::ostream& os, const TrackedLNS<LNSType, S>& v) {
	return os << double(v.value());
}

// ============================================================================
// Free Function Mathematical Operations
// ============================================================================

/// Absolute value (no error, preserves absorptions)
template<typename LNSType, typename S>
TrackedLNS<LNSType, S> abs(const TrackedLNS<LNSType, S>& v) {
	using std::abs;
	LNSType val = v.value();
	LNSType abs_val = (double(val) < 0) ? -val : val;
	S abs_shadow = std::abs(v.shadow());
	return TrackedLNS<LNSType, S>(abs_val, abs_shadow, v.addition_error(),
	                               v.additions(), v.multiplications(),
	                               v.divisions(), v.cancellations(), v.absorptions());
}

/// Square root (introduces error like addition, preserves absorptions)
template<typename LNSType, typename S>
TrackedLNS<LNSType, S> sqrt(const TrackedLNS<LNSType, S>& v) {
	using std::sqrt;
	LNSType result = sqrt(v.value());
	S exact = sqrt(v.shadow());
	double this_error = std::abs(exact - static_cast<S>(double(result)));
	return TrackedLNS<LNSType, S>(result, exact,
	                               v.addition_error() + this_error,
	                               v.additions() + 1,  // sqrt counts as error-introducing
	                               v.multiplications(),
	                               v.divisions(),
	                               v.cancellations(),
	                               v.absorptions());
}

/// Square (EXACT - just multiplication)
template<typename LNSType, typename S>
TrackedLNS<LNSType, S> sqr(const TrackedLNS<LNSType, S>& v) {
	return v * v;
}

/// Power with integer exponent (EXACT - repeated multiplication)
template<typename LNSType, typename S>
TrackedLNS<LNSType, S> pow(const TrackedLNS<LNSType, S>& base, int exp) {
	if (exp == 0) return TrackedLNS<LNSType, S>(1.0);
	if (exp == 1) return base;

	TrackedLNS<LNSType, S> result = base;
	int n = std::abs(exp);
	for (int i = 1; i < n; ++i) {
		result = result * base;  // EXACT!
	}
	if (exp < 0) {
		result = TrackedLNS<LNSType, S>(1.0) / result;  // EXACT!
	}
	return result;
}

// ============================================================================
// Scalar + TrackedLNS operations
// ============================================================================

template<typename U, typename LNSType, typename S,
         typename = std::enable_if_t<std::is_arithmetic_v<U>>>
TrackedLNS<LNSType, S> operator+(U lhs, const TrackedLNS<LNSType, S>& rhs) {
	return TrackedLNS<LNSType, S>(lhs) + rhs;
}

template<typename U, typename LNSType, typename S,
         typename = std::enable_if_t<std::is_arithmetic_v<U>>>
TrackedLNS<LNSType, S> operator-(U lhs, const TrackedLNS<LNSType, S>& rhs) {
	return TrackedLNS<LNSType, S>(lhs) - rhs;
}

template<typename U, typename LNSType, typename S,
         typename = std::enable_if_t<std::is_arithmetic_v<U>>>
TrackedLNS<LNSType, S> operator*(U lhs, const TrackedLNS<LNSType, S>& rhs) {
	return TrackedLNS<LNSType, S>(lhs) * rhs;
}

template<typename U, typename LNSType, typename S,
         typename = std::enable_if_t<std::is_arithmetic_v<U>>>
TrackedLNS<LNSType, S> operator/(U lhs, const TrackedLNS<LNSType, S>& rhs) {
	return TrackedLNS<LNSType, S>(lhs) / rhs;
}

// ============================================================================
// Type tag
// ============================================================================

template<typename LNSType, typename S>
inline std::string type_tag(const TrackedLNS<LNSType, S>& = {}) {
	return "TrackedLNS<" + std::string(typeid(LNSType).name()) + ">";
}

}} // namespace sw::universal
