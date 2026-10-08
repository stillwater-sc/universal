// assign.cpp: assigning a bfloat16 from its encoding written as a binary string
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// assign("0b<sign>.<exponent>.<fraction>") sets the encoding.  A well-formed string has 16
// bits in three fields of widths 1, 8 and 7; anything else must be rejected and leave the
// value at zero (#1647: a misplaced first '.' used to be accepted).
#include <universal/utility/directives.hpp>
#include <string>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;
	std::string test_suite  = "bfloat16 assign from a binary string";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;
	ReportTestSuiteHeader(test_suite, reportTestCases);

	{
		// every encoding round-trips through to_binary and assign
		int fails = 0;
		for (unsigned i = 0; i < 65536u; ++i) {
			bfloat16 a; a.setbits(i);
			bfloat16 b; b.assign(to_binary(a));
			if (b.bits() != a.bits()) { ++fails; if (reportTestCases && fails < 10) std::cerr << "FAIL round trip " << to_binary(a) << " -> " << to_binary(b) << '\n'; }
		}
		nrOfFailedTestCases += ReportTestResult(fails, "bfloat16", "round trip");
	}
	{
		int fails = 0;
		auto expect = [&](bool ok, const std::string& what) { if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << what << '\n'; } };
		bfloat16 one; one.assign("0b0.01111111.0000000");
		expect(float(one) == 1.0f, "0b0.01111111.0000000 assigns 1.0");
		bfloat16 two; two.assign("0b0.1000'0000.000'0000");
		expect(float(two) == 2.0f, "' digit separators are accepted");
		expect(to_binary(one) == "0b0.01111111.0000000", "to_binary of a positive value starts with 0b, not 0x");
		// all with 16 bits: the first '.' one bit late, the second '.' one bit early or late
		for (const char* bad : { "0b01.00000000.000000", "0b0.0111111.10000000", "0b0.011111110.000000",
		                         "0b0.01111111.000000", "0b0.01111111.00000000", "0b001111111.0000000", "0b0.01111111.000000x", "0.01111111.0000000" }) {
			bfloat16 b; b.assign(bad);
			expect(b.iszero(), std::string(bad) + " is rejected and leaves zero, got " + to_binary(b));
		}
		nrOfFailedTestCases += ReportTestResult(fails, "bfloat16", "field widths");
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
