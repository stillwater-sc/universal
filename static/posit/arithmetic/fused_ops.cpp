// fused_ops.cpp: test suite runner for posit fused multiply-add and related operators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>  // for std::fma
// Configure the posit template environment
// first: enable general or specialized posit configurations
//#define POSIT_FAST_SPECIALIZATION
// second: enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable tracing
//#define ALGORITHM_VERBOSE_OUTPUT
//#define ALGORITHM_TRACE_ADD
//#define ALGORITHM_TRACE_MUL
#include <universal/number/posit/posit.hpp>
#include <universal/verification/posit_test_suite.hpp>

// generate specific test case for fma
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCaseFma(Ty a, Ty b, Ty c) {
	Ty ref;
	sw::universal::posit<nbits, es> pa, pb, pc, pref, pfma;
	pa = a;
	pb = b;
	pc = c;
	ref = std::fma(a, b, c);
	pref = ref;
	pfma = sw::universal::fma(pa, pb, pc);
	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " * " << std::setw(nbits) << b << " + " << std::setw(nbits) << c << " = " << std::setw(nbits) << ref << '\n';
	std::cout << to_binary(pa) << " * " << to_binary(pb) << " + " << to_binary(pc) << " = " << to_binary(pfma) << " (reference: " << to_binary(pref) << ")   ";
	std::cout << (pref == pfma ? "PASS" : "FAIL") << "\n\n";
	std::cout << std::dec << std::setprecision(oldPrecision);
}

// generate specific test case for fam
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCaseFam(Ty a, Ty b, Ty c) {
	Ty ref;
	sw::universal::posit<nbits, es> pa, pb, pc, pref, pfam;
	pa = a;
	pb = b;
	pc = c;
	ref = (a + b) * c;
	pref = ref;
	pfam = sw::universal::fam(pa, pb, pc);
	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << "(" << std::setw(nbits) << a << " + " << std::setw(nbits) << b << ") * " << std::setw(nbits) << c << " = " << std::setw(nbits) << ref << '\n';
	std::cout << "(" << to_binary(pa) << " + " << to_binary(pb) << ") * " << to_binary(pc) << " = " << to_binary(pfam) << " (reference: " << to_binary(pref) << ")   ";
	std::cout << (pref == pfam ? "PASS" : "FAIL") << "\n\n";
	std::cout << std::dec << std::setprecision(oldPrecision);
}

// Exhaustive verification of fma: a*b + c
// Compares posit fma against double-precision std::fma reference
template<typename TestType>
int VerifyFma(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned es = TestType::es;
	const unsigned NR_POSITS = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	sw::universal::posit<nbits, es> pa, pb, pc, pfma, pref;

	for (unsigned i = 0; i < NR_POSITS; i++) {
		pa.setbits(i);
		double da = double(pa);
		for (unsigned j = 0; j < NR_POSITS; j++) {
			pb.setbits(j);
			double db = double(pb);
			for (unsigned k = 0; k < NR_POSITS; k++) {
				pc.setbits(k);
				double dc = double(pc);

				double ref = std::fma(da, db, dc);
				pref = ref;
				pfma = sw::universal::fma(pa, pb, pc);

				if (pfma != pref) {
					nrOfFailedTests++;
					if (reportTestCases) {
						std::cout << "FAIL: fma(" << pa << ", " << pb << ", " << pc << ") = "
						          << pfma << " != " << pref << " (ref: " << ref << ")\n";
					}
				}
			}
		}
	}
	return nrOfFailedTests;
}

// Exhaustive verification of fam: (a + b) * c
template<typename TestType>
int VerifyFam(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned es = TestType::es;
	const unsigned NR_POSITS = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	sw::universal::posit<nbits, es> pa, pb, pc, pfam, pref;

	for (unsigned i = 0; i < NR_POSITS; i++) {
		pa.setbits(i);
		double da = double(pa);
		for (unsigned j = 0; j < NR_POSITS; j++) {
			pb.setbits(j);
			double db = double(pb);
			for (unsigned k = 0; k < NR_POSITS; k++) {
				pc.setbits(k);
				double dc = double(pc);

				double ref = (da + db) * dc;
				pref = ref;
				pfam = sw::universal::fam(pa, pb, pc);

				if (pfam != pref) {
					nrOfFailedTests++;
					if (reportTestCases) {
						std::cout << "FAIL: fam(" << pa << ", " << pb << ", " << pc << ") = "
						          << pfam << " != " << pref << " (ref: " << ref << ")\n";
					}
				}
			}
		}
	}
	return nrOfFailedTests;
}

// Exhaustive verification of fmma: (a * b) + (c * d)
template<typename TestType>
int VerifyFmma(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned es = TestType::es;
	const unsigned NR_POSITS = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	sw::universal::posit<nbits, es> pa, pb, pc, pd, pfmma, pref;

	// four-deep exhaustive loop is expensive: use smaller sizes
	for (unsigned i = 0; i < NR_POSITS; i++) {
		pa.setbits(i);
		double da = double(pa);
		for (unsigned j = 0; j < NR_POSITS; j++) {
			pb.setbits(j);
			double db = double(pb);
			for (unsigned k = 0; k < NR_POSITS; k++) {
				pc.setbits(k);
				double dc = double(pc);
				for (unsigned l = 0; l < NR_POSITS; l++) {
					pd.setbits(l);
					double dd = double(pd);

					double ref = std::fma(da, db, dc * dd);
					pref = ref;
					pfmma = sw::universal::fmma(pa, pb, pc, pd, true);

					if (pfmma != pref) {
						nrOfFailedTests++;
						if (reportTestCases) {
							std::cout << "FAIL: fmma(" << pa << ", " << pb << ", " << pc << ", " << pd << ") = "
							          << pfmma << " != " << pref << " (ref: " << ref << ")\n";
						}
					}
				}
			}
		}
	}
	return nrOfFailedTests;
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

	std::string test_suite  = "posit fused operator verification";
	std::string test_tag    = "fused_ops";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// fma: a*b + c
	GenerateTestCaseFma<16, 1, double>(0.1, 10.0, -1.0);
	GenerateTestCaseFma<32, 2, double>(0.1, 10.0, -1.0);
	GenerateTestCaseFma<32, 2, double>(1.0, 1.0, 1.0);
	GenerateTestCaseFma<32, 2, double>(0.5, 0.5, 0.25);

	// fam: (a + b) * c
	GenerateTestCaseFam<16, 1, double>(1.0, 2.0, 3.0);
	GenerateTestCaseFam<32, 2, double>(0.5, 0.5, 2.0);

	// exhaustive small posit tests
	nrOfFailedTestCases += ReportTestResult(VerifyFma<posit<3, 0>>(true), "posit<3,0>", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFam<posit<3, 0>>(true), "posit<3,0>", "fam");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	// NOTE: posit<3,1> is excluded because nbits - 3 - es underflows (unsigned) when nbits < 3 + es + 1
	nrOfFailedTestCases += ReportTestResult(VerifyFma<posit<3, 0>>(reportTestCases), "posit<3,0>", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFma<posit<4, 0>>(reportTestCases), "posit<4,0>", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFma<posit<4, 1>>(reportTestCases), "posit<4,1>", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFma<posit<5, 0>>(reportTestCases), "posit<5,0>", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFma<posit<5, 1>>(reportTestCases), "posit<5,1>", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFma<posit<5, 2>>(reportTestCases), "posit<5,2>", "fma");

	nrOfFailedTestCases += ReportTestResult(VerifyFam<posit<3, 0>>(reportTestCases), "posit<3,0>", "fam");
	nrOfFailedTestCases += ReportTestResult(VerifyFam<posit<4, 0>>(reportTestCases), "posit<4,0>", "fam");
	nrOfFailedTestCases += ReportTestResult(VerifyFam<posit<4, 1>>(reportTestCases), "posit<4,1>", "fam");
	nrOfFailedTestCases += ReportTestResult(VerifyFam<posit<5, 0>>(reportTestCases), "posit<5,0>", "fam");
	nrOfFailedTestCases += ReportTestResult(VerifyFam<posit<5, 1>>(reportTestCases), "posit<5,1>", "fam");
	nrOfFailedTestCases += ReportTestResult(VerifyFam<posit<5, 2>>(reportTestCases), "posit<5,2>", "fam");

	// fmma is a TODO stub, test that it returns zero without crashing
	nrOfFailedTestCases += ReportTestResult(VerifyFmma<posit<3, 0>>(reportTestCases), "posit<3,0>", "fmma");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyFma<posit<6, 0>>(reportTestCases), "posit<6,0>", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFma<posit<6, 1>>(reportTestCases), "posit<6,1>", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFma<posit<6, 2>>(reportTestCases), "posit<6,2>", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFma<posit<6, 3>>(reportTestCases), "posit<6,3>", "fma");

	nrOfFailedTestCases += ReportTestResult(VerifyFam<posit<6, 0>>(reportTestCases), "posit<6,0>", "fam");
	nrOfFailedTestCases += ReportTestResult(VerifyFam<posit<6, 1>>(reportTestCases), "posit<6,1>", "fam");
	nrOfFailedTestCases += ReportTestResult(VerifyFam<posit<6, 2>>(reportTestCases), "posit<6,2>", "fam");
	nrOfFailedTestCases += ReportTestResult(VerifyFam<posit<6, 3>>(reportTestCases), "posit<6,3>", "fam");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyFma<posit<7, 0>>(reportTestCases), "posit<7,0>", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFma<posit<7, 1>>(reportTestCases), "posit<7,1>", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFma<posit<7, 2>>(reportTestCases), "posit<7,2>", "fma");

	nrOfFailedTestCases += ReportTestResult(VerifyFam<posit<7, 0>>(reportTestCases), "posit<7,0>", "fam");
	nrOfFailedTestCases += ReportTestResult(VerifyFam<posit<7, 1>>(reportTestCases), "posit<7,1>", "fam");
	nrOfFailedTestCases += ReportTestResult(VerifyFam<posit<7, 2>>(reportTestCases), "posit<7,2>", "fam");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyFma<posit<8, 0>>(reportTestCases), "posit<8,0>", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFma<posit<8, 1>>(reportTestCases), "posit<8,1>", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFma<posit<8, 2>>(reportTestCases), "posit<8,2>", "fma");

	nrOfFailedTestCases += ReportTestResult(VerifyFam<posit<8, 0>>(reportTestCases), "posit<8,0>", "fam");
	nrOfFailedTestCases += ReportTestResult(VerifyFam<posit<8, 1>>(reportTestCases), "posit<8,1>", "fam");
	nrOfFailedTestCases += ReportTestResult(VerifyFam<posit<8, 2>>(reportTestCases), "posit<8,2>", "fam");
#endif

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
