// compare.cpp: tests for elreal comparison operators (Phase D)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/utility/directives.hpp>

#define ELREAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "elreal Phase D comparison";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// --- compare() ------------------------------------------------------
	{
		elreal a(1.0), b(2.0);
		if (compare(a, b) >= 0) { std::cerr << "FAIL: compare(1, 2) >= 0\n"; ++nrOfFailedTestCases; }
		if (compare(b, a) <= 0) { std::cerr << "FAIL: compare(2, 1) <= 0\n"; ++nrOfFailedTestCases; }
		if (compare(a, a) != 0) { std::cerr << "FAIL: compare(a, a) != 0\n"; ++nrOfFailedTestCases; }
	}

	// --- All operators on a < b ----------------------------------------
	{
		elreal a(1.0), b(2.0);
		if (!(a <  b)) { std::cerr << "FAIL: 1 < 2 returned false\n"; ++nrOfFailedTestCases; }
		if (!(a <= b)) { std::cerr << "FAIL: 1 <= 2 returned false\n"; ++nrOfFailedTestCases; }
		if (!(b >  a)) { std::cerr << "FAIL: 2 > 1 returned false\n"; ++nrOfFailedTestCases; }
		if (!(b >= a)) { std::cerr << "FAIL: 2 >= 1 returned false\n"; ++nrOfFailedTestCases; }
		if ( (a == b)) { std::cerr << "FAIL: 1 == 2 returned true\n"; ++nrOfFailedTestCases; }
		if (!(a != b)) { std::cerr << "FAIL: 1 != 2 returned false\n"; ++nrOfFailedTestCases; }
	}

	// --- a == a (same object), a == b (equal doubles) -------------------
	{
		elreal a(3.14);
		if (!(a == a)) { std::cerr << "FAIL: a == a (same object) returned false\n"; ++nrOfFailedTestCases; }
		if ( (a != a)) { std::cerr << "FAIL: a != a (same object) returned true\n"; ++nrOfFailedTestCases; }

		elreal b(3.14);
		if (!(a == b)) { std::cerr << "FAIL: 3.14 == 3.14 (different objects) returned false\n"; ++nrOfFailedTestCases; }
	}

	// --- Lazy-real distinctness: rational vs rounded double -------------
	// This is the test that demonstrates the *point* of lazy arithmetic.
	// elreal(1, 3) represents 1/3 exactly (Phase B's promise); elreal(1.0/3.0)
	// represents the IEEE-754 round of 1/3, which is slightly less than 1/3.
	// At budget >= 2 (i.e. budget passes the depth-1 correction), the
	// comparison must distinguish them.
	{
		elreal rat(1LL, 3LL);
		elreal rnd(1.0/3.0);
		// at(0) is the same; only at(1) differs.
		if (rat.at(0) != rnd.at(0)) {
			std::cerr << "FAIL: rat.at(0) != rnd.at(0) (test setup wrong)\n";
			++nrOfFailedTestCases;
		}
		// The compare operator with default budget walks past depth 0
		// because the depth-0 difference is exactly zero (a - b at depth 0
		// is a0 - b0 = 0). It then materialises depth 1 of (rat - rnd),
		// which is the rational residual minus 0 = positive.
		if (!(rat > rnd)) {
			std::cerr << "FAIL: elreal(1, 3) > elreal(1.0/3.0) returned false "
				<< "(refinement past depth 0 not finding the rational residual)\n";
			++nrOfFailedTestCases;
		}
		if (rat == rnd) {
			std::cerr << "FAIL: elreal(1, 3) == elreal(1.0/3.0) returned true "
				<< "(lazy-real distinctness lost)\n";
			++nrOfFailedTestCases;
		}
	}

	// --- IEEE-754 NaN semantics: all ordering returns false; != is true -
	{
		elreal nan(SpecificValue::qnan), x(1.0);
		if (nan <  x)  { std::cerr << "FAIL: NaN <  x returned true\n";  ++nrOfFailedTestCases; }
		if (nan <= x)  { std::cerr << "FAIL: NaN <= x returned true\n";  ++nrOfFailedTestCases; }
		if (nan >  x)  { std::cerr << "FAIL: NaN >  x returned true\n";  ++nrOfFailedTestCases; }
		if (nan >= x)  { std::cerr << "FAIL: NaN >= x returned true\n";  ++nrOfFailedTestCases; }
		if (nan == x)  { std::cerr << "FAIL: NaN == x returned true\n";  ++nrOfFailedTestCases; }
		if (!(nan != x)) { std::cerr << "FAIL: NaN != x returned false (IEEE: must be true)\n"; ++nrOfFailedTestCases; }

		// NaN compared with itself
		if (nan == nan) { std::cerr << "FAIL: NaN == NaN returned true\n"; ++nrOfFailedTestCases; }
		if (!(nan != nan)) { std::cerr << "FAIL: NaN != NaN returned false (IEEE: must be true)\n"; ++nrOfFailedTestCases; }
	}

	// --- inf comparisons ------------------------------------------------
	{
		elreal pinf(SpecificValue::infpos), ninf(SpecificValue::infneg);
		elreal x(1.0);
		if (!(ninf < x))   { std::cerr << "FAIL: -inf < 1 returned false\n"; ++nrOfFailedTestCases; }
		if (!(x    < pinf)){ std::cerr << "FAIL: 1 < +inf returned false\n"; ++nrOfFailedTestCases; }
		if (!(ninf < pinf)){ std::cerr << "FAIL: -inf < +inf returned false\n"; ++nrOfFailedTestCases; }
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::exception& err) {
	std::cerr << "Caught exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
