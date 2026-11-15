// limb_layouts.cpp: cfloat conversion between block types, and ++/-- across many limbs
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Two defects of #2087 that only show with several limbs:
//   - converting between two block types went through double, so a layout wider than double lost
//     range and precision: cfloat<32,15,uint32_t> maxpos became inf as cfloat<32,15,uint16_t>, and
//     its minpos became 0.  Between two layouts that differ only in block type every encoding must
//     carry over bit for bit, which is what this test checks: the reference is the encoding itself.
//   - isminnegencoding() skipped a block from five blocks on, so ++ on cfloat<40,8,uint8_t> bits
//     1.00000001.0..01 collapsed to +0.  ++ and -- step one encoding: toward zero for a negative
//     value, away from it for a positive one, from minneg to +0, and back.  The reference is a
//     carry/borrow on the bit string, written here independently of the cfloat code.
#include <universal/utility/directives.hpp>
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace {

template<typename A, typename B>
bool same_bits(const A& a, const B& b) {
	static_assert(A::nbits == B::nbits, "same width");
	for (unsigned i = 0; i < A::nbits; ++i) if (a.at(i) != b.at(i)) return false;
	return true;
}

template<typename Cfloat>
Cfloat from_bits(const std::vector<bool>& v) {
	Cfloat c;
	c.setbits(0);
	for (unsigned i = 0; i < Cfloat::nbits; ++i) c.setbit(i, v[i]);
	return c;
}

template<typename Cfloat>
std::vector<bool> bits_of(const Cfloat& c) {
	std::vector<bool> v(Cfloat::nbits);
	for (unsigned i = 0; i < Cfloat::nbits; ++i) v[i] = c.at(i);
	return v;
}

// every encoding of Src, converted to Tgt (same layout, other block type), keeps its bits
template<typename Src, typename Tgt>
int VerifyBlockTypeRoundTrip(unsigned samples, bool reportTestCases) {
	int fails = 0;
	std::mt19937_64 rng(2087);
	std::vector<Src> cases = { Src(sw::universal::SpecificValue::maxpos), Src(sw::universal::SpecificValue::minpos),
	                           Src(sw::universal::SpecificValue::maxneg), Src(sw::universal::SpecificValue::minneg) };
	for (unsigned k = 0; k < samples; ++k) {
		std::vector<bool> v(Src::nbits);
		for (unsigned i = 0; i < Src::nbits; ++i) v[i] = (rng() & 1u) != 0;
		if (k % 4 == 0) for (unsigned i = Src::fbits; i < Src::nbits - 1; ++i) v[i] = false;   // subnormals
		const Src s = from_bits<Src>(v);
		// without subnormals, a zero exponent field with a fraction is not a value of the layout
		// (iszero() reads it as a zero, and a conversion writes the clean zero encoding)
		if constexpr (!Src::hasSubnormals) { if (!s.iszeroencoding() && !s.isnormal() && !s.ismaxexpvalue()) continue; }
		cases.push_back(s);
	}
	for (const Src& s : cases) {
		if (s.isnan()) continue;   // the NaN type is kept, its payload is not
		const Tgt t(s);
		if (!same_bits(s, t)) {
			if (reportTestCases && fails < 10) std::cerr << "FAIL: " << to_binary(s) << " -> " << to_binary(t) << '\n';
			++fails;
		}
	}
	return fails;
}

// the next encoding of the bit string, read as sign-magnitude: +1 on the magnitude for a
// positive value, -1 for a negative one, with minneg stepping to +0
std::vector<bool> step_up(std::vector<bool> v) {
	const unsigned n = static_cast<unsigned>(v.size());
	const bool negative = v[n - 1];
	bool magnitudeIsOne = v[0];
	for (unsigned i = 1; i + 1 < n; ++i) magnitudeIsOne = magnitudeIsOne && !v[i];
	if (negative && magnitudeIsOne) { for (unsigned i = 0; i < n; ++i) v[i] = false; return v; }
	for (unsigned i = 0; i + 1 < n; ++i) {
		if (!negative) { v[i] = !v[i]; if (v[i]) break; }   // carry
		else           { v[i] = !v[i]; if (!v[i]) break; }  // borrow
	}
	return v;
}

// ++ steps one encoding and -- steps back, for finite values away from the ends of the range
template<typename Cfloat>
int VerifyIncrementAcrossLimbs(unsigned samples, bool reportTestCases) {
	int fails = 0;
	std::mt19937_64 rng(2088);
	std::vector<std::vector<bool>> cases;
	{
		// the issue's encoding: 1.00000001.0..01 on cfloat<40,8>, and minneg itself
		std::vector<bool> v(Cfloat::nbits, false);
		v[Cfloat::nbits - 1] = true; v[Cfloat::fbits] = true; v[0] = true;
		cases.push_back(v);
		std::vector<bool> m(Cfloat::nbits, false);
		m[Cfloat::nbits - 1] = true; m[0] = true;
		cases.push_back(m);
	}
	for (unsigned k = 0; k < samples; ++k) {
		std::vector<bool> v(Cfloat::nbits);
		for (unsigned i = 0; i < Cfloat::nbits; ++i) v[i] = (rng() & 1u) != 0;
		// keep the exponent field clear of all-ones (inf/NaN, maxpos) and of all-zeros most of
		// the time; every third case lands at a limb boundary: the low limb is all ones or zeros
		v[Cfloat::nbits - 2] = false;
		if (k % 3 == 0) { const bool b = (rng() & 1u) != 0; for (unsigned i = 0; i < 8 && i < Cfloat::fbits; ++i) v[i] = b; }
		cases.push_back(v);
	}
	for (const auto& v : cases) {
		Cfloat c = from_bits<Cfloat>(v);
		if (c.isnan() || c.isinf() || c.iszero()) continue;
		const std::vector<bool> expected = step_up(v);
		Cfloat up = c; ++up;
		if (bits_of(up) != expected) {
			if (reportTestCases && fails < 10) std::cerr << "FAIL: ++" << to_binary(c) << " = " << to_binary(up) << ", expected " << to_binary(from_bits<Cfloat>(expected)) << '\n';
			++fails;
			continue;
		}
		Cfloat back = up; --back;
		if (!same_bits(back, c)) {
			if (reportTestCases && fails < 10) std::cerr << "FAIL: --" << to_binary(up) << " = " << to_binary(back) << ", expected " << to_binary(c) << '\n';
			++fails;
		}
	}
	return fails;
}

}  // namespace

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
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "cfloat limb layouts";
	std::string test_tag    = "limb layouts";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	cfloat<32, 15, uint32_t> mp(SpecificValue::maxpos);
	std::cout << to_binary(mp) << " -> " << to_binary(cfloat<32, 15, uint16_t>(mp)) << '\n';
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	// block type conversions: the issue's large-es layout, and fraction widths past 64 (the wide path)
	nrOfFailedTestCases += ReportTestResult((VerifyBlockTypeRoundTrip<cfloat<32, 15, uint32_t>, cfloat<32, 15, uint16_t>>(2000, reportTestCases)), "cfloat<32,15> u32 -> u16", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyBlockTypeRoundTrip<cfloat<32, 15, uint16_t>, cfloat<32, 15, uint8_t>>(2000, reportTestCases)),  "cfloat<32,15> u16 -> u8",  test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyBlockTypeRoundTrip<cfloat<32, 15, uint8_t, true, false, false>, cfloat<32, 15, uint32_t, true, false, false>>(2000, reportTestCases)), "cfloat<32,15,t,f,f> u8 -> u32", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyBlockTypeRoundTrip<cfloat<64, 11, uint32_t, true, false, false>, cfloat<64, 11, uint64_t, true, false, false>>(2000, reportTestCases)), "cfloat<64,11,t,f,f> u32 -> u64", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyBlockTypeRoundTrip<cfloat<80, 15, uint64_t, true, false, false>, cfloat<80, 15, uint16_t, true, false, false>>(2000, reportTestCases)), "cfloat<80,15,t,f,f> u64 -> u16", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyBlockTypeRoundTrip<cfloat<96, 11, uint8_t, true, true, false>, cfloat<96, 11, uint32_t, true, true, false>>(2000, reportTestCases)),   "cfloat<96,11,t,t,f> u8 -> u32", test_tag);

	// ++ / -- across five or more limbs
	nrOfFailedTestCases += ReportTestResult(VerifyIncrementAcrossLimbs<cfloat<40, 8, uint8_t, true, false, false>>(2000, reportTestCases),   "cfloat<40,8,uint8_t,t,f,f>",   "++/--");
	nrOfFailedTestCases += ReportTestResult(VerifyIncrementAcrossLimbs<cfloat<72, 11, uint16_t, true, false, false>>(2000, reportTestCases), "cfloat<72,11,uint16_t,t,f,f>", "++/--");
	nrOfFailedTestCases += ReportTestResult(VerifyIncrementAcrossLimbs<cfloat<96, 15, uint8_t, true, false, false>>(2000, reportTestCases),  "cfloat<96,15,uint8_t,t,f,f>",  "++/--");
	nrOfFailedTestCases += ReportTestResult(VerifyIncrementAcrossLimbs<cfloat<40, 8, uint8_t, true, true, false>>(2000, reportTestCases),    "cfloat<40,8,uint8_t,t,t,f>",   "++/--");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult((VerifyBlockTypeRoundTrip<cfloat<128, 15, uint8_t, true, false, false>, cfloat<128, 15, uint64_t, true, false, false>>(20000, reportTestCases)), "cfloat<128,15,t,f,f> u8 -> u64", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIncrementAcrossLimbs<cfloat<128, 15, uint8_t, true, false, false>>(20000, reportTestCases), "cfloat<128,15,uint8_t,t,f,f>", "++/--");
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
