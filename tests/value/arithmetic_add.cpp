// arithmetic_add.cpp: functional tests for arithmetic add of values
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/internal/bitblock/bitblock.hpp>  // TODO: remove: should not have an internal type in the public interface
#include <universal/internal/value/value.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult

// (sign, scale, fraction) representation using sbits for scale and fbits for fraction assuming a hidden bit
template<size_t sbits, size_t fbits>
int VerifyValueAdd(const std::string& tag, bool bReportIndividualTestCases) {
	using namespace sw::universal;
	using namespace sw::universal::internal;
	//constexpr size_t NR_OF_VALUES = (size_t(1) << (1 + scale + fbits));
	constexpr size_t abits = fbits + 4;
	int nrOfFailedTestCases = 0;
	value<fbits> a, b;
	value<abits+1> sum, ref;

	// assume scale is a 2's complement representation and thus ranges from -2^(sbits-1) to 2^(sbits-1) - 1
	int scale_lb = -(int(1) << (sbits - 1));
	int scale_ub = (int(1) << (sbits - 1)) - 1;
	size_t max_fract = (size_t(1) << fbits);
	bitblock<fbits> afraction, bfraction;
	for (size_t sign = 0; sign < 2; ++sign) {
		for (int scale = scale_lb; scale < scale_ub; ++scale) {
			for (size_t afrac = 0; afrac < max_fract; ++afrac) {
				afraction = convert_to_bitblock<fbits>(afrac);
				a.set(sign == 1, scale, afraction, false, false);
				std::cout << to_triple(a) << std::endl;
				for (size_t sign = 0; sign < 2; ++sign) {
					for (int scale = scale_lb; scale < scale_ub; ++scale) {
						for (size_t bfrac = 0; bfrac < max_fract; ++bfrac) {
							bfraction = convert_to_bitblock<fbits>(bfrac);
							b.set(sign == 1, scale, bfraction, false, false);
							module_add<fbits, abits>(a, b, sum);
							std::cout << to_triple(a) << " + " << to_triple(b) << " = " << to_triple(sum) << std::endl;

							double dsum = sum.to_double();
							ref = dsum;
							if (sum != ref) {
								++nrOfFailedTestCases;
								if (bReportIndividualTestCases)	std::cout << to_triple(sum) << " != " << to_triple(ref) << std::endl;
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
								if (bReportIndividualTestCases)	std::cout << to_triple(sum) << " != " << to_triple(ref) << std::endl;
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

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	// Arithmetic tests for value class
	std::cout << "\nvalue addition arithmetic tests\n";
	std::cout << (bReportIndividualTestCases ? " " : "not ") << "reporting individual testcases\n";

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

	nrOfFailedTestCases += ReportTestResult(VerifyValueAdd<3, 3>("FAIL", bReportIndividualTestCases), "value<3>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyValueAdd<3, 5>("FAIL", bReportIndividualTestCases), "value<5>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyValueAdd<3, 8>("FAIL", bReportIndividualTestCases), "value<8>", "addition");

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
