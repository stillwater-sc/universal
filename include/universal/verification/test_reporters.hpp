#pragma once
// test_reporters.hpp : test result reporters to guide verification
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>
#include <limits>
#include <complex>
#include <universal/math/complex_manipulators.hpp>

// NOTE: reporters write to cerr

namespace sw::universal {

#define NUMBER_COLUMN_WIDTH 20

// ReportTestSuiteResult prints to std::cerr whether or not the test suite passed or failed
void ReportTestSuiteResults(const std::string& test_suite, int nrOfFailedTestCases) {
	std::cerr << test_suite <<  (nrOfFailedTestCases == 0 ? ": PASS" : ": FAIL") << '\n';
}

template<typename TestType>
void ReportConversionError(const std::string& test_case, const std::string& op, double input, double reference, const TestType& result) {
	constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
	auto old_precision = std::cerr.precision();
	std::cerr << std::setprecision(10);
	std::cerr << test_case
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << input
		<< " did not convert to "
		<< std::setw(NUMBER_COLUMN_WIDTH) << reference << " instead it yielded  "
		<< std::setw(NUMBER_COLUMN_WIDTH) << double(result)
		<< "  raw " << std::setw(nbits) << to_binary(result);
	std::cerr << '\n';
	std::cerr << std::setprecision(old_precision);
}

template<typename TestType>
void ReportConversionSuccess(const std::string& test_case, const std::string& op, double input, double reference, const TestType& result) {
	constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
	std::cerr << test_case
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << input
		<< " success            "
		<< std::setw(NUMBER_COLUMN_WIDTH) << result << " golden reference is "
		<< std::setw(NUMBER_COLUMN_WIDTH) << reference
		<< "  raw " << std::setw(nbits) << to_binary(result)
		<< '\n';
}

template<typename TestType>
void ReportUnaryArithmeticError(const std::string& test_case, const std::string& op, const TestType& argument, const TestType& result, const TestType& ref) {
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< " " << op << " "
		<< std::setprecision(20)
		<< std::setw(NUMBER_COLUMN_WIDTH) << argument
		<< " != "
		<< std::setw(NUMBER_COLUMN_WIDTH) << ref << " instead it yielded "
		<< std::setw(NUMBER_COLUMN_WIDTH) << result
		<< " " << to_binary(ref) << " vs " << to_binary(result)
		<< std::setprecision(old_precision)
		<< '\n';
}

template<typename TestType>
void ReportUnaryArithmeticSuccess(const std::string& test_case, const std::string& op, const TestType& argument, const TestType& result, const TestType& ref) {
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< " " << op << " "
		<< std::setprecision(20)
		<< std::setw(NUMBER_COLUMN_WIDTH) << argument
		<< " == "
		<< std::setw(NUMBER_COLUMN_WIDTH) << result << " reference value is "
		<< std::setw(NUMBER_COLUMN_WIDTH) << ref
		<< std::setprecision(old_precision)
		<< '\n';
}

template<typename InputType, typename ResultType, typename RefType>
void ReportBinaryArithmeticError(const std::string& test_case, const std::string& op, 
	const InputType& lhs, const InputType& rhs, const ResultType& result, const RefType& ref) {
	using namespace sw::universal;
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(NUMBER_COLUMN_WIDTH) << lhs
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << rhs
		<< " != "
		<< std::setw(NUMBER_COLUMN_WIDTH) << result << " golden reference is "
		<< std::setw(NUMBER_COLUMN_WIDTH) << ref
		<< '\n'
		<< " result " << to_binary(result) 
		<< "\n vs ref " << to_binary(ref)
		<< '\n'
		<< to_binary(lhs)
		<< " " << op << " "
		<< to_binary(rhs)
		<< std::setprecision(old_precision)
		<< '\n';
}

template<typename TestType, typename ResultType, typename RefType>
void ReportBinaryArithmeticSuccess(const std::string& test_case, const std::string& op, const TestType& lhs, const TestType& rhs, const ResultType& result, const RefType& ref) {
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
		<< '\n';
}

/// <summary>
/// report an assignment error by comparing the input to the result and the golden reference
/// </summary>
/// <typeparam name="MarshallType"></typeparam>
/// <typeparam name="TestType"></typeparam>
/// <typeparam name="RefType"></typeparam>
/// <param name="test_case">string indicating the test</param>
/// <param name="op">string indicating the operator</param>
/// <param name="input">the input value to the assignment</param>
/// <param name="result">the result of the assignment operator</param>
/// <param name="ref">the golden reference for this assignment operator</param>
template<typename MarshallType, typename TestType, typename RefType>
void ReportAssignmentError(const std::string& test_case, const std::string& op, const MarshallType& input, const TestType& result, const RefType& ref) {
	std::cerr << test_case
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << input
		<< " != "
		<< std::setw(NUMBER_COLUMN_WIDTH) << result << " golden reference is "
		<< std::setw(NUMBER_COLUMN_WIDTH) << ref
		<< " " << to_binary(result) << " vs " << to_binary(ref) << '\n';
}

/// <summary>
/// report an assignment success by comparing the input to the result and the golden reference
/// </summary>
/// <typeparam name="MarshallType"></typeparam>
/// <typeparam name="TestType"></typeparam>
/// <typeparam name="RefType"></typeparam>
/// <param name="test_case">string indicating the test</param>
/// <param name="op">string indicating the operator</param>
/// <param name="input">the input value to the assignment</param>
/// <param name="result">the result of the assignment operator</param>
/// <param name="ref">the golden reference for this assignment operator</param>
template<typename MarshallType, typename TestType, typename RefType>
void ReportAssignmentSuccess(const std::string& test_case, const std::string& op, const MarshallType& input, const TestType& result, const RefType& ref) {
	std::cerr << test_case
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << input
		<< " == "
		<< std::setw(NUMBER_COLUMN_WIDTH) << result << " reference value is "
		<< std::setw(NUMBER_COLUMN_WIDTH) << ref
		<< "               bit pattern " << to_binary(result) << '\n';
}

template<typename TestType>
void ReportOneInputFunctionError(const std::string& test_case, const std::string& op, const TestType& rhs, const TestType& reference, const TestType& result) {
	std::cerr << test_case
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << rhs
		<< " != "
		<< std::setw(NUMBER_COLUMN_WIDTH) << reference << " instead it yielded "
		<< std::setw(NUMBER_COLUMN_WIDTH) << result
		<< " " << to_binary(reference) << " vs " << to_binary(result) << '\n';
}

template<typename TestType>
void ReportOneInputFunctionSuccess(const std::string& test_case, const std::string& op, const TestType& rhs, const TestType& reference, const TestType& result) {
	std::cerr << test_case
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << rhs
		<< " == "
		<< std::setw(NUMBER_COLUMN_WIDTH) << result << " reference value is "
		<< std::setw(NUMBER_COLUMN_WIDTH) << reference
		<< " " << components_to_string(result) << '\n';
}

template<typename TestType>
void ReportTwoInputFunctionError(const std::string& test_case, const std::string& op, const TestType& a, const TestType& b, const TestType& reference, const TestType& result) {
	auto precision = std::cerr.precision();
	std::cerr << test_case << " " << op << "("
		<< std::setprecision(20)
		<< std::setw(NUMBER_COLUMN_WIDTH) << a
		<< ","
		<< std::setw(NUMBER_COLUMN_WIDTH) << b << ")"
		<< " != "
		<< std::setw(NUMBER_COLUMN_WIDTH) << reference << " instead it yielded "
		<< std::setw(NUMBER_COLUMN_WIDTH) << result
		<< " " << reference << " vs " << result
		<< std::setprecision(precision)
		<< '\n';
}

template<typename TestType>
void ReportTwoInputFunctionSuccess(const std::string& test_case, const std::string& op, const TestType& a, const TestType& b, const TestType& reference, const TestType& result) {
	auto precision = std::cerr.precision();
	std::cerr << test_case << " " << op << "("
		<< std::setprecision(20)
		<< std::setw(NUMBER_COLUMN_WIDTH) << a
		<< ","
		<< std::setw(NUMBER_COLUMN_WIDTH) << b << ")"
		<< " == "
		<< std::setw(NUMBER_COLUMN_WIDTH) << reference << " ==  "
		<< std::setw(NUMBER_COLUMN_WIDTH) << result
		<< " " << reference << " vs " << result
		<< std::setprecision(precision)
		<< '\n';
}

} // namespace sw::universal
