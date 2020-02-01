// arithmetic_mul.cpp: functional tests for fixed-point multiplication
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
void GenerateTestCase(Ty _a, Ty _b) {
	Ty ref;
	sw::unum::fixpnt<nbits, rbits> a, b, cref, result;
	a = _a;
	b = _b;
	result = a * b;
	ref = _a * _b;
	cref = ref;
	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << _a << " * " << std::setw(nbits) << _b << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << a << " * " << b << " = " << result << " (reference: " << cref << ")   " ;
	std::cout << (cref == result ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::dec << std::setprecision(oldPrecision);
}

namespace sw {
namespace unum {

#define FIXPNT_TABLE_WIDTH 20
template<size_t nbits, size_t rbits>
void ReportBinaryArithmeticError(std::string test_case, std::string op, const fixpnt<nbits, rbits>& lhs, const fixpnt<nbits, rbits>& rhs, const fixpnt<nbits, rbits>& ref, const fixpnt<nbits, rbits>& result) {
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(FIXPNT_TABLE_WIDTH) << lhs
		<< " " << op << " "
		<< std::setw(FIXPNT_TABLE_WIDTH) << rhs
		<< " != "
		<< std::setw(FIXPNT_TABLE_WIDTH) << ref << " instead it yielded "
		<< std::setw(FIXPNT_TABLE_WIDTH) << result
		<< " " << to_binary(ref) << " vs " << to_binary(result)
		<< std::setprecision(old_precision)
		<< std::endl;
}

template<size_t nbits, size_t rbits>
void ReportBinaryArithmeticSuccess(std::string test_case, std::string op, const fixpnt<nbits, rbits>& lhs, const fixpnt<nbits, rbits>& rhs, const fixpnt<nbits, rbits>& ref, const fixpnt<nbits, rbits>& result) {
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(FIXPNT_TABLE_WIDTH) << lhs
		<< " " << op << " "
		<< std::setw(FIXPNT_TABLE_WIDTH) << rhs
		<< " == "
		<< std::setw(FIXPNT_TABLE_WIDTH) << ref << " matches reference "
		<< std::setw(FIXPNT_TABLE_WIDTH) << result
		<< " " << to_binary(ref) << " vs " << to_binary(result)
		<< std::setprecision(old_precision)
		<< std::endl;
}

// enumerate all multiplication cases for an fixpnt<nbits,rbits> configuration
template<size_t nbits, size_t rbits>
int VerifyMultiplication(std::string tag, bool bReportIndividualTestCases) {
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	int nrOfFailedTests = 0;
	fixpnt<nbits, rbits> a, b, result, cref;
	double ref;

	double da, db;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.set_raw_bits(i);
		da = double(a);
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.set_raw_bits(j);
			db = double(b);
			ref = da * db;
#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
			// catching overflow
			try {
				result = a * b;
			}
			catch (...) {
				if (ref > max_int<nbits>() || iref < min_int<nbits>()) {
					// correctly caught the exception
					continue;
				}
				else {
					nrOfFailedTests++;
				}
			}

#else
			result = a * b;
#endif // FIXPNT_THROW_ARITHMETIC_EXCEPTION
			cref = ref;
			if (result != cref) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "*", a, b, cref, result);
			}
			else {
				// if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "*", a, b, cref, result);
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

#define MANUAL_TESTING 1
#define STRESS_TESTING 0
#include <bitset>

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "Multiplication failed: ";

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug
	GenerateTestCase<8, 4>(0.5f, 1.0f);

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 1>(tag, bReportIndividualTestCases), "fixpnt<8,1>", "multiplication");

#if STRESS_TESTING
	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 0>("Manual Testing", true), "fixpnt<4,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 1>("Manual Testing", true), "fixpnt<4,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 2>("Manual Testing", true), "fixpnt<4,2>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 3>("Manual Testing", true), "fixpnt<4,3>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 4>("Manual Testing", true), "fixpnt<4,4>", "multiplication");
#endif

#else

	cout << "Fixed-point multiplication validation" << endl;

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 0>(tag, bReportIndividualTestCases), "fixpnt<8,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 1>(tag, bReportIndividualTestCases), "fixpnt<8,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 2>(tag, bReportIndividualTestCases), "fixpnt<8,2>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 3>(tag, bReportIndividualTestCases), "fixpnt<8,3>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 4>(tag, bReportIndividualTestCases), "fixpnt<8,4>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 5>(tag, bReportIndividualTestCases), "fixpnt<8,5>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 6>(tag, bReportIndividualTestCases), "fixpnt<8,6>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 7>(tag, bReportIndividualTestCases), "fixpnt<8,7>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 8>(tag, bReportIndividualTestCases), "fixpnt<8,8>", "multiplication");

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
