// precision.cpp: complex functions carry the precision of a high-precision element type (#2026)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// abs, arg and the complex transcendentals used to go through std::complex<double> for every
// element type, so complex<ereal<8>> sqrt(2+3i)^2 agreed with 2+3i to 53 bits. A type marked
// is_high_precision (ereal, efloat, dd, qd) now computes them in its own arithmetic.
//
// Each result is compared with an mpmath reference (reference_constants.hpp, 330 digits, from
// generate_reference_constants.py --complex) exactly: a dd, qd or ereal value is turned into
// an ereal without rounding and compared as a dyadic rational (ereal_precision.hpp), an efloat
// in efloat at its own precision. A check asks for the type's bits less a margin that covers
// the error of the type's own real functions (measured in the comments below). cfloat<64,11>
// stays on the double path and is held to double precision, to show the fallback still works.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cmath>
#include <string>
#include <string_view>
#include <universal/number/ereal/ereal.hpp>
#include <universal/number/efloat/efloat.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/dd/math/complex/complex.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/number/qd/math/complex/complex.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/math/complex.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/ereal_precision.hpp>
#include <math/constants/reference_constants.hpp>

namespace {
	using namespace sw::universal;

	using Ref = ereal<8>;   // holds a dd or qd value exactly

	// bits to which an ereal agrees with a signed decimal reference; -1 for a value that cannot
	template<typename Real>
	int bits_vs(const Real& v, std::string_view ref) {
		if (v.isnan() || v.isinf() || v.iszero()) return -1;
		Real w = v;
		if (!ref.empty() && ref.front() == '-') { ref.remove_prefix(1); w = -v; }
		if (w.isneg()) return -1;
		return agreed_bits(w, ref, 320);
	}

	// per-type: the precision to ask for, and the agreement of a value with a reference
	template<typename T> struct Measure;

	template<unsigned maxlimbs, typename FpType>
	struct Measure<ereal<maxlimbs, FpType>> {
		static constexpr int bits = ereal_bits_of<ereal<maxlimbs, FpType>>::value;
		static int agreement(const ereal<maxlimbs, FpType>& v, std::string_view ref) { return bits_vs(v, ref); }
	};
	template<>
	struct Measure<dd> {
		static constexpr int bits = 106;
		static int agreement(const dd& v, std::string_view ref) {
			if (isnan(v) || isinf(v)) return -1;
			return bits_vs(Ref(v[0]) + Ref(v[1]), ref);
		}
	};
	template<>
	struct Measure<qd> {
		static constexpr int bits = 212;
		static int agreement(const qd& v, std::string_view ref) {
			if (isnan(v) || isinf(v)) return -1;
			Ref s(v[0]);
			for (int i = 1; i < 4; ++i) s += Ref(v[i]);
			return bits_vs(s, ref);
		}
	};
	template<unsigned nlimbs>
	struct Measure<efloat<nlimbs>> {
		static constexpr int bits = static_cast<int>(32 * nlimbs);
		static int agreement(const efloat<nlimbs>& v, std::string_view ref) {
			if (v.isnan() || v.isinf() || v.iszero()) return -1;
			efloat<nlimbs> r;
			if (!parse(ref, r)) return -1;
			const efloat<nlimbs> d = v - r;
			if (d.iszero()) return bits;
			return static_cast<int>(r.scale() - d.scale());
		}
	};
	using Double = cfloat<64, 11, std::uint32_t, true, false, false>;
	template<>
	struct Measure<Double> {
		static constexpr int bits = 53;
		static int agreement(const Double& v, std::string_view ref) {
			const double d = double(v);
			if (d != d || d == 0.0 || d - d != 0.0) return -1;
			return bits_vs(Ref(d), ref);
		}
	};

	int expect_bits(int got, int wanted, const std::string& what, bool reportTestCases) {
#ifdef COMPLEX_PRECISION_TRACE
		std::cout << "    " << what << ": " << got << " bits (wanted " << wanted << ")\n";
#endif
		if (got >= wanted) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << ": " << got << " bits, wanted at least " << wanted << '\n';
		return 1;
	}

	struct Point {
		const char* tag;
		double re, im;
		std::string_view sre, sim;   // the point itself, for the round trips
		std::string_view exp_re, exp_im, log_re, log_im, log10_re, log10_im, sqrt_re, sqrt_im;
		std::string_view sin_re, sin_im, cos_re, cos_im, tan_re, tan_im;
		std::string_view asin_re, asin_im, acos_re, acos_im, atan_re, atan_im;
		std::string_view sinh_re, sinh_im, cosh_re, cosh_im, tanh_re, tanh_im;
		std::string_view asinh_re, asinh_im, acosh_re, acosh_im, atanh_re, atanh_im;
		std::string_view abs, arg;
	};

	const Point points[] = {
		{ "0.75+1.25i", 0.75, 1.25, "0.75", "1.25",
		  s_cexp_z1_re, s_cexp_z1_im, s_clog_z1_re, s_clog_z1_im, s_clog10_z1_re, s_clog10_z1_im, s_csqrt_z1_re, s_csqrt_z1_im,
		  s_csin_z1_re, s_csin_z1_im, s_ccos_z1_re, s_ccos_z1_im, s_ctan_z1_re, s_ctan_z1_im,
		  s_casin_z1_re, s_casin_z1_im, s_cacos_z1_re, s_cacos_z1_im, s_catan_z1_re, s_catan_z1_im,
		  s_csinh_z1_re, s_csinh_z1_im, s_ccosh_z1_re, s_ccosh_z1_im, s_ctanh_z1_re, s_ctanh_z1_im,
		  s_casinh_z1_re, s_casinh_z1_im, s_cacosh_z1_re, s_cacosh_z1_im, s_catanh_z1_re, s_catanh_z1_im,
		  s_cabs_z1, s_carg_z1 },
		{ "-2.5-0.375i", -2.5, -0.375, "-2.5", "-0.375",
		  s_cexp_z2_re, s_cexp_z2_im, s_clog_z2_re, s_clog_z2_im, s_clog10_z2_re, s_clog10_z2_im, s_csqrt_z2_re, s_csqrt_z2_im,
		  s_csin_z2_re, s_csin_z2_im, s_ccos_z2_re, s_ccos_z2_im, s_ctan_z2_re, s_ctan_z2_im,
		  s_casin_z2_re, s_casin_z2_im, s_cacos_z2_re, s_cacos_z2_im, s_catan_z2_re, s_catan_z2_im,
		  s_csinh_z2_re, s_csinh_z2_im, s_ccosh_z2_re, s_ccosh_z2_im, s_ctanh_z2_re, s_ctanh_z2_im,
		  s_casinh_z2_re, s_casinh_z2_im, s_cacosh_z2_re, s_cacosh_z2_im, s_catanh_z2_re, s_catanh_z2_im,
		  s_cabs_z2, s_carg_z2 },
	};

	template<typename T>
	int VerifyPrecision(int margin, bool reportTestCases) {
		using M = Measure<T>;
		using C = complex<T>;
		const int wanted = M::bits - margin;
		int nrOfFailedTestCases = 0;
		auto check = [&](const C& v, std::string_view re, std::string_view im, const std::string& what) {
			nrOfFailedTestCases += expect_bits(M::agreement(v.real(), re), wanted, "Re " + what, reportTestCases);
			nrOfFailedTestCases += expect_bits(M::agreement(v.imag(), im), wanted, "Im " + what, reportTestCases);
		};
		for (const Point& p : points) {
			const C z(T(p.re), T(p.im));
			const std::string at = std::string("(") + p.tag + ")";
			check(exp(z), p.exp_re, p.exp_im, "exp" + at);
			check(log(z), p.log_re, p.log_im, "log" + at);
			check(log10(z), p.log10_re, p.log10_im, "log10" + at);
			check(sqrt(z), p.sqrt_re, p.sqrt_im, "sqrt" + at);
			check(sin(z), p.sin_re, p.sin_im, "sin" + at);
			check(cos(z), p.cos_re, p.cos_im, "cos" + at);
			check(tan(z), p.tan_re, p.tan_im, "tan" + at);
			check(asin(z), p.asin_re, p.asin_im, "asin" + at);
			check(acos(z), p.acos_re, p.acos_im, "acos" + at);
			check(atan(z), p.atan_re, p.atan_im, "atan" + at);
			check(sinh(z), p.sinh_re, p.sinh_im, "sinh" + at);
			check(cosh(z), p.cosh_re, p.cosh_im, "cosh" + at);
			check(tanh(z), p.tanh_re, p.tanh_im, "tanh" + at);
			check(asinh(z), p.asinh_re, p.asinh_im, "asinh" + at);
			check(acosh(z), p.acosh_re, p.acosh_im, "acosh" + at);
			check(atanh(z), p.atanh_re, p.atanh_im, "atanh" + at);
			nrOfFailedTestCases += expect_bits(M::agreement(abs(z), p.abs), wanted, "abs" + at, reportTestCases);
			nrOfFailedTestCases += expect_bits(M::agreement(arg(z), p.arg), wanted, "arg" + at, reportTestCases);
			// the round trips the issue names: sqrt(z)^2 and exp(log(z)) give z back
			const C s = sqrt(z);
			check(s * s, p.sre, p.sim, "sqrt(z)^2" + at);
			check(exp(log(z)), p.sre, p.sim, "exp(log(z))" + at);
		}
		const C z1(T(points[0].re), T(points[0].im)), z2(T(points[1].re), T(points[1].im));
		check(pow(z1, z2), s_cpow_z1_z2_re, s_cpow_z1_z2_im, "pow(z1, z2)");
		// abs(1 + i) is sqrt(2)
		nrOfFailedTestCases += expect_bits(M::agreement(abs(C(T(1), T(1))), s_sqrt2), wanted, "abs(1+i)", reportTestCases);
		// sin^2 + cos^2 = 1
		const C sn = sin(z1), cs = cos(z1);
		nrOfFailedTestCases += expect_bits(M::agreement((sn * sn + cs * cs).real(), "1"), wanted, "Re sin^2+cos^2", reportTestCases);
		return nrOfFailedTestCases;
	}

	// far from the origin, where a square of a component leaves the range: atanh(x) ~ 1/x and
	// atan(x + i) ~ pi/2. dd's and qd's own atan2 squares its arguments and returned -4e15 at
	// 1e100; with float limbs the squares in atanh overflowed past 1.8e19 and gave 0 at 2^65.
	const double kDoubleRange[] = { 36893488147419103232.0, 1e100, 1e160, 1e300 };   // 2^65 first
	// 1/x stays above float's min(); its trailing limbs fall below it, so near 1e-37 float limbs
	// keep ~26 bits of 1/x (the limb floor, ereal.md): that check is loosened to 1e-6
	const double kFloatRange[]  = { 36893488147419103232.0, 1e30, 1e37 };

	template<typename T, std::size_t N>
	int VerifyLargeArguments(const double (&points)[N], double tolerance, bool reportTestCases) {
		using C = complex<T>;
		int nrOfFailedTestCases = 0;
		auto close = [tolerance](double got, double want) { return std::fabs(got - want) <= tolerance * std::fabs(want); };
		for (double x : points) {
			const C h = atanh(C(T(x), T(0)));
			const C a = atan(C(T(x), T(1)));
			if (!close(double(h.real()) * x, 1.0) || !close(double(h.imag()), 1.5707963267948966) ||
			    !close(double(a.real()), 1.5707963267948966)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "    FAIL x = " << x << ": atanh(x) = (" << double(h.real()) << ", " << double(h.imag())
				                               << "), Re atan(x + i) = " << double(a.real()) << '\n';
			}
		}
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

	std::string test_suite  = "complex functions at the element type's precision (#2026)";
	std::string test_tag    = "precision";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyPrecision<ereal<8>>(16, reportTestCases), "ereal<8>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyPrecision<ereal<8>>(16, reportTestCases), "ereal<8>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyPrecision<dd>(8, reportTestCases), "dd", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyPrecision<qd>(8, reportTestCases), "qd", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyPrecision<efloat<8>>(16, reportTestCases), "efloat<8>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyPrecision<Double>(6, reportTestCases), "cfloat<64,11> (double path)", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyPrecision<ereal<4, float>>(8, reportTestCases), "ereal<4, float>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLargeArguments<ereal<8>>(kDoubleRange, 1e-14, reportTestCases), "ereal<8>", "large arguments");
	nrOfFailedTestCases += ReportTestResult(VerifyLargeArguments<ereal<4, float>>(kFloatRange, 1e-6, reportTestCases), "ereal<4, float>", "large arguments");
	nrOfFailedTestCases += ReportTestResult(VerifyLargeArguments<dd>(kDoubleRange, 1e-14, reportTestCases), "dd", "large arguments");
	nrOfFailedTestCases += ReportTestResult(VerifyLargeArguments<qd>(kDoubleRange, 1e-14, reportTestCases), "qd", "large arguments");
	nrOfFailedTestCases += ReportTestResult(VerifyLargeArguments<efloat<8>>(kDoubleRange, 1e-14, reportTestCases), "efloat<8>", "large arguments");
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
