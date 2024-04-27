// multiplication.cpp: test suite runner for multiplication arithmetic of fixed-sized, arbitrary precision double-base logarithmic number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <vector>
#include <algorithm>
// configure the number system
#define DBNS_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/dbns/dbns.hpp>
#include <universal/number/dbns/table.hpp>
#include <universal/verification/test_reporters.hpp>
//#include <universal/verification/test_suite.hpp>   // the generic VerifyMultiplication doesn't deal with the LNS special cases
//#include <universal/verification/dbns_test_suite.hpp>  // is that the right solution to specialize?

namespace sw {
	namespace universal {
		namespace local {

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

				if constexpr (bCollectDbnsEventStatistics) dbnsStats.reset();

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
						}
						else {
							//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "*", a, b, c, ref);
						}
						if (nrOfFailedTestCases > 25) return nrOfFailedTestCases;
					}
				}
				if constexpr (bCollectDbnsEventStatistics) if (reportTestCases) std::cout << dbnsStats << '\n';
				return nrOfFailedTestCases;
			}

			template<typename DbnsType>
			struct DbnsSample {
				DbnsSample(const DbnsType& a, const DbnsType& b, const DbnsType& c, const DbnsType& cref, double ref, int p, int v) : a{ a }, b{ b }, c{ c }, cref{ cref }, ref{ ref }, patternOrder{ p }, valueOrder{ v } {}
				DbnsType a, b, c, cref;
				double   ref;
				int      patternOrder;
				int      valueOrder;
			};

			template<typename DbnsType>
			std::ostream& operator<<(std::ostream& ostr, const DbnsSample<DbnsType>& s) {
				ostr << std::setw(10) << s.patternOrder << " : "
					<< to_binary(s.a)
					<< " * "
					<< to_binary(s.b)
					<< " = "
					<< to_binary(s.c)
					<< " : "
					<< std::setw(10) << s.c
					<< " : "
					<< std::setw(10) << s.ref
					<< " = "
					<< std::setw(10) << s.a
					<< " * "
					<< std::setw(10) << s.b
					<< " : "
					<< to_binary(s.cref)
					<< " : "
					<< std::setw(10) << s.valueOrder;
				if (s.c.isnan()) ostr << " : PASS"; else if (s.c == s.cref) ostr << " : PASS"; else ostr << " :     FAIL";
				return ostr;
			}

			template<typename DbnsType,
				std::enable_if_t< is_dbns<DbnsType>, bool> = true
			>
			int GenerateOrdered(bool reportTestCases) {
				using std::abs;
				constexpr size_t nbits = DbnsType::nbits;
				//constexpr size_t fbbits = DbnsType::fbbits;
				//constexpr Behavior behavior = DbnsType::behavior;
				//using bt = typename DbnsType::BlockType;
				constexpr size_t NR_ENCODINGS = (1ull << nbits);
				int nrOfFailedTestCases = 0;

				std::vector<DbnsSample<DbnsType>> v;
				DbnsType a{}, b{}, c{}, cref{}, maxvalue(SpecificValue::maxpos);
				// double maxpos = double(maxvalue);
				for (size_t i = 0; i < NR_ENCODINGS; ++i) {
					a.setbits(i);
					double da = double(a);
					for (size_t j = 0; j < NR_ENCODINGS; ++j) {
						b.setbits(j);
						double db = double(b);

						double ref = da * db;
						c = a * b;
						cref = ref;
						DbnsSample<DbnsType> s(a, b, c, cref, ref, i * NR_ENCODINGS + j, 0);
						v.push_back(s);
					}
				}

				std::sort(v.begin(), v.end(),
					[](DbnsSample<DbnsType> a, DbnsSample<DbnsType> b) {
						if (a.a.isnan() && !b.b.isnan()) {
							return true;
						}
						else if (!a.a.isnan() && b.b.isnan()) {
							return false;
						}
						else if (a.a.isnan() && b.b.isnan()) {
							return false;
						}
						else {
							return a.ref < b.ref;
						}
					});

				// assigne the value order
				for (unsigned valueOrder = 0; valueOrder < v.size(); ++valueOrder) {
					v[valueOrder].valueOrder = valueOrder;
				}
				for (auto e : v) {
					std::cout << e << '\n';
				}
				return nrOfFailedTestCases;
			}
		}
	}
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

	std::string test_suite  = "dbns multiplication validation";
	std::string test_tag    = "multiplication";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	//using DBNS4_1_mod = dbns<4, 1, std::uint8_t, Behavior::Wrapping>;
	using DBNS4_1_sat = dbns<4, 1, std::uint8_t, Behavior::Saturating>;
	using DBNS4_2     = dbns<4, 2, std::uint8_t>;
	using DBNS5_2     = dbns<5, 2, std::uint8_t>;
	using DBNS8_3     = dbns<8, 3, std::uint8_t>;
	//using DBNS9_4     = dbns<9, 4, std::uint8_t>;
	using DBNS16_5    = dbns<16, 5, std::uint16_t>;

	GenerateOrdered<DBNS5_2>(false);
	return 0;

//	 nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<DBNS4_2>(true), "dbns<4,2, uint8_t>", test_tag);
//	 nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<DBNS5_2>(true), "dbns<5,2, uint8_t>", test_tag);

	{
		float d{ 0 };
		DBNS5_2 a{ 0.5 }, b{ 1.125 }, c{ 0 };
		a = 4.5; b = 3.375;
		c = a * b;
		ReportBinaryOperation(a, "*", b, c);
		d = 4.5 * 3.375;
		ReportValue(d, "d is 15.1875");
		c = d;
		ReportValue(c, "c should be 13.5");

		c = a * b;
		ReportBinaryOperation(a, "*", b, c);
		d = 0.5 * 1.125;
		c = d;
		ReportValue(c);
		d = 3.0 * 27.0;
		c = d;
		ReportValue(c);
		d = 9.0 * 27.0;
		c = d;
		ReportValue(c);
		a = 0.375; b = 3.375;
		c = a * b;
		ReportBinaryOperation(a, "*", b, c);


	}
	return 0;

	{
		DBNS4_2 a{ 3 }, b{ 0.375 }, c{ 0 }, one{ 1 };
		ReportValue(one, "one");
		c = a * b;
		ReportBinaryOperation(a, "*", b, c);
		a = 0.25; b = 0.375;
		c = a * b;
		ReportBinaryOperation(a, "*", b, c);
		b = -b;
		c = a * b;
		ReportBinaryOperation(a, "*", b, c);
		a = 0.375;
		c = a * b;
		ReportBinaryOperation(a, "*", b, c);
		DBNS4_2 d{ -0.14 };
		ReportValue(d);
		a = 0.5f; b = -0.25f;
		c = a * b;
		ReportBinaryOperation(a, "*", b, c);
	}

	// generate individual testcases to hand trace/debug
	TestCase<DBNS4_1_sat, float>(TestCaseOperator::MUL, 0.353f, -0.353f);
	TestCase<DBNS16_5, double>(TestCaseOperator::MUL, INFINITY, INFINITY);
	TestCase<DBNS8_3, float>(TestCaseOperator::MUL, 0.5f, -0.5f);

	reportTestCases = true;
//	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<DBNS4_1_mod>(false), "dbns<4,1,uint8_t,Behavior::Wrapping>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<DBNS4_1_sat>(reportTestCases), "dbns<4,1, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<DBNS4_2>(reportTestCases), "dbns<4,2, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<DBNS5_2>(reportTestCases), "dbns<5,2, uint8_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<DBNS8_3>(reportTestCases), "dbns<8,3, uint8_t>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	using DBNS4_1_sat = dbns<4, 1, std::uint8_t>;
	using DBNS4_2_sat = dbns<4, 2, std::uint8_t>;
	using DBNS5_2_sat = dbns<5, 2, std::uint8_t>;
	using DBNS6_2_sat = dbns<6, 2, std::uint8_t>;
	using DBNS6_3_sat = dbns<6, 3, std::uint8_t>;
	using DBNS8_1_sat = dbns<8, 1, std::uint8_t>;
	using DBNS8_2_sat = dbns<8, 2, std::uint8_t>;
	using DBNS8_3_sat = dbns<8, 3, std::uint8_t>;
	using DBNS8_4_sat = dbns<8, 4, std::uint8_t>;
	using DBNS8_5_sat = dbns<8, 5, std::uint8_t>;
	using DBNS8_6_sat = dbns<8, 6, std::uint8_t>;
	using DBNS9_4_sat = dbns<9, 4, std::uint8_t>;
	using DBNS9_7_sat = dbns<9, 7, std::uint8_t>;

	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<DBNS4_1_sat>(reportTestCases), "dbns<4,1, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<DBNS4_2_sat>(reportTestCases), "dbns<4,2, uint8_t>", test_tag);

	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<DBNS5_2_sat>(reportTestCases), "dbns<5,2, uint8_t>", test_tag);

	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<DBNS6_2_sat>(reportTestCases), "dbns<6,2, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<DBNS6_3_sat>(reportTestCases), "dbns<6,3, uint8_t>", test_tag);

	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<DBNS8_1_sat>(reportTestCases), "dbns<8,1, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<DBNS8_2_sat>(reportTestCases), "dbns<8,2, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<DBNS8_3_sat>(reportTestCases), "dbns<8,3, uint8_t>", test_tag);	
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<DBNS8_4_sat>(reportTestCases), "dbns<8,4, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<DBNS8_5_sat>(reportTestCases), "dbns<8,5, uint8_t>", test_tag);	
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<DBNS8_6_sat>(reportTestCases), "dbns<8,6, uint8_t>", test_tag);

	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<DBNS9_4_sat>(reportTestCases), "dbns<9,4, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<DBNS9_7_sat>(reportTestCases), "dbns<9,7, uint8_t>", test_tag);

#endif

#if REGRESSION_LEVEL_2
	using DBNS10_2_sat = dbns<10, 2, std::uint8_t>;
	using DBNS10_4_sat = dbns<10, 4, std::uint8_t>;
	using DBNS10_8_sat = dbns<10, 8, std::uint8_t>;

	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<DBNS10_2_sat>(reportTestCases), "dbns<10,2, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<DBNS10_4_sat>(reportTestCases), "dbns<10,4, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(local::VerifyMultiplication<DBNS10_8_sat>(reportTestCases), "dbns<10,8, uint8_t>", test_tag);
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
