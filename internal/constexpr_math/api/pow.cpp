// pow.cpp: regression for sw::math::constexpr_math::pow (integer + general overloads)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <bit>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <limits>

#include <math/constexpr_math.hpp>

namespace cm = sw::math::constexpr_math;

// Constexpr-friendly sign-bit accessor. std::signbit isn't constexpr until
// C++23 and Universal targets C++20. Bit-cast is constexpr for trivially-
// copyable types since C++20, so we extract the sign bit directly.
constexpr bool sign_bit(double v) {
	return (std::bit_cast<std::uint64_t>(v) >> 63) != 0;
}
constexpr bool sign_bit(float v) {
	return (std::bit_cast<std::uint32_t>(v) >> 31) != 0;
}

// Helpers for asserting the IEEE-754 contract on signed zeros. `value == 0.0`
// is true for both +0 and -0, so == cannot distinguish them. These predicates
// pin the sign explicitly.
constexpr bool is_pos_zero(double v) { return v == 0.0 && !sign_bit(v); }
constexpr bool is_neg_zero(double v) { return v == 0.0 &&  sign_bit(v); }

// ============================================================================
// Compile-time correctness via static_assert
// ============================================================================

// ----------------------------------------------------------------------------
// Integer-exponent fast path (pow(T, int))
// ----------------------------------------------------------------------------

// Positive integer exponent
static_assert(cm::pow(2.0, 0)   == 1.0,    "pow(2, 0) == 1");
static_assert(cm::pow(2.0, 1)   == 2.0,    "pow(2, 1) == 2");
static_assert(cm::pow(2.0, 10)  == 1024.0, "pow(2, 10) == 1024");
static_assert(cm::pow(3.0, 4)   == 81.0,   "pow(3, 4) == 81");
static_assert(cm::pow(0.5, 8)   == 1.0/256.0, "pow(0.5, 8) == 1/256");

// Negative integer exponent
static_assert(cm::pow(2.0, -1)  == 0.5,    "pow(2, -1) == 0.5");
static_assert(cm::pow(2.0, -3)  == 0.125,  "pow(2, -3) == 0.125");
static_assert(cm::pow(2.0, -10) == 1.0/1024.0, "pow(2, -10) == 1/1024");

// Negative base, integer exponent: sign depends on parity
static_assert(cm::pow(-2.0, 3)  == -8.0,   "pow(-2, 3) == -8 (odd exponent)");
static_assert(cm::pow(-2.0, 4)  == 16.0,   "pow(-2, 4) == 16 (even exponent)");
static_assert(cm::pow(-1.0, 100) == 1.0,   "pow(-1, 100) == 1");
static_assert(cm::pow(-1.0, 101) == -1.0,  "pow(-1, 101) == -1");

// Float overload
static_assert(cm::pow(2.0f, 10) == 1024.0f, "powf(2, 10) == 1024");
static_assert(cm::pow(0.5f, 8)  == 1.0f/256.0f, "powf(0.5, 8) == 1/256");
static_assert(cm::pow(-2.0f, 5) == -32.0f, "powf(-2, 5) == -32");

// ----------------------------------------------------------------------------
// General overload pow(T, T) -- integer arguments via fast-path
// ----------------------------------------------------------------------------

// When the floating-point exponent is exactly an integer, the integer fast
// path is dispatched; results match the integer overload bit-for-bit.
static_assert(cm::pow(2.0, 10.0)  == 1024.0, "pow(2.0, 10.0) == 1024 via fast path");
static_assert(cm::pow(3.0, 4.0)   == 81.0,   "pow(3.0, 4.0) == 81 via fast path");
static_assert(cm::pow(-2.0, 3.0)  == -8.0,   "pow(-2.0, 3.0) == -8 via fast path");
static_assert(cm::pow(-1.0, 100.0) == 1.0,   "pow(-1.0, 100.0) == 1 via fast path");

// Negative-base + integer exponent must use the squaring fast path for exact
// integer-power semantics (regression for the CodeRabbit Major fix).
static_assert(cm::pow(-3.0, 5.0)  == -243.0, "pow(-3.0, 5.0) == -243 via fast path (negative base, odd exp)");
static_assert(cm::pow(-3.0, 4.0)  == 81.0,   "pow(-3.0, 4.0) == 81 via fast path (negative base, even exp)");
static_assert(cm::pow(-7.0, 3.0)  == -343.0, "pow(-7.0, 3.0) == -343 via fast path");
static_assert(cm::pow(-2.0, -3.0) == -0.125, "pow(-2.0, -3.0) == -0.125 via fast path (negative base, negative odd exp)");
static_assert(cm::pow(-2.0, -4.0) == 0.0625, "pow(-2.0, -4.0) == 0.0625 via fast path (negative base, negative even exp)");

// Float overload regression for the same fix.
static_assert(cm::pow(-3.0f, 5.0f) == -243.0f, "powf(-3.0, 5.0) == -243 via fast path");
static_assert(cm::pow(-3.0f, 4.0f) == 81.0f,   "powf(-3.0, 4.0) == 81 via fast path");

// ----------------------------------------------------------------------------
// General overload pow(T, T) -- transcendental path
// ----------------------------------------------------------------------------

// pow(2, 0.5) == sqrt(2) ~= 1.4142135623730951
constexpr double cx_sqrt2 = cm::pow(2.0, 0.5);
static_assert(cx_sqrt2 > 1.4142135 && cx_sqrt2 < 1.4142137,
              "pow(2.0, 0.5) ~= sqrt(2)");

// pow(8, 1/3) ~= 2.0 (within transcendental round-trip error)
constexpr double cx_cbrt8 = cm::pow(8.0, 1.0/3.0);
static_assert(cx_cbrt8 > 1.99999 && cx_cbrt8 < 2.00001,
              "pow(8, 1/3) ~= 2");

// ----------------------------------------------------------------------------
// IEEE-754 special-case table (C99 7.12.7.4)
// ----------------------------------------------------------------------------

// pow(x, 0) == 1 for any x including NaN, infinity
static_assert(cm::pow(0.0, 0.0)   == 1.0, "pow(0, 0) == 1");
static_assert(cm::pow(-0.0, 0.0)  == 1.0, "pow(-0, 0) == 1");
static_assert(cm::pow(2.0, 0.0)   == 1.0, "pow(2, 0) == 1");
static_assert(cm::pow(-3.5, 0.0)  == 1.0, "pow(-3.5, 0) == 1");
static_assert(cm::pow(std::numeric_limits<double>::infinity(), 0.0) == 1.0,
              "pow(+inf, 0) == 1");
static_assert(cm::pow(std::numeric_limits<double>::quiet_NaN(), 0.0) == 1.0,
              "pow(NaN, 0) == 1");

// pow(1, y) == 1 for any y including NaN, infinity
static_assert(cm::pow(1.0, 5.0) == 1.0, "pow(1, 5) == 1");
static_assert(cm::pow(1.0, std::numeric_limits<double>::infinity()) == 1.0,
              "pow(1, +inf) == 1");
static_assert(cm::pow(1.0, std::numeric_limits<double>::quiet_NaN()) == 1.0,
              "pow(1, NaN) == 1");

// NaN propagation when neither override applies
static_assert(cm::pow(std::numeric_limits<double>::quiet_NaN(), 2.0)
              != cm::pow(std::numeric_limits<double>::quiet_NaN(), 2.0),
              "pow(NaN, 2) == NaN");
static_assert(cm::pow(2.0, std::numeric_limits<double>::quiet_NaN())
              != cm::pow(2.0, std::numeric_limits<double>::quiet_NaN()),
              "pow(2, NaN) == NaN");

// pow(-1, +/-inf) == 1
static_assert(cm::pow(-1.0,  std::numeric_limits<double>::infinity()) == 1.0,
              "pow(-1, +inf) == 1");
static_assert(cm::pow(-1.0, -std::numeric_limits<double>::infinity()) == 1.0,
              "pow(-1, -inf) == 1");

// |x| < 1, +/-inf
static_assert(cm::pow(0.5,  std::numeric_limits<double>::infinity()) == 0.0,
              "pow(0.5, +inf) == 0");
static_assert(cm::pow(0.5, -std::numeric_limits<double>::infinity())
              == std::numeric_limits<double>::infinity(),
              "pow(0.5, -inf) == +inf");

// |x| > 1, +/-inf
static_assert(cm::pow(2.0,  std::numeric_limits<double>::infinity())
              == std::numeric_limits<double>::infinity(),
              "pow(2, +inf) == +inf");
static_assert(cm::pow(2.0, -std::numeric_limits<double>::infinity()) == 0.0,
              "pow(2, -inf) == 0");

// pow(+inf, y < 0) == 0;  pow(+inf, y > 0) == +inf
static_assert(cm::pow(std::numeric_limits<double>::infinity(), -1.0) == 0.0,
              "pow(+inf, -1) == 0");
static_assert(cm::pow(std::numeric_limits<double>::infinity(), 2.0)
              == std::numeric_limits<double>::infinity(),
              "pow(+inf, 2) == +inf");

// pow(-inf, ...): sign depends on exponent parity. ==-inf vs +inf is OK
// (they compare unequal) but ==0.0 cannot distinguish +0 from -0, so use the
// is_pos_zero / is_neg_zero predicates for the underflow cases.
static_assert(cm::pow(-std::numeric_limits<double>::infinity(), 3.0)
              == -std::numeric_limits<double>::infinity(),
              "pow(-inf, 3) == -inf (odd integer)");
static_assert(cm::pow(-std::numeric_limits<double>::infinity(), 4.0)
              == std::numeric_limits<double>::infinity(),
              "pow(-inf, 4) == +inf (even integer)");
static_assert(is_neg_zero(cm::pow(-std::numeric_limits<double>::infinity(), -3.0)),
              "pow(-inf, -3) == -0 (odd integer, neg) -- sign matters");
static_assert(is_pos_zero(cm::pow(-std::numeric_limits<double>::infinity(), -4.0)),
              "pow(-inf, -4) == +0 (even integer, neg) -- sign matters");

// pow(+/-0, y > 0) -- y odd integer preserves sign of zero.
// Use is_pos_zero/is_neg_zero so the sign bit is asserted explicitly.
static_assert(is_pos_zero(cm::pow(0.0, 3.0)),  "pow(+0, 3) == +0");
static_assert(is_neg_zero(cm::pow(-0.0, 3.0)), "pow(-0, 3) == -0 (odd integer preserves sign)");
static_assert(is_pos_zero(cm::pow(-0.0, 4.0)), "pow(-0, 4) == +0 (even integer drops sign)");
static_assert(is_pos_zero(cm::pow(-0.0, 2.5)), "pow(-0, 2.5) == +0 (non-integer drops sign)");

// pow(+/-0, y < 0) -- y odd integer preserves sign of infinity
static_assert(cm::pow(0.0, -1.0)
              == std::numeric_limits<double>::infinity(),
              "pow(+0, -1) == +inf");
static_assert(cm::pow(-0.0, -3.0)
              == -std::numeric_limits<double>::infinity(),
              "pow(-0, -3) == -inf (odd integer)");
static_assert(cm::pow(-0.0, -4.0)
              == std::numeric_limits<double>::infinity(),
              "pow(-0, -4) == +inf (even integer)");
static_assert(cm::pow(-0.0, -2.5)
              == std::numeric_limits<double>::infinity(),
              "pow(-0, -2.5) == +inf (non-integer)");

// pow(x < 0, finite non-integer y) == NaN
static_assert(cm::pow(-2.0, 0.5)
              != cm::pow(-2.0, 0.5),
              "pow(-2, 0.5) == NaN");
static_assert(cm::pow(-3.5, 1.5)
              != cm::pow(-3.5, 1.5),
              "pow(-3.5, 1.5) == NaN");

// ============================================================================
// Runtime cross-check vs std::pow
// ============================================================================

int main() {
	std::cout << "sw::math::constexpr_math::pow verification\n";

	int errors = 0;
	auto check = [&](const char* name, double x, double y, double our, double ref, double tol) {
		// NaN reference: expect our to also be NaN (NaN != NaN test).
		if (std::isnan(ref)) {
			if (!std::isnan(our)) {
				++errors;
				std::cout << "FAIL " << name << "  x=" << x << "  y=" << y
				          << "  our=" << our << "  ref=NaN (expected NaN)\n";
			}
			return;
		}
		// Infinite reference: require matching sign explicitly. Computing
		// (our - ref) / ref for inf-vs-inf produces NaN which compares
		// false against tol and silently passes the test.
		if (std::isinf(ref)) {
			if (!std::isinf(our) || std::signbit(our) != std::signbit(ref)) {
				++errors;
				std::cout << "FAIL " << name << "  x=" << x << "  y=" << y
				          << "  our=" << our << "  ref=" << ref
				          << " (sign or finiteness mismatch)\n";
			}
			return;
		}
		// Finite reference but non-finite our: catch silently-passing NaN/inf.
		if (!std::isfinite(our)) {
			++errors;
			std::cout << "FAIL " << name << "  x=" << x << "  y=" << y
			          << "  our=" << our << "  ref=" << ref
			          << " (our is not finite)\n";
			return;
		}
		// Both finite: compute relative error (or absolute when ref == 0).
		double err = (ref == 0.0) ? std::abs(our) : std::abs((our - ref) / ref);
		if (err > tol) {
			++errors;
			std::cout << "FAIL " << name << "  x=" << x << "  y=" << y
			          << "  our=" << std::setprecision(17) << our
			          << "  ref=" << ref
			          << "  rel-err=" << err << '\n';
		}
	};

	// Sweep of (base, exponent) pairs against std::pow.
	struct Pair { double x; double y; };
	const Pair pts[] = {
		// Integer exponents (fast path)
		{ 2.0,   10.0  }, { 2.0,  -10.0 }, { 0.5,  20.0 }, { 1.5,  7.0  },
		{ 3.0,    4.0  }, { 10.0,  3.0  }, { 7.0,  -2.0 }, { 1.1,  100.0},
		// Negative base, integer exponent
		{ -2.0,   3.0  }, { -2.0,   4.0 }, { -1.0, 7.0  }, { -1.5, 5.0  },
		// Non-integer exponent (transcendental path)
		{ 2.0,    0.5  }, { 8.0, 1.0/3.0}, { 10.0, 0.301029995663981}, // ~ log10(2)
		{ 1.5,    2.7  }, { 0.7,  -1.5  }, { 100.0,  0.5 },
		// Large dynamic range
		{ 1e10,   2.0  }, { 1e-10, 3.0  }, { 1.001, 1000.0 },
	};
	for (auto p : pts) {
		double our = cm::pow(p.x, p.y);
		double ref = std::pow(p.x, p.y);
		check("double sweep", p.x, p.y, our, ref, 1e-13);
	}

	// Float sweep
	struct PairF { float x; float y; };
	const PairF fpts[] = {
		{ 2.0f, 10.0f }, { 2.0f, -10.0f }, { 0.5f, 8.0f }, { 3.14f, 2.5f },
		{ -2.0f, 3.0f }, { -1.0f, 100.0f }, { 1.5f, 0.5f },
	};
	for (auto p : fpts) {
		float our = cm::pow(p.x, p.y);
		float ref = std::pow(p.x, p.y);
		double err = (ref == 0.0f) ? std::abs(our) : std::abs((our - ref) / ref);
		if (err > 1e-5) {  // allow a bit more for float
			++errors;
			std::cout << "FAIL float sweep  x=" << p.x << "  y=" << p.y
			          << "  our=" << our << "  ref=" << ref
			          << "  rel-err=" << err << '\n';
		}
	}

	std::cout << "constexpr_math::pow: " << (errors == 0 ? "PASS" : "FAIL")
	          << " (" << errors << " error" << (errors == 1 ? "" : "s") << ")\n";
	return (errors == 0) ? 0 : 1;
}
