// natural_nann.cpp: NaturalNumber's NaNN, Not a Natural Number (#2137)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// A NaturalNumber is {1, 2, 3, ...}, so its all-zeros encoding is never a value: it is NaNN, Not a
// Natural Number (USER design, #2137). Every result outside the domain is NaNN, and NaNN propagates:
//   - a result whose bits are all zero is NaNN by its encoding: 10 % 5, 7 * 0, 16 * 16 at 8 bits,
//     1 & 2, 5 ^ 5, 1 << nbits;
//   - + - | ^ ++ -- and unary minus keep a NaNN operand, and a difference below 1 is NaNN;
//   - x / NaNN and x % NaNN are NaNN, not a division by zero;
//   - a literal 0 operand is NaNN, so n + 0 and n / 0 are NaNN;
//   - nothing throws (decision 1a), with every integer exception enabled here;
//   - comparisons are posit-style: NaNN == NaNN, below every value;
//   - native casts: 0, negative, fractional and NaN inputs give NaNN; NaNN gives a quiet NaN, the
//     signed integer NaR min() and the unsigned max(); all ones is a value, so the cast cap is max;
//   - text: NaNN prints as "NaNN" and parses back; "0" and negative input parse as NaNN.
// This file replaces natural_divide_by_zero_optout.cpp, whose rules (x / 0 a silent 0, 1 / 2 a
// thrown domain error) are superseded.
#include <universal/utility/directives.hpp>
// every integer exception on: a NaNN result must not throw
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#define INTEGER_THROW_DIVIDE_BY_ZERO 1
#include <universal/number/integer/integer.hpp>
#include <universal/number/quire/quire.hpp>
#include <universal/verification/test_reporters.hpp>
#include <cmath>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

namespace sw { namespace universal {

template<typename Natural>
int VerifyNaNN(bool reportTestCases) {
	int fails = 0;
	auto expect = [&](bool ok, const char* what) {
		if (!ok) {
			++fails;
			if (reportTestCases) std::cerr << "FAIL: " << type_tag(Natural()) << ": " << what << '\n';
		}
	};
	try {
		constexpr unsigned nbits = Natural::nbits;
		Natural nann;
		nann.clear();
		const Natural one(1), two(2), three(3), five(5), seven(7), ten(10), sixteen(16);

		// the encoding
		expect(nann.isnan() && !one.isnan() && !five.isnan(), "isnan() is the all-zeros encoding");

		// results outside the domain, NaNN by their bits
		expect((ten % five).isnan(), "10 % 5 is NaNN");
		expect((seven * nann).isnan(), "7 * NaNN is NaNN");
		expect((one & two).isnan(), "1 & 2 is NaNN");
		expect((five ^ five).isnan(), "5 ^ 5 is NaNN");
		expect((one << static_cast<int>(nbits)).isnan(), "1 << nbits is NaNN");
		expect((one >> 1).isnan(), "1 >> 1 is NaNN");
		expect((one / two).isnan(), "1 / 2 is NaNN");
		expect((three - five).isnan() && (five - five).isnan(), "3 - 5 and 5 - 5 are NaNN");
		{ Natural x(one); --x; expect(x.isnan(), "--1 is NaNN"); }
		expect((-five).isnan(), "-5 is NaNN");
		if constexpr (nbits == 8) expect((sixteen * sixteen).isnan(), "16 * 16 wraps to NaNN at 8 bits");

		// NaNN propagates through every operator, from either side
		expect((nann + five).isnan() && (five + nann).isnan(), "+ propagates");
		expect((nann - five).isnan() && (five - nann).isnan(), "- propagates");
		expect((nann * five).isnan() && (five * nann).isnan(), "* propagates");
		expect((nann / five).isnan() && (five / nann).isnan(), "/ propagates; x / NaNN is not a division by zero");
		expect((nann % five).isnan() && (five % nann).isnan(), "% propagates");
		expect((nann | five).isnan() && (five | nann).isnan(), "| propagates");
		expect((nann ^ five).isnan() && (five ^ nann).isnan(), "^ propagates");
		expect((nann & five).isnan(), "& propagates");
		expect((nann << 1).isnan() && (nann >> 1).isnan(), "shifts propagate");
		{ Natural x(nann); ++x; Natural y(nann); --y; expect(x.isnan() && y.isnan(), "++ and -- propagate"); }

		// a literal 0 is NaNN
		expect((five + 0).isnan() && (five - 0).isnan() && (five * 0).isnan(), "n + 0, n - 0, n * 0 are NaNN");
		expect((five / 0).isnan() && (five % 0).isnan() && (five | 0).isnan(), "n / 0, n % 0, n | 0 are NaNN");
		{ Natural x(five); x += 0; Natural y(five); y /= 0; expect(x.isnan() && y.isnan(), "n += 0 and n /= 0 are NaNN"); }

		// valid arithmetic is unchanged
		expect(unsigned(seven / two) == 3u && unsigned(seven % two) == 1u, "7 / 2 is 3, 7 % 2 is 1");
		expect(unsigned(five - three) == 2u && unsigned(three + five) == 8u && unsigned(three * five) == 15u, "5 - 3, 3 + 5, 3 * 5");
		{ Natural x(two); --x; expect(x.isone(), "--2 is 1"); }

		// posit-style comparisons
		expect(nann == nann && !(nann != nann), "NaNN == NaNN");
		expect(nann < one && !(one < nann) && nann <= five && !(nann > five), "NaNN is below every value");

		// native conversions in
		expect(Natural(0).isnan() && Natural(-4).isnan() && Natural(0u).isnan(), "0, -4 and 0u convert to NaNN");
		expect(Natural(std::numeric_limits<double>::quiet_NaN()).isnan(), "NaN converts to NaNN");
		expect(Natural(-3.0).isnan() && Natural(0.5).isnan(), "-3.0 and 0.5 convert to NaNN");
		expect(Natural(1.5).isone(), "1.5 truncates to 1");
		{
			Natural allOnes;
			allOnes.clear();
			allOnes.flip();
			expect(Natural(1e300) == allOnes, "past the range saturates to max(), all ones, a value");
			if constexpr (nbits <= 52) expect(Natural(std::ldexp(1.0, static_cast<int>(nbits)) - 1.0) == allOnes, "2^nbits - 1 is exact");
		}
		// and out
		expect(std::isnan(double(nann)), "NaNN converts to a quiet NaN");
		expect(int(nann) == std::numeric_limits<int>::min() && unsigned(nann) == std::numeric_limits<unsigned>::max(), "NaNN to native integers is the integer NaR");

		// text
		{
			std::ostringstream s;
			s << nann;
			expect(s.str() == "NaNN" && to_string(nann) == "NaNN", "NaNN prints as NaNN");
			Natural p(five), q(five), r(five), w;
			expect(parse(std::string("NaNN"), p) && p.isnan(), "\"NaNN\" parses to NaNN");
			expect(parse(std::string("0"), q) && q.isnan(), "\"0\" parses to NaNN");
			expect(parse(std::string("-10"), r) && r.isnan(), "\"-10\" parses to NaNN");
			expect(parse(std::string("105"), w) && unsigned(w) == 105u, "\"105\" parses to 105");
		}

		// the division helpers take a NaNN divisor as the operators do, not as a division by zero
		{
			const auto d = idiv(seven, nann);
			Natural r(five), q(five), rr(five);
			remainder(r, seven, nann);
			q.reduce(seven, nann, rr);
			expect(d.quot.isnan() && d.rem.isnan() && r.isnan() && q.isnan() && rr.isnan(), "idiv, remainder and reduce with a NaNN divisor give NaNN");
		}
		// plain char follows its signedness: where char is signed, char(-1) is NaNN
		if constexpr (std::is_signed_v<char>) {
			const char minusOne = static_cast<char>(-1);
			expect(Natural(minusOne).isnan(), "char(-1) is NaNN where char is signed");
		}
		expect(unsigned(Natural(static_cast<char>(100))) == 100u, "char(100) is 100");
		// twosComplement() is raw bit arithmetic: all ones flips to the 0 encoding, and incrementing
		// it must give 1, not keep NaNN as the public ++ does
		{
			Natural allOnes;
			allOnes.clear();
			allOnes.flip();
			Natural t(allOnes);
			t.twosComplement();
			expect(t.isone() && twosComplement(allOnes).isone(), "twosComplement(all ones) is 1, member and free helper");
		}

		// a NaNN operand makes a quire NaN: normalize() maps it to a zero blocktriple, which would add 0
		if constexpr (nbits <= 64) {
			quire<Natural> q;
			q = nann;
			const bool assigned = q.isnan();
			q.clear();
			q = five;
			q += nann;
			const bool added = q.isnan();
			q.clear();
			q = five;
			q -= nann;
			expect(assigned && added && q.isnan(), "quire = NaNN, += NaNN, -= NaNN give a NaN quire");
		}

		// a WholeNumber keeps its plain-char conversion: char(-1) is the unsigned value, not a throw inside
		// the noexcept constructor (this file enables INTEGER_THROW_ARITHMETIC_EXCEPTION)
		{
			using Whole = integer<nbits, typename Natural::BlockType, IntegerNumberType::WholeNumber>;
			const Whole w(static_cast<char>(-1));
			expect(!w.iszero(), "whole(char(-1)) converts without throwing, as before");
		}

		// numeric_limits
		using L = std::numeric_limits<Natural>;
		expect(L::has_quiet_NaN && L::quiet_NaN().isnan(), "has_quiet_NaN, and quiet_NaN() is NaNN");
		expect(L::min().isone(), "min() is 1");
	}
	catch (const std::exception& e) {
		++fails;
		if (reportTestCases) std::cerr << "FAIL: " << type_tag(Natural()) << ": a NaNN result threw: " << e.what() << '\n';
	}
	return fails;
}

} } // namespace sw::universal

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

	std::string test_suite  = "integer NaturalNumber NaNN, Not a Natural Number (#2137)";
	std::string test_tag    = "NaNN";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyNaNN< integer<16, std::uint8_t, IntegerNumberType::NaturalNumber> >(true), "integer<16,uint8_t,Natural>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	// single limb of exactly nbits, a partial limb, several limbs, and wider than 64 bits
	nrOfFailedTestCases += ReportTestResult(VerifyNaNN< integer<  8, std::uint8_t , IntegerNumberType::NaturalNumber> >(reportTestCases), "integer<8,uint8_t,Natural>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNN< integer< 16, std::uint16_t, IntegerNumberType::NaturalNumber> >(reportTestCases), "integer<16,uint16_t,Natural>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNN< integer< 16, std::uint8_t , IntegerNumberType::NaturalNumber> >(reportTestCases), "integer<16,uint8_t,Natural>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNN< integer< 20, std::uint32_t, IntegerNumberType::NaturalNumber> >(reportTestCases), "integer<20,uint32_t,Natural>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNN< integer<128, std::uint32_t, IntegerNumberType::NaturalNumber> >(reportTestCases), "integer<128,uint32_t,Natural>", test_tag);
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
catch (const sw::universal::integer_arithmetic_exception& err) {
	std::cerr << "Caught unexpected integer arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::integer_internal_exception& err) {
	std::cerr << "Caught unexpected integer internal exception: " << err.what() << std::endl;
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
