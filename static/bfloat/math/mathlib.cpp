// mathlib.cpp: test suite runner for bfloat16 mathlib shim
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/bfloat/bfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_mathlib.hpp>

// generate specific test case that you can trace with the trace conditions in cfloat.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<typename Ty,
    typename = typename std::enable_if<std::is_floating_point<Ty>::type, Ty>::value
>
void GenerateTestCase(Ty fa, Ty fb) {
	constexpr unsigned nbits = 16;
	Ty fref;
	sw::universal::bfloat16 a, b, ref, power;
	a = fa;
	b = fb;
	fref = std::pow(fa, fb);
	ref = fref;
	power = sw::universal::pow(a, b);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << " -> pow(" << fa << "," << fb << ") = " << std::setw(nbits) << fref << std::endl;
	std::cout << " -> pow( " << a << "," << b << ") = " << to_binary(power) << " (reference: " << to_binary(ref) << ")   " ;
	std::cout << (ref == power ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "bfloat16 mathlib function validation";
	std::string test_tag    = "mathlib";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	//GenerateTestCase<float>(4.0f, 2.0f);

	std::cout << std::endl;

	//nrOfFailedTestCases += ReportTestResult(VerifyHypot< bfloat16 >(true, 10), "bfloat16", "hypot");

	bfloat16 a;
	a.setbits(0x7f80u);  // +inf
	if (a.isinf(INF_TYPE_POSITIVE)) {
		std::cout << "bfloat16 +inf is recognized as positive infinity\n";
	} else {
		std::cerr << "bfloat16 +inf is NOT recognized as positive infinity\n";
		nrOfFailedTestCases++;
	}
	bfloat16 b;
	b.setbits(0xff80u);  // -inf
	if (b.isinf(INF_TYPE_NEGATIVE)) {
		std::cout << "bfloat16 -inf is recognized as negative infinity\n";
	} else {
		std::cerr << "bfloat16 -inf is NOT recognized as negative infinity\n";
		nrOfFailedTestCases++;
	}
	if (a.isnan() || b.isnan()) {
		std::cerr << "bfloat16 +inf or -inf is recognized as NaN\n";
		nrOfFailedTestCases++;
	} else {
		std::cout << "bfloat16 +inf and -inf are NOT recognized as NaN\n";
	}
	//nrOfFailedTestCases += ReportTestResult(VerifyRound< bfloat16 >(true, 0), "bfloat16", "round");

	a = -1.0f;
	b = sqrt(a);
	if (b.isnan()) {
		std::cout << "bfloat16 sqrt(-1.0f) is recognized as NaN\n";
	} else {
		std::cerr << "bfloat16 sqrt(-1.0f) is NOT recognized as NaN\n";
		nrOfFailedTestCases++;
	}
	//nrOfFailedTestCases += ReportTestResult(VerifySqrt< bfloat16 >(true, 0), "bfloat16", "sqrt");

	//nrOfFailedTestCases += ReportTestResult(VerifyLog< bfloat16 >(true, 0), "bfloat16", "log");

	// hyperbolic cotangent and hyperbolic secant are very sensitive to the input value
	// if you compute them in double, they induce 1 ULP errors for small inputs
	// you need to compute through floats to get the same values as bfloat16
	nrOfFailedTestCases += ReportTestResult(VerifyAtanh< bfloat16, float >(true, 0), "bfloat16", "atanh");
	nrOfFailedTestCases += ReportTestResult(VerifyAsinh< bfloat16, float >(true, 0), "bfloat16", "asinh");
	nrOfFailedTestCases += ReportTestResult(VerifyAcosh< bfloat16, float >(true, 0), "bfloat16", "acosh");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#ifdef LATER
	std::cout << "Integer power function\n";
	int a = 2;
	unsigned b = 32;
	std::cout << "2 ^ 32   = " << ipow(a, b) << '\n';
	std::cout << "2 ^ 32   = " << fastipow(a, uint8_t(b)) << '\n';

	int64_t c = 1024;
	uint8_t d = 2;
	std::cout << "1024 ^ 2 = " << ipow(c, d) << '\n';
	std::cout << "1M ^ 2   = " << ipow(ipow(c, d), d) << '\n';

#endif // LATER
    
	// If set to 0, the test suite will run all test cases
	constexpr unsigned NR_TEST_SAMPLES = 16384;

	std::cout << "bfloat16 Sqrt function validation\n";
	nrOfFailedTestCases += ReportTestResult(VerifySqrt< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "sqrt");

	std::cout << "bfloat16 Power function validation\n";
	nrOfFailedTestCases += ReportTestResult(VerifyPow< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "pow");

	{
		std::cout << "bfloat16 min/max function validation\n";
		int currentFailedTestCases = nrOfFailedTestCases;
		bfloat16 a, b;
		a = 1.0f; b = 2.0f;
		if (min(a, b) != a) {
			std::cerr << "min(1.0f, 2.0f) failed\n";
			nrOfFailedTestCases++;
		}
		if (max(a, b) != b) {
			std::cerr << "max(1.0f, 2.0f) failed\n";
			nrOfFailedTestCases++;
		}
		ReportTestResult(nrOfFailedTestCases - currentFailedTestCases, "bfloat16", "min/max");
	}

	std::cout << "bfloat16 hypothenuse function validation\n";
	nrOfFailedTestCases += ReportTestResult(VerifyHypot< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "hypot");

	std::cout << "bfloat16 trigonometric function validation\n";
	nrOfFailedTestCases += ReportTestResult(VerifySine< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "sin");
	nrOfFailedTestCases += ReportTestResult(VerifyCosine< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "cos");
	nrOfFailedTestCases += ReportTestResult(VerifyTangent< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "tan");
	nrOfFailedTestCases += ReportTestResult(VerifyAtan< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "atan");
	nrOfFailedTestCases += ReportTestResult(VerifyAcos< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "acos");
	nrOfFailedTestCases += ReportTestResult(VerifyAsin< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "asin");

	std::cout << "bfloat16 hyperbolic function validation\n";
	nrOfFailedTestCases += ReportTestResult(VerifySinh< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifyCosh< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "cosh");
	nrOfFailedTestCases += ReportTestResult(VerifyTanh< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "tanh");
	nrOfFailedTestCases += ReportTestResult(VerifyAtanh< bfloat16, float >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "atanh");
	nrOfFailedTestCases += ReportTestResult(VerifyAcosh< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "acosh");
	nrOfFailedTestCases += ReportTestResult(VerifyAsinh< bfloat16, float >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "asinh");

	std::cout << "bfloat16 logarithm function validation\n";
	nrOfFailedTestCases += ReportTestResult(VerifyLog< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog2< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "log2");
	nrOfFailedTestCases += ReportTestResult(VerifyLog10< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "log10");
	nrOfFailedTestCases += ReportTestResult(VerifyLog1p< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "log1p");

	std::cout << "bfloat16 exponential function validation\n";
	nrOfFailedTestCases += ReportTestResult(VerifyExp< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "exp2");
	//nrOfFailedTestCases += ReportTestResult(VerifyExp10< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "exp10");
	nrOfFailedTestCases += ReportTestResult(VerifyExpm1< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "expm1");

	std::cout << "bfloat16 truncation function validation\n";
	nrOfFailedTestCases += ReportTestResult(VerifyRound< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "round");
	nrOfFailedTestCases += ReportTestResult(VerifyTrunc< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "trunc");
	nrOfFailedTestCases += ReportTestResult(VerifyFloor< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "floor");
	nrOfFailedTestCases += ReportTestResult(VerifyCeil< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "ceil");

	std::cout << "bfloat16 fractional function validation\n";
	nrOfFailedTestCases += ReportTestResult(VerifyFmod< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "fmod");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "remainder");

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

/*
bfloat16 mathlib function validation: results only
bfloat16 Sqrt function validation
bfloat16                                                     sqrt PASS
bfloat16 Power function validation
bfloat16                                                     pow PASS
bfloat16 min/max function validation
bfloat16                                                     min/max PASS
bfloat16 hypothenuse function validation
bfloat16                                                     hypot PASS
bfloat16 hyperbolic function validation
bfloat16                                                     sinh PASS
bfloat16                                                     cosh PASS
bfloat16                                                     tanh PASS
bfloat16                                                     atanh FAIL 26 failed test cases
bfloat16                                                     acosh PASS
bfloat16                                                     asinh FAIL 26 failed test cases
bfloat16 trigonometric function validation
bfloat16                                                     sin PASS
bfloat16                                                     cos PASS
bfloat16                                                     tan PASS
bfloat16                                                     atan PASS
bfloat16                                                     acos PASS
bfloat16                                                     asin PASS
bfloat16 logarithm function validation
bfloat16                                                     log FAIL 26 failed test cases
bfloat16                                                     log2 FAIL 26 failed test cases
bfloat16                                                     log10 FAIL 26 failed test cases
bfloat16                                                     log1p FAIL 26 failed test cases
bfloat16 exponential function validation
bfloat16                                                     exp PASS
bfloat16                                                     exp2 PASS
bfloat16                                                     expm1 PASS
bfloat16 truncation function validation
bfloat16                                                     round PASS
bfloat16                                                     trunc PASS
bfloat16                                                     floor PASS
bfloat16                                                     ceil PASS
bfloat16 mathlib function validation: FAIL
*/