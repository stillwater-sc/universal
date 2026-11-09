// special_values.cpp: branch cuts and C99 Annex G special values of the complex functions
// computed in a high-precision element type (#2026)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Two checks, for ereal<4>, dd, qd and efloat<4>:
//  - a table of Annex G values and branch-cut sides, run everywhere;
//  - where the C library is glibc, whose complex functions follow Annex G, every function on
//    a 10 x 10 grid of zeros, finite values, infinities and NaNs against std::complex<double>,
//    the sign of a zero, an infinity and a NaN included. Annex G leaves some of those signs
//    unspecified; the cases where this implementation and glibc choose differently are listed
//    in kUnspecified and skipped.
// The signs are read in T itself, never through double: dd's -0 converts to +0.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <complex>
#include <cstring>
#include <limits>
#include <string>
#include <universal/number/ereal/ereal.hpp>
#include <universal/number/efloat/efloat.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/dd/math/complex/complex.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/number/qd/math/complex/complex.hpp>
#include <universal/math/complex.hpp>
#include <universal/verification/test_suite.hpp>

namespace {
	using namespace sw::universal;

	const double INF = std::numeric_limits<double>::infinity();
	const double NaN = std::numeric_limits<double>::quiet_NaN();
	const double PI  = 3.14159265358979323846;

	// the value of a T as a double, with T's own sign of zero, infinity and NaN
	template<typename T>
	double classify(const T& v) {
		if (isnan(v)) return NaN;
		if (isinf(v)) return signbit(v) ? -INF : INF;
		if (v == T(0)) return signbit(v) ? -0.0 : 0.0;
		return double(v);
	}

	// NaN matches NaN whatever its sign; zeros and infinities must match in sign
	bool same(double got, double want) {
		if (std::isnan(got) || std::isnan(want)) return std::isnan(got) && std::isnan(want);
		if (std::signbit(got) != std::signbit(want)) return false;
		if (std::isinf(got) || std::isinf(want) || got == 0.0 || want == 0.0) return got == want;
		return std::fabs(got - want) <= 1e-13 * std::fabs(want);
	}

	enum Fn { Exp, Log, Log10, Sqrt, Sin, Cos, Tan, Asin, Acos, Atan, Sinh, Cosh, Tanh, Asinh, Acosh, Atanh };
	const char* const kName[] = { "exp", "log", "log10", "sqrt", "sin", "cos", "tan", "asin", "acos", "atan",
	                              "sinh", "cosh", "tanh", "asinh", "acosh", "atanh" };

	template<typename C>
	C apply(Fn f, const C& z) {
		switch (f) {
		case Exp:   return exp(z);
		case Log:   return log(z);
		case Log10: return log10(z);
		case Sqrt:  return sqrt(z);
		case Sin:   return sin(z);
		case Cos:   return cos(z);
		case Tan:   return tan(z);
		case Asin:  return asin(z);
		case Acos:  return acos(z);
		case Atan:  return atan(z);
		case Sinh:  return sinh(z);
		case Cosh:  return cosh(z);
		case Tanh:  return tanh(z);
		case Asinh: return asinh(z);
		case Acosh: return acosh(z);
		case Atanh: return atanh(z);
		}
		return z;
	}

	struct Case { Fn f; double x, y, re, im; };

	// Annex G (C11 G.6) and the branch-cut sides; each zero's sign is part of the case
	const Case kAnnexG[] = {
		{ Exp,   0.0,  0.0,  1.0, 0.0 },
		{ Exp,   0.5, -0.0,  std::exp(0.5), -0.0 },
		{ Exp,   INF,  0.0,  INF, 0.0 },
		{ Exp,  -INF,  2.5,  -0.0, 0.0 },                       // +0 cis(2.5)
		{ Exp,   1.0,  INF,  NaN, NaN },
		{ Exp,   INF,  NaN,  INF, NaN },
		{ Exp,   NaN,  0.0,  NaN, 0.0 },
		{ Log,  -0.0,  0.0, -INF, PI },
		{ Log,   0.0,  0.0, -INF, 0.0 },
		{ Log,  -2.5,  0.0,  std::log(2.5), PI },              // the cut: above
		{ Log,  -2.5, -0.0,  std::log(2.5), -PI },             //          below
		{ Log,  -INF,  2.5,  INF, PI },
		{ Log,   INF,  INF,  INF, PI / 4 },
		{ Log,  -INF,  INF,  INF, 3 * PI / 4 },
		{ Log,   NaN,  INF,  INF, NaN },
		{ Sqrt, -4.0,  0.0,  0.0, 2.0 },
		{ Sqrt, -4.0, -0.0,  0.0, -2.0 },
		{ Sqrt,  0.0, -0.0,  0.0, -0.0 },
		{ Sqrt,  2.5,  INF,  INF, INF },
		{ Sqrt,  NaN,  INF,  INF, INF },
		{ Sqrt, -INF,  2.5,  0.0, INF },
		{ Sqrt,  INF,  NaN,  INF, NaN },
		{ Asin,  2.5,  0.0,  PI / 2,  std::acosh(2.5) },
		{ Asin,  2.5, -0.0,  PI / 2, -std::acosh(2.5) },
		{ Acos,  2.5,  0.0,  0.0, -std::acosh(2.5) },
		{ Acos,  2.5, -0.0,  0.0,  std::acosh(2.5) },
		{ Acos,  0.0,  NaN,  PI / 2, NaN },
		{ Acos, -INF,  2.5,  PI, -INF },
		{ Atan,  0.0,  2.5,  PI / 2,  std::atanh(0.4) },
		{ Atan, -0.0,  2.5, -PI / 2,  std::atanh(0.4) },
		{ Sinh,  INF,  0.0,  INF, 0.0 },
		{ Sinh,  INF,  2.5,  -INF, INF },                      // inf cis(2.5)
		{ Sinh,  NaN,  0.0,  NaN, 0.0 },
		{ Cosh,  0.0,  0.0,  1.0, 0.0 },
		{ Cosh,  INF,  0.0,  INF, 0.0 },
		{ Cosh,  INF,  2.5,  -INF, INF },
		{ Cosh,  INF,  NaN,  INF, NaN },
		{ Tanh,  INF,  2.5,  1.0, -0.0 },                      // 1 + i0 sin(5)
		{ Tanh,  0.0,  NaN,  0.0, NaN },
		{ Tanh,  NaN,  0.0,  NaN, 0.0 },
		{ Asinh,  0.0, 2.5,  std::acosh(2.5), PI / 2 },
		{ Asinh, -0.0, 2.5, -std::acosh(2.5), PI / 2 },
		{ Asinh,  INF, INF,  INF, PI / 4 },
		{ Asinh,  NaN, 0.0,  NaN, 0.0 },
		{ Acosh,  0.0, 0.0,  0.0, PI / 2 },
		{ Acosh, -2.5, 0.0,  std::acosh(2.5), PI },
		{ Acosh, -2.5, -0.0, std::acosh(2.5), -PI },
		{ Acosh, -INF, 2.5,  INF, PI },
		{ Acosh,  0.0, NaN,  NaN, PI / 2 },
		{ Acosh,  NaN, INF,  INF, NaN },
		{ Atanh,  2.5,  0.0, std::atanh(0.4),  PI / 2 },
		{ Atanh,  2.5, -0.0, std::atanh(0.4), -PI / 2 },
		{ Atanh,  1.0,  0.0, INF, 0.0 },
		{ Atanh,  0.0,  NaN, 0.0, NaN },
		{ Atanh,  INF,  2.5, 0.0, PI / 2 },
	};

	template<typename T>
	int VerifyAnnexG(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		for (const Case& c : kAnnexG) {
			const complex<T> w = apply(c.f, complex<T>(T(c.x), T(c.y)));
			const double re = classify(w.real()), im = classify(w.imag());
			if (!same(re, c.re) || !same(im, c.im)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "    FAIL " << kName[c.f] << "(" << c.x << ", " << c.y << ") = (" << re << ", " << im
				                               << "), expected (" << c.re << ", " << c.im << ")\n";
			}
		}
		return nrOfFailedTestCases;
	}

#if defined(__GLIBC__)
	// where Annex G leaves a sign unspecified and glibc picks the other one
	struct Unspecified { Fn f; double x, y; };
	const Unspecified kUnspecified[] = {
		{ Exp,  -INF, -INF },
		{ Sin,  -INF, -INF }, { Sin, INF, -INF }, { Sin, NaN, -INF },
		{ Tan,  -INF, -INF }, { Tan, -INF, INF },
		{ Asin, -INF, NaN },  { Asin, INF, NaN },
		{ Acos, -INF, NaN },  { Acos, INF, NaN },
		{ Atan, -INF, NaN },  { Atan, INF, NaN },
		{ Sinh, -INF, -INF }, { Sinh, -INF, INF }, { Sinh, -INF, NaN },
		{ Tanh, -INF, -INF }, { Tanh, INF, -INF },
	};

	bool bitwise_equal(double a, double b) {
		return std::memcmp(&a, &b, sizeof(double)) == 0 || (std::isnan(a) && std::isnan(b));
	}

	bool unspecified(Fn f, double x, double y) {
		for (const Unspecified& u : kUnspecified) {
			if (u.f == f && bitwise_equal(u.x, x) && bitwise_equal(u.y, y)) return true;
		}
		return false;
	}

	template<typename F>
	std::complex<double> reference(Fn f, F z) {
		switch (f) {
		case Exp:   return std::exp(z);
		case Log:   return std::log(z);
		case Log10: return std::log10(z);
		case Sqrt:  return std::sqrt(z);
		case Sin:   return std::sin(z);
		case Cos:   return std::cos(z);
		case Tan:   return std::tan(z);
		case Asin:  return std::asin(z);
		case Acos:  return std::acos(z);
		case Atan:  return std::atan(z);
		case Sinh:  return std::sinh(z);
		case Cosh:  return std::cosh(z);
		case Tanh:  return std::tanh(z);
		case Asinh: return std::asinh(z);
		case Acosh: return std::acosh(z);
		case Atanh: return std::atanh(z);
		}
		return z;
	}

	template<typename T>
	int VerifyGrid(bool reportTestCases) {
		const double grid[] = { -INF, -2.5, -1.0, -0.0, 0.0, 0.5, 1.0, 2.5, INF, NaN };
		int nrOfFailedTestCases = 0;
		for (int fi = Exp; fi <= Atanh; ++fi) {
			const Fn f = static_cast<Fn>(fi);
			for (double x : grid) {
				for (double y : grid) {
					if (unspecified(f, x, y)) continue;
					const std::complex<double> r = reference(f, std::complex<double>(x, y));
					const complex<T> w = apply(f, complex<T>(T(x), T(y)));
					const double re = classify(w.real()), im = classify(w.imag());
					if (!same(re, r.real()) || !same(im, r.imag())) {
						++nrOfFailedTestCases;
						if (reportTestCases) std::cout << "    FAIL " << kName[f] << "(" << x << ", " << y << ") = (" << re << ", " << im
						                               << "), glibc (" << r.real() << ", " << r.imag() << ")\n";
					}
				}
			}
		}
		return nrOfFailedTestCases;
	}
#endif

	template<typename T>
	int VerifySpecialValues(bool reportTestCases) {
		int nrOfFailedTestCases = VerifyAnnexG<T>(reportTestCases);
#if defined(__GLIBC__)
		nrOfFailedTestCases += VerifyGrid<T>(reportTestCases);
#endif
		return nrOfFailedTestCases;
	}

}  // namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "complex branch cuts and Annex G special values (#2026)";
	std::string test_tag    = "special values";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<ereal<4>>(reportTestCases), "ereal<4>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<ereal<4>>(reportTestCases), "ereal<4>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<dd>(reportTestCases), "dd", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<qd>(reportTestCases), "qd", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<efloat<4>>(reportTestCases), "efloat<4>", test_tag);
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
