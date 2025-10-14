// arithmetic_add.cpp: functional tests for arithmetic addition of floating-point values
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#define VALUE_THROW_ARITHMETIC_EXCEPTION 1
#define BITBLOCK_THROW_ARITHMETIC_EXCEPTION 1
//#include <universal/internal/bitblock/bitblock.hpp>  // TODO: remove: should not have an internal type in the public interface
#include <universal/internal/value/value.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult

// (sign, scale, fraction) representation using sbits for scale and fbits for fraction assuming a hidden bit
template<unsigned sbits, unsigned fbits>
int VerifyValueAdd(bool reportTestCases) {
	using namespace sw::universal;
	using namespace sw::universal::internal;
	//constexpr unsigned NR_OF_VALUES = (unsigned(1) << (1 + scale + fbits));
	constexpr unsigned abits = fbits + 4;
	int nrOfFailedTestCases = 0;
	value<fbits> a, b;
	value<abits+1> sum, ref;

	// assume scale is a 2's complement representation and thus ranges from -2^(sbits-1) to 2^(sbits-1) - 1
	int scale_lb = -(int(1) << (sbits - 1));
	int scale_ub = (int(1) << (sbits - 1)) - 1;
	unsigned max_fract = (unsigned(1) << fbits);
	bitblock<fbits> afraction, bfraction;
	for (unsigned lhssign = 0; lhssign < 2; ++lhssign) {
		for (int ascale = scale_lb; ascale < scale_ub; ++ascale) {
			for (unsigned afrac = 0; afrac < max_fract; ++afrac) {
				afraction = convert_to_bitblock<fbits>(afrac);
				a.set(lhssign == 1, ascale, afraction, false, false);
				// std::cout << to_triple(a) << std::endl;
				for (unsigned rhssign = 0; rhssign < 2; ++rhssign) {
					for (int bscale = scale_lb; bscale < scale_ub; ++bscale) {
						for (unsigned bfrac = 0; bfrac < max_fract; ++bfrac) {
							bfraction = convert_to_bitblock<fbits>(bfrac);
							b.set(rhssign == 1, bscale, bfraction, false, false);
							module_add<fbits, abits>(a, b, sum);
							//std::cout << to_triple(a) << " + " << to_triple(b) << " = " << to_triple(sum) << std::endl;

							double dsum = sum.to_double();
							ref = dsum;
							if (sum != ref) {
								++nrOfFailedTestCases;
								if (reportTestCases)	std::cout << to_triple(sum) << " != " << to_triple(ref) << std::endl;
								if (nrOfFailedTestCases > 25) return nrOfFailedTestCases;
								std::cout << a << " + " << b << " = " << sum << " vs " << ref << std::endl;
							}
#if 0
							// we can't use regular algebra as reference because it rounds the result
							double da = a.to_double();
							double db = b.to_double();
							double dsum = da + db;
							ref = dsum;
							if (sum != ref) {
								++nrOfFailedTestCases;
								if (reportTestCases)	std::cout << to_triple(sum) << " != " << to_triple(ref) << std::endl;
								if (nrOfFailedTestCases > 25) return nrOfFailedTestCases;
								std::cout << a << " + " << b << " = " << sum << " vs " << ref << std::endl;
							}
#endif
						}
					}
				}
			}
		}
	}

	return nrOfFailedTestCases;
}

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
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	bool reportTestCases = true;
	int nrOfFailedTestCases = 0;

	// Arithmetic tests for value class
	std::cout << "\nvalue addition arithmetic tests\n";
	std::cout << (reportTestCases ? " " : "not ") << "reporting individual testcases\n";

#if MANUAL_TESTING

	value<5> a = 8;
	value<5> b = -64;

	value<10> sum;
	cout << "a = " << components(a) << endl;
	cout << "b = " << components(b) << endl;
	module_add<5,9>(a, b, sum);
	cout << components(sum) << endl;

	cout << "0 transition\n";
	for (int i = -8; i < 8; ++i) {
		value<7> a = i;
		cout << a.get_fixed_point() << " " << components(a) << " " << a << endl;
	}

#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyValueAdd<5, 3>(reportTestCases), "value<3> scale 2^5", "addition");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyValueAdd<5, 4>(reportTestCases), "value<4> scale 2^5", "addition");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyValueAdd<5, 5>(reportTestCases), "value<5> scale 2^5", "addition");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyValueAdd<3, 8>(reportTestCases), "value<8> scale 2^3", "addition");
#endif

#endif // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
