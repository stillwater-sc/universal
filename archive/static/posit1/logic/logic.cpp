// logic.cpp :test suite runner for logic operators between posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// enable literals to simplify the testing codes
#define POSIT_ENABLE_LITERALS 1
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/posit_test_suite.hpp>

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

	std::string test_suite  = "posit logic operator validation";
	std::string test_tag    = "comparisons";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		constexpr size_t nbits = 8;
		constexpr size_t es = 0;
		double nan    = NAN;
		double inf    = INFINITY;
		double normal = 0;
		posit<nbits, es> pa(nan), pb(inf), pc(normal);
		std::cout << pa << " " << pb << " " << pc << std::endl;
	
		// showcasing the differences between posit and IEEE float
		std::cout << "NaN ==  NaN: IEEE=" << (nan == nan ? "true" : "false")    << "    Posit=" << (pa == pa ? "true" : "false") << std::endl;
		std::cout << "NaN == real: IEEE=" << (nan == normal ? "true" : "false") << "    Posit=" << (pa == pc ? "true" : "false") << std::endl;
		std::cout << "INF ==  INF: IEEE=" << (inf == inf ? "true" : "false")    << "    Posit=" << (pb == pb ? "true" : "false") << std::endl;
		std::cout << "NaN !=  NaN: IEEE=" << (nan != nan ? "true" : "false")    << "   Posit=" << (pa != pb ? "true" : "false") << std::endl;
		std::cout << "INF !=  INF: IEEE=" << (inf != inf ? "true" : "false")    << "   Posit=" << (pb != pb ? "true" : "false") << std::endl;
		std::cout << "NaN <= real: IEEE=" << (nan <= normal ? "true" : "false") << "    Posit=" << (pa <= pc ? "true" : "false") << std::endl;
		std::cout << "NaN >= real: IEEE=" << (nan >= normal ? "true" : "false") << "    Posit=" << (pa >= pc ? "true" : "false") << std::endl;
		std::cout << "INF <  real: IEEE=" << (inf <  normal ? "true" : "false") << "   Posit=" << (pa <  pc ? "true" : "false") << std::endl;
		std::cout << "INF >  real: IEEE=" << (inf  > normal ? "true" : "false") << "    Posit=" << (pa  > pc ? "true" : "false") << std::endl;
	}

	{
		nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<posit<3, 0>>(reportTestCases), "posit<3,0>", "==");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual<posit<3, 0>>(reportTestCases), "posit<3,0>", "!=");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan<posit<3, 0>>(reportTestCases), "posit<3,0>", "<");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan<posit<3, 0>>(reportTestCases), "posit<3,0>", ">");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan<posit<3, 0>>(reportTestCases), "posit<3,0>", "<=");
		nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<posit<3, 0>>(reportTestCases), "posit<3,0>", ">=");
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else
	posit<16, 1> p;

#if REGRESSION_LEVEL_1
	std::cout << "Logic: operator==()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<posit<3, 0>>(reportTestCases), "posit<3,0>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<posit<4, 0>>(reportTestCases), "posit<4,0>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<posit<4, 1>>(reportTestCases), "posit<4,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<posit<5, 0>>(reportTestCases), "posit<5,0>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<posit<5, 1>>(reportTestCases), "posit<5,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<posit<5, 2>>(reportTestCases), "posit<5,2>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<posit<6, 0>>(reportTestCases), "posit<6,0>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<posit<6, 1>>(reportTestCases), "posit<6,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<posit<6, 2>>(reportTestCases), "posit<6,2>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<posit<6, 3>>(reportTestCases), "posit<6,3>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<posit<7, 0>>(reportTestCases), "posit<7,0>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<posit<7, 1>>(reportTestCases), "posit<7,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<posit<7, 2>>(reportTestCases), "posit<7,2>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<posit<7, 3>>(reportTestCases), "posit<7,3>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<posit<7, 4>>(reportTestCases), "posit<7,4>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<posit<8, 0>>(reportTestCases), "posit<8,0>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<posit<8, 1>>(reportTestCases), "posit<8,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<posit<8, 2>>(reportTestCases), "posit<8,2>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<posit<8, 3>>(reportTestCases), "posit<8,3>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<posit<8, 4>>(reportTestCases), "posit<8,4>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<posit<8, 5>>(reportTestCases), "posit<8,5>", "==");
	if (!(p == 0)) { 
		nrOfFailedTestCases += ReportTestResult(1, "posit<16,1> == 0", "== int literal");
	}
	else {
		ReportTestResult(0, "posit<16,1> == 0", "== int literal");
	}
	if (!(p == 0.0f)) {
		nrOfFailedTestCases += ReportTestResult(1, "posit<16,1> == 0.0", "== float literal");
	}
	else {
		ReportTestResult(0, "posit<16,1> == 0.0", "== float literal");
	}
	if (!(p == 0.0)) { 
		nrOfFailedTestCases += ReportTestResult(1, "posit<16,1> == 0.0", "== double literal");
	}
	else {
		ReportTestResult(0, "posit<16,1> == 0.0", "== double literal");
	}
	if (!(p == 0.0l)) {
		nrOfFailedTestCases += ReportTestResult(1, "posit<16,1> == 0.0", "== long double literal");
	}
	else {
		ReportTestResult(0, "posit<16,1> == 0.0", "== long double literal");
	}
	
	std::cout << "Logic: operator!=()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual<posit<3, 0>>(reportTestCases), "posit<3,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual<posit<4, 0>>(reportTestCases), "posit<4,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual<posit<4, 1>>(reportTestCases), "posit<4,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual<posit<5, 0>>(reportTestCases), "posit<5,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual<posit<5, 1>>(reportTestCases), "posit<5,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual<posit<5, 2>>(reportTestCases), "posit<5,2>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual<posit<6, 0>>(reportTestCases), "posit<6,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual<posit<6, 1>>(reportTestCases), "posit<6,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual<posit<6, 2>>(reportTestCases), "posit<6,2>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual<posit<6, 3>>(reportTestCases), "posit<6,3>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual<posit<7, 0>>(reportTestCases), "posit<7,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual<posit<7, 1>>(reportTestCases), "posit<7,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual<posit<7, 2>>(reportTestCases), "posit<7,2>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual<posit<7, 3>>(reportTestCases), "posit<7,3>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual<posit<8, 0>>(reportTestCases), "posit<8,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual<posit<8, 1>>(reportTestCases), "posit<8,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual<posit<8, 2>>(reportTestCases), "posit<8,2>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual<posit<8, 3>>(reportTestCases), "posit<8,3>", "!=");
	if (p != 0) { 
		nrOfFailedTestCases += ReportTestResult(1, "posit<16,1> != 0", "!= int literal");
	}
	else {
		ReportTestResult(0, "posit<16,1> != 0", "!= int literal");
	}
	if (p != 0.0f) {
		nrOfFailedTestCases += ReportTestResult(1, "posit<16,1> != 0.0", "!= float literal");
	}
	else {
		ReportTestResult(0, "posit<16,1> != 0.0", "!= float literal");
	}
	if (p != 0.0) {
		nrOfFailedTestCases += ReportTestResult(1, "posit<16,1> != 0.0", "!= double literal");
	}
	else {
		ReportTestResult(0, "posit<16,1> != 0.0", "!= double literal");
	}
	if (p != 0.0l) {
		nrOfFailedTestCases += ReportTestResult(1, "posit<16,1> != 0.0", "!= long double literal");
	}
	else {
		ReportTestResult(0, "posit<16,1> != 0.0", "!= long double literal");
	}

	std::cout << "Logic: operator<()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan<posit<3, 0>>(reportTestCases), "posit<3,0>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan<posit<4, 0>>(reportTestCases), "posit<4,0>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan<posit<4, 1>>(reportTestCases), "posit<4,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan<posit<5, 0>>(reportTestCases), "posit<5,0>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan<posit<5, 1>>(reportTestCases), "posit<5,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan<posit<5, 2>>(reportTestCases), "posit<5,2>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan<posit<6, 0>>(reportTestCases), "posit<6,0>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan<posit<6, 1>>(reportTestCases), "posit<6,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan<posit<6, 2>>(reportTestCases), "posit<6,2>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan<posit<6, 3>>(reportTestCases), "posit<6,3>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan<posit<7, 0>>(reportTestCases), "posit<7,0>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan<posit<7, 1>>(reportTestCases), "posit<7,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan<posit<7, 2>>(reportTestCases), "posit<7,2>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan<posit<7, 3>>(reportTestCases), "posit<7,3>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan<posit<8, 0>>(reportTestCases), "posit<8,0>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan<posit<8, 1>>(reportTestCases), "posit<8,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan<posit<8, 2>>(reportTestCases), "posit<8,2>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan<posit<8, 3>>(reportTestCases), "posit<8,3>", "<");
	if (p < 0) { 
		nrOfFailedTestCases += ReportTestResult(1, "posit<16,1> < 0", "< int literal");
	}
	else {
		ReportTestResult(0, "posit<16,1> < 0", "< int literal");
	}
	if (p < 0.0f) {
		nrOfFailedTestCases += ReportTestResult(1, "posit<16,1> < 0.0", "< float literal");
	}
	else {
		ReportTestResult(0, "posit<16,1> < 0.0", "< float literal");
	}
	if (p < 0.0) { 
		nrOfFailedTestCases += ReportTestResult(1, "posit<16,1> < 0.0", "< double literal");
	}
	else {
		ReportTestResult(0, "posit<16,1> < 0.0", "< double literal");
	}
	if (p < 0.0l) {
		nrOfFailedTestCases += ReportTestResult(1, "posit<16,1> < 0.0", "< long double literal");
	}
	else {
		ReportTestResult(0, "posit<16,1> < 0.0", "< long double literal");
	}

	std::cout << "Logic: operator<=()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan<posit<3, 0>>(reportTestCases), "posit<3,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan<posit<4, 0>>(reportTestCases), "posit<4,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan<posit<4, 1>>(reportTestCases), "posit<4,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan<posit<5, 0>>(reportTestCases), "posit<5,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan<posit<5, 1>>(reportTestCases), "posit<5,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan<posit<5, 2>>(reportTestCases), "posit<5,2>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan<posit<6, 0>>(reportTestCases), "posit<6,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan<posit<6, 1>>(reportTestCases), "posit<6,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan<posit<6, 2>>(reportTestCases), "posit<6,2>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan<posit<6, 3>>(reportTestCases), "posit<6,3>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan<posit<7, 0>>(reportTestCases), "posit<7,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan<posit<7, 1>>(reportTestCases), "posit<7,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan<posit<7, 2>>(reportTestCases), "posit<7,2>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan<posit<7, 3>>(reportTestCases), "posit<7,3>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan<posit<8, 0>>(reportTestCases), "posit<8,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan<posit<8, 1>>(reportTestCases), "posit<8,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan<posit<8, 2>>(reportTestCases), "posit<8,2>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan<posit<8, 3>>(reportTestCases), "posit<8,3>", "<=");
	if (!(p <= 0)) {
		nrOfFailedTestCases += ReportTestResult(1, "posit<16,1> <= 0", "<= int literal");
	}
	else {
		ReportTestResult(0, "posit<16,1> <= 0", "<= int literal");
	}
	if (!(p <= 0.0f)) {
		nrOfFailedTestCases += ReportTestResult(1, "posit<16,1> <= 0.0", "<= float literal");
	}
	else {
		ReportTestResult(0, "posit<16,1> <= 0.0", "<= float literal");
	}
	if (!(p <= 0.0)) {
		nrOfFailedTestCases += ReportTestResult(1, "posit<16,1> <= 0.0", "<= double literal");
	}
	else {
		ReportTestResult(0, "posit<16,1> <= 0.0", "<= double literal");
	}
	if (!(p <= 0.0l)) {
		nrOfFailedTestCases += ReportTestResult(1, "posit<16,1> <= 0.0", "<= long double literal");
	}
	else {
		ReportTestResult(0, "posit<16,1> <= 0.0", "<= long double literal");
	}

	std::cout << "Logic: operator>()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan<posit<3, 0>>(reportTestCases), "posit<3,0>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan<posit<4, 0>>(reportTestCases), "posit<4,0>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan<posit<4, 1>>(reportTestCases), "posit<4,1>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan<posit<5, 0>>(reportTestCases), "posit<5,0>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan<posit<5, 1>>(reportTestCases), "posit<5,1>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan<posit<5, 2>>(reportTestCases), "posit<5,2>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan<posit<6, 0>>(reportTestCases), "posit<6,0>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan<posit<6, 1>>(reportTestCases), "posit<6,1>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan<posit<6, 2>>(reportTestCases), "posit<6,2>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan<posit<6, 3>>(reportTestCases), "posit<6,3>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan<posit<7, 0>>(reportTestCases), "posit<7,0>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan<posit<7, 1>>(reportTestCases), "posit<7,1>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan<posit<7, 2>>(reportTestCases), "posit<7,2>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan<posit<7, 3>>(reportTestCases), "posit<7,3>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan<posit<8, 0>>(reportTestCases), "posit<8,0>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan<posit<8, 1>>(reportTestCases), "posit<8,1>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan<posit<8, 2>>(reportTestCases), "posit<8,2>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan<posit<8, 3>>(reportTestCases), "posit<8,3>", ">");
	if (p > 0) { 
		nrOfFailedTestCases += ReportTestResult(1, "posit<16,1> > 0", "> int literal");
	}
	else {
		ReportTestResult(0, "posit<16,1> > 0", "> int literal");
	}
	if (p > 0.0f) {
		nrOfFailedTestCases += ReportTestResult(1, "posit<16,1> > 0.0", "> float literal");
	}
	else {
		ReportTestResult(0, "posit<16,1> > 0.0", "> float literal");
	}
	if (p > 0.0) {
		nrOfFailedTestCases += ReportTestResult(1, "posit<16,1> > 0.0", "> double literal");
	}
	else {
		ReportTestResult(0, "posit<16,1> > 0.0", "> double literal");
	}
	if (p > 0.0l) {
		nrOfFailedTestCases += ReportTestResult(1, "posit<16,1> > 0.0", "> long double literal");
	}
	else {
		ReportTestResult(0, "posit<16,1> > 0.0", "> long double literal");
	}

	std::cout << "Logic: operator>=()\n";
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<posit<3, 0>>(reportTestCases), "posit<3,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<posit<4, 0>>(reportTestCases), "posit<4,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<posit<4, 1>>(reportTestCases), "posit<4,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<posit<5, 0>>(reportTestCases), "posit<5,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<posit<5, 1>>(reportTestCases), "posit<5,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<posit<5, 2>>(reportTestCases), "posit<5,2>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<posit<6, 0>>(reportTestCases), "posit<6,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<posit<6, 1>>(reportTestCases), "posit<6,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<posit<6, 2>>(reportTestCases), "posit<6,2>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<posit<6, 3>>(reportTestCases), "posit<6,3>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<posit<7, 0>>(reportTestCases), "posit<7,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<posit<7, 1>>(reportTestCases), "posit<7,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<posit<7, 2>>(reportTestCases), "posit<7,2>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<posit<7, 3>>(reportTestCases), "posit<7,3>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<posit<8, 0>>(reportTestCases), "posit<8,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<posit<8, 1>>(reportTestCases), "posit<8,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<posit<8, 2>>(reportTestCases), "posit<8,2>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<posit<8, 3>>(reportTestCases), "posit<8,3>", ">=");
	if (!(p >= 0)) { 
		nrOfFailedTestCases += ReportTestResult(1, "posit<16,1> >= 0", ">= int literal");
	}
	else {
		ReportTestResult(0, "posit<16,1> >= 0", ">= int literal");
	}
	if (!(p >= 0.0f)) {
		nrOfFailedTestCases += ReportTestResult(1, "posit<16,1> >= 0.0", ">= float literal");
	}
	else {
		ReportTestResult(0, "posit<16,1> >= 0.0", ">= float literal");
	}
	if (!(p >= 0.0)) { 
		nrOfFailedTestCases += ReportTestResult(1, "posit<16,1> >= 0.0", ">= double literal");
	}
	else {
		ReportTestResult(0, "posit<16,1> >= 0.0", ">= double literal");
	}
	if (!(p >= 0.0l)) {
		nrOfFailedTestCases += ReportTestResult(1, "posit<16,1> >= 0.0", ">= long double literal");
	}
	else {
		ReportTestResult(0, "posit<16,1> >= 0.0", ">= long double literal");
	}
#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual<posit<16, 1>>(reportTestCases), "posit<16,1>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual<posit<16, 1>>(reportTestCases), "posit<16,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan<posit<16, 1>>(reportTestCases), "posit<16,1>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan<posit<16, 1>>(reportTestCases), "posit<16,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan<posit<16, 1>>(reportTestCases), "posit<16,1>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<posit<16, 1>>(reportTestCases), "posit<16,1>", ">=");
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
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
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



