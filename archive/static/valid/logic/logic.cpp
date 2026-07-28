// logic.cpp: test suite runner for logic operators between valids
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// minimum set of include files to reflect source code dependencies
// enable literals to simplify the testing codes
#define Valid_ENABLE_LITERALS 1
#include <universal/number/valid/valid.hpp>
#include <universal/number/valid/manipulators.hpp>
//#include <universal/number/valid/math/classify.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>
#include <universal/verification/test_reporters.hpp>

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
try {;
	using namespace sw::universal;

//	int nrOfFailedTestCases = 0;
#if LATER
#if MANUAL_TESTING

	{
		constexpr size_t nbits = 8;
		constexpr size_t es = 0;
		double nan    = NAN;
		double inf    = INFINITY;
		double normal = 0;
		valid<nbits, es> pa(nan), pb(inf), pc(normal);
		std::cout << pa << " " << pb << " " << pc << std::endl;
	
		// showcasing the differences between valid and IEEE float
		std::cout << "NaN ==  NaN: IEEE=" << (nan == nan ? "true" : "false")    << "    Valid=" << (pa == pa ? "true" : "false") << std::endl;
		std::cout << "NaN == real: IEEE=" << (nan == normal ? "true" : "false") << "    Valid=" << (pa == pc ? "true" : "false") << std::endl;
		std::cout << "INF ==  INF: IEEE=" << (inf == inf ? "true" : "false")    << "    Valid=" << (pb == pb ? "true" : "false") << std::endl;
		std::cout << "NaN !=  NaN: IEEE=" << (nan != nan ? "true" : "false")    << "   Valid=" << (pa != pb ? "true" : "false") << std::endl;
		std::cout << "INF !=  INF: IEEE=" << (inf != inf ? "true" : "false")    << "   Valid=" << (pb != pb ? "true" : "false") << std::endl;
		std::cout << "NaN <= real: IEEE=" << (nan <= normal ? "true" : "false") << "    Valid=" << (pa <= pc ? "true" : "false") << std::endl;
		std::cout << "NaN >= real: IEEE=" << (nan >= normal ? "true" : "false") << "    Valid=" << (pa >= pc ? "true" : "false") << std::endl;
		std::cout << "INF <  real: IEEE=" << (inf <  normal ? "true" : "false") << "   Valid=" << (pa <  pc ? "true" : "false") << std::endl;
		std::cout << "INF >  real: IEEE=" << (inf  > normal ? "true" : "false") << "    Valid=" << (pa  > pc ? "true" : "false") << std::endl;
	}

	{
		nrOfFailedTestCases += ReportTestResult(VerifyValidLogicEqual<3, 0>(), "valid<3,0>", "==");
		nrOfFailedTestCases += ReportTestResult(VerifyValidLogicNotEqual<3, 0>(), "valid<3,0>", "!=");
		nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessThan<3, 0>(), "valid<3,0>", "<");
		nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterThan<3, 0>(), "valid<3,0>", ">");
		nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessOrEqualThan<3, 0>(), "valid<3,0>", "<=");
		nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterOrEqualThan<3, 0>(), "valid<3,0>", ">=");
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else
	valid<16, 1> p;

#if REGRESSION_LEVEL_1
	cout << "Logic: operator==()" << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicEqual<3, 0>(), "valid<3,0>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicEqual<4, 0>(), "valid<4,0>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicEqual<4, 1>(), "valid<4,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicEqual<5, 0>(), "valid<5,0>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicEqual<5, 1>(), "valid<5,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicEqual<5, 2>(), "valid<5,2>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicEqual<6, 0>(), "valid<6,0>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicEqual<6, 1>(), "valid<6,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicEqual<6, 2>(), "valid<6,2>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicEqual<6, 3>(), "valid<6,3>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicEqual<7, 0>(), "valid<7,0>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicEqual<7, 1>(), "valid<7,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicEqual<7, 2>(), "valid<7,2>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicEqual<7, 3>(), "valid<7,3>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicEqual<7, 4>(), "valid<7,4>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicEqual<8, 0>(), "valid<8,0>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicEqual<8, 1>(), "valid<8,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicEqual<8, 2>(), "valid<8,2>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicEqual<8, 3>(), "valid<8,3>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicEqual<8, 4>(), "valid<8,4>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicEqual<8, 5>(), "valid<8,5>", "==");
	if (!(p == 0)) { 
		nrOfFailedTestCases += ReportTestResult(1, "valid<16,1> == 0", "== int literal");
	}
	else {
		ReportTestResult(0, "valid<16,1> == 0", "== int literal");
	}
	if (!(p == 0.0f)) {
		nrOfFailedTestCases += ReportTestResult(1, "valid<16,1> == 0.0", "== float literal");
	}
	else {
		ReportTestResult(0, "valid<16,1> == 0.0", "== float literal");
	}
	if (!(p == 0.0)) { 
		nrOfFailedTestCases += ReportTestResult(1, "valid<16,1> == 0.0", "== double literal");
	}
	else {
		ReportTestResult(0, "valid<16,1> == 0.0", "== double literal");
	}
	if (!(p == 0.0l)) {
		nrOfFailedTestCases += ReportTestResult(1, "valid<16,1> == 0.0", "== long double literal");
	}
	else {
		ReportTestResult(0, "valid<16,1> == 0.0", "== long double literal");
	}
	
	cout << "Logic: operator!=()" << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicNotEqual<3, 0>(), "valid<3,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicNotEqual<4, 0>(), "valid<4,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicNotEqual<4, 1>(), "valid<4,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicNotEqual<5, 0>(), "valid<5,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicNotEqual<5, 1>(), "valid<5,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicNotEqual<5, 2>(), "valid<5,2>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicNotEqual<6, 0>(), "valid<6,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicNotEqual<6, 1>(), "valid<6,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicNotEqual<6, 2>(), "valid<6,2>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicNotEqual<6, 3>(), "valid<6,3>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicNotEqual<7, 0>(), "valid<7,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicNotEqual<7, 1>(), "valid<7,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicNotEqual<7, 2>(), "valid<7,2>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicNotEqual<7, 3>(), "valid<7,3>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicNotEqual<8, 0>(), "valid<8,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicNotEqual<8, 1>(), "valid<8,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicNotEqual<8, 2>(), "valid<8,2>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicNotEqual<8, 3>(), "valid<8,3>", "!=");
	if (p != 0) { 
		nrOfFailedTestCases += ReportTestResult(1, "valid<16,1> != 0", "!= int literal");
	}
	else {
		ReportTestResult(0, "valid<16,1> != 0", "!= int literal");
	}
	if (p != 0.0f) {
		nrOfFailedTestCases += ReportTestResult(1, "valid<16,1> != 0.0", "!= float literal");
	}
	else {
		ReportTestResult(0, "valid<16,1> != 0.0", "!= float literal");
	}
	if (p != 0.0) {
		nrOfFailedTestCases += ReportTestResult(1, "valid<16,1> != 0.0", "!= double literal");
	}
	else {
		ReportTestResult(0, "valid<16,1> != 0.0", "!= double literal");
	}
	if (p != 0.0l) {
		nrOfFailedTestCases += ReportTestResult(1, "valid<16,1> != 0.0", "!= long double literal");
	}
	else {
		ReportTestResult(0, "valid<16,1> != 0.0", "!= long double literal");
	}

	std::cout << "Logic: operator<()" << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessThan<3, 0>(), "valid<3,0>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessThan<4, 0>(), "valid<4,0>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessThan<4, 1>(), "valid<4,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessThan<5, 0>(), "valid<5,0>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessThan<5, 1>(), "valid<5,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessThan<5, 2>(), "valid<5,2>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessThan<6, 0>(), "valid<6,0>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessThan<6, 1>(), "valid<6,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessThan<6, 2>(), "valid<6,2>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessThan<6, 3>(), "valid<6,3>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessThan<7, 0>(), "valid<7,0>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessThan<7, 1>(), "valid<7,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessThan<7, 2>(), "valid<7,2>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessThan<7, 3>(), "valid<7,3>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessThan<8, 0>(), "valid<8,0>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessThan<8, 1>(), "valid<8,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessThan<8, 2>(), "valid<8,2>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessThan<8, 3>(), "valid<8,3>", "<");
	if (p < 0) { 
		nrOfFailedTestCases += ReportTestResult(1, "valid<16,1> < 0", "< int literal");
	}
	else {
		ReportTestResult(0, "valid<16,1> < 0", "< int literal");
	}
	if (p < 0.0f) {
		nrOfFailedTestCases += ReportTestResult(1, "valid<16,1> < 0.0", "< float literal");
	}
	else {
		ReportTestResult(0, "valid<16,1> < 0.0", "< float literal");
	}
	if (p < 0.0) { 
		nrOfFailedTestCases += ReportTestResult(1, "valid<16,1> < 0.0", "< double literal");
	}
	else {
		ReportTestResult(0, "valid<16,1> < 0.0", "< double literal");
	}
	if (p < 0.0l) {
		nrOfFailedTestCases += ReportTestResult(1, "valid<16,1> < 0.0", "< long double literal");
	}
	else {
		ReportTestResult(0, "valid<16,1> < 0.0", "< long double literal");
	}

	std::cout << "Logic: operator<=()" << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessOrEqualThan<3, 0>(), "valid<3,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessOrEqualThan<4, 0>(), "valid<4,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessOrEqualThan<4, 1>(), "valid<4,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessOrEqualThan<5, 0>(), "valid<5,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessOrEqualThan<5, 1>(), "valid<5,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessOrEqualThan<5, 2>(), "valid<5,2>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessOrEqualThan<6, 0>(), "valid<6,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessOrEqualThan<6, 1>(), "valid<6,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessOrEqualThan<6, 2>(), "valid<6,2>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessOrEqualThan<6, 3>(), "valid<6,3>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessOrEqualThan<7, 0>(), "valid<7,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessOrEqualThan<7, 1>(), "valid<7,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessOrEqualThan<7, 2>(), "valid<7,2>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessOrEqualThan<7, 3>(), "valid<7,3>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessOrEqualThan<8, 0>(), "valid<8,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessOrEqualThan<8, 1>(), "valid<8,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessOrEqualThan<8, 2>(), "valid<8,2>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessOrEqualThan<8, 3>(), "valid<8,3>", "<=");
	if (!(p <= 0)) {
		nrOfFailedTestCases += ReportTestResult(1, "valid<16,1> <= 0", "<= int literal");
	}
	else {
		ReportTestResult(0, "valid<16,1> <= 0", "<= int literal");
	}
	if (!(p <= 0.0f)) {
		nrOfFailedTestCases += ReportTestResult(1, "valid<16,1> <= 0.0", "<= float literal");
	}
	else {
		ReportTestResult(0, "valid<16,1> <= 0.0", "<= float literal");
	}
	if (!(p <= 0.0)) {
		nrOfFailedTestCases += ReportTestResult(1, "valid<16,1> <= 0.0", "<= double literal");
	}
	else {
		ReportTestResult(0, "valid<16,1> <= 0.0", "<= double literal");
	}
	if (!(p <= 0.0l)) {
		nrOfFailedTestCases += ReportTestResult(1, "valid<16,1> <= 0.0", "<= long double literal");
	}
	else {
		ReportTestResult(0, "valid<16,1> <= 0.0", "<= long double literal");
	}

	std::cout << "Logic: operator>()" << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterThan<3, 0>(), "valid<3,0>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterThan<4, 0>(), "valid<4,0>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterThan<4, 1>(), "valid<4,1>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterThan<5, 0>(), "valid<5,0>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterThan<5, 1>(), "valid<5,1>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterThan<5, 2>(), "valid<5,2>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterThan<6, 0>(), "valid<6,0>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterThan<6, 1>(), "valid<6,1>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterThan<6, 2>(), "valid<6,2>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterThan<6, 3>(), "valid<6,3>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterThan<7, 0>(), "valid<7,0>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterThan<7, 1>(), "valid<7,1>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterThan<7, 2>(), "valid<7,2>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterThan<7, 3>(), "valid<7,3>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterThan<8, 0>(), "valid<8,0>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterThan<8, 1>(), "valid<8,1>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterThan<8, 2>(), "valid<8,2>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterThan<8, 3>(), "valid<8,3>", ">");
	if (p > 0) { 
		nrOfFailedTestCases += ReportTestResult(1, "valid<16,1> > 0", "> int literal");
	}
	else {
		ReportTestResult(0, "valid<16,1> > 0", "> int literal");
	}
	if (p > 0.0f) {
		nrOfFailedTestCases += ReportTestResult(1, "valid<16,1> > 0.0", "> float literal");
	}
	else {
		ReportTestResult(0, "valid<16,1> > 0.0", "> float literal");
	}
	if (p > 0.0) {
		nrOfFailedTestCases += ReportTestResult(1, "valid<16,1> > 0.0", "> double literal");
	}
	else {
		ReportTestResult(0, "valid<16,1> > 0.0", "> double literal");
	}
	if (p > 0.0l) {
		nrOfFailedTestCases += ReportTestResult(1, "valid<16,1> > 0.0", "> long double literal");
	}
	else {
		ReportTestResult(0, "valid<16,1> > 0.0", "> long double literal");
	}

	std::cout << "Logic: operator>=()" << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterOrEqualThan<3, 0>(), "valid<3,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterOrEqualThan<4, 0>(), "valid<4,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterOrEqualThan<4, 1>(), "valid<4,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterOrEqualThan<5, 0>(), "valid<5,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterOrEqualThan<5, 1>(), "valid<5,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterOrEqualThan<5, 2>(), "valid<5,2>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterOrEqualThan<6, 0>(), "valid<6,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterOrEqualThan<6, 1>(), "valid<6,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterOrEqualThan<6, 2>(), "valid<6,2>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterOrEqualThan<6, 3>(), "valid<6,3>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterOrEqualThan<7, 0>(), "valid<7,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterOrEqualThan<7, 1>(), "valid<7,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterOrEqualThan<7, 2>(), "valid<7,2>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterOrEqualThan<7, 3>(), "valid<7,3>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterOrEqualThan<8, 0>(), "valid<8,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterOrEqualThan<8, 1>(), "valid<8,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterOrEqualThan<8, 2>(), "valid<8,2>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterOrEqualThan<8, 3>(), "valid<8,3>", ">=");
	if (!(p >= 0)) { 
		nrOfFailedTestCases += ReportTestResult(1, "valid<16,1> >= 0", ">= int literal");
	}
	else {
		ReportTestResult(0, "valid<16,1> >= 0", ">= int literal");
	}
	if (!(p >= 0.0f)) {
		nrOfFailedTestCases += ReportTestResult(1, "valid<16,1> >= 0.0", ">= float literal");
	}
	else {
		ReportTestResult(0, "valid<16,1> >= 0.0", ">= float literal");
	}
	if (!(p >= 0.0)) { 
		nrOfFailedTestCases += ReportTestResult(1, "valid<16,1> >= 0.0", ">= double literal");
	}
	else {
		ReportTestResult(0, "valid<16,1> >= 0.0", ">= double literal");
	}
	if (!(p >= 0.0l)) {
		nrOfFailedTestCases += ReportTestResult(1, "valid<16,1> >= 0.0", ">= long double literal");
	}
	else {
		ReportTestResult(0, "valid<16,1> >= 0.0", ">= long double literal");
	}
#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicEqual<16, 1>(), "valid<16,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicNotEqual<16, 1>(), "valid<16,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessThan<16, 1>(), "valid<16,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicLessOrEqualThan<16, 1>(), "valid<16,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterThan<16, 1>(), "valid<16,1>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyValidLogicGreaterOrEqualThan<16, 1>(), "valid<16,1>", ">=");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif // MANUAL_TESTING
#endif // LATER
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}



