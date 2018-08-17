// logic.cpp : tests for logic operators between posits
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include "common.hpp"
// minimum set of include files to reflect source code dependencies
// enable literals to simplify the testing codes
#define POSIT_ENABLE_LITERALS 1
#include "../../posit/posit.hpp"
#include "../../posit/posit_manipulators.hpp"
#include "../test_helpers.hpp"
#include "../posit_test_helpers.hpp"

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::unum;

	int nrOfFailedTestCases = 0;

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
		nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<3, 0>(), "posit<3,0>", "==");
		nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<3, 0>(), "posit<3,0>", "!=");
		nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<3, 0>(), "posit<3,0>", "<");
		nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<3, 0>(), "posit<3,0>", ">");
		nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<3, 0>(), "posit<3,0>", "<=");
		nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<3, 0>(), "posit<3,0>", ">=");
	}


#else
	posit<16, 1> p;

	cout << "Logic: operator==()" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<3, 0>(), "posit<3,0>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<4, 0>(), "posit<4,0>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<4, 1>(), "posit<4,1>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<5, 0>(), "posit<5,0>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<5, 1>(), "posit<5,1>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<5, 2>(), "posit<5,2>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<6, 0>(), "posit<6,0>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<6, 1>(), "posit<6,1>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<6, 2>(), "posit<6,2>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<6, 3>(), "posit<6,3>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<7, 0>(), "posit<7,0>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<7, 1>(), "posit<7,1>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<7, 2>(), "posit<7,2>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<7, 3>(), "posit<7,3>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<7, 4>(), "posit<7,4>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<8, 0>(), "posit<8,0>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<8, 1>(), "posit<8,1>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<8, 2>(), "posit<8,2>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<8, 3>(), "posit<8,3>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<8, 4>(), "posit<8,4>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<8, 5>(), "posit<8,5>", "==");
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
	
	cout << "Logic: operator!=()" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<3, 0>(), "posit<3,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<4, 0>(), "posit<4,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<4, 1>(), "posit<4,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<5, 0>(), "posit<5,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<5, 1>(), "posit<5,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<5, 2>(), "posit<5,2>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<6, 0>(), "posit<6,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<6, 1>(), "posit<6,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<6, 2>(), "posit<6,2>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<6, 3>(), "posit<6,3>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<7, 0>(), "posit<7,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<7, 1>(), "posit<7,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<7, 2>(), "posit<7,2>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<7, 3>(), "posit<7,3>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<8, 0>(), "posit<8,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<8, 1>(), "posit<8,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<8, 2>(), "posit<8,2>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<8, 3>(), "posit<8,3>", "!=");
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

	std::cout << "Logic: operator<()" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<3, 0>(), "posit<3,0>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<4, 0>(), "posit<4,0>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<4, 1>(), "posit<4,1>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<5, 0>(), "posit<5,0>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<5, 1>(), "posit<5,1>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<5, 2>(), "posit<5,2>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<6, 0>(), "posit<6,0>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<6, 1>(), "posit<6,1>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<6, 2>(), "posit<6,2>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<6, 3>(), "posit<6,3>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<7, 0>(), "posit<7,0>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<7, 1>(), "posit<7,1>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<7, 2>(), "posit<7,2>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<7, 3>(), "posit<7,3>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<8, 0>(), "posit<8,0>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<8, 1>(), "posit<8,1>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<8, 2>(), "posit<8,2>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<8, 3>(), "posit<8,3>", "<");
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

	std::cout << "Logic: operator<=()" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<3, 0>(), "posit<3,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<4, 0>(), "posit<4,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<4, 1>(), "posit<4,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<5, 0>(), "posit<5,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<5, 1>(), "posit<5,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<5, 2>(), "posit<5,2>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<6, 0>(), "posit<6,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<6, 1>(), "posit<6,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<6, 2>(), "posit<6,2>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<6, 3>(), "posit<6,3>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<7, 0>(), "posit<7,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<7, 1>(), "posit<7,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<7, 2>(), "posit<7,2>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<7, 3>(), "posit<7,3>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<8, 0>(), "posit<8,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<8, 1>(), "posit<8,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<8, 2>(), "posit<8,2>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<8, 3>(), "posit<8,3>", "<=");
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

	std::cout << "Logic: operator>()" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<3, 0>(), "posit<3,0>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<4, 0>(), "posit<4,0>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<4, 1>(), "posit<4,1>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<5, 0>(), "posit<5,0>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<5, 1>(), "posit<5,1>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<5, 2>(), "posit<5,2>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<6, 0>(), "posit<6,0>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<6, 1>(), "posit<6,1>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<6, 2>(), "posit<6,2>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<6, 3>(), "posit<6,3>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<7, 0>(), "posit<7,0>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<7, 1>(), "posit<7,1>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<7, 2>(), "posit<7,2>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<7, 3>(), "posit<7,3>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<8, 0>(), "posit<8,0>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<8, 1>(), "posit<8,1>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<8, 2>(), "posit<8,2>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<8, 3>(), "posit<8,3>", ">");
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

	std::cout << "Logic: operator>=()" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<3, 0>(), "posit<3,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<4, 0>(), "posit<4,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<4, 1>(), "posit<4,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<5, 0>(), "posit<5,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<5, 1>(), "posit<5,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<5, 2>(), "posit<5,2>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<6, 0>(), "posit<6,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<6, 1>(), "posit<6,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<6, 2>(), "posit<6,2>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<6, 3>(), "posit<6,3>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<7, 0>(), "posit<7,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<7, 1>(), "posit<7,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<7, 2>(), "posit<7,2>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<7, 3>(), "posit<7,3>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<8, 0>(), "posit<8,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<8, 1>(), "posit<8,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<8, 2>(), "posit<8,2>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<8, 3>(), "posit<8,3>", ">=");
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

#if STRESS_TESTING

	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<16, 1>(), "posit<16,1>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<16, 1>(), "posit<16,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<16, 1>(), "posit<16,1>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<16, 1>(), "posit<16,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<16, 1>(), "posit<16,1>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<16, 1>(), "posit<16,1>", ">=");

#endif // STRESS_TESTING

#endif // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_internal_exception& err) {
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



