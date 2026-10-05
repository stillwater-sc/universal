// api.cpp: application programming interface of the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// arithmetic exceptions on: division by zero and NaR operands throw
#define BPOSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/bposit/bposit.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

int expect(bool ok, const char* what, bool report) {
	if (ok) return 0;
	if (report) std::cerr << "FAIL: " << what << '\n';
	return 1;
}

// The encoder is constexpr: these are the reference encodings of bposit<8,4,0> (docs/number-systems/bposit.md)
using B840 = sw::universal::bposit<8, 4, 0, std::uint8_t>;
static_assert(B840::encode(false, -4, 1ull, 3u, false) == 0x01, "minpos = 9/128 is 0x01");
static_assert(B840::encode(false, 0, 0ull, 0u, false) == 0x40, "1.0 is 0x40");
static_assert(B840::encode(false, 3, 7ull, 3u, false) == 0x7F, "maxpos = 15 is 0x7F");
static_assert(B840::encode(true, 0, 0ull, 0u, false) == 0xC0, "-1.0 is the two's complement of 0x40");
static_assert(B840::encode(false, -4, 0ull, 0u, false) == 0x01, "2^minscale is the zero pattern: clamps to minpos");
static_assert(B840::encode(false, 9, 0ull, 0u, false) == 0x7F, "above maxpos saturates to maxpos");
static_assert(B840::encode(false, -9, 0ull, 0u, false) == 0x01, "below minpos saturates to minpos");
static_assert(B840::encode(false, -3, 0b0011ull, 4u, false) == 0x0A, "9/128 + 5/64 = 19/128 ties to the even encoding 0x0A = 5/32");
static_assert(sw::universal::bposit16::minscale == -192 && sw::universal::bposit16::maxscale == 191, "rS = 6, eS = 5: 2^-192 .. 2^192");
static_assert(sw::universal::bposit16::fbitsmin == 4 && sw::universal::bposit32::fbitsmin == 20 && sw::universal::bposit64::fbitsmin == 52, "F_min = nbits - 12");

}  // anonymous namespace

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "bposit API";
	std::string test_tag    = "api";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// triviality: no member initializers, defaulted special members
	ReportTrivialityOfType<bposit16>();
	ReportTrivialityOfType<bposit32>();
	ReportTrivialityOfType<bposit64>();
	ReportTrivialityOfType<B840>();

	{
		int f = 0;
		f += expect(type_tag(bposit16{}) == "bposit< 16, 6, 5, uint16_t>", "type_tag of bposit16", reportTestCases);
		f += expect(is_bposit<bposit32> && !is_bposit<double>, "is_bposit trait", reportTestCases);
		std::cout << bposit_range(bposit16{}) << '\n' << bposit_range(B840{}) << '\n';
		nrOfFailedTestCases += ReportTestResult(f, "bposit", "type tag and traits");
	}

	{
		int f = 0;
		const B840 a(3), b(3.0f), c(3.0), d(3.0l), e(3u);
		f += expect(a == b && b == c && c == d && d == e && double(a) == 3.0, "construction from int, float, double, long double, unsigned", reportTestCases);
		f += expect(B840(1).raw() == 0x40 && B840(-1).raw() == 0xC0, "1 and -1", reportTestCases);
		f += expect(B840(SpecificValue::maxpos).raw() == 0x7F && double(B840(SpecificValue::maxpos)) == 15.0, "maxpos", reportTestCases);
		f += expect(B840(SpecificValue::minpos).raw() == 0x01 && double(B840(SpecificValue::minpos)) == 9.0 / 128.0, "minpos", reportTestCases);
		f += expect(B840(SpecificValue::minneg).raw() == 0xFF && B840(SpecificValue::maxneg).raw() == 0x81, "minneg, maxneg", reportTestCases);
		f += expect(B840(SpecificValue::nar).isnar() && B840(SpecificValue::zero).iszero(), "nar and zero", reportTestCases);
		f += expect(B840(std::numeric_limits<double>::infinity()).isnar() && B840(std::nan("")).isnar(), "inf and NaN convert to NaR", reportTestCases);
		f += expect((-B840(5)).raw() == B840(-5).raw() && (-B840(SpecificValue::nar)).isnar() && (-B840(0)).iszero(), "negation", reportTestCases);
		f += expect(abs(B840(-5)) == B840(5), "abs", reportTestCases);
		nrOfFailedTestCases += ReportTestResult(f, "bposit<8,4,0>", "construction and special values");
	}

	{
		// saturation per the posit standard: never 0, never NaR
		int f = 0;
		f += expect(B840(1.0e-30) == B840(SpecificValue::minpos) && B840(-1.0e-30) == B840(SpecificValue::minneg), "below minpos clamps to +/-minpos", reportTestCases);
		f += expect(B840(1.0e30) == B840(SpecificValue::maxpos) && B840(-1.0e30) == B840(SpecificValue::maxneg), "above maxpos clamps to +/-maxpos", reportTestCases);
		f += expect(B840(SpecificValue::maxpos) * B840(2) == B840(SpecificValue::maxpos), "overflowing product saturates", reportTestCases);
		f += expect(B840(SpecificValue::minpos) / B840(4) == B840(SpecificValue::minpos), "underflowing quotient saturates", reportTestCases);
		nrOfFailedTestCases += ReportTestResult(f, "bposit<8,4,0>", "saturation");
	}

	{
		// regime cap: the extreme encodings keep F_min fraction bits
		int f = 0;
		f += expect(B840(SpecificValue::minpos).regime() == -4 && B840(SpecificValue::minpos).regime_size() == 4, "minpos uses the capped regime", reportTestCases);
		f += expect(B840(SpecificValue::maxpos).regime() == 3 && B840(SpecificValue::maxpos).regime_size() == 4, "maxpos uses the capped regime", reportTestCases);
		f += expect(fraction_bits(B840(SpecificValue::minpos)) == 3 && fraction_bits(B840(1)) == 5, "fraction bits at the extremes and at 1.0", reportTestCases);
		f += expect(to_binary(B840(1)) == "0b0.10..00000", "to_binary of 1.0", reportTestCases);
		f += expect(to_binary(bposit16(1)) == "0b0.10.00000.00000000", "to_binary of bposit16 1.0", reportTestCases);
		nrOfFailedTestCases += ReportTestResult(f, "bposit", "regime cap and fields");
	}

	{
		int f = 0;
		using L = std::numeric_limits<bposit16>;
		f += expect(double(L::epsilon()) == std::ldexp(1.0, -8) && L::digits == 9, "epsilon and digits", reportTestCases);
		f += expect(L::min() == bposit16(SpecificValue::minpos) && L::max() == bposit16(SpecificValue::maxpos) && L::lowest() == bposit16(SpecificValue::maxneg), "min, max, lowest", reportTestCases);
		f += expect(L::min_exponent == -191 && L::max_exponent == 192 && !L::has_infinity && L::quiet_NaN().isnar(), "exponent range, no infinity, NaR", reportTestCases);
		nrOfFailedTestCases += ReportTestResult(f, "bposit16", "numeric_limits");
	}

	{
		int f = 0;
		const bposit32 a(3.0), b(7.0);
		f += expect(double(a + b) == 10.0 && double(a * b) == 21.0 && double(b - a) == 4.0, "exact + * -", reportTestCases);
		f += expect(std::fabs(double(a / b) - 3.0 / 7.0) < 1.0e-7, "division", reportTestCases);
		f += expect(double(a + 1) == 4.0 && double(2.0 * a) == 6.0, "mixed with literals", reportTestCases);
		bposit32 c(1);
		++c;
		f += expect(c > bposit32(1) && (c - bposit32(1)) == std::numeric_limits<bposit32>::epsilon(), "++ steps by epsilon above 1.0", reportTestCases);
		nrOfFailedTestCases += ReportTestResult(f, "bposit32", "arithmetic");
	}

	{
		int f = 0;
		bool threw = false;
		try { bposit16 x = bposit16(1) / bposit16(0); (void)x; } catch (const bposit_divide_by_zero&) { threw = true; }
		f += expect(threw, "division by zero throws bposit_divide_by_zero", reportTestCases);
		threw = false;
		try { bposit16 x = bposit16(SpecificValue::nar) + bposit16(1); (void)x; } catch (const bposit_operand_is_nar&) { threw = true; }
		f += expect(threw, "a NaR operand throws bposit_operand_is_nar", reportTestCases);
		nrOfFailedTestCases += ReportTestResult(f, "bposit16", "arithmetic exceptions");
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
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
