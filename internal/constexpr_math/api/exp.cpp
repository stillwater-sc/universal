// exp.cpp: regression for sw::math::constexpr_math::exp (natural exponential)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cmath>
#include <iostream>
#include <iomanip>
#include <limits>

#include <math/constexpr_math.hpp>

namespace cm = sw::math::constexpr_math;

// ============================================================================
// Compile-time correctness via static_assert
// ============================================================================

// exp(0) == 1 exactly (the exp2(0) == 1 base case propagates)
static_assert(cm::exp(0.0)  == 1.0,  "exp(0) == 1");
static_assert(cm::exp(0.0f) == 1.0f, "expf(0) == 1");

// exp(1) ~= e = 2.71828182845904523536
constexpr double cx_e = cm::exp(1.0);
static_assert(cx_e > 2.71828182 && cx_e < 2.71828183,
              "exp(1) ~= 2.71828182845904523536");

// exp(-1) ~= 1/e = 0.36787944117144232
constexpr double cx_inv_e = cm::exp(-1.0);
static_assert(cx_inv_e > 0.36787944 && cx_inv_e < 0.36787945,
              "exp(-1) ~= 0.367879441171442");

// exp(ln(2)) ~= 2  (cross-check via the LN2 constant in detail::)
constexpr double cx_2 = cm::exp(cm::detail::LN2);
static_assert(cx_2 > 1.9999999 && cx_2 < 2.0000001,
              "exp(ln(2)) ~= 2");

// log(exp(x)) ~= x  (round-trip with log -- the typical encode/decode loop)
constexpr double cx_rt_pi = cm::log(cm::exp(3.14159265358979323846));
static_assert(cx_rt_pi > 3.1415926 && cx_rt_pi < 3.1415927,
              "log(exp(pi)) ~= pi");

// Special values
static_assert(cm::exp(std::numeric_limits<double>::infinity())
              == std::numeric_limits<double>::infinity(),
              "exp(+inf) == +inf");
static_assert(cm::exp(-std::numeric_limits<double>::infinity()) == 0.0,
              "exp(-inf) == 0");
static_assert(cm::exp(std::numeric_limits<double>::quiet_NaN())
              != cm::exp(std::numeric_limits<double>::quiet_NaN()),
              "exp(NaN) == NaN");

// Float specials
static_assert(cm::exp(std::numeric_limits<float>::infinity())
              == std::numeric_limits<float>::infinity(),
              "expf(+inf) == +inf");
static_assert(cm::exp(-std::numeric_limits<float>::infinity()) == 0.0f,
              "expf(-inf) == 0");
static_assert(cm::exp(std::numeric_limits<float>::quiet_NaN())
              != cm::exp(std::numeric_limits<float>::quiet_NaN()),
              "expf(NaN) == NaN");

// Overflow / underflow saturation (at the bounds of double's exponent range)
//   exp(710) -> +inf  (since 710 * log2(e) > 1024)
//   exp(-746) -> 0    (since -746 * log2(e) < -1075)
static_assert(cm::exp(710.0)  == std::numeric_limits<double>::infinity(),
              "exp(710) saturates to +inf");
static_assert(cm::exp(-746.0) == 0.0, "exp(-746) underflows to 0");

// Off-by-one regression guards: pin values just inside the true saturation
// boundaries to catch a future tightening of the saturation thresholds.
// True top boundary: x ~= 709.78 (= 1024 / LOG2E). At x = 709,
//   x * LOG2E ~= 1022.83, comfortably inside exp2's finite range.
// True bottom boundary: x ~= -744.44 (= log(smallest subnormal)). At x = -744,
//   the result is a positive subnormal just above denorm_min.
static_assert(cm::exp(709.0) < std::numeric_limits<double>::infinity()
           && cm::exp(709.0) > 0.0,
              "exp(709) is finite and positive (off-by-one guard for top saturation)");
static_assert(cm::exp(-744.0) > 0.0,
              "exp(-744) is positive subnormal (off-by-one guard for bottom underflow)");

// ============================================================================
// Runtime cross-check vs std::exp
// ============================================================================

int main() {
	std::cout << "sw::math::constexpr_math::exp verification\n";

	int errors = 0;
	// Generic lambda (C++20 auto-parameters) so float and double sweeps share
	// the same failure-printing path with one tolerance type per call.
	auto check = [&](const char* name, auto x, auto our, auto ref, auto tol) {
		auto err = (ref == decltype(ref){0}) ? std::abs(our)
		                                     : std::abs((our - ref) / ref);
		if (err > tol) {
			++errors;
			std::cout << "FAIL " << name
			          << "  x=" << std::setprecision(17) << x
			          << "  our=" << our
			          << "  ref=" << ref
			          << "  rel-err=" << err << '\n';
		}
	};

	// Sweep across a representative range.
	const double points[] = {
		-700.0, -100.0, -10.0, -3.0, -1.5, -0.5, -0.0001,
		 0.0,    0.0001, 0.5,   1.5,  3.0,   10.0, 100.0, 700.0,
		 0.6931471805599453,  // ln(2) -> should give 2
		 2.302585092994046,   // ln(10) -> should give 10
	};
	for (double x : points) {
		double our = cm::exp(x);
		double ref = std::exp(x);
		// At extreme |x| (e.g. +/-700), the x * LOG2E multiply is sub-ulp
		// accurate but the magnitude (~1010) amplifies the rounding error to
		// ~3e-14. 1e-13 leaves comfortable margin without masking algorithmic
		// regressions.
		check("double sweep", x, our, ref, 1e-13);
	}

	// Round-trip: log(exp(x)) ~= x
	const double rt_points[] = {
		-50.0, -10.0, -1.0, -0.1, 0.0, 0.1, 1.0, 10.0, 50.0, 100.0,
	};
	for (double x : rt_points) {
		double rt = cm::log(cm::exp(x));
		check("round-trip log(exp(x))", x, rt, x, 1e-13);
	}

	// Float sweep -- shares the generic check lambda above.
	const float fpoints[] = {
		-80.0f, -10.0f, -1.0f, 0.0f, 1.0f, 2.71828183f, 10.0f, 80.0f,
	};
	for (float x : fpoints) {
		float our = cm::exp(x);
		float ref = std::exp(x);
		check("float sweep", x, our, ref, 1e-6f);
	}

	std::cout << "constexpr_math::exp: " << (errors == 0 ? "PASS" : "FAIL")
	          << " (" << errors << " error" << (errors == 1 ? "" : "s") << ")\n";
	return (errors == 0) ? 0 : 1;
}
