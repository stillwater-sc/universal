// division.cpp: test suite runner for posit division
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

//
// Configure the posit template environment
// enable/disable general or specialized posit configurations
#define POSIT_FAST_SPECIALIZATION
// enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
// enable/disable tracing 
// when you define ALGORITHM_VERBOSE_OUTPUT executing an ADD the code will print intermediate results
//#define ALGORITHM_VERBOSE_OUTPUT
#define ALGORITHM_TRACE_DIV
#include <universal/number/posito/posito.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_randoms.hpp>

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<typename PositType, typename Ty>
void GenerateTestCase(Ty a, Ty b) {
	constexpr unsigned nbits = PositType::nbits;
	PositType pa, pb, pc, pref;
	pa = a;
	pb = b;
	pc = pa / pb;
	Ty ref = a / b;
	pref = ref;
	auto prec = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	ReportBinaryOperation(pa, "/", pb, pc);
	std::cout << (pref == pc ? "PASS" : "FAIL") << "\n\n";
	std::cout << std::setprecision(prec);
}

template<typename PositType>
void GenerateWorstCaseDivision() {
	constexpr unsigned nbits = PositType::nbits;
	constexpr unsigned es = PositType::es;
	PositType p_plus_eps(1), p_minus_eps(1), p_result;
	p_plus_eps++;
	p_minus_eps--;
	p_result = p_plus_eps / p_minus_eps;
	if (es < 2) {
		std::cout << type_tag(PositType()) << " minpos = " << std::fixed << std::setprecision(nbits) << sw::universal::posit<nbits, es>(sw::universal::SpecificValue::minpos) << std::dec << std::endl;
	}
	else {
		std::cout << type_tag(PositType()) << " minpos = " << std::setprecision(nbits) << sw::universal::posit<nbits, es>(sw::universal::SpecificValue::minpos) << std::endl;
	}
	std::cout << p_plus_eps.get() << " / " << p_minus_eps.get() << " = " << p_result.get() << std::endl;
	std::cout << std::setprecision(nbits - 2) << std::setw(nbits) << p_plus_eps << " / " << std::setw(nbits) << p_minus_eps << " = " << std::setw(nbits) << p_result << std::endl;
	std::cout << std::endl;
}

/*
As we discussed, I think the following cases are tricky for the divide function. I discovered them when trying to approximate x/y with x times (1/y). All are in the <16,1> environment, so you should be able to test them easily.

Let

A = posit represented by integer 20479 (value is 8191/4096 = 1.999755859375)
B = posit represented by integer 2 (value is 1/67108864 = 0.00000001490116119384765625)
C = posit represented by integer 16383 (value is 8191/8192 = 0.9998779296875)
D = posit represented by integer 16385 (value is 4097/4096 = 1.000244140625)

Then the divide routine should return the following:

B / A = posit represented by integer 2 (that is, the division leaves B unchanged)
A / B = posit represented by integer 32766 (value is 67108864)
C / D = posit represented by integer 16381 (value is 0.996337890625)
D / C = posit represented by integer 16386 (value is 1.00048828125)

Notice that multiplying the B/A and A/B results gives 1 exactly, but multiplying the C/D and D/C results gives 1.000121891498565673828125.
*/
template<typename PositType>
void ToughDivisions2() {
	constexpr unsigned nbits = PositType::nbits;
	PositType a, b, c, d;
	a.setbits(20479);
	b.setbits(2);
	c.setbits(16383);
	d.setbits(16385);

	GenerateTestCase<PositType>(b, a);
	GenerateTestCase<PositType>(a, b);
	GenerateTestCase<PositType>(c, d);
	GenerateTestCase<PositType>(d, c);
}

namespace sw {
	namespace testing {
		// enumerate all division cases for a posit configuration: is within 10sec till about nbits = 14
		template<typename PositType>
		int VerifyDivision(bool reportTestCases) {
			constexpr unsigned nbits = PositType::nbits;
			constexpr unsigned NR_POSITS = (unsigned(1) << nbits);
			int nrOfFailedTests = 0;
			for (unsigned i = 0; i < NR_POSITS; i++) {
				PositType pa;
				pa.setbits(i);
				double da = double(pa);
				for (unsigned j = 0; j < NR_POSITS; j++) {
					PositType pb, pdiv, pref;
					pb.setbits(j);
					double db = double(pb);

#if POSIT_THROW_ARITHMETIC_EXCEPTION
					try {
						pdiv = pa / pb;
						pref = da / db;
					}
					catch (const posit_divide_by_zero&) {
						if (pb.iszero()) {
							// correctly caught the divide by zero condition
							continue;
							//pdiv.setnar();
						}
						else {
							if (reportTestCases) ReportBinaryArithmeticError("FAIL", "/", pa, pb, pdiv, pref);
							throw; // rethrow
						}
					}
					catch (const posit_divide_by_nar&) {
						if (pb.isnar()) {
							// correctly caught the divide by nar condition
							continue;
							//pdiv = 0.0f;
						}
						else {
							if (reportTestCases) ReportBinaryArithmeticError("FAIL", "/", pa, pb, pdiv, pref);
							throw; // rethrow
						}
					}
					catch (const posit_numerator_is_nar&) {
						if (pa.isnar()) {
							// correctly caught the numerator is nar condition
							continue;
							//pdiv.setnar();
						}
						else {
							if (reportTestCases) ReportBinaryArithmeticError("FAIL", "/", pa, pb, pdiv, pref);
							throw; // rethrow
						}
					}
#else
					pdiv = pa / pb;
					pref = da / db;
#endif
					// check against the IEEE reference
					if (pdiv != pref) {
						if (reportTestCases) ReportBinaryArithmeticError("FAIL", "/", pa, pb, pdiv, pref);
						nrOfFailedTests++;
					}
					else {
						//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "/", pa, pb, pdiv, pref);
					}

				}
			}
			return nrOfFailedTests;
		}
	}
}

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

	std::string test_suite  = "posit division validation";
	std::string test_tag    = "division";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

//	ToughDivisions2<posit<16,1>>();

	posito<16, 2> pa(1), pb(1);
	pa--; pb++;
	double a, b;
	a = double(pa);
	b = double(pb);
	GenerateTestCase<posit<16, 2>, double>(a, b);
	GenerateTestCase<posito<16, 2>, double>(a, b);

	// Generate the worst fraction pressure for different posit configurations
//	GenerateWorstCaseDivision<posit< 8, 1>>();
//	GenerateWorstCaseDivision<posit<16, 2>>();

	nrOfFailedTestCases += ReportTestResult(sw::testing::VerifyDivision<posit<8, 0>>(true), "posit<8,0>", "division");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<2, 0>(reportTestCases), "posit< 2,0>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<3, 0>(reportTestCases), "posit< 3,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<3, 1>(reportTestCases), "posit< 3,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<3, 2>(reportTestCases), "posit< 3,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<3, 3>(reportTestCases), "posit< 3,3>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<4, 0>(reportTestCases), "posit< 4,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<4, 1>(reportTestCases), "posit< 4,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<4, 2>(reportTestCases), "posit< 4,2>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<5, 0>(reportTestCases), "posit< 5,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<5, 1>(reportTestCases), "posit< 5,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<5, 2>(reportTestCases), "posit< 5,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<5, 3>(reportTestCases), "posit< 5,3>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<6, 0>(reportTestCases), "posit< 6,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<6, 1>(reportTestCases), "posit< 6,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<6, 2>(reportTestCases), "posit< 6,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<6, 3>(reportTestCases), "posit< 6,3>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<6, 4>(reportTestCases), "posit< 6,4>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<7, 0>(reportTestCases), "posit< 7,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<7, 1>(reportTestCases), "posit< 7,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<7, 2>(reportTestCases), "posit< 7,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<7, 3>(reportTestCases), "posit< 7,3>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<7, 4>(reportTestCases), "posit< 7,4>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 0>(reportTestCases), "posit< 8,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 1>(reportTestCases), "posit< 8,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 2>(reportTestCases), "posit< 8,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 3>(reportTestCases), "posit< 8,3>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 4>(reportTestCases), "posit< 8,4>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 5>(reportTestCases), "posit< 8,5>", "division");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<10, 0>(reportTestCases), "posit<10,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<10, 1>(reportTestCases), "posit<10,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<10, 2>(reportTestCases), "posit<10,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<10, 3>(reportTestCases), "posit<10,3>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<16, 2>(reportTestCases, OPCODE_DIV, 1000), "posit<16,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<24, 2>(reportTestCases, OPCODE_DIV, 1000), "posit<24,2>", "division");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<20, 1>(reportTestCases, OPCODE_DIV, 1000), "posit<20,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<28, 1>(reportTestCases, OPCODE_DIV, 1000), "posit<28,1>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<32, 1>(reportTestCases, OPCODE_DIV, 1000), "posit<32,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<32, 2>(reportTestCases, OPCODE_DIV, 1000), "posit<32,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<32, 3>(reportTestCases, OPCODE_DIV, 1000), "posit<32,3>", "division");
#endif

#if REGRESSION_LEVEL_4
	// nbits = 48 also shows failures
    nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<48, 2>(reportTestCases, OPCODE_DIV, 1000), "posit<48,2>", "division");

    // nbits=64 requires long double compiler support
    nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<64, 2>(reportTestCases, OPCODE_DIV, 1000), "posit<64,2>", "division");
    nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<64, 3>(reportTestCases, OPCODE_DIV, 1000), "posit<64,3>", "division");
    // posit<64,4> is hitting subnormal numbers
    nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<64, 4>(reportTestCases, OPCODE_DIV, 1000), "posit<64,4>", "division");

#ifdef HARDWARE_ACCELERATION
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<12, 1>(reportTestCases), "posit<12,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<14, 1>(reportTestCases), "posit<14,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<16, 1>(reportTestCases), "posit<16,1>", "division");
#endif // HARDWARE_ACCELERATION

#endif // REGRESSION_LEVEL_4

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
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

