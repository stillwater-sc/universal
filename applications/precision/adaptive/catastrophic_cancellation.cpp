// catastrophic_cancellation.cpp: demonstrate how efloat's adaptive precision
//                                defeats catastrophic cancellation (issue #1096)
//
// f(x) = (1 - cos x) / x^2   is numerically UNSTABLE for small x: cos x is very
// close to 1, so (1 - cos x) loses almost all its significant digits in fixed-
// precision double. The mathematically equivalent
//
//   g(x) = 2 * sin(x/2)^2 / x^2
//
// is STABLE (no subtraction of nearly-equal quantities). Both tend to 1/2 as
// x -> 0. Written as a single templated kernel, the same code runs on `double`
// and on `efloat`; with enough working precision `efloat` gets the right answer
// even from the unstable formula.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <iostream>
#include <iomanip>
#include <string>
#include <universal/number/efloat/efloat.hpp>

// Unstable form: subtracts two nearly-equal quantities for small x.
template<typename Real>
Real f_unstable(const Real& x) {
	using namespace sw::universal;
	using std::cos;
	return (Real(1.0) - cos(x)) / (x * x);
}

// Stable form: no cancellation. Algebraically identical to f.
template<typename Real>
Real g_stable(const Real& x) {
	using namespace sw::universal;
	using std::sin;
	Real s = sin(x / Real(2.0));
	return (Real(2.0) * s * s) / (x * x);
}

int main() try {
	using namespace sw::universal;
	using efloat256 = efloat<8>;  // 8 limbs * 32 = 256 bits of working precision

	std::cout << "Catastrophic cancellation: f(x) = (1 - cos x) / x^2   (unstable)\n";
	std::cout << "                     vs    g(x) = 2 sin^2(x/2) / x^2   (stable)\n";
	std::cout << "Both tend to 1/2 as x -> 0.  efloat working precision: 256 bits (~77 digits)\n\n";

	// -------------------------------------------------------------------------
	// The four results the issue asks for, at x = 1e-10.
	// double and efloat start from the SAME x, so the only difference is the
	// arithmetic precision.
	// -------------------------------------------------------------------------
	const double x0 = 1e-10;
	efloat256    xe(x0);

	std::cout << std::setprecision(17);
	std::cout << "At x = " << x0 << ":\n";
	std::cout << "  double  f(x) = " << std::setw(22) << f_unstable(x0) << "   <- catastrophic cancellation\n";
	std::cout << "  double  g(x) = " << std::setw(22) << g_stable(x0) << "   <- double's correct reference\n";
	std::cout << "  efloat  f(x) = " << std::setw(22) << double(f_unstable(xe))
	          << "   <- correct, from the UNSTABLE formula\n";
	std::cout << "  efloat  g(x) = " << std::setw(22) << double(g_stable(xe)) << "\n";
	{
		efloat256 diff = f_unstable(xe) - g_stable(xe);
		diff.setsign(false);
		long long bits = diff.iszero() ? 256 : -static_cast<long long>(diff.scale());
		std::cout << "  efloat f and g agree to ~" << bits << " bits (~" << (bits * 3) / 10 << " digits)\n\n";
	}

	// -------------------------------------------------------------------------
	// Breakdown table: as x shrinks, double f(x) progressively loses digits and
	// finally collapses to 0, while efloat f(x) stays accurate. The reference is
	// the stable efloat g(x).
	// -------------------------------------------------------------------------
	std::cout << "  " << std::left << std::setw(10) << "x" << std::setw(24) << "double f(x)" << std::setw(24)
	          << "efloat f(x)  (accurate)" << "double f rel.error\n";
	std::cout << "  " << std::string(72, '-') << "\n";
	const double xs[] = {1e-2, 1e-4, 1e-6, 1e-8, 1e-10, 1e-12, 1e-14};
	for (double x : xs) {
		efloat256 xe2(x);
		double    df     = f_unstable(x);
		efloat256 ef     = f_unstable(xe2);
		efloat256 ref    = g_stable(xe2);  // stable, high-precision reference
		double    eref   = double(ref);
		double    relerr = (eref != 0.0) ? std::abs(df - eref) / std::abs(eref) : std::abs(df);
		std::cout << "  " << std::left << std::setw(10) << std::scientific << std::setprecision(0) << x << std::right
		          << std::fixed << std::setprecision(15) << std::setw(22) << df << "  " << std::setw(22) << double(ef)
		          << "  " << std::scientific << std::setprecision(2) << std::setw(10) << relerr << "\n";
	}

	std::cout << "\nThe unstable formula is fine in efloat because 256-bit arithmetic keeps the\n";
	std::cout << "tiny (1 - cos x) term that double rounds away. Same code, two number types.\n";

	return EXIT_SUCCESS;
} catch (const char* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
} catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
} catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
