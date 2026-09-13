// dpd_trailing.cpp: dfloat DPD keeps every significand digit at every precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Regression for #1482. A DPD dfloat stores the leading digit in the combination field
// and the other ndigits - 1 in 10-bit declets of three. When ndigits - 1 is not a
// multiple of 3, one or two digits are left over, and dpd_trailing_bits() gives them
// the top 4 or 7 trailing bits, but the encoder and decoder only handled full declets:
// the leftover digits were never stored. dfloat<6,6,DPD>(123456) read back as 100456,
// and dfloat<8,6,DPD>(12345678) as 10345678. decimal32, decimal64 and decimal128 (7,
// 16 and 34 digits) have no leftover digits and were not affected.
//
// BID, which stores the trailing significand as one binary integer, is the reference:
// both encodings must unpack every value to the same sign, exponent and significand,
// and must agree on + - * /.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <universal/number/dfloat/dfloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

// a random N-digit decimal significand as a string. The digits just below the leading
// one, which land in the leftover group, are 8 or 9 in every third sample. The last
// digit is never 0: parse() strips trailing zeros, and a short divisor significand
// overflows long division (#1484), which is not what this test is about.
template<unsigned N>
std::string RandomSignificand(std::mt19937_64& rng, int k) {
	std::string s;
	s += static_cast<char>('1' + rng() % 9);
	for (unsigned i = 1; i < N; ++i) {
		const bool leftover = i <= 2;
		unsigned d = (leftover && k % 3 == 0) ? 8u + static_cast<unsigned>(rng() % 2) : static_cast<unsigned>(rng() % 10);
		if (i == N - 1 && d == 0) d = 1u + static_cast<unsigned>(rng() % 9);
		s += static_cast<char>('0' + d);
	}
	return s;
}

// the same decimal text in BID and DPD: it must unpack identically, and BID must hold
// exactly the digits written
template<typename BID, typename DPD>
bool CheckSameValue(const std::string& sig, const std::string& txt, BID& b, DPD& d, std::string& why) {
	b.assign(txt);
	d.assign(txt);
	bool bs, ds; int be, de;
	typename BID::significand_t bsig; typename DPD::significand_t dsig;
	b.unpack(bs, be, bsig);
	d.unpack(ds, de, dsig);
	if (BID::sig_to_string(bsig) != sig) { why = txt + ": BID reference holds " + BID::sig_to_string(bsig); return false; }
	if (bs != ds || be != de || !(bsig == dsig)) {
		why = txt + ": DPD unpacks to " + std::string(ds ? "-" : "") + DPD::sig_to_string(dsig) + "e" + std::to_string(de)
		    + ", BID to " + std::string(bs ? "-" : "") + BID::sig_to_string(bsig) + "e" + std::to_string(be);
		return false;
	}
	return true;
}

// dfloat<N, ES> in BID and DPD: random values must unpack identically, and + - * / on
// pairs of them must give the same results. The two operands of a pair are at most
// two decades apart: a wider gap overflows the significand while add aligns exponents
// (#1484), in both encodings. With N-digit significands and that gap every
// intermediate stays within N + 2 digits, which the significand type holds.
template<unsigned N, unsigned ES>
int VerifyDpdMatchesBid(int nrSamples, bool reportTestCases) {
	using BID = dfloat<N, ES, DecimalEncoding::BID, std::uint32_t>;
	using DPD = dfloat<N, ES, DecimalEncoding::DPD, std::uint32_t>;
	std::mt19937_64 rng(1482 + N);   // deterministic: the engine's output sequence is specified by the standard
	int nrOfFailedTests = 0;
	auto fail = [&](const std::string& what) {
		++nrOfFailedTests;
		if (reportTestCases && nrOfFailedTests < 10) std::cerr << "FAIL: dfloat<" << N << ',' << ES << "> " << what << '\n';
	};

	std::vector<BID> bids;
	std::vector<DPD> dpds;
	for (int k = 0; k < nrSamples; ++k) {
		// exponents well inside the range, so nothing overflows or goes subnormal
		const int exponent = static_cast<int>(rng() % 21) - 10 - static_cast<int>(N) / 2;
		for (int side = 0; side < 2; ++side) {
			const std::string sig = RandomSignificand<N>(rng, k + side);
			const int e = exponent + (side ? static_cast<int>(rng() % 5) - 2 : 0);
			const std::string txt = ((k + side) % 2 ? "-" : "") + sig + "e" + std::to_string(e);
			BID b; DPD d; std::string why;
			if (!CheckSameValue(sig, txt, b, d, why)) fail(why);
			bids.push_back(b);
			dpds.push_back(d);
		}
	}

	// arithmetic on each pair
	for (size_t i = 0; i + 1 < bids.size(); i += 2) {
		const BID& a = bids[i]; const BID& b = bids[i + 1];
		const DPD& c = dpds[i]; const DPD& d = dpds[i + 1];
		if ((a + b).str() != (c + d).str()) fail("sum " + a.str() + " + " + b.str() + ": BID " + (a + b).str() + ", DPD " + (c + d).str());
		if ((a - b).str() != (c - d).str()) fail("difference " + a.str() + " - " + b.str() + ": BID " + (a - b).str() + ", DPD " + (c - d).str());
		if ((a * b).str() != (c * d).str()) fail("product " + a.str() + " * " + b.str() + ": BID " + (a * b).str() + ", DPD " + (c * d).str());
		if ((a / b).str() != (c / d).str()) fail("quotient " + a.str() + " / " + b.str() + ": BID " + (a / b).str() + ", DPD " + (c / d).str());
	}
	return nrOfFailedTests;
}

template<unsigned ES, unsigned... Ns>
int VerifyDpdMatchesBidAt(int nrSamples, bool reportTestCases) {
	return (VerifyDpdMatchesBid<Ns, ES>(nrSamples, reportTestCases) + ...);
}

// dpd_encode_significand / dpd_decode_significand: the trailing ndigits - 1 digits of
// every significand below 10^ndigits must round-trip. Two leftover digits used to be
// packed as (d1 << 4) | d0, which needs 8 bits once d1 >= 8, into a 7-bit field.
int VerifySignificandCodec(unsigned ndigits, bool reportTestCases) {
	std::uint64_t lim = 1;
	for (unsigned i = 0; i < ndigits; ++i) lim *= 10;
	const std::uint64_t trailing = lim / 10;
	int nrOfFailedTests = 0;
	for (std::uint64_t s = 0; s < lim; ++s) {
		const std::uint64_t decoded = dpd_decode_significand(dpd_encode_significand(s, ndigits), ndigits);
		if (decoded != s % trailing) {
			++nrOfFailedTests;
			if (reportTestCases && nrOfFailedTests < 10) std::cerr << "FAIL: significand codec, " << ndigits << " digits: " << s << " -> " << decoded << '\n';
		}
	}
	return nrOfFailedTests;
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

	std::string test_suite  = "dfloat DPD trailing significand digits (#1482)";
	std::string test_tag    = "dpd trailing";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		dfloat<6, 6, DecimalEncoding::DPD, std::uint32_t> v(123456);
		std::cout << "dfloat<6,6,DPD>(123456) = " << v << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	// every precision from 2 to 12 digits: (ndigits - 1) % 3 is 0, 1 and 2 in turn
	nrOfFailedTestCases += ReportTestResult(VerifyDpdMatchesBidAt<6, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12>(200, reportTestCases), "dfloat<2..12,6> DPD == BID", test_tag);
	// the IEEE interchange precisions, which have no leftover digits
	nrOfFailedTestCases += ReportTestResult(VerifyDpdMatchesBid<16, 8>(200, reportTestCases), "decimal64 DPD == BID", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySignificandCodec(5, reportTestCases), "significand codec, 5 digits", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySignificandCodec(6, reportTestCases), "significand codec, 6 digits", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyDpdMatchesBidAt<8, 13, 14, 15, 17, 18, 19>(500, reportTestCases), "dfloat<13..19,8> DPD == BID", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySignificandCodec(7, reportTestCases), "significand codec, 7 digits", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyDpdMatchesBidAt<6, 5, 6, 8, 9>(5000, reportTestCases), "dfloat<5,6,8,9> DPD == BID, 5000", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyDpdMatchesBid<34, 12>(500, reportTestCases), "decimal128 DPD == BID", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySignificandCodec(8, reportTestCases), "significand codec, 8 digits", test_tag);
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::exception& err) {
	std::cerr << "Caught exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
