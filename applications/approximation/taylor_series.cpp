//  taylor_series.cpp: experiments with number systems approximating the Reals approximating functions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/verification/test_suite.hpp>
#include <universal/number/cfloat/cfloat.hpp>

/*
 *  From Wikipedia:
 * 
 * In mathematics, the Taylor series of a function is an infinite sum of terms 
 * that are expressed in terms of the function's derivatives at a single point. 
 * For most common functions, the function and the sum of its Taylor series are 
 * equal near this point. Taylor's series are named after Brook Taylor, who 
 * introduced them in 1715.  
 * 
 * If 0 is the point where the derivatives are considered, a Taylor series is also 
 * called a Maclaurin series, after Colin Maclaurin, who made extensive use of 
 * this special case of Taylor series in the 18th century.  The partial sum 
 * formed by the first n + 1 terms of a Taylor series is a polynomial of degree n 
 * that is called the nth Taylor polynomial of the function. 
 * 
 * Taylor polynomials are approximations of a function, which become generally 
 * better as n increases. Taylor's theorem gives quantitative estimates on the 
 * error introduced by the use of such approximations. If the Taylor series 
 * of a function is convergent, its sum is the limit of the infinite sequence 
 * of the Taylor polynomials. A function may differ from the sum of its Taylor 
 * series, even if its Taylor series is convergent. A function is analytic at 
 * a point x if it is equal to the sum of its Taylor series in some open interval 
 * (or open disk in the complex plane) containing x. This implies that the 
 * function is analytic at every point of the interval (or disk).
 * 
 */

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	std::string test_suite = "Experiments with Taylor Series Expansion";
	std::string test_tag = "operator";
	std::cout << test_suite << '\n';
	int nrOfFailedTestCases = 0;

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
