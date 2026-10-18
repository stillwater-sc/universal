// long_literals.cpp: regression tests for efloat's parse of long decimal literals.
//
// Guards issue #1713: parse() capped the target precision below a working budget that fell as
// the literal got longer, so assigning more digits produced fewer correct bits -- a 300-digit
// constant parsed to 64 bits, and from ~310 digits on to 0 -- with assign() discarding the
// failure. A literal of any length must now be correctly rounded to the value's precision,
// against exact rational references (tools/generators/efloat_parse_gen.py), including literals
// whose rounding is decided only by digits far past the target: the exact midpoint of two
// neighbours, and the same with one more digit either way.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdio>
#include <string>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/efloat_parse_vectors.hpp>

namespace {

using namespace sw::universal;

// the mantissa of v as hex, MSB first, in precision/4 digits
template<unsigned nlimbs>
std::string mantissa_hex(const efloat<nlimbs>& v, unsigned precision) {
	std::string hex;
	const auto& limbs = v.bits();
	for (unsigned i = 0; i < precision / 32u; ++i) {
		char buf[9];
		std::snprintf(buf, sizeof(buf), "%08x", static_cast<unsigned>(i < limbs.size() ? limbs[i] : 0u));
		hex += buf;
	}
	return hex;
}

// parse() and assign() of every vector, at the vector's precision, must give its rounding bit for bit
template<unsigned nlimbs>
int VerifyParseVectors(bool reportTestCases) {
	using namespace sw::universal::efloat_parse_verification;
	int nrOfFailedTests = 0;
	for (const auto& v : parse_vectors) {
		for (int through_assign = 0; through_assign < 2; ++through_assign) {
			efloat<nlimbs> x;
			x.set_precision(v.precision);
			bool ok = true;
			if (through_assign) x.assign(v.literal); else ok = parse(std::string(v.literal), x);
			const bool match = ok && !x.iszero() && !x.isnan() && x.get_precision() == v.precision
			                && (x.sign() == -1) == v.negative && x.scale() == v.scale
			                && mantissa_hex(x, v.precision) == v.mantissa;
			if (!match) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cerr << "FAIL: " << (through_assign ? "assign" : "parse") << " of " << v.label << " at "
					          << v.precision << " bits: ok " << ok << ", scale " << x.scale() << " (want " << v.scale
					          << "), precision " << x.get_precision() << '\n';
				}
			}
		}
	}
	return nrOfFailedTests;
}

// a well-formed literal past the converter's reach is reported, and leaves a NaN, not a zero
int VerifyUnreachable(bool reportTestCases) {
	int nrOfFailedTests = 0;
	for (const char* txt : { "1e100000", "-7.5e-100000" }) {
		efloat<10> x;
		const bool ok = parse(std::string(txt), x);
		efloat<10> y;
		y.assign(txt);
		if (ok || !x.isnan() || !y.isnan()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << txt << " was not reported (ok " << ok << ")\n";
		}
	}
	return nrOfFailedTests;
}

}  // namespace

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
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "efloat long literals";
	std::string test_tag    = "long literals";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyParseVectors<64>(true), "efloat<64>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyParseVectors<64>(reportTestCases), "efloat<64>", "608 and 2048 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyUnreachable(reportTestCases), "efloat<10>", "unreachable literal");
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
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
