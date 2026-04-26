// exp2.cpp: regression for sw::math::constexpr_math::exp2(float|double)
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

// ----------------------------------------------------------------------------
// Compile-time correctness via static_assert
// ----------------------------------------------------------------------------

// Exact integer powers of 2: must be bit-exact (the 2^N path).
static_assert(cm::exp2(0.0)    == 1.0,    "exp2(0) == 1");
static_assert(cm::exp2(1.0)    == 2.0,    "exp2(1) == 2");
static_assert(cm::exp2(2.0)    == 4.0,    "exp2(2) == 4");
static_assert(cm::exp2(10.0)   == 1024.0, "exp2(10) == 1024");
static_assert(cm::exp2(-1.0)   == 0.5,    "exp2(-1) == 0.5");
static_assert(cm::exp2(-10.0)  == 1.0/1024.0, "exp2(-10) == 1/1024");

// Float overload, exact integer powers
static_assert(cm::exp2(0.0f)   == 1.0f,    "exp2f(0) == 1");
static_assert(cm::exp2(8.0f)   == 256.0f,  "exp2f(8) == 256");
static_assert(cm::exp2(-3.0f)  == 0.125f,  "exp2f(-3) == 0.125");

// Special values
static_assert(cm::exp2(std::numeric_limits<double>::infinity())
              == std::numeric_limits<double>::infinity(), "exp2(+inf) == +inf");
static_assert(cm::exp2(-std::numeric_limits<double>::infinity()) == 0.0,
              "exp2(-inf) == 0");
// Overflow / underflow
static_assert(cm::exp2(2000.0)  == std::numeric_limits<double>::infinity(),
              "exp2(2000) -> +inf");
static_assert(cm::exp2(-2000.0) == 0.0, "exp2(-2000) -> 0");
// NaN propagation
static_assert(cm::exp2(std::numeric_limits<double>::quiet_NaN())
              != cm::exp2(std::numeric_limits<double>::quiet_NaN()),
              "exp2(NaN) == NaN");

// Acceptance forms for issue #722 (lns full constexpr).
// Round-trip: exp2(log2(x)) ~= x  -- the lns encode/decode loop.
constexpr double rt_pi   = cm::exp2(cm::log2(3.14159265358979323846));
static_assert(rt_pi > 3.14159 && rt_pi < 3.14160,
              "exp2(log2(pi)) ~= pi");
constexpr double rt_e    = cm::exp2(cm::log2(2.71828182845904523536));
static_assert(rt_e  > 2.71828 && rt_e  < 2.71829,
              "exp2(log2(e)) ~= e");
constexpr double rt_1024 = cm::exp2(cm::log2(1024.0));
static_assert(rt_1024 > 1023.999999 && rt_1024 < 1024.000001,
              "exp2(log2(1024)) ~= 1024");

// Fractional argument: exp2(0.5) == sqrt(2) ~= 1.4142135623730951
constexpr double cx_sqrt2 = cm::exp2(0.5);
static_assert(cx_sqrt2 > 1.4142135 && cx_sqrt2 < 1.4142137,
              "exp2(0.5) ~= sqrt(2)");

// ----------------------------------------------------------------------------
// Runtime cross-check vs std::exp2 + round-trip stress
// ----------------------------------------------------------------------------

int main() {
	std::cout << "sw::math::constexpr_math::exp2 verification\n";

	int errors = 0;
	auto check = [&](const char* name, double our, double ref, double tol) {
		double err = (ref == 0.0) ? std::abs(our) : std::abs((our - ref) / ref);
		if (err > tol) {
			++errors;
			std::cout << "FAIL " << name
			          << "  our=" << std::setprecision(17) << our
			          << "  ref=" << ref
			          << "  rel-err=" << err << '\n';
		}
	};

	// Direct sweep against std::exp2 over a representative range.
	const double points[] = {
		-1023.5, -100.5, -10.25, -3.5, -1.5, -0.5, -0.25, -0.0001,
		 0.0,    0.0001, 0.25,    0.5,  1.5,   3.5,   10.25, 100.5, 1023.5,
	};
	for (double x : points) {
		double our = cm::exp2(x);
		double ref = std::exp2(x);
		check("double sweep", our, ref, 1e-15);
	}

	// Round-trip stress against log2: for any positive x, exp2(log2(x)) ~= x.
	// This is the encode/decode loop the lns work in #722 will exercise.
	const double rt_points[] = {
		1e-100, 1e-50, 1e-10, 1e-5, 1e-3, 0.1, 0.5, 0.999, 1.0, 1.001,
		1.5, 2.0, 3.14, 7.0, 10.0, 100.0, 1024.0, 1e3, 1e10, 1e50, 1e100,
	};
	for (double x : rt_points) {
		double rt = cm::exp2(cm::log2(x));
		// At extreme x (~1e+/-100), log2(x) reaches ~332 in magnitude. The
		// derivative of 2^y amplifies absolute error in y by ln(2) * 2^y, so
		// even at machine-precision log2 the round-trip relative error reaches
		// ~5e-14 at the tails. 1e-13 leaves comfortable margin without masking
		// any real algorithmic regression.
		check("round-trip exp2(log2(x))", rt, x, 1e-13);
	}

	// Float sweep
	const float fpoints[] = {
		-127.5f, -10.25f, -3.5f, -0.5f, 0.0f, 0.5f, 3.5f, 10.25f, 127.5f,
	};
	for (float x : fpoints) {
		float our = cm::exp2(x);
		float ref = std::exp2(x);
		double err = (ref == 0.0f) ? std::abs(our) : std::abs((our - ref) / ref);
		// Float epsilon ~1.19e-7; allow 1e-6 relative.
		if (err > 1e-6) {
			++errors;
			std::cout << "FAIL float sweep  x=" << x
			          << "  our=" << our << "  ref=" << ref
			          << "  rel-err=" << err << '\n';
		}
	}

	std::cout << "constexpr_math::exp2: " << (errors == 0 ? "PASS" : "FAIL")
	          << " (" << errors << " error" << (errors == 1 ? "" : "s") << ")\n";
	return (errors == 0) ? 0 : 1;
}
