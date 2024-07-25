// construct.cpp: functional tests to construct arbitrary configuration unums
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//  SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <universal/number/unum/unum.hpp>

template<typename Real>
void ReportNumberTraits(std::ostream& ostr) {
	using namespace sw::universal;
	ostr << "Real type          : " << typeid(Real).name() << '\n';
	ostr << "minimum exponent   : " << std::numeric_limits<Real>::min_exponent << '\n';
	ostr << "maximum exponent   : " << std::numeric_limits<Real>::max_exponent << '\n';
	ostr << "radix              : " << std::numeric_limits<Real>::radix << '\n';
	ostr << "radix digits       : " << std::numeric_limits<Real>::digits << '\n';
	ostr << "minimum value      : " << std::numeric_limits<Real>::min() << '\n';
	ostr << "maximum value      : " << std::numeric_limits<Real>::max() << '\n';
	ostr << "epsilon value      : " << std::numeric_limits<Real>::epsilon() << '\n';
	ostr << "max rounding error : " << std::numeric_limits<Real>::round_error() << '\n';
	ostr << "infinite           : " << std::numeric_limits<Real>::infinity() << '\n';
	ostr << "quiet NaN          : " << std::numeric_limits<Real>::quiet_NaN() << '\n';
	ostr << "signalling NaN     : " << std::numeric_limits<Real>::signaling_NaN() << "\n\n";
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main(int argc, char* argv[]) 
try {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	std::cout << "Constructing flexible configuration unums\n";

	unum<2, 2> u2_2;
	std::cout << typeid(unum<2,2>).name() << " : " << u2_2 << '\n';

	ReportNumberTraits<unum<2, 2>>(std::cout);
	ReportNumberTraits<unum<2, 3>>(std::cout);
	ReportNumberTraits<unum<2, 4>>(std::cout);
	ReportNumberTraits<unum<3, 2>>(std::cout);
	ReportNumberTraits<unum<3, 3>>(std::cout);
	ReportNumberTraits<unum<3, 4>>(std::cout);

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
