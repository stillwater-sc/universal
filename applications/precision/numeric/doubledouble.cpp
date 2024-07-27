// doubledouble.cpp: experiments with double-double floating-point arithmetic
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#include <utility>
#if (__cplusplus == 202003L) || (_MSVC_LANG == 202003L)
#include <numbers>    // high-precision numbers
#endif
#include <universal/benchmark/performance_runner.hpp>
#include <universal/verification/test_suite.hpp>

// select the number systems we would like to compare
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>

/*
Definition of FAITHFUL arithmetic
   For a t-digit number a and b, and op element {+,-,*,/}, let c = a op b exactly.
   Suppose x and y are consecutive t-digit floating-point numbers with the same 
   sign as c such at |x| <= |c| < |y|. Then the floating-point arithmetic is
   called faithful if fl(a op b) = x whenever c = x and fl(a op b) is either x or y
   whenever c != x.

 */

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "experiment with double-double floating-point arithmetic";
	std::string test_tag    = "double-double";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	std::streamsize precision = std::cout.precision();
	
	{
		using LNS = lns<16, 10, std::uint16_t>;

		LNS a{}, b{}, c{};
		a = 0.5;
		b = 2.0;
		c = a * b;
		ReportBinaryOperation(a, "*", b, c);
	}

	std::cout << std::setprecision(precision);
	std::cout << std::endl;
	
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
