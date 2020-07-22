#pragma once
//  test_helpers.cpp : functions to aid in testing and test reporting
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>

// test reporting helper
int ReportTestResult(int nrOfFailedTests, const std::string& description, const std::string& test_operation) {
	using namespace std;
	if (nrOfFailedTests > 0) {
		cout << description << " " << test_operation << " FAIL " << nrOfFailedTests << " failed test cases\n";
	}
	else {
		cout << description << " " << test_operation << " PASS\n";
	}
	return nrOfFailedTests;
}

// simple checker
int ReportCheck(const std::string& tag, const std::string& test, bool success) {
	using namespace std;
	constexpr int TEST_TAG_WIDTH = 26;
	int nrOfFailedTestCases = 0;
	if (success) {
		cout << tag << " " << left << setw(TEST_TAG_WIDTH) << test << "PASS\n";
	}
	else {
		++nrOfFailedTestCases;
		cout << tag << " " << left << setw(TEST_TAG_WIDTH) << test << "FAIL\n";
	}
	return nrOfFailedTestCases;
}
