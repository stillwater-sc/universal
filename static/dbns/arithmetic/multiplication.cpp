// multiplication.cpp: test suite runner for multiplication arithmetic of fixed-sized, arbitrary precision double-base logarithmic number system
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// configure the number system
#define DBNS_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/dbns/dbns.hpp>
#include <universal/number/dbns/table.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	//template<typename DbnsType,
	//	std::enable_if_t<is_dbns<DbnsType>, DbnsType> = 0
	//>
	template<typename DbnsType>
	int VerifyMultiplication(bool reportTestCases) {
		using std::abs;
		constexpr size_t nbits = DbnsType::nbits;
		//constexpr size_t fbbits = DbnsType::fbbits;
		//constexpr Behavior behavior = DbnsType::behavior;
		//using bt = typename DbnsType::BlockType;
		constexpr size_t NR_ENCODINGS = (1ull << nbits);
		int nrOfFailedTestCases = 0;

		DbnsType a{}, b{}, c{}, cref{}, maxvalue(SpecificValue::maxpos);
		double maxpos = double(maxvalue);
		for (size_t i = 0; i < NR_ENCODINGS; ++i) {
			a.setbits(i);
			double da = double(a);
			for (size_t j = 0; j < NR_ENCODINGS; ++j) {
				b.setbits(j);
				double db = double(b);

				double ref = da * db;
//				if (reportTestCases && !isInRange<DbnsType>(ref)) {
//					std::cerr << da << " * " << db << " = " << ref << " which is not in range " << range(a) << '\n';
//				}
				c = a * b;
				cref = ref;
//				std::cout << "ref  : " << to_binary(ref) << " : " << ref << '\n';
//				std::cout << "cref : " << std::setw(68) << to_binary(cref) << " : " << cref << '\n';
				if (c != cref) {
					if (!isInRange<DbnsType>(ref)) {
						if (abs(ref) > maxpos) {
							if (cref == maxvalue) continue;
						}
						else {
							if (cref.iszero()) continue;
						}
					}
					if (c.isnan() && cref.isnan()) continue; // NaN non-equivalence
					++nrOfFailedTestCases;
					if (reportTestCases) ReportBinaryArithmeticError("FAIL", "*", a, b, c, cref);
//					std::cout << "ref  : " << to_binary(ref) << " : " << ref << '\n';
//					std::cout << "cref : " << std::setw(68) << to_binary(cref) << " : " << cref << '\n';
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "*", a, b, c, ref);
				}
				if (nrOfFailedTestCases > 25) return nrOfFailedTestCases;
			}
		}
		return nrOfFailedTestCases;
	}

} }

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
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

	std::string test_suite  = "dbns multiplication validation";
	std::string test_tag    = "multiplication";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);

#if MANUAL_TESTING

	//using DBNS4_1_mod = dbns<4, 1, std::uint8_t, Behavior::Wrapping>;
	using DBNS4_1_sat = dbns<4, 1, std::uint8_t, Behavior::Saturating>;
	using DBNS4_2     = dbns<4, 2, std::uint8_t>;
	//using DBNS5_2     = dbns<5, 2, std::uint8_t>;
	using DBNS8_3     = dbns<8, 3, std::uint8_t>;
	//using DBNS9_4     = dbns<9, 4, std::uint8_t>;
	using DBNS16_5    = dbns<16, 5, std::uint16_t>;

	// nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<DBNS4_2>(true), "dbns<4,2, uint8_t>", test_tag);

	{
		DBNS4_2 a{ 3 }, b{ 0.375 }, c{ 0 }, one{ 1 };
		ReportValue(one, "one");
		c = a * b;
		ReportBinaryOperation(a, "*", b, c);
		a = 0.25; b = 0.25;
		c = a * b;
		ReportBinaryOperation(a, "*", b, c);
	}
	return 0;

	// generate individual testcases to hand trace/debug
	TestCase<DBNS4_1_sat, float>(TestCaseOperator::MUL, 0.353f, -0.353f);
	TestCase<DBNS16_5, double>(TestCaseOperator::MUL, INFINITY, INFINITY);
	TestCase<DBNS8_3, float>(TestCaseOperator::MUL, 0.5f, -0.5f);

	reportTestCases = true;
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<DBNS4_1_mod>(false), "dbns<4,1,uint8_t,Behavior::Wrapping>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<DBNS4_1_sat>(reportTestCases), "dbns<4,1, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<DBNS4_2>(reportTestCases), "dbns<4,2, uint8_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<DBNS5_2>(reportTestCases), "dbns<5,2, uint8_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<DBNS8_3>(reportTestCases), "dbns<8,3, uint8_t>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	using DBNS4_0_sat = dbns<4, 0, std::uint8_t>;
	using DBNS4_1_sat = dbns<4, 1, std::uint8_t>;
	using DBNS4_2_sat = dbns<4, 2, std::uint8_t>;
//	using DBNS4_3_sat = dbns<4, 3, std::uint8_t>;
	using DBNS5_2_sat = dbns<5, 2, std::uint8_t>;
	using DBNS8_1_sat = dbns<8, 1, std::uint8_t>;
	using DBNS8_4_sat = dbns<8, 4, std::uint8_t>;
	using DBNS8_6_sat = dbns<8, 6, std::uint8_t>;
	using DBNS9_0_sat = dbns<9, 0, std::uint8_t>;
	using DBNS9_4_sat = dbns<9, 4, std::uint8_t>;
	using DBNS9_7_sat = dbns<9, 7, std::uint8_t>;
//	using DBNS9_8_sat = dbns<9, 8, std::uint8_t>;

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<DBNS4_0_sat>(reportTestCases), "dbns<4,0, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<DBNS4_1_sat>(reportTestCases), "dbns<4,1, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<DBNS4_2_sat>(reportTestCases), "dbns<4,2, uint8_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<DBNS4_3_sat>(reportTestCases), "dbns<4,3, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<DBNS5_2_sat>(reportTestCases), "dbns<5,2, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<DBNS8_1_sat>(reportTestCases), "dbns<8,1, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<DBNS8_4_sat>(reportTestCases), "dbns<8,4, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<DBNS8_6_sat>(reportTestCases), "dbns<8,6, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<DBNS9_0_sat>(reportTestCases), "dbns<9,0, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<DBNS9_4_sat>(reportTestCases), "dbns<9,4, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<DBNS9_7_sat>(reportTestCases), "dbns<9,7, uint8_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<DBNS9_8_sat>(reportTestCases), "dbns<9,8, uint8_t>", test_tag);

#endif

#if REGRESSION_LEVEL_2
	using DBNS10_0_sat = dbns<10, 0, std::uint8_t>;
	using DBNS10_4_sat = dbns<10, 4, std::uint8_t>;
	using DBNS10_8_sat = dbns<10, 8, std::uint8_t>;

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<DBNS10_0_sat>(reportTestCases), "dbns<10,0, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<DBNS10_4_sat>(reportTestCases), "dbns<10,4, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<DBNS10_8_sat>(reportTestCases), "dbns<10,8, uint8_t>", test_tag);
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
	std::cerr << msg << std::endl;
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
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
