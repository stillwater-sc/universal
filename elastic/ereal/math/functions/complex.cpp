// complex.cpp: regression tests for sw::universal::complex<ereal> support.
//
// ereal plugs into the portable sw::universal::complex<T> (NOT std::complex<T>,
// which is UB on user-defined types per ISO C++ 26.2/2). Complex arithmetic,
// real/imag/conj/norm, abs, arg and the transcendentals all compute in ereal (#2026) and
// are checked here exactly, to the configuration's precision (ereal_precision.hpp, never
// through double: #2008). (Issue #1167) The transcendentals are checked against mpmath,
// and their special values, in static/complex/math/.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/ereal_precision.hpp>
#include <math/constants/reference_constants.hpp>

namespace {

	// A check asks for ereal_bits_of<E>::value less kMarginBits.  The inexact checks measure
	// 476-880 bits on ereal<8> (424 bits; the limb budget lets a result carry twice the limbs),
	// so 16 bits of margin leaves ample slack.
	constexpr int kMarginBits = 16;

	// exact: the dyadic values are equal, whatever the limb structure
	template<unsigned maxlimbs>
	bool Exactly(const sw::universal::ereal<maxlimbs>& a, const sw::universal::ereal<maxlimbs>& b) {
		return sw::universal::agreed_bits_absolute(a, b) == sw::universal::kExactAgreement;
	}

	int VerifyErealComplex(bool reportTestCases) {
		using namespace sw::universal;
		using E = ereal<8>;   // ~127 digits
		int failures = 0;

		// -----------------------------------------------------------------
		// 1. ereal is registered as a Universal number type for complex
		// -----------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying is_universal_number<ereal>...\n";
		{
			if (!is_universal_number_v<E>) {
				if (reportTestCases) std::cout << "    FAIL: is_universal_number<ereal> is false\n";
				++failures;
			}
		}

		// -----------------------------------------------------------------
		// 2. construction, real/imag (members and free functions)
		// -----------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying construction / real / imag...\n";
		{
			complex<E> z(E(3.0), E(4.0));
			if (!Exactly(z.real(), E(3.0)) || !Exactly(z.imag(), E(4.0))) {
				if (reportTestCases) std::cout << "    FAIL: member real/imag\n";
				++failures;
			}
			if (!Exactly(real(z), E(3.0)) || !Exactly(imag(z), E(4.0))) {
				if (reportTestCases) std::cout << "    FAIL: free real/imag\n";
				++failures;
			}
		}

		// -----------------------------------------------------------------
		// 3. arithmetic is exact at full precision:  (3+4i)(1+2i) = -5+10i
		// -----------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying arithmetic (full precision)...\n";
		{
			complex<E> a(E(3.0), E(4.0)), b(E(1.0), E(2.0));
			complex<E> p = a * b;
			if (p.real() != E(-5.0) || p.imag() != E(10.0)) {
				if (reportTestCases) std::cout << "    FAIL: (3+4i)(1+2i) != -5+10i\n";
				++failures;
			}
			complex<E> s = a + b, d = a - b;
			if (s.real() != E(4.0) || s.imag() != E(6.0) || d.real() != E(2.0) || d.imag() != E(2.0)) {
				if (reportTestCases) std::cout << "    FAIL: complex +/-\n";
				++failures;
			}
		}

		// -----------------------------------------------------------------
		// 4. conj / norm at full precision; abs(3+4i) == 5 is exact even through double
		// -----------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying conj / norm / abs...\n";
		{
			complex<E> z(E(3.0), E(4.0));
			complex<E> c = conj(z);
			if (c.real() != E(3.0) || c.imag() != E(-4.0)) {
				if (reportTestCases) std::cout << "    FAIL: conj\n";
				++failures;
			}
			if (norm(z) != E(25.0)) {   // |z|^2 = 9 + 16
				if (reportTestCases) std::cout << "    FAIL: norm != 25\n";
				++failures;
			}
			if (!Exactly(abs(z), E(5.0))) {     // sqrt(25) is exact
				if (reportTestCases) std::cout << "    FAIL: abs != 5\n";
				++failures;
			}
		}

		// -----------------------------------------------------------------
		// 5. high-precision identity: z * conj(z) == norm(z) (real), imag == 0
		//    (pure ereal arithmetic -> exact)
		// -----------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying z*conj(z) == |z|^2...\n";
		{
			complex<E> z(E(0.7), E(1.3));
			complex<E> zz = z * conj(z);
			if (zz.real() != norm(z)) {
				if (reportTestCases) std::cout << "    FAIL: Re(z*conj z) != norm(z)\n";
				++failures;
			}
			if (!zz.imag().iszero()) {
				if (reportTestCases) std::cout << "    FAIL: Im(z*conj z) != 0\n";
				++failures;
			}
		}

		// -----------------------------------------------------------------
		// 6. high-precision identity: (a / b) * b == a to the configuration's bits
		// -----------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying (a/b)*b == a to high precision...\n";
		{
			const int wanted = ereal_bits_of<E>::value - kMarginBits;
			complex<E> a(E(2.0), E(3.0)), b(E(-1.5), E(0.75));
			complex<E> r = (a / b) * b;
			if (!agrees_to_bits(r.real(), a.real(), wanted) || !agrees_to_bits(r.imag(), a.imag(), wanted)) {
				if (reportTestCases) std::cout << "    FAIL: (a/b)*b bits re=" << measured_bits(r.real(), a.real())
				                               << " im=" << measured_bits(r.imag(), a.imag()) << ", wanted at least " << wanted << "\n";
				++failures;
			}
		}

		// -----------------------------------------------------------------
		// 7. abs, arg and the transcendentals compute in ereal (#2026): on ereal<8> they used
		//    to go through double and agree with the truth to 53 bits.  Measured now:
		//    abs(1+i) 880 bits, arg(1+i) 485, sqrt(2+3i)^2 865-866, exp(log(2+3i)) 476-477.
		// -----------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying abs / arg / sqrt / exp / log to full precision...\n";
		{
			const int wanted = ereal_bits_of<E>::value - kMarginBits;
			const complex<E> one_i(E(1.0), E(1.0));
			if (!agrees_to_bits(abs(one_i), s_sqrt2, wanted)) {
				if (reportTestCases) std::cout << "    FAIL: abs(1+i) bits=" << measured_bits(abs(one_i), s_sqrt2) << ", wanted at least " << wanted << "\n";
				++failures;
			}
			if (!agrees_to_bits(arg(one_i), s_pi_4, wanted)) {
				if (reportTestCases) std::cout << "    FAIL: arg(1+i) bits=" << measured_bits(arg(one_i), s_pi_4) << ", wanted at least " << wanted << "\n";
				++failures;
			}
			const complex<E> z(E(2.0), E(3.0));
			const complex<E> s = sqrt(z);
			const complex<E> back = s * s;
			if (!agrees_to_bits(back.real(), z.real(), wanted) || !agrees_to_bits(back.imag(), z.imag(), wanted)) {
				if (reportTestCases) std::cout << "    FAIL: sqrt(z)^2 bits re=" << measured_bits(back.real(), z.real())
				                               << " im=" << measured_bits(back.imag(), z.imag()) << ", wanted at least " << wanted << "\n";
				++failures;
			}
			const complex<E> rt = exp(log(z));
			if (!agrees_to_bits(rt.real(), z.real(), wanted) || !agrees_to_bits(rt.imag(), z.imag(), wanted)) {
				if (reportTestCases) std::cout << "    FAIL: exp(log(z)) bits re=" << measured_bits(rt.real(), z.real())
				                               << " im=" << measured_bits(rt.imag(), z.imag()) << ", wanted at least " << wanted << "\n";
				++failures;
			}
		}

		return failures;
	}

}  // anonymous namespace

#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "ereal complex number support library";
	std::string test_tag            = "complex";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyErealComplex(reportTestCases), "ereal", "complex");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
