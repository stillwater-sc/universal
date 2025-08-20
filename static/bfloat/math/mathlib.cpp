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

	nrOfFailedTestCases += ReportTestResult(VerifyHypot< bfloat16 >(true, 10), "bfloat16", "hypot");

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
    
	std::cout << "bfloat16 Sqrt function validation\n";
	nrOfFailedTestCases += ReportTestResult(VerifySqrt< bfloat16 >(reportTestCases, 100), "bfloat16", "sqrt");

	std::cout << "bfloat16 Power function validation\n";
	nrOfFailedTestCases += ReportTestResult(VerifyPow< bfloat16 >(reportTestCases, 100), "bfloat16", "pow");

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
	nrOfFailedTestCases += ReportTestResult(VerifyHypot< bfloat16 >(reportTestCases, 100), "bfloat16", "hypot");

	std::cout << "bfloat16 hyperbolic function validation\n";
	nrOfFailedTestCases += ReportTestResult(VerifySinh< bfloat16 >(reportTestCases, 100), "bfloat16", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifyCosh< bfloat16 >(reportTestCases, 100), "bfloat16", "cosh");
	nrOfFailedTestCases += ReportTestResult(VerifyTanh< bfloat16 >(reportTestCases, 100), "bfloat16", "tanh");
	nrOfFailedTestCases += ReportTestResult(VerifyAtanh< bfloat16 >(reportTestCases, 100), "bfloat16", "atanh");
	nrOfFailedTestCases += ReportTestResult(VerifyAcosh< bfloat16 >(reportTestCases, 100), "bfloat16", "acosh");
	nrOfFailedTestCases += ReportTestResult(VerifyAsinh< bfloat16 >(reportTestCases, 100), "bfloat16", "asinh");

	std::cout << "bfloat16 trigonometric function validation\n";
	nrOfFailedTestCases += ReportTestResult(VerifySine< bfloat16 >(reportTestCases, 100), "bfloat16", "sin");
	nrOfFailedTestCases += ReportTestResult(VerifyCosine< bfloat16 >(reportTestCases, 100), "bfloat16", "cos");
	nrOfFailedTestCases += ReportTestResult(VerifyTangent< bfloat16 >(reportTestCases, 100), "bfloat16", "tan");
	nrOfFailedTestCases += ReportTestResult(VerifyAtan< bfloat16 >(reportTestCases, 100), "bfloat16", "atan");
	nrOfFailedTestCases += ReportTestResult(VerifyAcos< bfloat16 >(reportTestCases, 100), "bfloat16", "acos");
	nrOfFailedTestCases += ReportTestResult(VerifyAsin< bfloat16 >(reportTestCases, 100), "bfloat16", "asin");

	std::cout << "bfloat16 logarithm function validation\n";
	nrOfFailedTestCases += ReportTestResult(VerifyLog< bfloat16 >(reportTestCases, 100), "bfloat16", "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog2< bfloat16 >(reportTestCases, 100), "bfloat16", "log2");
	nrOfFailedTestCases += ReportTestResult(VerifyLog10< bfloat16 >(reportTestCases, 100), "bfloat16", "log10");
	nrOfFailedTestCases += ReportTestResult(VerifyLog1p< bfloat16 >(reportTestCases, 100), "bfloat16", "log1p");

	std::cout << "bfloat16 exponential function validation\n";
	nrOfFailedTestCases += ReportTestResult(VerifyExp< bfloat16 >(reportTestCases, 100), "bfloat16", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< bfloat16 >(reportTestCases, 100), "bfloat16", "exp2");
	//nrOfFailedTestCases += ReportTestResult(VerifyExp10< bfloat16 >(reportTestCases, 100), "bfloat16", "exp10");
	nrOfFailedTestCases += ReportTestResult(VerifyExpm1< bfloat16 >(reportTestCases, 100), "bfloat16", "expm1");

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
