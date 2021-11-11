// classify.cpp: test suite runner for classification functions specialized for classic floats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// use default number system configuration
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/cfloat_math_test_suite.hpp>

#define MANUAL_TESTING 1
#define STRESS_TESTING 0


int main()
try {
	using namespace sw::universal;

	//bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "Addition failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

#define MY_DBL_MIN          2.2250738585072014e-308 // minpos value

	constexpr size_t nbits = 32;
	constexpr size_t es = 8;
	using bt = uint32_t;
	constexpr bool hasSubnormals = true;
	constexpr bool hasSupernormals = true;
	constexpr bool isSaturating = false;
	using Number = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Number cnan; cnan.setnan();
	Number cinf; cinf.setinf();
	Number czero(0);
	// Number minpos(SpecificValue::minpos);
	Number dblmin(MY_DBL_MIN);
	Number cone(1);

	std::cout << std::boolalpha
		<< "isnormal(NaN) = " << std::isnormal(NAN) << '\n'
		<< "isnormal(Inf) = " << std::isnormal(INFINITY) << '\n'
		<< "isnormal(0.0) = " << std::isnormal(0.0) << '\n'
		<< "isnormal(DBL_MIN/2.0) = " << std::isnormal(MY_DBL_MIN / 2.0) << '\n'
		<< "isnormal(1.0) = " << std::isnormal(1.0) << '\n';
	std::cout << std::boolalpha
		<< "isnormal(NaR) = " << isnormal(cnan) << '\n'
		<< "isnormal(Inf) = " << isnormal(cinf) << '\n'
		<< "isnormal(0.0) = " << isnormal(czero) << '\n'
//		<< "isnormal(DBL_MIN/2.0) = " << isnormal(dblmin / 2.0) << '\n'
		<< "isnormal(1.0) = " << isnormal(cone) << '\n';

	std::cout << std::boolalpha
		<< "isfinite(NaN) = " << std::isfinite(NAN) << '\n'
		<< "isfinite(Inf) = " << std::isfinite(INFINITY) << '\n'
		<< "isfinite(0.0) = " << std::isfinite(0.0) << '\n'
		<< "isfinite(DBL_MIN/2.0) = " << std::isfinite(MY_DBL_MIN / 2.0) << '\n'
		<< "isfinite(1.0) = " << std::isfinite(1.0) << '\n';
	std::cout << std::boolalpha
		<< "isfinite(NaR) = " << isfinite(cnan) << '\n'
		<< "isfinite(Inf) = " << isfinite(cinf) << '\n'
		<< "isfinite(0.0) = " << isfinite(czero) << '\n'
//		<< "isfinite(DBL_MIN/2.0) = " << isfinite(dblmin / 2.0) << '\n'
		<< "isfinite(1.0) = " << isfinite(cone) << '\n';

	std::cout << std::boolalpha
		<< "isinf(NaN) = " << std::isinf(NAN) << '\n'
		<< "isinf(Inf) = " << std::isinf(INFINITY) << '\n'
		<< "isinf(0.0) = " << std::isinf(0.0) << '\n'
		<< "isinf(DBL_MIN/2.0) = " << std::isinf(MY_DBL_MIN / 2.0) << '\n'
		<< "isinf(1.0) = " << std::isinf(1.0) << '\n';
	std::cout << std::boolalpha
		<< "isinf(NaR) = " << isinf(cnan) << '\n'
		<< "isinf(Inf) = " << isinf(cinf) << '\n'
		<< "isinf(0.0) = " << isinf(czero) << '\n'
//		<< "isinf(DBL_MIN/2.0) = " << isinf(dblmin / 2.0) << '\n'
		<< "isinf(1.0) = " << isinf(cone) << '\n';

	std::cout << std::boolalpha
		<< "isnan(NaN) = " << std::isnan(NAN) << '\n'
		<< "isnan(Inf) = " << std::isnan(INFINITY) << '\n'
		<< "isnan(0.0) = " << std::isnan(0.0) << '\n'
		<< "isnan(DBL_MIN/2.0) = " << std::isnan(MY_DBL_MIN / 2.0) << '\n'
		<< "isnan(1.0) = " << std::isnan(1.0) << '\n';
	std::cout << std::boolalpha
		<< "isnan(NaR) = " << isnan(cnan) << '\n'
		<< "isnan(Inf) = " << isnan(cinf) << '\n'
		<< "isnan(0.0) = " << isnan(czero) << '\n'
//		<< "isnan(DBL_MIN/2.0) = " << isnan(dblmin / 2.0) << '\n'
		<< "isnan(1.0) = " << isnan(cone) << '\n';

#else

	cout << "cfloat classification function validation" << endl;


#if STRESS_TESTING
	
#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
