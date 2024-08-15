// logic.cpp :test suite runner for logic operators between fixpnts and literals
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/fixpnt_test_suite.hpp>

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

	std::string test_suite  = "fixed-point logic operators ";
	std::string test_tag    = "comparisons";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		constexpr size_t nbits = 8;
		constexpr size_t es = 0;
		fixpnt<nbits, es> a(SpecificValue::maxpos), b(SpecificValue::maxpos), c(SpecificValue::maxneg);
		if (a == b) std::cout << "equal\n";
	
	}

	{
		nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<3, 0> >(reportTestCases), "fixpnt<3,0>", "==");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<3, 0> > (reportTestCases), "fixpnt<3,0>", "!=");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<3, 0> >(reportTestCases), "fixpnt<3,0>", "<");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<3, 0> >(reportTestCases), "fixpnt<3,0>", ">");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<3, 0> >(reportTestCases), "fixpnt<3,0>", "<=");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<3, 0> >(reportTestCases), "fixpnt<3,0>", ">=");
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else
	fixpnt<16, 1> a{0};

#if REGRESSION_LEVEL_1
	std::cout << "Logic: operator==()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual <fixpnt<3, 0> >(reportTestCases), "fixpnt<3,0>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<4, 0> >(reportTestCases), "fixpnt<4,0>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<4, 1> >(reportTestCases), "fixpnt<4,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<5, 0> >(reportTestCases), "fixpnt<5,0>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<5, 1> >(reportTestCases), "fixpnt<5,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<5, 2> >(reportTestCases), "fixpnt<5,2>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<6, 0> >(reportTestCases), "fixpnt<6,0>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<6, 1> >(reportTestCases), "fixpnt<6,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<6, 2> >(reportTestCases), "fixpnt<6,2>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<6, 3> >(reportTestCases), "fixpnt<6,3>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<7, 0> >(reportTestCases), "fixpnt<7,0>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<7, 1> >(reportTestCases), "fixpnt<7,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<7, 2> >(reportTestCases), "fixpnt<7,2>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<7, 3> >(reportTestCases), "fixpnt<7,3>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<7, 4> >(reportTestCases), "fixpnt<7,4>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<8, 0> >(reportTestCases), "fixpnt<8,0>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<8, 1> >(reportTestCases), "fixpnt<8,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<8, 2> >(reportTestCases), "fixpnt<8,2>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<8, 3> >(reportTestCases), "fixpnt<8,3>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<8, 4> >(reportTestCases), "fixpnt<8,4>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<8, 5> >(reportTestCases), "fixpnt<8,5>", "==");
	if (!(a == 0)) { 
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> == 0", "== int literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> == 0", "== int literal");
	}
	if (!(a == 0.0f)) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> == 0.0f", "== float literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> == 0.0f", "== float literal");
	}
	if (!(a == 0.0)) { 
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> == 0.0", "== double literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> == 0.0", "== double literal");
	}
#if LONG_DOUBLE_SUPPORT
	if (!(a == 0.0l)) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> == 0.0l", "== long double literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> == 0.0l", "== long double literal");
	}
#endif

	std::cout << "Logic: operator!=()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<3, 0> >(reportTestCases), "fixpnt<3,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<4, 0> >(reportTestCases), "fixpnt<4,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<4, 1> >(reportTestCases), "fixpnt<4,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<5, 0> >(reportTestCases), "fixpnt<5,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<5, 1> >(reportTestCases), "fixpnt<5,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<5, 2> >(reportTestCases), "fixpnt<5,2>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<6, 0> >(reportTestCases), "fixpnt<6,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<6, 1> >(reportTestCases), "fixpnt<6,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<6, 2> >(reportTestCases), "fixpnt<6,2>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<6, 3> >(reportTestCases), "fixpnt<6,3>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<7, 0> >(reportTestCases), "fixpnt<7,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<7, 1> >(reportTestCases), "fixpnt<7,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<7, 2> >(reportTestCases), "fixpnt<7,2>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<7, 3> >(reportTestCases), "fixpnt<7,3>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<8, 0> >(reportTestCases), "fixpnt<8,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<8, 1> >(reportTestCases), "fixpnt<8,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<8, 2> >(reportTestCases), "fixpnt<8,2>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<8, 3> >(reportTestCases), "fixpnt<8,3>", "!=");
	if (a != 0) { 
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> != 0", "!= int literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> != 0", "!= int literal");
	}
	if (a != 0.0f) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> != 0.0f", "!= float literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> != 0.0f", "!= float literal");
	}
	if (a != 0.0) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> != 0.0", "!= double literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> != 0.0", "!= double literal");
	}
#if LONG_DOUBLE_SUPPORT
	if (a != 0.0l) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> != 0.0l", "!= long double literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> != 0.0l", "!= long double literal");
	}
#endif

	std::cout << "Logic: operator<()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<3, 0> >(reportTestCases), "fixpnt<3,0>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<4, 0> >(reportTestCases), "fixpnt<4,0>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<4, 1> >(reportTestCases), "fixpnt<4,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<5, 0> >(reportTestCases), "fixpnt<5,0>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<5, 1> >(reportTestCases), "fixpnt<5,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<5, 2> >(reportTestCases), "fixpnt<5,2>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<6, 0> >(reportTestCases), "fixpnt<6,0>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<6, 1> >(reportTestCases), "fixpnt<6,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<6, 2> >(reportTestCases), "fixpnt<6,2>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<6, 3> >(reportTestCases), "fixpnt<6,3>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<7, 0> >(reportTestCases), "fixpnt<7,0>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<7, 1> >(reportTestCases), "fixpnt<7,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<7, 2> >(reportTestCases), "fixpnt<7,2>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<7, 3> >(reportTestCases), "fixpnt<7,3>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<8, 0> >(reportTestCases), "fixpnt<8,0>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<8, 1> >(reportTestCases), "fixpnt<8,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<8, 2> >(reportTestCases), "fixpnt<8,2>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<8, 3> >(reportTestCases), "fixpnt<8,3>", "<");
	if (a < 0) { 
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> < 0", "< int literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> < 0", "< int literal");
	}
	if (a < 0.0f) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> < 0.0f", "< float literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> < 0.0f", "< float literal");
	}
	if (a < 0.0) { 
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> < 0.0", "< double literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> < 0.0", "< double literal");
	}
#if LONG_DOUBLE_SUPPORT
	if (a < 0.0l) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> < 0.0l", "< long double literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> < 0.0l", "< long double literal");
	}
#endif

	std::cout << "Logic: operator<=()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<3, 0> >(reportTestCases), "fixpnt<3,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<4, 0> >(reportTestCases), "fixpnt<4,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<4, 1> >(reportTestCases), "fixpnt<4,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<5, 0> >(reportTestCases), "fixpnt<5,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<5, 1> >(reportTestCases), "fixpnt<5,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<5, 2> >(reportTestCases), "fixpnt<5,2>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<6, 0> >(reportTestCases), "fixpnt<6,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<6, 1> >(reportTestCases), "fixpnt<6,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<6, 2> >(reportTestCases), "fixpnt<6,2>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<6, 3> >(reportTestCases), "fixpnt<6,3>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<7, 0> >(reportTestCases), "fixpnt<7,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<7, 1> >(reportTestCases), "fixpnt<7,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<7, 2> >(reportTestCases), "fixpnt<7,2>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<7, 3> >(reportTestCases), "fixpnt<7,3>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<8, 0> >(reportTestCases), "fixpnt<8,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<8, 1> >(reportTestCases), "fixpnt<8,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<8, 2> >(reportTestCases), "fixpnt<8,2>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<8, 3> >(reportTestCases), "fixpnt<8,3>", "<=");
	if (!(a <= 0)) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> <= 0", "<= int literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> <= 0", "<= int literal");
	}
	if (!(a <= 0.0f)) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> <= 0.0f", "<= float literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> <= 0.0f", "<= float literal");
	}
	if (!(a <= 0.0)) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> <= 0.0", "<= double literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> <= 0.0", "<= double literal");
	}
#if LONG_DOUBLE_SUPPORT
	if (!(a <= 0.0l)) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> <= 0.0l", "<= long double literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> <= 0.0l", "<= long double literal");
	}
#endif

	std::cout << "Logic: operator>()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<3, 0> >(reportTestCases), "fixpnt<3,0>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<4, 0> >(reportTestCases), "fixpnt<4,0>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<4, 1> >(reportTestCases), "fixpnt<4,1>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<5, 0> >(reportTestCases), "fixpnt<5,0>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<5, 1> >(reportTestCases), "fixpnt<5,1>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<5, 2> >(reportTestCases), "fixpnt<5,2>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<6, 0> >(reportTestCases), "fixpnt<6,0>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<6, 1> >(reportTestCases), "fixpnt<6,1>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<6, 2> >(reportTestCases), "fixpnt<6,2>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<6, 3> >(reportTestCases), "fixpnt<6,3>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<7, 0> >(reportTestCases), "fixpnt<7,0>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<7, 1> >(reportTestCases), "fixpnt<7,1>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<7, 2> >(reportTestCases), "fixpnt<7,2>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<7, 3> >(reportTestCases), "fixpnt<7,3>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<8, 0> >(reportTestCases), "fixpnt<8,0>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<8, 1> >(reportTestCases), "fixpnt<8,1>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<8, 2> >(reportTestCases), "fixpnt<8,2>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<8, 3> >(reportTestCases), "fixpnt<8,3>", ">");
	if (a > 0) { 
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> > 0", "> int literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> > 0", "> int literal");
	}
	if (a > 0.0f) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> > 0.0f", "> float literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> > 0.0f", "> float literal");
	}
	if (a > 0.0) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> > 0.0", "> double literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> > 0.0", "> double literal");
	}
#if LONG_DOUBLE_SUPPORT
	if (a > 0.0l) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> > 0.0l", "> long double literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> > 0.0l", "> long double literal");
	}
#endif

	std::cout << "Logic: operator>=()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<3, 0> >(reportTestCases), "fixpnt<3,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<4, 0> >(reportTestCases), "fixpnt<4,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<4, 1> >(reportTestCases), "fixpnt<4,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<5, 0> >(reportTestCases), "fixpnt<5,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<5, 1> >(reportTestCases), "fixpnt<5,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<5, 2> >(reportTestCases), "fixpnt<5,2>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<6, 0> >(reportTestCases), "fixpnt<6,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<6, 1> >(reportTestCases), "fixpnt<6,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<6, 2> >(reportTestCases), "fixpnt<6,2>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<6, 3> >(reportTestCases), "fixpnt<6,3>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<7, 0> >(reportTestCases), "fixpnt<7,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<7, 1> >(reportTestCases), "fixpnt<7,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<7, 2> >(reportTestCases), "fixpnt<7,2>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<7, 3> >(reportTestCases), "fixpnt<7,3>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<8, 0> >(reportTestCases), "fixpnt<8,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<8, 1> >(reportTestCases), "fixpnt<8,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<8, 2> >(reportTestCases), "fixpnt<8,2>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<8, 3> >(reportTestCases), "fixpnt<8,3>", ">=");
	if (!(a >= 0)) { 
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> >= 0", ">= int literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> >= 0", ">= int literal");
	}
	if (!(a >= 0.0f)) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> >= 0.0f", ">= float literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> >= 0.0f", ">= float literal");
	}
	if (!(a >= 0.0)) { 
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> >= 0.0", ">= double literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> >= 0.0", ">= double literal");
	}
#if LONG_DOUBLE_SUPPORT
	if (!(a >= 0.0l)) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> >= 0.0l", ">= long double literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> >= 0.0l", ">= long double literal");
	}
#endif

#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<16, 1> >(reportTestCases), "fixpnt<16,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<16, 1> >(reportTestCases), "fixpnt<16,1>", "!=");
	/* <, <=, >, and >= are implemented using subtraction and thus are arithmetic complexity */
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<10, 6> >(reportTestCases), "fixpnt<10,6>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<10, 6> >(reportTestCases), "fixpnt<10,6>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<10, 6> >(reportTestCases), "fixpnt<10,6>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<10, 6> >(reportTestCases), "fixpnt<10,6>", ">=");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_internal_exception& err) {
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



