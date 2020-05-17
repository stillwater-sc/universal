//  test_helpers.cpp : functions to aid in testing and test reporting
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


// test reporting helper
int ReportTestResult(int nrOfFailedTests, const std::string& description, const std::string& test_operation)
{
	if (nrOfFailedTests > 0) {
		std::cout << description << " " << test_operation << " FAIL " << nrOfFailedTests << " failed test cases" << std::endl;
	}
	else {
		std::cout << description << " " << test_operation << " PASS" << std::endl;
	}
	return nrOfFailedTests;
}

