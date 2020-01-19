// arithmetic_add.cpp: functional tests for fixed-point addition
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
//#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 0

// minimum set of include files to reflect source code dependencies
#include "universal/fixpnt/fixed_point.hpp"
// fixed-point type manipulators such as pretty printers
#include "universal/fixpnt/fixpnt_manipulators.hpp"
#include "universal/fixpnt/math_functions.hpp"
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"

// generate specific test case that you can trace with the trace conditions in fixpnt.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t rbits, typename Ty>
void GenerateTestCase(Ty a, Ty b) {
	Ty ref;
	sw::unum::fixpnt<nbits, rbits> pa, pb, pref, psum;
	pa = a;
	pb = b;
	ref = a + b;
	pref = ref;
	psum = pa + pb;
	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " + " << std::setw(nbits) << b << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " + " << pb.get() << " = " << psum.get() << " (reference: " << pref.get() << ")   " ;
	std::cout << (pref == psum ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::dec << std::setprecision(oldPrecision);
}

namespace sw {
namespace unum {

#define FIXPNT_TABLE_WIDTH 20
template<size_t nbits, size_t rbits>
void ReportBinaryArithmeticError(std::string test_case, std::string op, const fixpnt<nbits, rbits>& lhs, const fixpnt<nbits, rbits>& rhs, const fixpnt<nbits, rbits>& pref, const fixpnt<nbits, rbits>& presult) {
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(FIXPNT_TABLE_WIDTH) << lhs
		<< " " << op << " "
		<< std::setw(FIXPNT_TABLE_WIDTH) << rhs
		<< " != "
		<< std::setw(FIXPNT_TABLE_WIDTH) << pref << " instead it yielded "
		<< std::setw(FIXPNT_TABLE_WIDTH) << presult
		<< " " << to_binary(pref) << " vs " << to_binary(presult)
		<< std::setprecision(old_precision)
		<< std::endl;
}

// enumerate all addition cases for an fixpnt<nbits,rbits> configuration
template<size_t nbits, size_t rbits>
int VerifyAddition(std::string tag, bool bReportIndividualTestCases) {
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	int nrOfFailedTests = 0;
	fixpnt<nbits, rbits> ia, ib, iresult, iref;

	int64_t i64a, i64b;
	for (size_t i = 0; i < NR_VALUES; i++) {
		ia.set_raw_bits(i);
		i64a = int64_t(ia);
		for (size_t j = 0; j < NR_VALUES; j++) {
			ib.set_raw_bits(j);
			i64b = int64_t(ib);
			iref = i64a + i64b;
#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
			try {
				iresult = ia + ib;
			}
			catch (...) {
				if (iref > max_int<nbits>() || iref < min_int<nbits>()) {
					// correctly caught the exception

				}
				else {
					nrOfFailedTests++;
				}
			}

#else
			iresult = ia + ib;
#endif // FIXPNT_THROW_ARITHMETIC_EXCEPTION
			if (iresult != iref) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", ia, ib, iref, iresult);
			}
			else {
				//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", ia, ib, iref, iresult);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		if (i % 1024 == 0) std::cout << '.';
	}
	std::cout << std::endl;
	return nrOfFailedTests;
}


} // namespace unum
} // namespace sw

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Addition failed: ";

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug
	GenerateTestCase<8, 4>(0.5f, 1.0f);

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<3, 0>("Manual Testing", true), "fixpnt<3,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<3, 1>("Manual Testing", true), "fixpnt<3,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<3, 2>("Manual Testing", true), "fixpnt<3,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<3, 3>("Manual Testing", true), "fixpnt<3,3>", "addition");

#else

	cout << "Fixed-point addition validation" << endl;

	nrOfFailedTestCases += ReportTestResult(VerifyAddition<8, 0>(tag, bReportIndividualTestCases), "fixpnt<8,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<8, 1>(tag, bReportIndividualTestCases), "fixpnt<8,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<8, 2>(tag, bReportIndividualTestCases), "fixpnt<8,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<8, 3>(tag, bReportIndividualTestCases), "fixpnt<8,3>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<8, 4>(tag, bReportIndividualTestCases), "fixpnt<8,4>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<8, 5>(tag, bReportIndividualTestCases), "fixpnt<8,5>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<8, 6>(tag, bReportIndividualTestCases), "fixpnt<8,6>", "addition");

#if STRESS_TESTING

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::unum::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::unum::fixpnt_internal_exception& err) {
	std::cerr << "Uncaught fixpnt internal exception: " << err.what() << std::endl;
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
