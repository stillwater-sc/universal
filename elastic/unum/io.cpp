// io.cpp: IO, display, and parsing tests for unum Type I
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#define UNUM_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/unum/unum.hpp>
#include <universal/number/unum/manipulators.hpp>
#include <universal/verification/test_reporters.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "unum Type I IO and display tests";
	std::string test_tag    = "unum io";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using Unum = unum<3, 4>;

	/////////////////////////////////////////////////////////////////////////////////////
	// parse decimal strings
	std::cout << "*** parse()\n";
	{
		int start = nrOfFailedTestCases;
		Unum a;

		if (!parse("1.5", a)) ++nrOfFailedTestCases;
		if (a.to_double() != 1.5) { ++nrOfFailedTestCases; std::cout << "  FAIL: parse 1.5\n"; }

		if (!parse("-3.25", a)) ++nrOfFailedTestCases;
		if (a.to_double() != -3.25) { ++nrOfFailedTestCases; std::cout << "  FAIL: parse -3.25\n"; }

		if (!parse("1.25e3", a)) ++nrOfFailedTestCases;
		if (a.to_double() != 1250.0) { ++nrOfFailedTestCases; std::cout << "  FAIL: parse 1.25e3\n"; }

		// reject trailing junk
		if (parse("1.5abc", a)) { ++nrOfFailedTestCases; std::cout << "  FAIL: should reject trailing junk\n"; }

		// reject empty string
		if (parse("", a)) { ++nrOfFailedTestCases; std::cout << "  FAIL: should reject empty\n"; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: parse\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// operator>> via istringstream
	std::cout << "*** operator>>\n";
	{
		int start = nrOfFailedTestCases;
		Unum a;

		std::istringstream is("2.5");
		is >> a;
		if (a.to_double() != 2.5) { ++nrOfFailedTestCases; std::cout << "  FAIL: istream 2.5\n"; }

		std::istringstream is2("-0.125");
		is2 >> a;
		if (a.to_double() != -0.125) { ++nrOfFailedTestCases; std::cout << "  FAIL: istream -0.125\n"; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: operator>>\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// operator<< output
	std::cout << "*** operator<<\n";
	{
		int start = nrOfFailedTestCases;
		Unum a;

		a = 0.0;
		std::ostringstream os1;
		os1 << a;
		if (os1.str() != "0") { ++nrOfFailedTestCases; std::cout << "  FAIL: ostream 0 -> '" << os1.str() << "'\n"; }

		a.setnan();
		std::ostringstream os2;
		os2 << a;
		if (os2.str() != "NaN") { ++nrOfFailedTestCases; std::cout << "  FAIL: ostream NaN -> '" << os2.str() << "'\n"; }

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: operator<<\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// display functions
	std::cout << "*** display functions\n";
	{
		Unum a;
		a = 2.5;
		std::cout << "  to_binary:    " << to_binary(a) << '\n';
		std::cout << "  to_binary(m): " << to_binary(a, true) << '\n';
		std::cout << "  color_print:  " << color_print(a) << '\n';
		std::cout << "  pretty_print: " << pretty_print(a) << '\n';
		std::cout << "  info_print:   " << info_print(a) << '\n';
		std::cout << "  type_tag:     " << type_tag(a) << '\n';
		std::cout << "  components:   " << components(a) << '\n';
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// numeric_limits
	std::cout << "*** numeric_limits\n";
	{
		using Unum22 = unum<2, 2>;
		std::cout << "  unum<2,2> digits:       " << std::numeric_limits<Unum22>::digits << '\n';
		std::cout << "  unum<2,2> min_exponent:  " << std::numeric_limits<Unum22>::min_exponent << '\n';
		std::cout << "  unum<2,2> max_exponent:  " << std::numeric_limits<Unum22>::max_exponent << '\n';
		std::cout << "  unum<2,2> epsilon:       " << std::numeric_limits<Unum22>::epsilon() << '\n';
		std::cout << "  unum<2,2> min:           " << std::numeric_limits<Unum22>::min() << '\n';
		std::cout << "  unum<2,2> max:           " << std::numeric_limits<Unum22>::max() << '\n';
		std::cout << "  unum<2,2> quiet_NaN:     " << std::numeric_limits<Unum22>::quiet_NaN() << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::unum_arithmetic_exception& err) {
	std::cerr << "Uncaught unum arithmetic exception: " << err.what() << std::endl;
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
