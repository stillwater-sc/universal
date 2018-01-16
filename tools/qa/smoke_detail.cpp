// smoke_details.cpp: take a testcase and show processing details
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_ALL

#include <posit>
#include "../tests/posit_test_helpers.hpp"
#include "qa_helpers.hpp"

using namespace std;

/*
smoke tests focus on the boundary cases of posit arithmetic.

When a smoke test fails, this program can read that test case and reissue the command
and show all the intermediate pipeline processing details.
 */

int main(int argc, char** argv)
try {
	cout << "Generating smoke test details" << endl;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	if (argc == 1) {
		cout << "Usage: smoke_detail test-file-name" << endl;
		return EXIT_SUCCESS;
	}
	std::string filename;
	if (argc == 2) {
		filename = argv[1];
	}
	cout << "Test file: " << filename << endl;

#ifdef WIN32
	char full_path[1024];
	_getcwd(full_path, 1024);
	cout << "CWD: " << full_path << endl;
#endif

	ifstream testFile;
	testFile.open(filename);
	if (testFile.is_open()) {
		string config;
		testFile >> config;
		cout << config << endl;
		int testCase = 0;
		while (!testFile.eof()) {
			string op1, op, op2, eq, ref, refhex;
			testFile >> op1 >> op >> op2 >> eq >> ref >> refhex;
			cout << endl;
			cout << "------------------------------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;
			cout << "Test case [" << testCase++ << "] : " << op1 << " " << op << " " << op2 << " = " << ref << endl;
			
			
			size_t nbits = op1.length();
			if (nbits != 32 && nbits != op2.length() && nbits != ref.length()) {
				cerr << "operand lengths are not compatible";
				return EXIT_FAILURE;
			}
			// for the moment only posit<32,2>
			size_t Idx = 0;
			sw::unum::posit<32, 2> pa, pb, pref;
			uint64_t va = stoull(op1, &Idx, 2);
			uint64_t vb = stoull(op2, &Idx, 2);
			uint64_t vref = stoull(ref, &Idx, 2);
			cout << endl;

			pa.set_raw_bits(va);
			pb.set_raw_bits(vb);
			pref.set_raw_bits(vref);
			cout << pa << " " << op << " " << pb << " = " << pref << endl;

			sw::unum::posit<32, 2> presult;
			if (op == "+") {
				presult = pa + pb;
			} 
			else if (op == "-") {
				presult = pa - pb;
			}
			else if (op == "*") {
				presult = pa * pb;
			}
			else if (op == "/") {
				presult = pa / pb;
			}
			cout << sw::unum::components_to_string(presult) << endl;
			cout << "------------------------------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;
			cout << endl;
		}
		testFile.close();
	}
	
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
