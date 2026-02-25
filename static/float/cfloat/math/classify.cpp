// classify.cpp: test suite runner for classification functions specialized for classic floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/cfloat_test_suite_mathlib.hpp>

bool isdenorm(float f) {
	return (std::fpclassify(f) == FP_SUBNORMAL);
}
bool isdenorm(double d) {
	return (std::fpclassify(d) == FP_SUBNORMAL);
}

#define MANUAL_TESTING 0

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "cfloat<> mathlib classification validation";
	std::string test_tag    = "classify";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING


	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#define MY_DBL_MIN          2.2250738585072014e-308 // minpos value

	constexpr size_t nbits = 32;
	constexpr size_t es = 8;
	using bt = uint32_t;
	constexpr bool hasSubnormals = true;
	constexpr bool hasSupernormals = false;
	constexpr bool isSaturating = false;
	using Number = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Number cnan; cnan.setnan();
	Number cinf; cinf.setinf();
	Number czero(0);
	Number cminpos(SpecificValue::minpos);
	Number csubnormal;
	Number cone(1);

	// cminpos with submornals : 0b0.00000000.00000000000000000000001 : 1.4013e-45
	// csubnormal              : 0b0.00000000.10000000000000000000000 : 5.87747e-39
	std::cout << "cminpos with submornals : " << to_binary(cminpos) << " : " << cminpos << '\n';
	csubnormal.setbits(0x00400000);
	std::cout << "csubnormal              : " << to_binary(csubnormal) << " : " << csubnormal << '\n';
	std::cout << '\n';

	std::cout << type_tag(double()) << '\n' << std::boolalpha
		<< "isnormal(NaN) = " << std::isnormal(NAN) << '\n'
		<< "isnormal(Inf) = " << std::isnormal(INFINITY) << '\n'
		<< "isnormal(0.0) = " << std::isnormal(0.0) << '\n'
		<< "isnormal(DBL_MIN/2.0) = " << std::isnormal(MY_DBL_MIN / 2.0) << '\n'
		<< "isnormal(1.0) = " << std::isnormal(1.0) << '\n';
	std::cout << '\n' << type_tag(czero) << '\n' << std::boolalpha
		<< "isnormal(NaR) = " << isnormal(cnan) << '\n'
		<< "isnormal(Inf) = " << isnormal(cinf) << '\n'
		<< "isnormal(0.0) = " << isnormal(czero) << '\n'
		<< "isnormal(cminpos) = " << isnormal(cminpos) << '\n'
		<< "isnormal(1.0) = " << isnormal(cone) << '\n';

	std::cout << '\n' << type_tag(double()) << '\n' << std::boolalpha
		<< "isdenorm(NaN) = " << isdenorm(NAN) << '\n'
		<< "isdenorm(Inf) = " << isdenorm(INFINITY) << '\n'
		<< "isdenorm(0.0) = " << isdenorm(0.0) << '\n'
		<< "isdenorm(DBL_MIN/2.0) = " << isdenorm(MY_DBL_MIN / 2.0) << "  " << to_binary(MY_DBL_MIN / 2.0) << '\n'
		<< "isdenorm(1.0) = " << isdenorm(1.0) << '\n';
	std::cout << '\n' << type_tag(czero) << '\n' << std::boolalpha
		<< "isdenorm(NaR) = " << isdenorm(cnan) << '\n'
		<< "isdenorm(Inf) = " << isdenorm(cinf) << '\n'
		<< "isdenorm(0.0) = " << isdenorm(czero) << '\n'
		<< "isdenorm(cminpos) = " << isdenorm(cminpos) << "  " << to_binary(cminpos) << '\n'
		<< "isdenorm(1.0) = " << isdenorm(cone) << '\n';

	std::cout << '\n' << type_tag(double()) << '\n' << std::boolalpha
		<< "isfinite(NaN) = " << std::isfinite(NAN) << '\n'
		<< "isfinite(Inf) = " << std::isfinite(INFINITY) << '\n'
		<< "isfinite(0.0) = " << std::isfinite(0.0) << '\n'
		<< "isfinite(DBL_MIN/2.0) = " << std::isfinite(MY_DBL_MIN / 2.0) << '\n'
		<< "isfinite(1.0) = " << std::isfinite(1.0) << '\n';
	std::cout << '\n' << type_tag(czero) << '\n' << std::boolalpha
		<< "isfinite(NaR) = " << isfinite(cnan) << '\n'
		<< "isfinite(Inf) = " << isfinite(cinf) << '\n'
		<< "isfinite(0.0) = " << isfinite(czero) << '\n'
		<< "isfinite(cminpos) = " << isfinite(cminpos) << '\n'
		<< "isfinite(1.0) = " << isfinite(cone) << '\n';

	std::cout << '\n' << type_tag(double()) << '\n' << std::boolalpha
		<< "isinf(NaN) = " << std::isinf(NAN) << '\n'
		<< "isinf(Inf) = " << std::isinf(INFINITY) << '\n'
		<< "isinf(0.0) = " << std::isinf(0.0) << '\n'
		<< "isinf(DBL_MIN/2.0) = " << std::isinf(MY_DBL_MIN / 2.0) << '\n'
		<< "isinf(1.0) = " << std::isinf(1.0) << '\n';
	std::cout << '\n' << type_tag(czero) << '\n' << std::boolalpha
		<< "isinf(NaR) = " << isinf(cnan) << '\n'
		<< "isinf(Inf) = " << isinf(cinf) << '\n'
		<< "isinf(0.0) = " << isinf(czero) << '\n'
		<< "isinf(cminpos) = " << isinf(cminpos) << '\n'
		<< "isinf(1.0) = " << isinf(cone) << '\n';

	std::cout << '\n' << type_tag(double()) << '\n' << std::boolalpha
		<< "isnan(NaN) = " << std::isnan(NAN) << '\n'
		<< "isnan(Inf) = " << std::isnan(INFINITY) << '\n'
		<< "isnan(0.0) = " << std::isnan(0.0) << '\n'
		<< "isnan(DBL_MIN/2.0) = " << std::isnan(MY_DBL_MIN / 2.0) << '\n'
		<< "isnan(1.0) = " << std::isnan(1.0) << '\n';
	std::cout << '\n' << type_tag(czero) << '\n' << std::boolalpha
		<< "isnan(NaR) = " << isnan(cnan) << '\n'
		<< "isnan(Inf) = " << isnan(cinf) << '\n'
		<< "isnan(0.0) = " << isnan(czero) << '\n'
		<< "isnan(cminpos) = " << isnan(cminpos) << '\n'
		<< "isnan(1.0) = " << isnan(cone) << '\n';

	std::cout << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
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
