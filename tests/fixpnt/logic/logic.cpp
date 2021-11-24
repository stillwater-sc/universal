// logic.cpp :test suite runner for logic operators between fixpnts and literals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/fixpnt_test_suite.hpp>

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
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
//	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

#if MANUAL_TESTING

	{
		constexpr size_t nbits = 8;
		constexpr size_t es = 0;
		fixpnt<nbits, es> a(SpecialValue::maxpos), b(SpecialValue::maxpos), c(SpecialValue::maxneg);
		if (a == b) std::cout << "equal\n";
	
	}

	{
		nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<3, 0>(), "fixpnt<3,0>", "==");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual<3, 0>(), "fixpnt<3,0>", "!=");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan<3, 0>(), "fixpnt<3,0>", "<");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan<3, 0>(), "fixpnt<3,0>", ">");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan<3, 0>(), "fixpnt<3,0>", "<=");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<3, 0>(), "fixpnt<3,0>", ">=");
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else
	fixpnt<16, 1> a;

#if REGRESSION_LEVEL_1
	std::cout << "Logic: operator==()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual <fixpnt<3, 0> >(), "fixpnt<3,0>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<4, 0> >(), "fixpnt<4,0>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<4, 1> >(), "fixpnt<4,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<5, 0> >(), "fixpnt<5,0>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<5, 1> >(), "fixpnt<5,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<5, 2> >(), "fixpnt<5,2>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<6, 0> >(), "fixpnt<6,0>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<6, 1> >(), "fixpnt<6,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<6, 2> >(), "fixpnt<6,2>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<6, 3> >(), "fixpnt<6,3>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<7, 0> >(), "fixpnt<7,0>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<7, 1> >(), "fixpnt<7,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<7, 2> >(), "fixpnt<7,2>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<7, 3> >(), "fixpnt<7,3>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<7, 4> >(), "fixpnt<7,4>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<8, 0> >(), "fixpnt<8,0>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<8, 1> >(), "fixpnt<8,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<8, 2> >(), "fixpnt<8,2>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<8, 3> >(), "fixpnt<8,3>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<8, 4> >(), "fixpnt<8,4>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< fixpnt<8, 5> >(), "fixpnt<8,5>", "==");
	if (!(a == 0)) { 
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> == 0", "== int literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> == 0", "== int literal");
	}
	if (!(a == 0.0f)) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> == 0.0", "== float literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> == 0.0", "== float literal");
	}
	if (!(a == 0.0)) { 
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> == 0.0", "== double literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> == 0.0", "== double literal");
	}
#if LONG_DOUBLE_SUPPORT
	if (!(a == 0.0l)) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> == 0.0", "== long double literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> == 0.0", "== long double literal");
	}
#endif

	std::cout << "Logic: operator!=()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<3, 0> >(), "fixpnt<3,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<4, 0> >(), "fixpnt<4,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<4, 1> >(), "fixpnt<4,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<5, 0> >(), "fixpnt<5,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<5, 1> >(), "fixpnt<5,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<5, 2> >(), "fixpnt<5,2>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<6, 0> >(), "fixpnt<6,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<6, 1> >(), "fixpnt<6,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<6, 2> >(), "fixpnt<6,2>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<6, 3> >(), "fixpnt<6,3>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<7, 0> >(), "fixpnt<7,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<7, 1> >(), "fixpnt<7,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<7, 2> >(), "fixpnt<7,2>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<7, 3> >(), "fixpnt<7,3>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<8, 0> >(), "fixpnt<8,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<8, 1> >(), "fixpnt<8,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<8, 2> >(), "fixpnt<8,2>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< fixpnt<8, 3> >(), "fixpnt<8,3>", "!=");
	if (a != 0) { 
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> != 0", "!= int literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> != 0", "!= int literal");
	}
	if (a != 0.0f) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> != 0.0", "!= float literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> != 0.0", "!= float literal");
	}
	if (a != 0.0) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> != 0.0", "!= double literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> != 0.0", "!= double literal");
	}
#if LONG_DOUBLE_SUPPORT
	if (a != 0.0l) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> != 0.0", "!= long double literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> != 0.0", "!= long double literal");
	}
#endif

	std::cout << "Logic: operator<()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<3, 0> >(), "fixpnt<3,0>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<4, 0> >(), "fixpnt<4,0>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<4, 1> >(), "fixpnt<4,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<5, 0> >(), "fixpnt<5,0>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<5, 1> >(), "fixpnt<5,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<5, 2> >(), "fixpnt<5,2>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<6, 0> >(), "fixpnt<6,0>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<6, 1> >(), "fixpnt<6,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<6, 2> >(), "fixpnt<6,2>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<6, 3> >(), "fixpnt<6,3>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<7, 0> >(), "fixpnt<7,0>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<7, 1> >(), "fixpnt<7,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<7, 2> >(), "fixpnt<7,2>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<7, 3> >(), "fixpnt<7,3>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<8, 0> >(), "fixpnt<8,0>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<8, 1> >(), "fixpnt<8,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<8, 2> >(), "fixpnt<8,2>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< fixpnt<8, 3> >(), "fixpnt<8,3>", "<");
	if (a < 0) { 
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> < 0", "< int literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> < 0", "< int literal");
	}
	if (a < 0.0f) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> < 0.0", "< float literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> < 0.0", "< float literal");
	}
	if (a < 0.0) { 
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> < 0.0", "< double literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> < 0.0", "< double literal");
	}
#if LONG_DOUBLE_SUPPORT
	if (a < 0.0l) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> < 0.0", "< long double literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> < 0.0", "< long double literal");
	}
#endif

	std::cout << "Logic: operator<=()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<3, 0> >(), "fixpnt<3,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<4, 0> >(), "fixpnt<4,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<4, 1> >(), "fixpnt<4,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<5, 0> >(), "fixpnt<5,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<5, 1> >(), "fixpnt<5,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<5, 2> >(), "fixpnt<5,2>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<6, 0> >(), "fixpnt<6,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<6, 1> >(), "fixpnt<6,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<6, 2> >(), "fixpnt<6,2>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<6, 3> >(), "fixpnt<6,3>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<7, 0> >(), "fixpnt<7,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<7, 1> >(), "fixpnt<7,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<7, 2> >(), "fixpnt<7,2>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<7, 3> >(), "fixpnt<7,3>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<8, 0> >(), "fixpnt<8,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<8, 1> >(), "fixpnt<8,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<8, 2> >(), "fixpnt<8,2>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< fixpnt<8, 3> >(), "fixpnt<8,3>", "<=");
	if (!(a <= 0)) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> <= 0", "<= int literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> <= 0", "<= int literal");
	}
	if (!(a <= 0.0f)) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> <= 0.0", "<= float literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> <= 0.0", "<= float literal");
	}
	if (!(a <= 0.0)) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> <= 0.0", "<= double literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> <= 0.0", "<= double literal");
	}
#if LONG_DOUBLE_SUPPORT
	if (!(a <= 0.0l)) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> <= 0.0", "<= long double literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> <= 0.0", "<= long double literal");
	}
#endif

	std::cout << "Logic: operator>()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<3, 0> >(), "fixpnt<3,0>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<4, 0> >(), "fixpnt<4,0>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<4, 1> >(), "fixpnt<4,1>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<5, 0> >(), "fixpnt<5,0>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<5, 1> >(), "fixpnt<5,1>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<5, 2> >(), "fixpnt<5,2>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<6, 0> >(), "fixpnt<6,0>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<6, 1> >(), "fixpnt<6,1>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<6, 2> >(), "fixpnt<6,2>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<6, 3> >(), "fixpnt<6,3>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<7, 0> >(), "fixpnt<7,0>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<7, 1> >(), "fixpnt<7,1>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<7, 2> >(), "fixpnt<7,2>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<7, 3> >(), "fixpnt<7,3>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<8, 0> >(), "fixpnt<8,0>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<8, 1> >(), "fixpnt<8,1>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<8, 2> >(), "fixpnt<8,2>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< fixpnt<8, 3> >(), "fixpnt<8,3>", ">");
	if (a > 0) { 
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> > 0", "> int literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> > 0", "> int literal");
	}
	if (a > 0.0f) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> > 0.0", "> float literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> > 0.0", "> float literal");
	}
	if (a > 0.0) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> > 0.0", "> double literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> > 0.0", "> double literal");
	}
#if LONG_DOUBLE_SUPPORT
	if (a > 0.0l) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> > 0.0", "> long double literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> > 0.0", "> long double literal");
	}
#endif

	std::cout << "Logic: operator>=()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<3, 0> >(), "fixpnt<3,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<4, 0> >(), "fixpnt<4,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<4, 1> >(), "fixpnt<4,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<5, 0> >(), "fixpnt<5,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<5, 1> >(), "fixpnt<5,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<5, 2> >(), "fixpnt<5,2>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<6, 0> >(), "fixpnt<6,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<6, 1> >(), "fixpnt<6,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<6, 2> >(), "fixpnt<6,2>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<6, 3> >(), "fixpnt<6,3>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<7, 0> >(), "fixpnt<7,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<7, 1> >(), "fixpnt<7,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<7, 2> >(), "fixpnt<7,2>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<7, 3> >(), "fixpnt<7,3>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<8, 0> >(), "fixpnt<8,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<8, 1> >(), "fixpnt<8,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<8, 2> >(), "fixpnt<8,2>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< fixpnt<8, 3> >(), "fixpnt<8,3>", ">=");
	if (!(a >= 0)) { 
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> >= 0", ">= int literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> >= 0", ">= int literal");
	}
	if (!(a >= 0.0f)) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> >= 0.0", ">= float literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> >= 0.0", ">= float literal");
	}
	if (!(a >= 0.0)) { 
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> >= 0.0", ">= double literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> >= 0.0", ">= double literal");
	}
#if LONG_DOUBLE_SUPPORT
	if (!(a >= 0.0l)) {
		nrOfFailedTestCases += ReportTestResult(1, "fixpnt<16,1> >= 0.0", ">= long double literal");
	}
	else {
		ReportTestResult(0, "fixpnt<16,1> >= 0.0", ">= long double literal");
	}
#endif

#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<16, 1>(), "fixpnt<16,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual<16, 1>(), "fixpnt<16,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan<16, 1>(), "fixpnt<16,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan<16, 1>(), "fixpnt<16,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan<16, 1>(), "fixpnt<16,1>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<16, 1>(), "fixpnt<16,1>", ">=");
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



