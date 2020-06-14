// function_classify.cpp: functional tests for classification functions of the Reals specialized for posits
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// minimum set of include files to reflect source code dependencies
#define POSIT_ENABLE_LITERALS 1
#include "universal/posit/posit.hpp"
#include "universal/posit/posit_manipulators.hpp"
#include "universal/posit/math/classify.hpp"
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"
#include "../utils/posit_math_helpers.hpp"

#define MANUAL_TESTING 1
#define STRESS_TESTING 0


int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	//bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "Addition failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

#define MY_DBL_MIN          2.2250738585072014e-308 // min positive value

	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	posit<nbits, es> pnar; pnar.setnar();
	posit<nbits, es> pinf; pinf.setnar();
	posit<nbits, es> pzero(0);
	posit<nbits, es> pminpos; minpos<nbits, es>(pminpos);
	posit<nbits, es> pdblmin(MY_DBL_MIN);
	posit<nbits, es> pone(1);

	std::cout << std::boolalpha
		<< "isnormal(NaN) = " << std::isnormal(NAN) << '\n'
		<< "isnormal(Inf) = " << std::isnormal(INFINITY) << '\n'
		<< "isnormal(0.0) = " << std::isnormal(0.0) << '\n'
		<< "isnormal(DBL_MIN/2.0) = " << std::isnormal(MY_DBL_MIN / 2.0) << '\n'
		<< "isnormal(1.0) = " << std::isnormal(1.0) << '\n';
	std::cout << std::boolalpha
		<< "isnormal(NaR) = " << isnormal(pnar) << '\n'
		<< "isnormal(Inf) = " << isnormal(pinf) << '\n'
		<< "isnormal(0.0) = " << isnormal(pzero) << '\n'
		<< "isnormal(DBL_MIN/2.0) = " << isnormal(pdblmin / 2.0) << '\n'
		<< "isnormal(1.0) = " << isnormal(pone) << '\n';

	std::cout << std::boolalpha
		<< "isfinite(NaN) = " << std::isfinite(NAN) << '\n'
		<< "isfinite(Inf) = " << std::isfinite(INFINITY) << '\n'
		<< "isfinite(0.0) = " << std::isfinite(0.0) << '\n'
		<< "isfinite(DBL_MIN/2.0) = " << std::isfinite(MY_DBL_MIN / 2.0) << '\n'
		<< "isfinite(1.0) = " << std::isfinite(1.0) << '\n';
	std::cout << std::boolalpha
		<< "isfinite(NaR) = " << isfinite(pnar) << '\n'
		<< "isfinite(Inf) = " << isfinite(pinf) << '\n'
		<< "isfinite(0.0) = " << isfinite(pzero) << '\n'
		<< "isfinite(DBL_MIN/2.0) = " << isfinite(pdblmin / 2.0) << '\n'
		<< "isfinite(1.0) = " << isfinite(pone) << '\n';

	std::cout << std::boolalpha
		<< "isinf(NaN) = " << std::isinf(NAN) << '\n'
		<< "isinf(Inf) = " << std::isinf(INFINITY) << '\n'
		<< "isinf(0.0) = " << std::isinf(0.0) << '\n'
		<< "isinf(DBL_MIN/2.0) = " << std::isinf(MY_DBL_MIN / 2.0) << '\n'
		<< "isinf(1.0) = " << std::isinf(1.0) << '\n';
	std::cout << std::boolalpha
		<< "isinf(NaR) = " << isinf(pnar) << '\n'
		<< "isinf(Inf) = " << isinf(pinf) << '\n'
		<< "isinf(0.0) = " << isinf(pzero) << '\n'
		<< "isinf(DBL_MIN/2.0) = " << isinf(pdblmin / 2.0) << '\n'
		<< "isinf(1.0) = " << isinf(pone) << '\n';

	std::cout << std::boolalpha
		<< "isnan(NaN) = " << std::isnan(NAN) << '\n'
		<< "isnan(Inf) = " << std::isnan(INFINITY) << '\n'
		<< "isnan(0.0) = " << std::isnan(0.0) << '\n'
		<< "isnan(DBL_MIN/2.0) = " << std::isnan(MY_DBL_MIN / 2.0) << '\n'
		<< "isnan(1.0) = " << std::isnan(1.0) << '\n';
	std::cout << std::boolalpha
		<< "isnan(NaR) = " << isnan(pnar) << '\n'
		<< "isnan(Inf) = " << isnan(pinf) << '\n'
		<< "isnan(0.0) = " << isnan(pzero) << '\n'
		<< "isnan(DBL_MIN/2.0) = " << isnan(pdblmin / 2.0) << '\n'
		<< "isnan(1.0) = " << isnan(pone) << '\n';

#else

	cout << "Posit classification function validation" << endl;


#if STRESS_TESTING
	
#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_internal_exception& err) {
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
