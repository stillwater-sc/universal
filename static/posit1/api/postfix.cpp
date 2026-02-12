// postfix.cpp test suite runner for postfix operators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/posit_test_suite_mathlib.hpp>

int main()
try {
	using namespace sw::universal;

	bool reportTestCases = false;
	int nrOfFailedTestCases = 0;

	nrOfFailedTestCases += ReportTestResult(VerifyPostfix<posit<3, 0>>(reportTestCases), "posit<3,0>", "posit++");

	nrOfFailedTestCases += ReportTestResult(VerifyPostfix<posit<4, 0>>(reportTestCases), "posit<4,0>", "posit++");
	nrOfFailedTestCases += ReportTestResult(VerifyPostfix<posit<4, 1>>(reportTestCases), "posit<4,1>", "posit++");

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
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

#ifdef TEACHING_MOMENT
// just because you can, doesn't mean you should
void DoNotDoStuffLikeThis() {
	using namespace sw::universal;

	// order of function evaluation is not defined, so there is no
	// mechanism for these methods to evaluate left to right
	// integer example
	// DON'T
	int i = 0;
	cout << i << " " << --i << " " << i << " " << i++ << " " << i << endl;
	cout << i << " " << i++ << " " << i << endl;
	i = 0;
	cout << --i << " " << --i << " " << --i << endl;
	i = 0;
	cout << --(--(--i)) << endl;
	i = 0;
	cout << ------i << " " << endl;

	// equivalent posit example
	const size_t nbits = 4;
	const size_t es = 0;
	posit<nbits, es> result, p = 0.0f;
	cout << p << " " << --p << " " << p << " " << p++ << " " << p << endl;
	cout << p << " " << p++ << " " << p << endl;
	p = 0.0f;
	cout << --p << " " << --p << " " << --p << endl;
	p = 0.0f;
	cout << --(--(--p)) << endl;


	p = 0.0f;
	result = --p++;
	cout << "result " << result << endl;

	int nrOfFailedTestCases = 0;
	p = 0.0f;
	if (!p.iszero()) {
		cout << "FAIL 1 " << p << endl; nrOfFailedTestCases++;
	}
	p = 0.0f; --(--(--(p++)++)++);
	if (!p.iszero()) {
		cout << "FAIL 2 " << p << endl; nrOfFailedTestCases++;
	}
	p = 0.0f; ++(++(++(p--)--)--);
	if (!p.iszero()) {
		cout << "FAIL 3 " << p << endl; nrOfFailedTestCases++;
	}
	p = 0.0f; ----------p++++++++++;
	if (!p.iszero()) {
		cout << "FAIL 4 " << p << endl; nrOfFailedTestCases++;
	}
	p = 0.0f; p++++++++++;
	if (p != posit<nbits, es>(1.0f)) {
		cout << "FAIL 5 " << p << endl; nrOfFailedTestCases++;
	}
}
#endif
