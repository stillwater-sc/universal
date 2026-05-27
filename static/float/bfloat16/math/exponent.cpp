// exponent.cpp: test suite for bfloat16 exponent-manipulation functions
//               ldexp / frexp / scalbn / logb / ilogb (issue #941)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	// Verify the exponent-manipulation functions by their mathematical
	// properties (not by re-deriving the double delegation): scaling by a power
	// of two is exact in bfloat16, so these identities must hold exactly.
	int VerifyExponentFunctions(bool reportTestCases) {
		int nrOfFailedTestCases = 0;

		// representative finite, normal bfloat16 values (all exactly representable)
		const float values[] = { 1.0f, 0.5f, 2.0f, 3.5f, 0.75f, 100.0f, 0.015625f, -4.0f, -0.25f };
		const int   shifts[] = { -5, -1, 0, 1, 3, 7 };

		for (float fv : values) {
			bfloat16 x(fv);

			// frexp: x == fraction * 2^e, with |fraction| in [0.5, 1)
			int e = 0;
			bfloat16 fraction = frexp(x, &e);
			double fabsd = std::abs(double(fraction));
			if (!(fabsd >= 0.5 && fabsd < 1.0)) {
				if (reportTestCases) std::cout << "FAIL frexp fraction out of [0.5,1): " << fv << " -> " << double(fraction) << '\n';
				++nrOfFailedTestCases;
			}
			if (ldexp(fraction, e) != x) {
				if (reportTestCases) std::cout << "FAIL frexp/ldexp roundtrip: " << fv << '\n';
				++nrOfFailedTestCases;
			}

			// ilogb / logb / frexp exponent relationship: frexp exponent == ilogb + 1
			int il = ilogb(x);
			if (il != e - 1) {
				if (reportTestCases) std::cout << "FAIL ilogb vs frexp exponent: " << fv << " ilogb=" << il << " frexp_e=" << e << '\n';
				++nrOfFailedTestCases;
			}
			// logb returns the unbiased exponent as a value: logb(x) == ilogb(x)
			if (logb(x) != bfloat16(il)) {
				if (reportTestCases) std::cout << "FAIL logb != ilogb for " << fv << '\n';
				++nrOfFailedTestCases;
			}
			// 2^ilogb(x) <= |x| < 2^(ilogb(x)+1)
			double ax = std::abs(double(x));
			if (!(std::ldexp(1.0, il) <= ax && ax < std::ldexp(1.0, il + 1))) {
				if (reportTestCases) std::cout << "FAIL ilogb range for " << fv << '\n';
				++nrOfFailedTestCases;
			}

			for (int n : shifts) {
				// scalbn == ldexp (radix 2)
				if (scalbn(x, n) != ldexp(x, n)) {
					if (reportTestCases) std::cout << "FAIL scalbn != ldexp: " << fv << " n=" << n << '\n';
					++nrOfFailedTestCases;
				}
				// ldexp by a power of two is exact -> exact round-trip (no over/underflow in this range)
				if (ldexp(ldexp(x, n), -n) != x) {
					if (reportTestCases) std::cout << "FAIL ldexp roundtrip: " << fv << " n=" << n << '\n';
					++nrOfFailedTestCases;
				}
				// ldexp(x, n) == x * 2^n (exact power-of-two multiply)
				if (ldexp(x, n) != bfloat16(double(x) * std::ldexp(1.0, n))) {
					if (reportTestCases) std::cout << "FAIL ldexp value: " << fv << " n=" << n << '\n';
					++nrOfFailedTestCases;
				}
			}
		}

		// ldexp(x, 0) == x for every value
		for (float fv : values) {
			bfloat16 x(fv);
			if (ldexp(x, 0) != x || scalbn(x, 0) != x) {
				if (reportTestCases) std::cout << "FAIL ldexp/scalbn by 0: " << fv << '\n';
				++nrOfFailedTestCases;
			}
		}

		return nrOfFailedTestCases;
	}

}} // namespace sw::universal

#define MANUAL_TESTING 0

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "bfloat16 mathlib exponent-manipulation functions";
	std::string test_tag    = "ldexp/frexp/scalbn/logb/ilogb";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	{
		bfloat16 x(3.5f);
		int e;
		bfloat16 f = frexp(x, &e);
		std::cout << "frexp(3.5) = " << double(f) << " * 2^" << e << '\n';
		std::cout << "ldexp(3.5, 2) = " << double(ldexp(x, 2)) << '\n';
		std::cout << "ilogb(3.5) = " << ilogb(x) << ", logb(3.5) = " << double(logb(x)) << '\n';
	}
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else
	nrOfFailedTestCases += ReportTestResult(VerifyExponentFunctions(reportTestCases), "bfloat16", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
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
