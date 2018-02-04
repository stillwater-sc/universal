// arithmetic_add.cpp: functional tests for addition
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

// when you define POSIT_VERBOSE_OUTPUT executing an ADD the code will print intermediate results
//#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_ADD

// minimum set of include files to reflect source code dependencies
#include "../../bitset/bitset_helpers.hpp"
#include "../../posit/posit.hpp"
#include "../../posit/posit_manipulators.hpp"
#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"

using namespace std;
using namespace sw::unum;

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a, Ty b) {
	Ty reference;
	posit<nbits, es> pa, pb, psum;
	pa = a;
	pb = b;
	reference = a + b;
	psum = pa + pb;
	cout << "reference " << reference << " result " << psum << endl << endl;
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Addition failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateTestCase<6, 3, double>(INFINITY, INFINITY);
	GenerateTestCase<8, 4, float>(0.5f, -0.5f);

	{
		double da, db;
		posit<8, 4> pa, pb, psum;
		pa.set_raw_bits(uint64_t(0b00000001));
		pb.set_raw_bits(uint64_t(0b10000001));
		da = double(pa);
		db = double(pb);
		std::cout << "double values: " << setprecision(20) << da << " + " << db << " = " << da+db << std::endl;
		psum = pa + pb;
		std::cout << to_binary(pa.get()) << " + " << to_binary(pb.get()) << " = " << to_binary(psum.get()) << " value " << psum << std::endl;
		psum = da + db;
		std::cout << to_binary(pa.get()) << " + " << to_binary(pb.get()) << " = " << to_binary(psum.get()) << " value " << psum << std::endl;
		GenerateTestCase<8, 4>(da, db);
	}

	{
		// FAIL 1001111101101000 + 0010000000001010 != 1001111111101001 instead it yielded 1001111111101000
		posit<16, 1> pa, pb, psum, pref, podd;
		pa.set_raw_bits(0b1001111101101000);
		pb.set_raw_bits(0b0010000000001010);
		podd.set_raw_bits(0b1001111111101001);
		double da, db;
		da = (double)pa;
		db = double(pb);
		psum = pa + pb;
		pref = da + db;
		cout << pa << " + " << pb << " = " << psum << " vs " << pref << " guidance " << podd << endl;
		cout << pa.get() << " + " << pb.get() << " = " << psum.get() << " vs " << pref.get() << " guidance " << podd.get() << endl;
	}

	{
		// FAIL 0110010010111000 + 1001111100101011 != 0100111100011100 instead it yielded 0100111100011000
		posit<16, 1> pa, pb, psum, pref, podd;
		pa.set_raw_bits(0b0110010010111000);
		pb.set_raw_bits(0b1001111100101011);
		podd.set_raw_bits(0b0100111100011100);
		double da, db;
		da = (double)pa;
		db = double(pb);
		psum = pa + pb;
		pref = da + db;
		cout << pa << " + " << pb << " = " << psum << " vs " << pref << " guidance " << podd << endl;
		cout << pa.get() << " + " << pb.get() << " = " << psum.get() << " vs " << pref.get() << " guidance " << podd.get() << endl;
	}

	// manual exhaustive test
//	nrOfFailedTestCases += ReportTestResult(ValidateAddition<3, 0>("Manual Testing", true), "posit<3,0>", "addition");
//	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 4>("Manual Testing", true), "posit<8,4>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<16, 1>(tag, true, OPCODE_ADD, 1000), "posit<16,1>", "addition");
//	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 2>(tag, true, OPCODE_ADD, 1000), "posit<64,2>", "addition");

#else

	cout << "Posit addition validation" << endl;

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<4, 0>(tag, bReportIndividualTestCases), "posit<4,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<4, 1>(tag, bReportIndividualTestCases), "posit<4,1>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<5, 0>(tag, bReportIndividualTestCases), "posit<5,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<5, 1>(tag, bReportIndividualTestCases), "posit<5,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<5, 2>(tag, bReportIndividualTestCases), "posit<5,2>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<6, 0>(tag, bReportIndividualTestCases), "posit<6,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<6, 1>(tag, bReportIndividualTestCases), "posit<6,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<6, 2>(tag, bReportIndividualTestCases), "posit<6,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<6, 3>(tag, bReportIndividualTestCases), "posit<6,3>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<7, 0>(tag, bReportIndividualTestCases), "posit<7,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<7, 1>(tag, bReportIndividualTestCases), "posit<7,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<7, 2>(tag, bReportIndividualTestCases), "posit<7,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<7, 3>(tag, bReportIndividualTestCases), "posit<7,3>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<7, 4>(tag, bReportIndividualTestCases), "posit<7,4>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 4>(tag, bReportIndividualTestCases), "posit<8,4>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 5>(tag, bReportIndividualTestCases), "posit<8,5>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<16, 1>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "posit<16,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<24, 1>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "posit<24,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<32, 1>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "posit<32,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<32, 2>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "posit<32,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<48, 2>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "posit<48,2>", "addition");

#if STRESS_TESTING
	// nbits=64 requires long double compiler support
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 2>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "posit<64,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 3>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "posit<64,3>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 4>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "posit<64,4>", "addition");


	nrOfFailedTestCases += ReportTestResult(ValidateAddition<10, 1>(tag, bReportIndividualTestCases), "posit<10,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<12, 1>(tag, bReportIndividualTestCases), "posit<12,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<14, 1>(tag, bReportIndividualTestCases), "posit<14,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<16, 1>(tag, bReportIndividualTestCases), "posit<16,1>", "addition");
#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
catch (...) {
	cerr << "Caught unknown exception" << endl;
	return EXIT_FAILURE;
}
