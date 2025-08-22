// mathlib.cpp: test suite runner for bfloat16 mathlib shim
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_mathlib.hpp>

// generate specific test case that you can trace with the trace conditions in cfloat.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<typename Ty,
         typename std::enable_if<std::is_floating_point<Ty>::value, Ty>::type = 0
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
	GenerateTestCase<float>(4.0f, 2.0f);
	GenerateTestCase<double>(4.0, 2.0);
	//GenerateTestCase<bfloat16>(4.0f, 2.0f);  this will not compile, and should not

	return 0;

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
	//nrOfFailedTestCases += ReportTestResult(VerifyAtanh< bfloat16, float >(true, 0), "bfloat16", "atanh");
	//nrOfFailedTestCases += ReportTestResult(VerifyAsinh< bfloat16, float >(true, 0), "bfloat16", "asinh");
	//nrOfFailedTestCases += ReportTestResult(VerifyAcosh< bfloat16, float >(true, 0), "bfloat16", "acosh");

	// same thing here, tgamma is very sensitive to input values and needs to be computed in float for bfloat16 to match
	nrOfFailedTestCases += ReportTestResult(VerifyTgamma< bfloat16, float >(true, 0), "bfloat16", "tgamma");

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

	{
		int failures{ 0 };
		bfloat16 a;
		a.setinf();
		if (fpclassify(a) != FP_INFINITE) {
			std::cerr << "bfloat16 fpclassify(+inf) != FP_INFINITE\n";
			++failures;
		}
		a.setnan();
		if (fpclassify(a) != FP_NAN) {
			std::cerr << "bfloat16 fpclassify(NaN) != FP_NAN\n";
			++failures;
		}
		a = 0.0f;
		if (fpclassify(a) != FP_ZERO) {
			std::cerr << "bfloat16 fpclassify(0.0f) != FP_ZERO\n";
			++failures;
		}
		a.setbits(0x0001u); // smallest positive subnormal
		if (fpclassify(a) != FP_SUBNORMAL) {
			std::cerr << "bfloat16 fpclassify(smallest positive subnormal) != FP_SUBNORMAL\n";
			++failures;
		}
		a = 1.0f;
		if (fpclassify(a) != FP_NORMAL) {
			std::cerr << "bfloat16 fpclassify(1.0f) != FP_NORMAL\n";
			++failures;
		}
		nrOfFailedTestCases += ReportTestResult(failures, "bfloat16", "fpclassify");
	}

	nrOfFailedTestCases += ReportTestResult(VerifySqrt< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "sqrt");

	nrOfFailedTestCases += ReportTestResult(VerifyPow< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "pow");

	{
		// integer power
		bfloat16 a, b, ref;
		a = 71.0f;
		int failures{ 0 };
		for (int i = 0; i < 100; ++i) {
			b = ipow(a, i);
			ref = bfloat16(std::pow(float(a), float(i)));
			if (b != ref) {
				if (true) std::cerr << "bfloat16 ipow(2.0f, " << i << ") " << to_binary(b) << " != " << to_binary(ref) << '\n';
				failures++;
			}
		}
		nrOfFailedTestCases += ReportTestResult(failures, "bfloat16", "ipow");
	}

	{
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

	nrOfFailedTestCases += ReportTestResult(VerifyHypot< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "hypot");

	// bfloat16 trigonometric function validation
	nrOfFailedTestCases += ReportTestResult(VerifySine< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "sin");
	nrOfFailedTestCases += ReportTestResult(VerifyCosine< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "cos");
	nrOfFailedTestCases += ReportTestResult(VerifyTangent< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "tan");
	nrOfFailedTestCases += ReportTestResult(VerifyAtan< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "atan");
	nrOfFailedTestCases += ReportTestResult(VerifyAcos< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "acos");
	nrOfFailedTestCases += ReportTestResult(VerifyAsin< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "asin");

	// bfloat16 hyperbolic function validation
	// sinh and tanh were failing on G++ 13.3.0 and required using float as reference type
	nrOfFailedTestCases += ReportTestResult(VerifySinh< bfloat16, float >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifyCosh< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "cosh");
	nrOfFailedTestCases += ReportTestResult(VerifyTanh< bfloat16, float >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "tanh");
	// atanh and sinh were failing with 1 ULP errors on small values for all compilers and required using float as reference type
	nrOfFailedTestCases += ReportTestResult(VerifyAtanh< bfloat16, float >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "atanh");
	nrOfFailedTestCases += ReportTestResult(VerifyAcosh< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "acosh");
	nrOfFailedTestCases += ReportTestResult(VerifyAsinh< bfloat16, float >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "asinh");

	// bfloat16 logarithm function validation
	nrOfFailedTestCases += ReportTestResult(VerifyLog< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog2< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "log2");
	nrOfFailedTestCases += ReportTestResult(VerifyLog10< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "log10");
	nrOfFailedTestCases += ReportTestResult(VerifyLog1p< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "log1p");

	// bfloat16 exponential function validation
	nrOfFailedTestCases += ReportTestResult(VerifyExp< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "exp2");
	//nrOfFailedTestCases += ReportTestResult(VerifyExp10< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "exp10");
	nrOfFailedTestCases += ReportTestResult(VerifyExpm1< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "expm1");

	// bfloat16 truncation function validation
	nrOfFailedTestCases += ReportTestResult(VerifyRound< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "round");
	nrOfFailedTestCases += ReportTestResult(VerifyTrunc< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "trunc");
	nrOfFailedTestCases += ReportTestResult(VerifyFloor< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "floor");
	nrOfFailedTestCases += ReportTestResult(VerifyCeil< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "ceil");

	// bfloat16 fractional function validation\n";
	nrOfFailedTestCases += ReportTestResult(VerifyFmod< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "fmod");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "remainder");

	// bfloat16 error function validation\n";
	nrOfFailedTestCases += ReportTestResult(VerifyErf< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "erf");
	nrOfFailedTestCases += ReportTestResult(VerifyErfc< bfloat16 >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "erfc");
	// tgamma is very sensitive to the input value and needs to be computed in float for bfloat16 to match
	nrOfFailedTestCases += ReportTestResult(VerifyTgamma< bfloat16, float >(reportTestCases, NR_TEST_SAMPLES), "bfloat16", "tgamma");

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