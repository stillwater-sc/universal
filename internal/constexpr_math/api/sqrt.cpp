// sqrt.cpp: regression for sw::math::constexpr_math::sqrt(float|double)
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

// Perfect squares: must be bit-exact (Newton converges to the exact answer).
static_assert(cm::sqrt(0.0)    == 0.0,   "sqrt(0) == 0");
static_assert(cm::sqrt(1.0)    == 1.0,   "sqrt(1) == 1");
static_assert(cm::sqrt(4.0)    == 2.0,   "sqrt(4) == 2");
static_assert(cm::sqrt(9.0)    == 3.0,   "sqrt(9) == 3");
static_assert(cm::sqrt(16.0)   == 4.0,   "sqrt(16) == 4");
static_assert(cm::sqrt(25.0)   == 5.0,   "sqrt(25) == 5");
static_assert(cm::sqrt(100.0)  == 10.0,  "sqrt(100) == 10");
static_assert(cm::sqrt(10000.0) == 100.0, "sqrt(10000) == 100");
static_assert(cm::sqrt(0.25)   == 0.5,   "sqrt(0.25) == 0.5");

// Float overload, perfect squares
static_assert(cm::sqrt(1.0f)   == 1.0f,  "sqrtf(1) == 1");
static_assert(cm::sqrt(4.0f)   == 2.0f,  "sqrtf(4) == 2");
static_assert(cm::sqrt(81.0f)  == 9.0f,  "sqrtf(81) == 9");

// Non-perfect squares within a few ulp
constexpr double cx_sqrt2 = cm::sqrt(2.0);
static_assert(cx_sqrt2 > 1.4142135623 && cx_sqrt2 < 1.4142135624,
              "sqrt(2) ~= 1.4142135623730951");

constexpr double cx_sqrt3 = cm::sqrt(3.0);
static_assert(cx_sqrt3 > 1.7320508075 && cx_sqrt3 < 1.7320508076,
              "sqrt(3) ~= 1.7320508075688772");

// Cross-check with detail::SQRT2 constant
static_assert(cm::sqrt(2.0) > cm::detail::SQRT2 - 1e-15
           && cm::sqrt(2.0) < cm::detail::SQRT2 + 1e-15,
              "sqrt(2) matches detail::SQRT2 within machine epsilon");

// Special values
static_assert(cm::sqrt(std::numeric_limits<double>::infinity())
              == std::numeric_limits<double>::infinity(),
              "sqrt(+inf) == +inf");
static_assert(cm::sqrt(-1.0) != cm::sqrt(-1.0), "sqrt(-1) == NaN");
static_assert(cm::sqrt(-2.5f) != cm::sqrt(-2.5f), "sqrtf(-2.5) == NaN");
static_assert(cm::sqrt(-std::numeric_limits<double>::infinity())
              != cm::sqrt(-std::numeric_limits<double>::infinity()),
              "sqrt(-inf) == NaN");
static_assert(cm::sqrt(std::numeric_limits<double>::quiet_NaN())
              != cm::sqrt(std::numeric_limits<double>::quiet_NaN()),
              "sqrt(NaN) == NaN");

// Signed-zero preservation: sqrt(-0) == -0 per IEEE-754, NOT NaN.
static_assert(cm::sqrt(-0.0)  == -0.0, "sqrt(-0) == -0");
static_assert(cm::sqrt(-0.0f) == -0.0f, "sqrtf(-0) == -0");

// Wide dynamic range
constexpr double cx_sqrt_big = cm::sqrt(1e100);
static_assert(cx_sqrt_big > 9.9999e49 && cx_sqrt_big < 1.0001e50,
              "sqrt(1e100) ~= 1e50");
constexpr double cx_sqrt_small = cm::sqrt(1e-100);
static_assert(cx_sqrt_small > 9.9999e-51 && cx_sqrt_small < 1.0001e-50,
              "sqrt(1e-100) ~= 1e-50");

// ============================================================================
// Runtime cross-check vs std::sqrt
// ============================================================================

int main() {
	std::cout << "sw::math::constexpr_math::sqrt verification\n";

	int errors = 0;
	auto check = [&](const char* name, double x, double our, double ref, double tol) {
		double err = (ref == 0.0) ? std::abs(our) : std::abs((our - ref) / ref);
		if (err > tol) {
			++errors;
			std::cout << "FAIL " << name
			          << "  x=" << std::setprecision(17) << x
			          << "  our=" << our
			          << "  ref=" << ref
			          << "  rel-err=" << err << '\n';
		}
	};

	// Sweep across a representative dynamic range.
	const double points[] = {
		1e-300, 1e-100, 1e-50, 1e-10, 1e-5, 1e-3, 0.1, 0.5, 0.999, 1.0, 1.001,
		1.5, 2.0, 3.14, 7.0, 10.0, 100.0, 1024.0, 1e3, 1e10, 1e50, 1e100, 1e300,
	};
	for (double x : points) {
		double our = cm::sqrt(x);
		double ref = std::sqrt(x);
		check("double sweep", x, our, ref, 1e-15);
	}

	// Subnormal sample (after normalization in sqrt's bit handling).
	{
		double sub = std::numeric_limits<double>::min() / 4.0;
		double our = cm::sqrt(sub);
		double ref = std::sqrt(sub);
		check("double subnormal", sub, our, ref, 1e-14);
	}

	// Round-trip stress: sqrt(x)^2 ~= x.
	const double rt_points[] = {
		1e-50, 1e-10, 1e-5, 0.5, 1.5, 2.0, 3.14, 100.0, 1e10, 1e50,
	};
	for (double x : rt_points) {
		double s = cm::sqrt(x);
		double rt = s * s;
		check("round-trip sqrt(x)^2", x, rt, x, 1e-14);
	}

	// Float sweep
	const float fpoints[] = {
		1e-30f, 1e-10f, 0.001f, 0.5f, 1.0f, 2.0f, 3.14f, 100.0f, 1e10f, 1e30f,
	};
	for (float x : fpoints) {
		float our = cm::sqrt(x);
		float ref = std::sqrt(x);
		double err = (ref == 0.0f) ? std::abs(our) : std::abs((our - ref) / ref);
		if (err > 1e-6) {
			++errors;
			std::cout << "FAIL float sweep  x=" << x
			          << "  our=" << our << "  ref=" << ref
			          << "  rel-err=" << err << '\n';
		}
	}

	std::cout << "constexpr_math::sqrt: " << (errors == 0 ? "PASS" : "FAIL")
	          << " (" << errors << " error" << (errors == 1 ? "" : "s") << ")\n";
	return (errors == 0) ? 0 : 1;
}
