// api.cpp: functional tests of the value type API
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include "universal/bitblock/bitblock.hpp"
#include "universal/value/value.hpp"
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"

template<size_t fbits>
int Check(const sw::unum::value<fbits>& v, double ref, bool bReportIndividualTestCases) {
	int fails = 0;
	if (v.to_double() != ref) {
		++fails;
		if (bReportIndividualTestCases) {
			std::cout << v << " != " << ref << '\n';
		}
	}
	return fails;
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	// API tests for value class
	cout << endl << "value API tests" << endl;
	cout << (bReportIndividualTestCases ? " " : "not ") << "reporting individual testcases" << endl;

#if MANUAL_TESTING

	// assignment
	{
		constexpr double reference = 8;
		signed char        sc  = (signed char)reference;
		short              ss  = (short)reference;
		int                si  = (int)reference;
		long               sl  = (long)reference;
		long long          sll = (long long)reference;
		char               uc  = (char)reference;
		unsigned short     us  = (unsigned short)reference;
		unsigned int       ui  = (unsigned int)reference;
		unsigned long      ul  = (unsigned long)reference;
		unsigned long long ull = (unsigned long long)reference;
		float              f   = (float)reference;
		double             d   = (double)reference;
		long double        ld  = (long double)reference;

		value<11> v;
		v = sc;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
		v = ss;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
		v = si;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
		v = sl;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
		v = sll;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
		v = uc;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
		v = us;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
		v = ui;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
		v = ul;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
		v = ull;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
		v = f;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
		v = d;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
		v = ld;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
	}

	{
		float f = 1.23456789;
		auto components = ieee_components(f);
		cout << std::get<0>(components) << ", " << std::get<1>(components) << ", " << std::get<2>(components) << endl;
	}

	{
		double d = 1.23456789;
		auto components = ieee_components(d);
		cout << std::get<0>(components) << ", " << std::get<1>(components) << ", " << std::get<2>(components) << endl;
	}

#else

	cout << "TBD" << endl;

#endif // MANUAL_TESTING

	if (nrOfFailedTestCases > 0) cout << "FAIL"; else cout << "PASS";

	cout.flush();
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
