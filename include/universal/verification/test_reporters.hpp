#pragma once
// test_reporters.hpp : test result reporters to guide verification
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>
#include <limits>
#include <complex>
#include <universal/native/integers.hpp>
#include <universal/native/ieee754.hpp>
#include <universal/math/complex/manipulators.hpp>
#include <universal/verification/test_status.hpp>

// NOTE: reporters write to cerr

namespace sw { namespace universal {

#define NUMBER_COLUMN_WIDTH 25

void ReportTestSuiteHeader(const std::string& test_suite, bool reportTestCases) {
	std::cerr << test_suite << (reportTestCases ? ": report test cases" : ": results only") << '\n';
}

// ReportTestSuiteResult prints to std::cerr whether or not the test suite passed or failed
void ReportTestSuiteResults(const std::string& test_suite, int nrOfFailedTestCases) {
	std::cerr << test_suite <<  (nrOfFailedTestCases == 0 ? ": PASS" : ": FAIL") << '\n';
}

template<typename TestType>
void ReportConversionError(const std::string& test_case, const std::string& op, double input, const TestType& result, double ref) {
	constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
	std::streamsize old_precision = std::cerr.precision();
	std::cerr << std::setprecision(10);
	std::cerr << test_case
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << input
		<< " did not convert to "
		<< std::setw(NUMBER_COLUMN_WIDTH) << ref << " instead it yielded  "
		<< std::setw(NUMBER_COLUMN_WIDTH) << double(result)
		<< "  raw " << std::setw(nbits) << to_binary(result);
	std::cerr << '\n';
	std::cerr << std::setprecision(static_cast<int>(old_precision));
}

template<typename TestType>
void ReportConversionSuccess(const std::string& test_case, const std::string& op, double input, const TestType& result, double ref) {
	constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
	std::cerr << test_case
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << input
		<< " success            "
		<< std::setw(NUMBER_COLUMN_WIDTH) << result << " golden reference is "
		<< std::setw(NUMBER_COLUMN_WIDTH) << ref
		<< "  raw " << std::setw(nbits) << to_binary(result)
		<< '\n';
}

template<typename TestType>
void ReportLogicError(const std::string& test_case, const std::string& op, const TestType& lhs, const TestType& rhs, bool testResult) {
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "

		<< std::setprecision(20)
		<< std::setw(NUMBER_COLUMN_WIDTH) << lhs
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << rhs
		<< " yielded "
		<< std::setw(NUMBER_COLUMN_WIDTH) << (testResult ? "true" : "false")
		<< " " << to_binary(lhs) << op << to_binary(rhs)
		<< std::setprecision(old_precision)
		<< '\n';
}

template<typename TestType>
void ReportLogicSuccess(const std::string& test_case, const std::string& op, const TestType& lhs, const TestType& rhs, bool testResult) {
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(NUMBER_COLUMN_WIDTH) << lhs
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << rhs
		<< " yielded "
		<< std::setw(NUMBER_COLUMN_WIDTH) << (testResult ? "true" : "false")
		<< " " << to_binary(lhs) << op << to_binary(rhs)
		<< std::setprecision(old_precision)
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

template<typename TestType>
void ReportArithmeticShiftError(const std::string& test_case, const std::string& op, const TestType& a, const size_t divider, const TestType& result, int64_t ref) {
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(NUMBER_COLUMN_WIDTH) << (long long)a    // to_hex(a, true)
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << divider    // to_hex(b, true)
		<< " != "
		<< std::setw(NUMBER_COLUMN_WIDTH) << (long long)result // to_hex(result, true) 
		<< " golden reference is "
		<< std::setw(NUMBER_COLUMN_WIDTH) << ref << ' ' << to_binary(ref, TestType::nbits)
		<< " " << to_binary(result, true) << " vs " << to_binary(ref, TestType::nbits)
		<< std::setprecision(old_precision)
		<< std::endl;
}

template<typename TestType>
void ReportArithmeticShiftSuccess(const std::string& label, const std::string& op, const TestType& a, const size_t divider, const TestType& result, int64_t ref) {
	auto old_precision = std::cerr.precision();
	std::cerr << std::setprecision(20)
		<< label << " "	
		<< std::setw(NUMBER_COLUMN_WIDTH) << (long long)a
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << divider
		<< " == "
		<< std::setw(NUMBER_COLUMN_WIDTH) << (long long)result
		<< " matches reference   "
		<< std::setw(NUMBER_COLUMN_WIDTH) << ref << ' ' << to_binary(ref, TestType::nbits)
		<< " " << to_binary(result, true) << " vs " << to_binary(ref, TestType::nbits)
		<< std::setprecision(old_precision)
		<< '\n';
}

template<typename InputType, typename ResultType, typename RefType>
void ReportBinaryArithmeticError(const std::string& label, const std::string& op, 
	const InputType& lhs, const InputType& rhs, const ResultType& result, const RefType& ref) {
	using namespace sw::universal;
	auto old_precision = std::cerr.precision();
	std::cerr << std::setprecision(20)
		<< label << '\n'
		<< std::setw(NUMBER_COLUMN_WIDTH) << lhs
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << rhs
		<< " != "
		<< std::setw(NUMBER_COLUMN_WIDTH) << result 
		<< " golden reference is "
		<< std::setw(NUMBER_COLUMN_WIDTH) << ref
		<< '\n'
		<< std::setw(NUMBER_COLUMN_WIDTH) << to_binary(lhs)
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << to_binary(rhs)
		<< " != "
		<< std::setw(NUMBER_COLUMN_WIDTH) << to_binary(result)
		<< " golden reference is "
		<< std::setw(NUMBER_COLUMN_WIDTH) << to_binary(ResultType(ref))
		<< std::setprecision(old_precision)
		<< '\n';
}

template<typename TestType, typename ResultType, typename RefType>
void ReportBinaryArithmeticSuccess(const std::string& label, const std::string& op, const TestType& lhs, const TestType& rhs, const ResultType& result, const RefType& ref) {
	auto old_precision = std::cerr.precision();
	std::cerr << std::setprecision(20)
		<< label << ' '
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
		<< " " << to_binary(input) << " vs " << to_binary(result) << '\n';
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
void ReportOneInputFunctionError(const std::string& test_case, const std::string& op, const TestType& rhs, const TestType& result, const TestType& ref) {
	std::cerr << test_case
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << rhs
		<< " != "
		<< std::setw(NUMBER_COLUMN_WIDTH) << result << " reference value is "
		<< std::setw(NUMBER_COLUMN_WIDTH) << ref
		<< " " << to_binary(ref) << " vs " << to_binary(result) << '\n';
}

template<typename TestType>
void ReportOneInputFunctionSuccess(const std::string& test_case, const std::string& op, const TestType& rhs, const TestType& result, const TestType& ref) {
	std::cerr << test_case
		<< " " << op << " "
		<< std::setw(NUMBER_COLUMN_WIDTH) << rhs
		<< " == "
		<< std::setw(NUMBER_COLUMN_WIDTH) << result << " reference value is "
		<< std::setw(NUMBER_COLUMN_WIDTH) << ref
		<< " " << components_to_string(result) << '\n';
}

template<typename TestType>
void ReportTwoInputFunctionError(const std::string& test_case, const std::string& op, const TestType& a, const TestType& b, const TestType& result, const TestType& ref) {
	auto precision = std::cerr.precision();
	std::cerr << test_case << " " << op << "("
		<< std::setprecision(20)
		<< std::setw(NUMBER_COLUMN_WIDTH) << a
		<< ","
		<< std::setw(NUMBER_COLUMN_WIDTH) << b << ")"
		<< " != "
		<< std::setw(NUMBER_COLUMN_WIDTH) << result << " reference value is "
		<< std::setw(NUMBER_COLUMN_WIDTH) << ref
		<< " " << ref << " vs " << result
		<< std::setprecision(precision)
		<< '\n';
}

template<typename TestType>
void ReportTwoInputFunctionSuccess(const std::string& test_case, const std::string& op, const TestType& a, const TestType& b, const TestType& result, const TestType& ref) {
	auto precision = std::cerr.precision();
	std::cerr << test_case << " " << op << "("
		<< std::setprecision(20)
		<< std::setw(NUMBER_COLUMN_WIDTH) << a
		<< ","
		<< std::setw(NUMBER_COLUMN_WIDTH) << b << ")"
		<< " == "
		<< std::setw(NUMBER_COLUMN_WIDTH) << result << " ==  "
		<< std::setw(NUMBER_COLUMN_WIDTH) << ref
		<< " " << ref << " vs " << result
		<< std::setprecision(precision)
		<< '\n';
}

}} // namespace sw::universal
