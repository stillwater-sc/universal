// constexpr_parse.cpp: decimal literals read by the compiler, and values handed out of a constant
// evaluation (#1819, phase 5)
//
//   agreement    parse() of a set of literals -- short and long, tiny and huge, rounding ties, the
//                special tokens and malformed text -- each a constant evaluation of its own,
//                against the run time: the result, the limbs, the scale, the sign, the state and
//                the precision are the same
//   exact cases  the values parse() reads exactly, ties to even, and its failures, in static_assert
//   constants    efloat_pi, efloat_e and efloat_phi by the compiler, bit for bit the run time's,
//                and pi from its literal equal to pi computed by acos(-1)
//   expansion    to_double_expansion() in static_assert: pi as a dd and as a qd, equal to
//                dd_pi and to qd_pi's first three components (its fourth is two units off)
//
// The constants past a few hundred bits need more than clang's and MSVC's default constexpr step
// limits, which CMakeLists.txt raises for this test (-fconstexpr-steps, /constexpr:steps).
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- the literals at 64 and 128 bits, the exact cases, the constants at 128 and 1024
//              bits, and the expansions.
//   LEVEL 2 -- the literals at 512 bits as well, and the constants at 3300 bits, the most their
//              literals hold.
//   LEVEL 3 -- as level 2.
//   LEVEL 4 -- as level 3.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string_view>
#include <utility>
#include <universal/number/efloat/efloat.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using sw::universal::efloat;

	// whether parse() succeeded, and the state, sign, scale, precision and limbs of its value
	struct digest {
		std::array<std::uint64_t, 64> word{};
		std::size_t n = 0;
		constexpr void put(std::uint64_t v) { if (n < word.size()) word[n] = v; ++n; }
		template<unsigned nlimbs>
		constexpr void put(const efloat<nlimbs>& v) {
			put(static_cast<std::uint64_t>(v.scale()));
			put((v.iszero() ? 1u : 0u) | (v.isinf() ? 2u : 0u) | (v.isnan() ? 4u : 0u) | (v.sign() < 0 ? 8u : 0u));
			put(v.get_precision());
			put(v.limb_count());
			for (std::size_t i = 0; i < v.limb_count(); ++i) put(v.limb_data()[i]);
		}
		constexpr bool operator==(const digest&) const = default;
	};

	// short and long digit strings, the 1000-digit pi literal, exponents of both signs and of the
	// double range's ends, midpoints between two values (ties to even), a literal past the
	// converter's reach, digits past the precision on a rounding boundary, whitespace, the special
	// tokens and text that is not a number
	constexpr std::array<std::string_view, 24> literals{ {
		"0.1", "-3.25", "  42  ", "1e-300", "6.02214076e23", "-0", "0e5", "000.000123",
		"18446744073709551617",      // 2^64 + 1: the midpoint, rounds to the even 2^64 at 64 bits
		"18446744073709551619",      // 2^64 + 3: the midpoint, rounds up to 2^64 + 4
		"4.9406564584124654e-324", "1.7976931348623157e308", "2.5e-400", "1.5e400", "1e100000",
		"0.333333333333333333333333333333333333333333333333333333333333333333333333333333333333",
		"1.00000000000000000000000000000000000000000000000000000000000000000000000000000000000001",
		"0.50000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001e-5",
		"nan", "-Infinity", "inf", "garbage", "",
		sw::universal::EFLOAT_PI_DIGITS   // 1011 digits: all but the leading ones dropped to a sticky bit
	} };

	template<unsigned bits, std::size_t i>
	constexpr digest parsed() {
		efloat<4> v;
		v.set_precision(bits);
		digest d;
		d.put(sw::universal::parse(literals[i], v) ? 1u : 0u);
		d.put(v);
		return d;
	}

	// one literal: read by the compiler, a constant evaluation of its own, and at run time
	template<unsigned bits, std::size_t i>
	int verify_one(bool reportTestCases) {
		constexpr digest compiled = parsed<bits, i>();
		static_assert(compiled.n <= compiled.word.size(), "the digest holds every word");
		const digest run = parsed<bits, i>();
		if (compiled == run) return 0;
		if (reportTestCases) {
			std::cerr << "FAIL: parse(\"" << literals[i] << "\") at " << bits << " bits: constant evaluation differs from the run time\n";
		}
		return 1;
	}

	template<unsigned bits, std::size_t... i>
	int verify_all(std::index_sequence<i...>, bool reportTestCases) {
		return (verify_one<bits, i>(reportTestCases) + ...);
	}

	// every literal at `bits` bits
	template<unsigned bits>
	int VerifyLiterals(bool reportTestCases) {
		return verify_all<bits>(std::make_index_sequence<literals.size()>{}, reportTestCases);
	}

	// parse(text) at `bits` bits
	template<unsigned bits>
	constexpr efloat<4> read(std::string_view text) {
		efloat<4> v;
		v.set_precision(bits);
		sw::universal::parse(text, v);
		return v;
	}

	template<unsigned bits>
	constexpr bool reads(std::string_view text) {
		efloat<4> v;
		v.set_precision(bits);
		return sw::universal::parse(text, v);
	}

	// the values parse() reads exactly, the ties, and its failures
	int VerifyStaticAsserts() {
		static_assert(double(read<128>("3.25")) == 3.25 && double(read<128>("-0.125e3")) == -125.0, "exact literals");
		static_assert(double(read<64>("0.1")) == 0.1 && double(read<128>("1e-300")) == 1e-300, "correctly rounded");
		// at 64 bits (two 32-bit limbs), the midpoints above 2^64 round to the even neighbour
		constexpr efloat<4> two64 = efloat<4>(std::uint64_t(1) << 63) * 2;
		static_assert(read<64>("18446744073709551617") == two64, "a tie rounds to even, down");
		static_assert(read<64>("18446744073709551619") - two64 == efloat<4>(4), "a tie rounds to even, up");
		static_assert(read<64>("18446744073709551617.00000000000000000000000000000000000001") - two64 == efloat<4>(2), "past the tie: up");
		static_assert(read<64>("18446744073709551616.99999999999999999999999999999999999999") == two64, "below the tie: down");
		static_assert(double(read<128>(" \t-2.5\n")) == -2.5, "surrounding whitespace");
		static_assert(read<128>("-0").iszero() && read<128>("-0").sign() < 0, "-0");
		static_assert(read<128>("NaN").isnan() && read<128>("-inf").isinf() && read<128>("-inf").sign() < 0, "the special tokens");
		static_assert(read<128>("1e1000").scale() == 3321, "past the double range");
		static_assert(!reads<128>("garbage") && read<128>("garbage").iszero(), "not a number: false, and a zero");
		static_assert(!reads<128>("") && !reads<128>("1.5e") && !reads<128>("--1"), "malformed text");
		static_assert(!reads<128>("1e100000") && read<128>("1e100000").isnan(), "past the converter's reach: false, and a NaN");
		return 0;
	}

	// the limbs of a value, least significant first
	template<std::size_t K>
	struct limbs {
		std::array<std::uint64_t, K> limb{};
		std::int64_t scale = 0;
		unsigned precision = 0;
		constexpr bool operator==(const limbs&) const = default;
	};

	template<std::size_t K, unsigned nlimbs>
	constexpr limbs<K> limbs_of(const efloat<nlimbs>& v) {
		limbs<K> r;
		for (std::size_t i = 0; i < v.limb_count() && i < K; ++i) r.limb[i] = v.limb_data()[i];
		r.scale = v.scale();
		r.precision = v.get_precision();
		return r;
	}

	template<unsigned nlimbs> constexpr efloat<nlimbs> pi_value()  { return sw::universal::efloat_pi<nlimbs>(); }
	template<unsigned nlimbs> constexpr efloat<nlimbs> e_value()   { return sw::universal::efloat_e<nlimbs>(); }
	template<unsigned nlimbs> constexpr efloat<nlimbs> phi_value() { return sw::universal::efloat_phi<nlimbs>(); }

	// pi, e and phi of efloat<nlimbs> (nlimbs * 32 bits, at most 3300): by the compiler, and bit for
	// bit the run time's
	template<unsigned nlimbs>
	int VerifyConstants(bool reportTestCases) {
		constexpr std::size_t K = (std::min(nlimbs * 32u, 3300u) + 63u) / 64u;
		constexpr limbs<K> pi  = limbs_of<K>(pi_value<nlimbs>());
		constexpr limbs<K> e   = limbs_of<K>(e_value<nlimbs>());
		constexpr limbs<K> phi = limbs_of<K>(phi_value<nlimbs>());
		static_assert(pi.limb[K - 1] >> 32 == 0xC90F'DAA2u && pi.scale == 1, "pi = 11.0010010000111111011...");
		static_assert(e.limb[K - 1] >> 32 == 0xADF8'5458u && e.scale == 1, "e = 10.1011011111100001010...");
		static_assert(phi.limb[K - 1] >> 32 == 0xCF1B'BCDCu && phi.scale == 0, "phi = 1.1001111000110111011...");
		const limbs<K> compiled[3] = { pi, e, phi };
		const limbs<K> run[3] = { limbs_of<K>(sw::universal::efloat_pi<nlimbs>()), limbs_of<K>(sw::universal::efloat_e<nlimbs>()),
		                          limbs_of<K>(sw::universal::efloat_phi<nlimbs>()) };
		const char* names[3] = { "pi", "e", "phi" };
		int failures = 0;
		for (int k = 0; k < 3; ++k) {
			if (!(compiled[k] == run[k])) {
				++failures;
				if (reportTestCases) std::cerr << "FAIL: efloat_" << names[k] << "<" << nlimbs << ">: constant evaluation differs from the run time\n";
			}
		}
		return failures;
	}

	// pi read from its literal and pi computed as acos(-1), both at 1024 bits by the compiler: the
	// two round correctly, so they are the same number
	int VerifyPiTwoWays() {
		constexpr limbs<16> literal = limbs_of<16>(pi_value<32>());
		constexpr limbs<16> computed = limbs_of<16>([] { efloat<32> m(-1); m.set_precision(1024); return acos(m); }());
		static_assert(literal == computed, "pi from its literal is acos(-1)");
		return 0;
	}

	// to_double_expansion(): an efloat as the doubles of a dd and a qd, in a constant evaluation
	int VerifyExpansion() {
		using sw::universal::to_double_expansion;
		constexpr std::array<double, 2> pi2 = to_double_expansion<2>(pi_value<4>());
		constexpr sw::universal::dd pi_dd(pi2[0], pi2[1]);
		static_assert(pi_dd.high() == sw::universal::dd_pi.high() && pi_dd.low() == sw::universal::dd_pi.low(), "pi as a dd");
		constexpr std::array<double, 4> pi4 = to_double_expansion<4>(pi_value<12>());
		constexpr sw::universal::qd pi_qd(pi4[0], pi4[1], pi4[2], pi4[3]);
		// qd_pi's last component is 0x1.4cf98e804177fp-163, two units above the nearest double to
		// pi - x0 - x1 - x2, which mpmath gives as 0x1.4cf98e804177dp-163
		static_assert(pi_qd[0] == sw::universal::qd_pi[0] && pi_qd[1] == sw::universal::qd_pi[1] &&
		              pi_qd[2] == sw::universal::qd_pi[2] && pi_qd[3] == 0x1.4cf98e804177dp-163, "pi as a qd");
		constexpr std::array<double, 3> third = to_double_expansion<3>(efloat<4>(1) / efloat<4>(3));
		static_assert(third[0] == 1.0 / 3.0 && third[1] == (1.0 / 3.0) * 0x1p-54 && third[2] != 0.0, "1/3: three nonzero components");
		constexpr std::array<double, 3> half = to_double_expansion<3>(efloat<4>(0.5));
		static_assert(half[0] == 0.5 && half[1] == 0.0 && half[2] == 0.0, "an exact double: one component");
		constexpr std::array<double, 2> huge = to_double_expansion<2>(read<128>("1e400"));
		static_assert(huge[0] > 1.7976931348623157e308 && huge[1] == 0.0, "past the double range: inf, then 0");
		return 0;
	}

}  // namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "efloat constexpr parse verification";
	std::string test_tag            = "constexpr parse";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyStaticAsserts(), "efloat", "static_assert");
	nrOfFailedTestCases += ReportTestResult(VerifyLiterals<128>(reportTestCases), "efloat", "literals at 128 bits");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyStaticAsserts(), "efloat", "static_assert");
	nrOfFailedTestCases += ReportTestResult(VerifyLiterals<64>(reportTestCases), "efloat", "literals at 64 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyLiterals<128>(reportTestCases), "efloat", "literals at 128 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyConstants<4>(reportTestCases), "efloat", "pi, e, phi at 128 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyConstants<32>(reportTestCases), "efloat", "pi, e, phi at 1024 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyPiTwoWays(), "efloat", "pi from its literal = acos(-1)");
	nrOfFailedTestCases += ReportTestResult(VerifyExpansion(), "efloat", "to_double_expansion");
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyLiterals<512>(reportTestCases), "efloat", "literals at 512 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyConstants<104>(reportTestCases), "efloat", "pi, e, phi at 3300 bits");
#	endif

#	if REGRESSION_LEVEL_3
#	endif

#	if REGRESSION_LEVEL_4
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
