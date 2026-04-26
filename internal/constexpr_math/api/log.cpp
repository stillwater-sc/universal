// log.cpp: regression for sw::math::constexpr_math::log (natural logarithm)
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

// Anchors derivable from the algorithm: log(1) = 0 (independent of implementation).
static_assert(cm::log(1.0)  == 0.0, "log(1) == 0");
static_assert(cm::log(1.0f) == 0.0f, "logf(1) == 0");

// log(2) ~= ln(2) ~= 0.6931471805599453 (the LN2 constant itself)
constexpr double cx_log2 = cm::log(2.0);
static_assert(cx_log2 > 0.69314718 && cx_log2 < 0.69314719,
              "log(2) ~= 0.6931471805599453");

// log(e) ~= 1 (within ~1 ulp transcendental composition error)
constexpr double cx_loge = cm::log(2.71828182845904523536);
static_assert(cx_loge > 0.99999999 && cx_loge < 1.00000001,
              "log(e) ~= 1");

// log(10) ~= 2.302585092994046
constexpr double cx_log10 = cm::log(10.0);
static_assert(cx_log10 > 2.3025850 && cx_log10 < 2.3025851,
              "log(10) ~= 2.302585092994046");

// Cross-check: log(8) == log(2) * 3 (within transcendental round-trip error)
constexpr double cx_log8 = cm::log(8.0);
constexpr double cx_3log2 = 3.0 * cm::log(2.0);
static_assert(cx_log8 == cx_3log2,
              "log(8) == 3 * log(2) -- log2(8) is exact, propagates through");

// Special values (inherited from log2)
static_assert(cm::log(0.0) == -std::numeric_limits<double>::infinity(),
              "log(0) == -inf");
static_assert(cm::log(0.0f) == -std::numeric_limits<float>::infinity(),
              "logf(0) == -inf");
static_assert(cm::log(std::numeric_limits<double>::infinity())
              == std::numeric_limits<double>::infinity(),
              "log(+inf) == +inf");
static_assert(cm::log(-1.0) != cm::log(-1.0), "log(-1) == NaN");
static_assert(cm::log(-2.5f) != cm::log(-2.5f), "logf(-2.5) == NaN");
static_assert(cm::log(std::numeric_limits<double>::quiet_NaN())
              != cm::log(std::numeric_limits<double>::quiet_NaN()),
              "log(NaN) == NaN");

// ============================================================================
// Runtime cross-check vs std::log
// ============================================================================

int main() {
	std::cout << "sw::math::constexpr_math::log verification\n";

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

	// Sweep over a representative dynamic range.
	const double points[] = {
		1e-300, 1e-100, 1e-10, 1e-5, 0.001, 0.1, 0.5, 0.999, 1.0, 1.001,
		1.5, 2.0, 2.71828182845904523536, 3.14159265358979323846,
		7.0, 10.0, 100.0, 1024.0, 1e3, 1e10, 1e100, 1e300,
	};
	for (double x : points) {
		double our = cm::log(x);
		double ref = std::log(x);
		check("double sweep", x, our, ref, 1e-15);
	}

	// Float sweep
	const float fpoints[] = {
		1e-30f, 1e-10f, 0.001f, 0.5f, 1.0f, 2.0f, 2.71828183f, 10.0f, 1e10f, 1e30f,
	};
	for (float x : fpoints) {
		float our = cm::log(x);
		float ref = std::log(x);
		double err = (ref == 0.0f) ? std::abs(our) : std::abs((our - ref) / ref);
		if (err > 1e-6) {
			++errors;
			std::cout << "FAIL float sweep  x=" << x
			          << "  our=" << our << "  ref=" << ref
			          << "  rel-err=" << err << '\n';
		}
	}

	std::cout << "constexpr_math::log: " << (errors == 0 ? "PASS" : "FAIL")
	          << " (" << errors << " error" << (errors == 1 ? "" : "s") << ")\n";
	return (errors == 0) ? 0 : 1;
}
