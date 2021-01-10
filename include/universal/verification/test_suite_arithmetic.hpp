#pragma once
// test_suite_arithmetic.hpp : arithmetic test suite for arbitrary universal number systems
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <iostream>
#include <typeinfo>
#include <random>
#include <limits>
#include <complex>

// CALLING ENVIRONMENT PREREQUISITE!!!!!
// We want the test suite to be used with different configurations of number systems
// so the calling environment needs to set the configuration
// This usually entails setting environment variables, such as
// #default POSIT_THOW_ARITHMETIC_EXCEPTIONS 1

#include <universal/verification/test_status.hpp>

namespace sw::universal {

#define NUMBER_COLUMN_WIDTH 20

template<typename TestType>
void ReportConversionError(const std::string& test_case, const std::string& op, double input, double reference, const TestType& result) {
	constexpr size_t nbits = result.nbits;  // number system concept requires a static member indicating its size in bits
	auto old_precision = std::cerr.precision();
	std::cerr << test_case
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << input
		<< " did not convert to "
		<< std::setw(NUMBER_COLUMN_WIDTH) << reference << " instead it yielded  "
		<< std::setw(NUMBER_COLUMN_WIDTH) << double(result)
		<< "  raw " << std::setw(nbits) << to_binary(result)
		<< std::setprecision(old_precision)
		<< std::endl;
}

template<typename TestType>
void ReportConversionSuccess(const std::string& test_case, const std::string& op, double input, double reference, const TestType& result) {
	constexpr size_t nbits = result.nbits;  // number system concept requires a static member indicating its size in bits
	std::cerr << test_case
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << input
		<< " success            "
		<< std::setw(NUMBER_COLUMN_WIDTH) << result << " golden reference is "
		<< std::setw(NUMBER_COLUMN_WIDTH) << reference
		<< "  raw " << std::setw(nbits) << to_binary(result)
		<< std::endl;
}

template<typename TestType>
void ReportBinaryArithmeticError(const std::string& test_case, const std::string& op, const TestType& lhs, const TestType& rhs, const TestType& ref, const TestType& result) {
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(NUMBER_COLUMN_WIDTH) << lhs
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << rhs
		<< " != "
		<< std::setw(NUMBER_COLUMN_WIDTH) << result << " golden reference is "
		<< std::setw(NUMBER_COLUMN_WIDTH) << ref
		<< " " << to_binary(result) << " vs " << to_binary(ref)
		<< std::setprecision(old_precision)
		<< std::endl;
}

template<typename TestType>
void ReportBinaryArithmeticSuccess(const std::string& test_case, const std::string& op, const TestType& lhs, const TestType& rhs, const TestType& ref, const TestType& result) {
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(NUMBER_COLUMN_WIDTH) << lhs
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << rhs
		<< " == "
		<< std::setw(NUMBER_COLUMN_WIDTH) << result << " matches reference "
		<< std::setw(NUMBER_COLUMN_WIDTH) << ref
		<< " " << to_binary(result) << " vs " << to_binary(ref)
		<< std::setprecision(old_precision)
		<< std::endl;
}

template<typename RefType, typename TestType>
void ReportAssignmentError(const std::string& test_case, const std::string& op, const TestType& value, const TestType& result, const RefType& ref) {
	std::cerr << test_case
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << value
		<< " != "
		<< std::setw(NUMBER_COLUMN_WIDTH) << result << " golden reference is "
		<< std::setw(NUMBER_COLUMN_WIDTH) << ref
		<< " " << to_binary(result) << " vs " << to_binary(ref) << std::endl;
}

template<typename RefType, typename TestType>
void ReportAssignmentSuccess(const std::string& test_case, const std::string& op, const TestType& value, const TestType& result, const RefType& ref) {
	std::cerr << test_case
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << value
		<< " == "
		<< std::setw(NUMBER_COLUMN_WIDTH) << result << " reference value is "
		<< std::setw(NUMBER_COLUMN_WIDTH) << ref
		<< "               fixpnt bits " << to_binary(result) << std::endl;
}

/////////////////////////////// VERIFICATION TEST SUITES ////////////////////////////////

template<typename TestType>
int Compare(double input, const TestType& presult, double reference, bool bReportIndividualTestCases) {
	int fail = 0;
	double result = double(presult);
	if (std::fabs(result - reference) > 0.000000001) {
		fail++;
		if (bReportIndividualTestCases)	ReportConversionError("FAIL", "=", input, reference, presult);
	}
	else {
		// if (bReportIndividualTestCases) ReportConversionSuccess("PASS", "=", input, reference, presult);
	}
	return fail;
}

template<typename RefType, typename TestType>
int VerifyAssignment(bool bReportIndividualTestCases, bool verbose = false) {
	// algorithm: TestType raw -> to value in RefType -> assign back to TestType -> compare resulting TestTypes
	TestType number, assigned;
	constexpr size_t nbits = number.nbits;  // number system concept requires a static member indicating its size in bits
	const size_t NR_NUMBERS = (size_t(1) << nbits);
	int nrOfFailedTestCases = 0;

	for (size_t i = 0; i < NR_NUMBERS; i++) {
		number.set_raw_bits(i); 
		if (verbose) std::cout << to_binary(number) << std::endl;
		RefType value = (RefType)(number);
		assigned = value;
		if (verbose) std::cout << number << " " << value << " " << assigned << std::endl;
		if (number != assigned) {
			nrOfFailedTestCases++;
			if (bReportIndividualTestCases) ReportAssignmentError("FAIL", "=", number, assigned, value);
		}
		else {
			if (verbose && bReportIndividualTestCases) ReportAssignmentSuccess("PASS", "=", number, assigned, value);
		}
	}
	return nrOfFailedTestCases;
}

// enumerate all addition cases for a number system configuration
template<typename TestType>
int VerifyAddition(const std::string& tag, bool bReportIndividualTestCases) {
	TestType a{ 0 }, b{ 0 }, result, cref;
	constexpr size_t nbits = a.nbits;  // number system concept requires a static member indicating its size in bits
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	int nrOfFailedTests = 0;

	// set the saturation clamps
	TestType maxpositive{ 0 }, maxnegative{ 0 };
	maxpos(maxpositive); // depend on ADL to specialize
	maxneg(maxnegative);

	double da, db, ref;  // make certain that IEEE doubles are sufficient as reference
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.set_raw_bits(i); // number system concept requires a member function set_raw_bits()
		da = double(a);
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.set_raw_bits(j);
			db = double(b);
			ref = da + db;
#if THROW_ARITHMETIC_EXCEPTION
			// catching overflow
			try {
				result = a + b;
			}
			catch (...) {
				if (ref < double(maxnegitive) || ref > double(maxpositive)) {
					// correctly caught the overflow exception
					continue;
				}
				else {
					nrOfFailedTests++;
				}
			}

#else
			result = a + b;
#endif // THROW_ARITHMETIC_EXCEPTION
			cref = ref;
			if (result != cref) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", a, b, cref, result);
			}
			else {
				//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", a, b, cref, result);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		if (NR_VALUES > 256*256) if (i % (NR_VALUES / 25) == 0) std::cout << '.';
	}
	std::cout << std::endl;
	return nrOfFailedTests;
}

// enumerate all subtraction cases for an fixpnt<nbits,rbits> configuration

// enumerate all multiplication cases for an fixpnt<nbits,rbits> configuration

// enumerate all division cases for an fixpnt<nbits,rbits> configuration


} // namespace sw::universal
