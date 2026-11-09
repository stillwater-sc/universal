// addition.cpp: test suite runner for complex addition on classic floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// complex<cfloat> addition is component-wise cfloat addition, so each component of the sum
// must be bit-identical to the cfloat sum of the components. For cfloat<32,8> with subnormals,
// an IEEE single, that is also float addition. (This suite was gated on a BUILD_COMPLEX
// variable that nothing sets, and never ran: #2033.)
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <random>
#include <string>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/math/complex.hpp>
#include <universal/verification/test_suite.hpp>

namespace {
	using namespace sw::universal;

	// every component sum of complex<Real> against the cfloat sum, over all encodings of a small
	// type (both components drawn from the same stride through the encodings)
	template<typename Real>
	int VerifyComponentwiseAddition(unsigned stride, bool reportTestCases) {
		constexpr unsigned nbits = Real::nbits;
		const std::uint64_t n = 1ull << nbits;
		int nrOfFailedTestCases = 0;
		Real a, b, c, d;
		for (std::uint64_t i = 0; i < n; i += stride) {
			a.setbits(i);
			b.setbits((i * 7) & (n - 1));
			for (std::uint64_t j = 0; j < n; j += stride) {
				c.setbits(j);
				d.setbits((j * 5 + 3) & (n - 1));
				const complex<Real> s = complex<Real>(a, b) + complex<Real>(c, d);
				const Real re = a + c, im = b + d;
				// the encodings, which also tell -0 from +0
				const bool same_re = (s.real().isnan() && re.isnan()) || to_binary(s.real()) == to_binary(re);
				const bool same_im = (s.imag().isnan() && im.isnan()) || to_binary(s.imag()) == to_binary(im);
				if (!same_re || !same_im) {
					++nrOfFailedTestCases;
					if (reportTestCases && nrOfFailedTestCases < 10) {
						std::cout << "    FAIL (" << a << ", " << b << ") + (" << c << ", " << d << ") = " << s
						          << ", components (" << re << ", " << im << ")\n";
					}
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// complex<cfloat<32,8>> addition against float addition on random operands
	int VerifySingleAgainstFloat(unsigned nrRandoms, bool reportTestCases) {
		using Single = cfloat<32, 8, std::uint32_t, true, false, false>;
		std::mt19937 rng(2033);
		std::uniform_real_distribution<float> dist(-1.0e6f, 1.0e6f);
		int nrOfFailedTestCases = 0;
		for (unsigned k = 0; k < nrRandoms; ++k) {
			const float a = dist(rng), b = dist(rng), c = dist(rng), d = dist(rng);
			const complex<Single> s = complex<Single>(Single(a), Single(b)) + complex<Single>(Single(c), Single(d));
			if (float(s.real()) != a + c || float(s.imag()) != b + d) {
				++nrOfFailedTestCases;
				if (reportTestCases && nrOfFailedTestCases < 10) {
					std::cout << "    FAIL (" << a << ", " << b << ") + (" << c << ", " << d << ") = " << s << ", float ("
					          << a + c << ", " << b + d << ")\n";
				}
			}
		}
		return nrOfFailedTestCases;
	}

}  // namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
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

	std::string test_suite         = "classic cfloat complex addition validation";
	std::string test_tag           = "complex addition";
	bool reportTestCases           = true;
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using c8  = cfloat< 8, 3, std::uint8_t,  true, false, false>;
	using c12 = cfloat<12, 4, std::uint16_t, true, false, false>;

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyComponentwiseAddition<c8>(1, reportTestCases), "complex<cfloat<8,3>>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures

#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyComponentwiseAddition<c8>(1, reportTestCases), "complex<cfloat<8,3>>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComponentwiseAddition<c12>(17, reportTestCases), "complex<cfloat<12,4>>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySingleAgainstFloat(10000, reportTestCases), "complex<cfloat<32,8>>", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyComponentwiseAddition<c12>(1, reportTestCases), "complex<cfloat<12,4>>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComponentwiseAddition<cfloat<16, 5, std::uint16_t, true, false, false>>(257, reportTestCases), "complex<cfloat<16,5>>", test_tag);
#endif

#if	REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifySingleAgainstFloat(1000000, reportTestCases), "complex<cfloat<32,8>>", test_tag);
#endif

#if	REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyComponentwiseAddition<cfloat<16, 5, std::uint16_t, true, false, false>>(17, reportTestCases), "complex<cfloat<16,5>>", test_tag);
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
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
