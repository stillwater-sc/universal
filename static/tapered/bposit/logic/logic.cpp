// logic.cpp: comparison operators of the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Comparison is integer comparison of the two's-complement patterns.  For every pair of
// encodings this must agree with comparing the values, with the posit standard's rule for
// NaR: equal only to itself, and below every real.
#include <universal/utility/directives.hpp>
#include <universal/number/bposit/bposit.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

template<unsigned nbits, unsigned rs, unsigned es>
int VerifyComparisons(bool report) {
	using B = sw::universal::bposit<nbits, rs, es, std::uint16_t>;
	int fails = 0;
	const unsigned N = 1u << nbits;
	for (unsigned i = 0; i < N; ++i) {
		B a;
		a.setbits(i);
		for (unsigned j = 0; j < N; ++j) {
			B b;
			b.setbits(j);
			// the reference order: NaR below every real, equal to itself
			const bool an = a.isnar(), bn = b.isnar();
			const double da = an ? 0.0 : double(a), db = bn ? 0.0 : double(b);
			const bool eq = (an && bn) || (!an && !bn && da == db);
			const bool lt = (an && !bn) || (!an && !bn && da < db);
			const bool ok = (a == b) == eq && (a != b) == !eq && (a < b) == lt && (a > b) == (!eq && !lt)
			             && (a <= b) == (eq || lt) && (a >= b) == !lt;
			if (!ok) {
				++fails;
				if (report && fails < 10) std::cerr << "FAIL " << sw::universal::type_tag(a) << " comparison of patterns " << i << ", " << j << '\n';
			}
		}
	}
	return fails;
}

}  // anonymous namespace

int main()
try {
	using namespace sw::universal;
	std::string test_suite  = "bposit logic";
	std::string test_tag    = "comparison";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;
	ReportTestSuiteHeader(test_suite, reportTestCases);

	nrOfFailedTestCases += ReportTestResult(VerifyComparisons<8, 4, 0>(reportTestCases),  "bposit<8,4,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComparisons<8, 3, 1>(reportTestCases),  "bposit<8,3,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComparisons<10, 3, 2>(reportTestCases), "bposit<10,3,2>", test_tag);

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
