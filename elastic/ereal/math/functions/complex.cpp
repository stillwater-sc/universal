// complex.cpp: regression tests for sw::universal::complex<ereal> support.
//
// ereal plugs into the portable sw::universal::complex<T> (NOT std::complex<T>,
// which is UB on user-defined types per ISO C++ 26.2/2). Complex arithmetic and
// real/imag/conj/norm/arg/abs carry ereal's full working precision; the shared
// library's complex transcendentals pivot through double, so those are checked
// only to ~double tolerance here. (Issue #1167)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <limits>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	template<unsigned maxlimbs>
	bool IsClose(const sw::universal::ereal<maxlimbs>& a, double target_val, double tolerance = 1e-12) {
		double diff = std::abs(double(a) - target_val);
		return diff <= tolerance * (1.0 + std::abs(target_val));
	}

	// agreement of a and b in bits: ilogb(|a - b|); a large negative value (or the
	// -100000 sentinel for an exact match) means agreement to that many bits.
	template<unsigned maxlimbs>
	int AgreementBits(const sw::universal::ereal<maxlimbs>& a, const sw::universal::ereal<maxlimbs>& b) {
		using namespace sw::universal;
		ereal<maxlimbs> d = a - b;
		return d.iszero() ? -100000 : ilogb(d);
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
			if (!IsClose(z.real(), 3.0) || !IsClose(z.imag(), 4.0)) {
				if (reportTestCases) std::cout << "    FAIL: member real/imag\n";
				++failures;
			}
			if (!IsClose(real(z), 3.0) || !IsClose(imag(z), 4.0)) {
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
		// 4. conj / norm / abs at full precision
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
			if (abs(z) != E(5.0)) {     // sqrt(25) is exact
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
		// 6. high-precision identity: (a / b) * b == a to many bits
		// -----------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying (a/b)*b == a to high precision...\n";
		{
			complex<E> a(E(2.0), E(3.0)), b(E(-1.5), E(0.75));
			complex<E> r = (a / b) * b;
			int br = AgreementBits(r.real(), a.real());
			int bi = AgreementBits(r.imag(), a.imag());
			if (br > -200 || bi > -200) {
				if (reportTestCases) std::cout << "    FAIL: (a/b)*b bits re=" << br << " im=" << bi << "\n";
				++failures;
			}
		}

		// -----------------------------------------------------------------
		// 7. transcendentals available and correct to ~double (shared library
		//    pivots through std::complex<double>)
		// -----------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying complex transcendentals (~double)...\n";
		{
			complex<E> z(E(2.0), E(3.0));
			complex<E> back = sqrt(z) * sqrt(z);
			if (!IsClose(back.real(), 2.0, 1e-10) || !IsClose(back.imag(), 3.0, 1e-10)) {
				if (reportTestCases) std::cout << "    FAIL: sqrt round-trip\n";
				++failures;
			}
			complex<E> w(E(0.5), E(0.7));
			complex<E> rt = log(exp(w));
			if (!IsClose(rt.real(), 0.5, 1e-10) || !IsClose(rt.imag(), 0.7, 1e-10)) {
				if (reportTestCases) std::cout << "    FAIL: log(exp(z)) round-trip\n";
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
