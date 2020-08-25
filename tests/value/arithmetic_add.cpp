// arithmetic_add.cpp: functional tests for arithmetic add of values
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include "universal/bitblock/bitblock.hpp"
#include "universal/value/value.hpp"
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"

// (sign, scale, fraction) representation using sbits for scale and fbits for fraction assuming a hidden bit
template<size_t sbits, size_t fbits>
int VerifyValueAdd(const std::string& tag, bool bReportIndividualTestCases) {
	//constexpr size_t NR_OF_VALUES = (size_t(1) << (1 + scale + fbits));
	constexpr size_t abits = fbits + 4;
	int nrOfFailedTestCases = 0;
	sw::unum::value<fbits> a, b;
	sw::unum::value<abits+1> sum, ref;

	// assume scale is a 2's complement representation and thus ranges from -2^(sbits-1) to 2^(sbits-1) - 1
	int scale_lb = -(int(1) << (sbits - 1));
	int scale_ub = (int(1) << (sbits - 1)) - 1;
	size_t max_fract = (size_t(1) << fbits);
	sw::unum::bitblock<fbits> afraction, bfraction;
	for (size_t sign = 0; sign < 2; ++sign) {
		for (int scale = scale_lb; scale < scale_ub; ++scale) {
			for (size_t afrac = 0; afrac < max_fract; ++afrac) {
				afraction = sw::unum::convert_to_bitblock<fbits>(afrac);
				a.set(sign == 1, scale, afraction, false, false);
				// std::cout << components(a) << std::endl;
				for (size_t sign = 0; sign < 2; ++sign) {
					for (int scale = scale_lb; scale < scale_ub; ++scale) {
						for (size_t bfrac = 0; bfrac < max_fract; ++bfrac) {
							bfraction = sw::unum::convert_to_bitblock<fbits>(bfrac);
							b.set(sign == 1, scale, bfraction, false, false);
							sw::unum::module_add<fbits, abits>(a, b, sum);
							double da = a.to_double();
							double db = b.to_double();
							double dsum = sum.to_double();
							ref = dsum;
							if (sum != ref) {
								++nrOfFailedTestCases;
								if (bReportIndividualTestCases)
									std::cout << components(a) << " + " << components(b) << " = " << components(sum) << std::endl;
							}
						}
					}
				}
			}
		}
	}

	return nrOfFailedTestCases;
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	// Arithmetic tests for value class
	cout << endl << "value addition arithmetic tests" << endl;
	cout << (bReportIndividualTestCases ? " " : "not ") << "reporting individual testcases" << endl;

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

	nrOfFailedTestCases += ReportTestResult(VerifyValueAdd<3, 5>("FAIL", bReportIndividualTestCases), "value<5>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyValueAdd<3, 8>("FAIL", bReportIndividualTestCases), "value<8>", "addition");

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
