// log2.cpp: regression for sw::math::constexpr_math::log2(float|double)
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

// Exact powers of 2 -- the integer-exponent path, must be bit-exact.
static_assert(cm::log2(1.0)    == 0.0, "log2(1) == 0");
static_assert(cm::log2(2.0)    == 1.0, "log2(2) == 1");
static_assert(cm::log2(4.0)    == 2.0, "log2(4) == 2");
static_assert(cm::log2(8.0)    == 3.0, "log2(8) == 3");
static_assert(cm::log2(0.5)    == -1.0, "log2(0.5) == -1");
static_assert(cm::log2(0.25)   == -2.0, "log2(0.25) == -2");
static_assert(cm::log2(1024.0) == 10.0, "log2(1024) == 10");

// Float overload, exact powers of 2
static_assert(cm::log2(1.0f) == 0.0f, "log2f(1) == 0");
static_assert(cm::log2(8.0f) == 3.0f, "log2f(8) == 3");
static_assert(cm::log2(0.5f) == -1.0f, "log2f(0.5) == -1");

// Special values
static_assert(cm::log2(0.0)  == -std::numeric_limits<double>::infinity(),
              "log2(0) == -inf");
static_assert(cm::log2(0.0f) == -std::numeric_limits<float>::infinity(),
              "log2f(0) == -inf");
static_assert(cm::log2(std::numeric_limits<double>::infinity())
              == std::numeric_limits<double>::infinity(),
              "log2(+inf) == +inf");

// log2(x) for x < 0 must yield NaN. NaN != NaN, so the test is `result != result`.
static_assert(cm::log2(-1.0) != cm::log2(-1.0), "log2(-1) == NaN");
static_assert(cm::log2(-2.5f) != cm::log2(-2.5f), "log2f(-2.5) == NaN");

// Acceptance form for issue #423: enables `constexpr takum16 t = 3.14f;`
// log2(3.14) = ln(3.14) / ln(2) ~= 1.65076455911...
constexpr double cx_log2_3p14 = cm::log2(3.14);
static_assert(cx_log2_3p14 > 1.65076 && cx_log2_3p14 < 1.65077,
              "log2(3.14) ~= 1.65076455...");

// Acceptance: enables compile-time lns encoding from a float literal.
// For lns<8,4>, log2(0.75) is needed in the value->encoding mapping.
constexpr double cx_log2_0p75 = cm::log2(0.75);
static_assert(cx_log2_0p75 > -0.4151 && cx_log2_0p75 < -0.4149,
              "log2(0.75) ~= -0.41504");

// ----------------------------------------------------------------------------
// Runtime cross-check vs std::log2 over a representative grid
// ----------------------------------------------------------------------------

int main() {
	std::cout << "sw::math::constexpr_math::log2 verification\n";

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

	// Sweep across a wide dynamic range and a fine-grained set inside [1, 2).
	const double points[] = {
		0.5, 0.75, 1.0, 1.25, 1.5, 1.7320508, 1.9999999,
		2.0, 3.0, 3.14, 5.0, 7.0, 10.0, 100.0, 1024.0,
		1e-10, 1e-5, 1e-3, 1e3, 1e10, 1e100, 1e-300, 1e300,
	};
	for (double x : points) {
		double our = cm::log2(x);
		double ref = std::log2(x);
		// Allow up to 4 ulp (double eps ~2.2e-16, so ~1e-15)
		check("double sweep", our, ref, 1e-15);
	}

	// Subnormal sample
	{
		double sub = std::numeric_limits<double>::min() / 4.0;  // subnormal
		double our = cm::log2(sub);
		double ref = std::log2(sub);
		check("double subnormal", our, ref, 1e-14);
	}

	// Float sweep
	const float fpoints[] = {
		0.5f, 0.75f, 1.0f, 1.5f, 2.0f, 3.14f, 1024.0f, 1e10f, 1e-10f,
	};
	for (float x : fpoints) {
		float our = cm::log2(x);
		float ref = std::log2(x);
		double err = (ref == 0.0f) ? std::abs(our) : std::abs((our - ref) / ref);
		// Float epsilon ~1.19e-7; allow 1e-6 relative.
		if (err > 1e-6) {
			++errors;
			std::cout << "FAIL float sweep  x=" << x
			          << "  our=" << our << "  ref=" << ref
			          << "  rel-err=" << err << '\n';
		}
	}

	std::cout << "constexpr_math::log2: " << (errors == 0 ? "PASS" : "FAIL")
	          << " (" << errors << " error" << (errors == 1 ? "" : "s") << ")\n";
	return (errors == 0) ? 0 : 1;
}
