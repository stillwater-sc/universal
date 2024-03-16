#pragma once
//  test_status.cpp : functions for test status reporting
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>

namespace sw { namespace universal {

	// test reporting helper
	// takes an int reporting the number of test failures and prints a PASS/FAIL designation
	int ReportTestResult(int nrOfFailedTests, const std::string& description, const std::string& test_operation) {
		constexpr int TEST_TAG_WIDTH = 60;
		if (nrOfFailedTests > 0) {
			std::cerr << std::left << std::setw(TEST_TAG_WIDTH) << description << " " << test_operation << " FAIL " << nrOfFailedTests << " failed test cases\n";
		}
		else {
			std::cerr << std::left << std::setw(TEST_TAG_WIDTH) << description << " " << test_operation << " PASS\n";
		}
		return nrOfFailedTests;
	}

	// simple checker
	int ReportCheck(const std::string& tag, const std::string& test, bool success) {
		constexpr int TEST_TAG_WIDTH = 30;
		int nrOfFailedTestCases = 0;
		if (success) {
			std::cerr << tag << " " << std::left << std::setw(TEST_TAG_WIDTH) << test << " PASS\n";
		}
		else {
			++nrOfFailedTestCases;
			std::cerr << tag << " " << std::left << std::setw(TEST_TAG_WIDTH) << test << " FAIL\n";
		}
		return nrOfFailedTestCases;
	}

}} // namespace sw::universal
