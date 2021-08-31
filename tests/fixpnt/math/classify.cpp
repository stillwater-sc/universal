// classify.cpp: test suite runner for classification functions specialized for classic floats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// use default number system configuration
#include <universal/number/fixpnt/fixpnt.hpp>
//#include <universal/verification/fixpnt_math_test_suite.hpp>

#define MANUAL_TESTING 1
#define STRESS_TESTING 0


int main()
try {
	using namespace sw::universal;

	//bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "classify failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

#define MY_DBL_MIN          2.2250738585072014e-308 // minpos value

	constexpr size_t nbits = 32;
	constexpr size_t rbits = 8;
	constexpr bool arithmetic = Modulo;
	using bt = uint32_t;
	using Number = fixpnt<nbits, rbits, arithmetic, bt>;

	Number nan; // nan.setnan();
	Number inf; // inf.setinf();
	Number zero(0);
	Number minpos(SpecificValue::minpos);
	Number dblmin(MY_DBL_MIN);
	Number one(1);

	std::cout << std::boolalpha
		<< "isnormal(NaN) = " << std::isnormal(NAN) << '\n'
		<< "isnormal(Inf) = " << std::isnormal(INFINITY) << '\n'
		<< "isnormal(0.0) = " << std::isnormal(0.0) << '\n'
		<< "isnormal(DBL_MIN/2.0) = " << std::isnormal(MY_DBL_MIN / 2.0) << '\n'
		<< "isnormal(1.0) = " << std::isnormal(1.0) << '\n';
	std::cout << std::boolalpha
		<< "isnormal(NaR) = " << isnormal(nan) << '\n'
		<< "isnormal(Inf) = " << isnormal(inf) << '\n'
		<< "isnormal(0.0) = " << isnormal(zero) << '\n'
//		<< "isnormal(DBL_MIN/2.0) = " << isnormal(dblmin / 2.0) << '\n'
		<< "isnormal(1.0) = " << isnormal(one) << '\n';

	std::cout << std::boolalpha
		<< "isfinite(NaN) = " << std::isfinite(NAN) << '\n'
		<< "isfinite(Inf) = " << std::isfinite(INFINITY) << '\n'
		<< "isfinite(0.0) = " << std::isfinite(0.0) << '\n'
		<< "isfinite(DBL_MIN/2.0) = " << std::isfinite(MY_DBL_MIN / 2.0) << '\n'
		<< "isfinite(1.0) = " << std::isfinite(1.0) << '\n';
	std::cout << std::boolalpha
		<< "isfinite(NaR) = " << isfinite(nan) << '\n'
		<< "isfinite(Inf) = " << isfinite(inf) << '\n'
		<< "isfinite(0.0) = " << isfinite(zero) << '\n'
//		<< "isfinite(DBL_MIN/2.0) = " << isfinite(dblmin / 2.0) << '\n'
		<< "isfinite(1.0) = " << isfinite(one) << '\n';

	std::cout << std::boolalpha
		<< "isinf(NaN) = " << std::isinf(NAN) << '\n'
		<< "isinf(Inf) = " << std::isinf(INFINITY) << '\n'
		<< "isinf(0.0) = " << std::isinf(0.0) << '\n'
		<< "isinf(DBL_MIN/2.0) = " << std::isinf(MY_DBL_MIN / 2.0) << '\n'
		<< "isinf(1.0) = " << std::isinf(1.0) << '\n';
	std::cout << std::boolalpha
		<< "isinf(NaR) = " << isinf(nan) << '\n'
		<< "isinf(Inf) = " << isinf(inf) << '\n'
		<< "isinf(0.0) = " << isinf(zero) << '\n'
//		<< "isinf(DBL_MIN/2.0) = " << isinf(dblmin / 2.0) << '\n'
		<< "isinf(1.0) = " << isinf(one) << '\n';

	std::cout << std::boolalpha
		<< "isnan(NaN) = " << std::isnan(NAN) << '\n'
		<< "isnan(Inf) = " << std::isnan(INFINITY) << '\n'
		<< "isnan(0.0) = " << std::isnan(0.0) << '\n'
		<< "isnan(DBL_MIN/2.0) = " << std::isnan(MY_DBL_MIN / 2.0) << '\n'
		<< "isnan(1.0) = " << std::isnan(1.0) << '\n';
	std::cout << std::boolalpha
		<< "isnan(NaR) = " << isnan(nan) << '\n'
		<< "isnan(Inf) = " << isnan(inf) << '\n'
		<< "isnan(0.0) = " << isnan(zero) << '\n'
//		<< "isnan(DBL_MIN/2.0) = " << isnan(dblmin / 2.0) << '\n'
		<< "isnan(1.0) = " << isnan(one) << '\n';

#else

	cout << "fixpnt classification function validation" << endl;


#if STRESS_TESTING
	
#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_internal_exception& err) {
	std::cerr << "Uncaught fixpnt internal exception: " << err.what() << std::endl;
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
