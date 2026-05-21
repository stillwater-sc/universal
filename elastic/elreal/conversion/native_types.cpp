// native_types.cpp: round-trip conversion tests for elreal native-type ctors (Phase B)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/utility/directives.hpp>

#define ELREAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>
#include <climits>

template<typename Int>
static int check_int_round_trip(const char* label, Int v) {
	sw::universal::elreal x(v);
	double back = double(x);
	if (back != static_cast<double>(v)) {
		std::cerr << "FAIL: " << label << " (" << v << ") round-trip != double(v): got " << back << "\n";
		return 1;
	}
	return 0;
}

static int check_double_round_trip(const char* label, double v) {
	sw::universal::elreal x(v);
	double back = double(x);
	if (back != v) {
		// Treat NaN match as success
		if (std::isnan(v) && std::isnan(back)) return 0;
		std::cerr << "FAIL: " << label << " (" << v << ") round-trip != input: got " << back << "\n";
		return 1;
	}
	return 0;
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "elreal Phase B native-type conversion";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// Standard floating-point round-trip
	nrOfFailedTestCases += check_double_round_trip("0.0",             0.0);
	nrOfFailedTestCases += check_double_round_trip("1.0",             1.0);
	nrOfFailedTestCases += check_double_round_trip("-2.5",           -2.5);
	nrOfFailedTestCases += check_double_round_trip("pi",              3.141592653589793);
	nrOfFailedTestCases += check_double_round_trip("e",               2.718281828459045);
	nrOfFailedTestCases += check_double_round_trip("1e100",           1e100);
	nrOfFailedTestCases += check_double_round_trip("1e-300",          1e-300);

	// Subnormal
	{
		double sub = std::numeric_limits<double>::denorm_min();
		nrOfFailedTestCases += check_double_round_trip("denorm_min",  sub);
	}
	// Signed zero (should remain bitwise -0.0 through the round-trip).
	{
		double nz = -0.0;
		elreal x(nz);
		if (!std::signbit(double(x))) {
			std::cerr << "FAIL: signed zero -0.0 lost its sign through elreal round-trip\n";
			++nrOfFailedTestCases;
		}
	}
	// Infinities
	{
		double pinf = std::numeric_limits<double>::infinity();
		double ninf = -std::numeric_limits<double>::infinity();
		nrOfFailedTestCases += check_double_round_trip("+inf",        pinf);
		nrOfFailedTestCases += check_double_round_trip("-inf",        ninf);
	}
	// NaN (compare via std::isnan, not bit-for-bit)
	{
		double n = std::numeric_limits<double>::quiet_NaN();
		elreal x(n);
		if (!std::isnan(double(x))) {
			std::cerr << "FAIL: NaN did not survive elreal round-trip\n";
			++nrOfFailedTestCases;
		}
	}

	// Integer round-trip (values fit in double exactly)
	nrOfFailedTestCases += check_int_round_trip("int 0",              0);
	nrOfFailedTestCases += check_int_round_trip("int -1",            -1);
	nrOfFailedTestCases += check_int_round_trip("int 42",             42);
	nrOfFailedTestCases += check_int_round_trip("int INT_MAX",        INT_MAX);
	nrOfFailedTestCases += check_int_round_trip("int INT_MIN",        INT_MIN);
	nrOfFailedTestCases += check_int_round_trip("long long 1<<40",    static_cast<long long>(1) << 40);
	nrOfFailedTestCases += check_int_round_trip("unsigned int 1<<31", 1u << 31);

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
