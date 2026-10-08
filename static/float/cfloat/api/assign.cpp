// assign.cpp: assigning a cfloat from its encoding written as a binary string
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// assign("0b<sign>.<exponent>.<fraction>") sets the encoding.  A well-formed string has
// exactly nbits bits in three fields of widths 1, es and fbits; anything else must be
// rejected and leave the value at zero (#1647: a misplaced first '.' used to be accepted).
#include <universal/utility/directives.hpp>
#include <string>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

// every encoding round-trips through to_binary and assign
template<typename Cfloat>
int VerifyRoundTrip(bool report) {
	int fails = 0;
	constexpr unsigned N = (Cfloat::nbits <= 12) ? (1u << Cfloat::nbits) : 4096u;
	for (unsigned i = 0; i < N; ++i) {
		Cfloat a;
		a.setbits(Cfloat::nbits <= 12 ? i : (i * 2654435761u));   // exhaustive when small, spread out otherwise
		Cfloat b;
		b.assign(sw::universal::to_binary(a));
		if (to_binary(b) != to_binary(a)) {
			++fails;
			if (report && fails < 10) std::cerr << "FAIL round trip " << to_binary(a) << " -> " << to_binary(b) << '\n';
		}
	}
	return fails;
}

// malformed strings are rejected and leave zero; the valid ones set the exact encoding
template<typename Cfloat>
int VerifyFields(const std::string& valid, double value, const std::string& badSign, const std::string& badExponent, const std::string& badFraction, bool report) {
	int fails = 0;
	auto expect = [&](bool ok, const std::string& what) { if (!ok) { ++fails; if (report) std::cerr << "FAIL " << sw::universal::type_tag(Cfloat{}) << ' ' << what << '\n'; } };
	Cfloat a; a.assign(valid);
	expect(double(a) == value, valid + " assigns " + std::to_string(value));
	std::string separated = valid;
	separated.insert(separated.size() - 1, "'");
	Cfloat s; s.assign(separated);
	expect(double(s) == value, separated + " accepts a ' digit separator");
	for (const std::string& bad : { badSign, badExponent, badFraction, std::string("0b") + valid.substr(2, valid.find('.') - 2) + valid.substr(valid.find('.') + 1),
	                                 valid.substr(1), valid + "0", valid.substr(0, valid.size() - 1) + "2" }) {
		Cfloat b; b.assign(bad);
		expect(b.iszero(), bad + " is rejected and leaves zero, got " + to_binary(b));
	}
	return fails;
}

}  // anonymous namespace

int main()
try {
	using namespace sw::universal;
	std::string test_suite  = "cfloat assign from a binary string";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;
	ReportTestSuiteHeader(test_suite, reportTestCases);

	using C8  = cfloat<8, 2, std::uint8_t, true, false, false>;
	using C16 = cfloat<16, 5, std::uint16_t, true, false, false>;
	using C32 = cfloat<32, 8, std::uint32_t, true, false, false>;

	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrip<C8>(reportTestCases),  "cfloat<8,2>",  "round trip");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrip<C16>(reportTestCases), "cfloat<16,5>", "round trip");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrip<C32>(reportTestCases), "cfloat<32,8>", "round trip");

	// all with the right total bit count -- bad sign: two sign bits, a correct exponent and a
	// short fraction (the case #1647 accepted); bad exponent: the second '.' one bit early;
	// bad fraction: the second '.' one bit late
	nrOfFailedTestCases += ReportTestResult(VerifyFields<C8>("0b0.01.00000", 1.0, "0b01.10.0000", "0b0.0.100000", "0b0.010.0000", reportTestCases), "cfloat<8,2>", "field widths");
	nrOfFailedTestCases += ReportTestResult(VerifyFields<C16>("0b0.01111.0000000000", 1.0, "0b00.01111.000000000", "0b0.0111.10000000000", "0b0.011110.000000000", reportTestCases), "cfloat<16,5>", "field widths");
	nrOfFailedTestCases += ReportTestResult(VerifyFields<C32>("0b1.10000000.00000000000000000000000", -2.0, "0b11.10000000.0000000000000000000000", "0b1.1000000.000000000000000000000000", "0b1.100000000.0000000000000000000000", reportTestCases), "cfloat<32,8>", "field widths");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
