// elementary_reference.cpp: posit<64,3> elementary functions against references that do not use the
// posit math engine (#2071)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// elementary.cpp checks each function against the same efloat code evaluated wider, so a defect the
// engine shares with its reference cannot show there. The encodings below come from mpmath at 300
// digits, rounded to posit<64,3> by the posit rule (the tie point between encodings a and a + 1 is
// the 65-bit posit 2a + 1, ties to even, no rounding to 0 or NaR). The arguments are doubles, which
// posit<64,3> holds exactly. Generator (python3, mpmath):
//
//   def round_posit(x, nbits=64, es=3): binary search for decode(lo) <= |x| < decode(lo + 1), then
//   compare with decode(2 lo + 1, nbits + 1, es); negate the encoding for x < 0
//   for f in exp log sin cos tan atan asin sinh tanh cot sec csc erf:
//       for a in 0.5 1.5 3.0 10.0 -2.25 0.001 0.75: emit (f, a, round_posit(f(mpf(a))))
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <cstring>
#include <string>
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	struct ElementaryReference { const char* function; double argument; std::uint64_t encoding; };

	static const ElementaryReference references[] = {
		{ "exp", 0.5, 0x42984a638781a6f2ull },
		{ "exp", 1.5, 0x487b4ff993f15056ull },
		{ "exp", 3.0, 0x5105796fdbec417dull },
		{ "exp", 10.0, 0x6cb053b9f2a0abf4ull },
		{ "exp", -2.25, 0x32bedc63a115fdd4ull },
		{ "exp", 0.001, 0x400106466dfb8cf4ull },
		{ "exp", 0.75, 0x443be76d19f73defull },
		{ "log", 0.5, 0xc2746f4041718433ull },
		{ "log", 1.5, 0x3a7cc8fb2fe612fdull },
		{ "log", 3.0, 0x4064fa9eab40c2a6ull },
		{ "log", 10.0, 0x449aec6eed554561ull },
		{ "log", 0.001, 0xb5179d599c0017f0ull },
		{ "log", 0.75, 0xc765a77bb2c91b62ull },
		{ "sin", 0.5, 0x3babba1d12c17bfaull },
		{ "sin", 1.5, 0x3ffadea6cb1b62b8ull },
		{ "sin", 3.0, 0x34840e1b6db556d4ull },
		{ "sin", 10.0, 0xc3a5d842832c2b6aull },
		{ "sin", -2.25, 0xc1c6818f4bc715aaull },
		{ "sin", 0.001, 0x1c0c49b4a4314c41ull },
		{ "sin", 0.75, 0x3d73ff05afe3c359ull },
		{ "cos", 0.5, 0x3f054a0196df53e7ull },
		{ "cos", 1.5, 0x3086f553f17e69d1ull },
		{ "cos", 3.0, 0xc0147ed05e868b68ull },
		{ "cos", 10.0, 0xc14994ddbce53838ull },
		{ "cos", -2.25, 0xc2f9801b919164daull },
		{ "cos", 0.001, 0x3fffffbce421dafbull },
		{ "cos", 0.75, 0x3dda7fb1954847baull },
		{ "tan", 0.5, 0x3c5ed3d6fcd1d291ull },
		{ "tan", 1.5, 0x4f0cfb542e21ddd7ull },
		{ "tan", 3.0, 0xcb70423b6ad1e43dull },
		{ "tan", 10.0, 0x3d2fd7cd2f8de097ull },
		{ "tan", -2.25, 0x40f45acc3e6b73a7ull },
		{ "tan", 0.001, 0x1c0c49c5d23d7c76ull },
		{ "tan", 0.75, 0x3f73e8d8443bbaf8ull },
		{ "atan", 0.5, 0x3b6b19c1586ed3daull },
		{ "atan", 1.5, 0x3fdcc2f4a07da6c8ull },
		{ "atan", 3.0, 0x40ff05dadea157ffull },
		{ "atan", 10.0, 0x41e26f4b058014e1ull },
		{ "atan", -2.25, 0xbf63c42a96b176daull },
		{ "atan", 0.001, 0x1c0c49aeea2dc2b1ull },
		{ "atan", 0.75, 0x3d25e3e8c9a7b849ull },
		{ "asin", 0.5, 0x3c30548e0b5cd961ull },
		{ "asin", 0.001, 0x1c0c49c0183962c9ull },
		{ "asin", 0.75, 0x3ec8d4c571a03702ull },
		{ "sinh", 0.5, 0x3c2b3403f98ee5b3ull },
		{ "sinh", 1.5, 0x444230eaaf5a981bull },
		{ "sinh", 3.0, 0x4d0249b9c2526b67ull },
		{ "sinh", 10.0, 0x6ab053b9dad3323eull },
		{ "sinh", -2.25, 0xb74f0f980a228bceull },
		{ "sinh", 0.001, 0x1c0c49c018393c5bull },
		{ "sinh", 0.75, 0x3e941acb74f1a3f9ull },
		{ "tanh", 0.5, 0x3b64d4f5d5a2bcd9ull },
		{ "tanh", 1.5, 0x3f3dbe5e1b7d5dd8ull },
		{ "tanh", 3.0, 0x3ff5df444682bff9ull },
		{ "tanh", 10.0, 0x3fffffff725bcd55ull },
		{ "tanh", -2.25, 0xc02d00a4f480b876ull },
		{ "tanh", 0.001, 0x1c0c49aeea2d9c43ull },
		{ "tanh", 0.75, 0x3d14c8f954bc8a0aull },
		{ "cot", 0.5, 0x43526b5f91f028a9ull },
		{ "cot", 1.5, 0x3089de6a452c393dull },
		{ "cot", 3.0, 0xb4fc1868a6687a9cull },
		{ "cot", 10.0, 0x422b5e128b099c07ull },
		{ "cot", -2.25, 0x3e75715fe51af7ccull },
		{ "cot", 0.001, 0x63e7ffea2798239eull },
		{ "cot", 0.75, 0x404b30396ac364a4ull },
		{ "sec", 0.5, 0x408ed77eff65ed9full },
		{ "sec", 1.5, 0x4f1183bd95a2e076ull },
		{ "sec", 3.0, 0xbff5a612ea136ea0ull },
		{ "sec", 10.0, 0xbf3b9a7ba4c2e5ecull },
		{ "sec", -2.25, 0xbda1e09f48162102ull },
		{ "sec", 0.001, 0x400000218df02bfcull },
		{ "sec", 0.75, 0x4177807fe77e98dbull },
		{ "csc", 0.5, 0x442bf1dceada7704ull },
		{ "csc", 1.5, 0x40029252c670a110ull },
		{ "csc", 3.0, 0x4b160f1102ac3649ull },
		{ "csc", 10.0, 0xbca5b858bd2c18c5ull },
		{ "csc", -2.25, 0xbedbedaec04860c2ull },
		{ "csc", 0.001, 0x63e8000aec33f752ull },
		{ "csc", 0.75, 0x41de4311c482efa0ull },
		{ "erf", 0.5, 0x3c29fbd7063b748bull },
		{ "erf", 1.5, 0x3fba9555c4deb7ecull },
		{ "erf", 3.0, 0x3ffff46b104d7e5full },
		{ "erf", 10.0, 0x4000000000000000ull },
		{ "erf", -2.25, 0xc002fee27f3992f0ull },
		{ "erf", 0.001, 0x1c4f987009a6e974ull },
		{ "erf", 0.75, 0x3db0725d674397dbull },
	};

	posit<64, 3> evaluate(const char* f, const posit<64, 3>& x) {
		if (!std::strcmp(f, "exp"))  return exp(x);
		if (!std::strcmp(f, "log"))  return log(x);
		if (!std::strcmp(f, "sin"))  return sin(x);
		if (!std::strcmp(f, "cos"))  return cos(x);
		if (!std::strcmp(f, "tan"))  return tan(x);
		if (!std::strcmp(f, "atan")) return atan(x);
		if (!std::strcmp(f, "asin")) return asin(x);
		if (!std::strcmp(f, "sinh")) return sinh(x);
		if (!std::strcmp(f, "tanh")) return tanh(x);
		if (!std::strcmp(f, "cot"))  return cot(x);
		if (!std::strcmp(f, "sec"))  return sec(x);
		if (!std::strcmp(f, "csc"))  return csc(x);
		if (!std::strcmp(f, "erf"))  return erf(x);
		posit<64, 3> nar;
		nar.setnar();
		return nar;
	}

	int VerifyAgainstMpmath(bool reportTestCases) {
		int fails = 0;
		for (const ElementaryReference& r : references) {
			const posit<64, 3> x(r.argument);
			posit<64, 3> want;
			want.setbits(r.encoding);
			const posit<64, 3> got = evaluate(r.function, x);
			if (got != want) {
				++fails;
				if (reportTestCases) std::cerr << "FAIL " << r.function << '(' << r.argument << ") = " << to_binary(got) << ", mpmath " << to_binary(want) << '\n';
			}
		}
		return fails;
	}

}} // namespace sw::universal

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "posit<64,3> elementary functions against mpmath (#2071)";
	std::string test_tag    = "independent reference";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyAgainstMpmath(reportTestCases), test_tag, "posit<64,3>");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstMpmath(reportTestCases), test_tag, "posit<64,3>");
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

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
