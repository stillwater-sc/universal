// constexpr_constant.cpp: constexpr efloat variables at every precision (#1819, phase 4)
//
//   inline      an efloat whose limbs fit its inline buffer -- up to EFLOAT_INLINE_LIMBS 64-bit
//               limbs, 512 bits by default -- is itself a constexpr variable, in static_assert
//   constant    efloat_constant<nlimbs> holds an efloat<nlimbs> value of any precision in fixed
//               storage: a constexpr variable that turns back into the same efloat, its state,
//               sign, exponent, precision and limbs, checked against the run time at 32 to 3300
//               bits and for zero, -0, the infinities and the NaNs
//   rounding    a value with more limbs than the array holds is rounded to nlimbs * 32 bits, as
//               set_precision() rounds at run time; one that fits is held bit for bit, a 64-bit
//               native integer in efloat_constant<1> too
//   comparison  with itself, with efloat and with native types, NaN unordered, and + - * / with
//               them, in static_assert; a long double past double's range at run time
//   constants   pi, e and phi from their literals as efloat_constant<32> and <104> (1024 and
//               3300 bits), limb for limb the run-time constants
//
// pi, e and phi past 512 bits need more than clang's and MSVC's default constexpr step limits,
// which CMakeLists.txt raises for this test.
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- the cases above, the constants at 1024 bits.
//   LEVEL 2 -- the constants at 3300 bits as well.
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
#include <limits>
#include <type_traits>
#include <utility>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using sw::universal::efloat;
	using sw::universal::efloat_constant;

	// the state, sign, scale, precision and limbs of an efloat or an efloat_constant
	template<typename Value>
	struct digest {
		std::array<std::uint64_t, 64> word{};
		std::size_t n = 0;
		constexpr void put(std::uint64_t v) { if (n < word.size()) word[n] = v; ++n; }
		constexpr explicit digest(const Value& v) {
			put(static_cast<std::uint64_t>(v.scale()));
			put((v.iszero() ? 1u : 0u) | (v.isinf() ? 2u : 0u) | (v.isnan() ? 4u : 0u) | (v.sign() < 0 ? 8u : 0u));
			put(v.get_precision());
			put(v.limb_count());
			for (std::size_t i = 0; i < v.limb_count(); ++i) put(v.limb_data()[i]);
		}
		template<typename Other>
		constexpr bool operator==(const digest<Other>& rhs) const { return n == rhs.n && word == rhs.word; }
	};

	// num / den at `bits` bits
	template<unsigned nlimbs>
	constexpr efloat<nlimbs> ratio(int num, int den, unsigned bits) {
		efloat<nlimbs> v(num);
		v.set_precision(bits);
		v /= den;
		return v;
	}

	// an efloat past 512 bits has its limbs on the heap; up to 512 bits it is a constexpr variable
	int VerifyInline() {
		constexpr efloat<4>  third  = ratio<4>(1, 3, 128);
		constexpr efloat<16> seventh = ratio<16>(1, 7, 512);   // 8 limbs: the most the inline buffer holds
		static_assert(third.limb_count() == 2 && seventh.limb_count() == 8, "the limbs fit the inline buffer");
		static_assert(third > efloat<4>(0.333) && third < efloat<4>(0.334), "1/3");
		static_assert(seventh > efloat<16>(0.142857) && seventh < efloat<16>(0.142858), "1/7");
		static_assert(double(third) == 1.0 / 3.0, "1/3 as a double");
		return 0;
	}

	// the values of the round trips: fractions at 32 to 3300 bits, a native integer with no
	// precision of its own, and the special values
	template<unsigned nlimbs, unsigned bits>
	constexpr efloat<nlimbs> value_of(int k) {
		switch (k) {
		case 0: return ratio<nlimbs>(1, 3, bits);
		case 1: return ratio<nlimbs>(-22, 7, bits);
		case 2: return ratio<nlimbs>(355, 113, bits) * ratio<nlimbs>(1, 1000000, bits);
		case 3: return efloat<nlimbs>(0xFFFF'FFFF'FFFF'FFFFull);   // exact, no precision of its own: 64 bits in efloat<1>
		case 4: { efloat<nlimbs> v; v.setzero(); return v; }
		case 5: { efloat<nlimbs> v; v.setzero(); v.setsign(true); return v; }
		case 6: { efloat<nlimbs> v; v.setinf(false); return v; }
		case 7: { efloat<nlimbs> v; v.setinf(true); return v; }
		case 8: { efloat<nlimbs> v; v.setnan(true); return v; }
		default: { efloat<nlimbs> v; v.setnan(false); return v; }
		}
	}
	constexpr int nr_values = 10;

	template<unsigned nlimbs, unsigned bits, int k>
	int verify_round_trip(bool reportTestCases) {
		using D  = digest<efloat<nlimbs>>;
		constexpr efloat_constant<nlimbs> frozen = value_of<nlimbs, bits>(k);
		constexpr digest<efloat_constant<nlimbs>> held(frozen);
		static_assert(held.n <= held.word.size(), "the digest holds every word");
		const efloat<nlimbs> run = value_of<nlimbs, bits>(k);
		const efloat<nlimbs> back = frozen;
		int failures = 0;
		if (!(D(back) == D(run))) ++failures;     // the efloat it turns back into is the run-time value
		if (!(held == D(run))) ++failures;        // and so is what it holds
		if (failures != 0 && reportTestCases) {
			std::cerr << "FAIL: efloat_constant<" << nlimbs << "> of value " << k << " at " << bits << " bits differs from the run time\n";
		}
		return failures;
	}

	template<unsigned nlimbs, unsigned bits, int... k>
	int verify_round_trips(std::integer_sequence<int, k...>, bool reportTestCases) {
		return (verify_round_trip<nlimbs, bits, k>(reportTestCases) + ...);
	}

	template<unsigned nlimbs, unsigned bits>
	int VerifyRoundTrips(bool reportTestCases) {
		return verify_round_trips<nlimbs, bits>(std::make_integer_sequence<int, nr_values>{}, reportTestCases);
	}

	// a significand longer than nlimbs * 32 bits is rounded to it, as set_precision() rounds
	int VerifyRounding(bool reportTestCases) {
		constexpr efloat_constant<4> third = ratio<4>(1, 3, 1024);   // 16 limbs into 2
		static_assert(third.limb_count() == 2 && third.get_precision() == 128, "rounded to 128 bits");
		constexpr efloat_constant<4> two_thirds = ratio<4>(2, 3, 1024);
		static_assert(two_thirds.limb_count() == 2 && two_thirds > third && two_thirds < 1, "2/3, rounded to 128 bits");
		efloat<4> run = ratio<4>(1, 3, 1024);
		run.set_precision(128);
		efloat<4> run2 = ratio<4>(2, 3, 1024);
		run2.set_precision(128);
		int failures = 0;
		if (!(digest<efloat<4>>(efloat<4>(third)) == digest<efloat<4>>(run))) ++failures;
		if (!(digest<efloat<4>>(efloat<4>(two_thirds)) == digest<efloat<4>>(run2))) ++failures;
		if (failures != 0 && reportTestCases) std::cerr << "FAIL: efloat_constant rounding differs from set_precision()\n";
		return failures;
	}

	// comparisons with itself, with efloat and with native types; a NaN is unordered
	int VerifyComparisons() {
		using C = efloat_constant<32>;
		using E = efloat<32>;
		constexpr C third = ratio<32>(1, 3, 1024);
		constexpr C half  = E(0.5);
		constexpr C nan   = value_of<32, 1024>(8);
		static_assert(third < half && half > third && third <= third && third >= third && third == third && third != half, "with itself");
		static_assert(third < E(0.5) && E(0.25) < third && third != E(0.5) && E(0.5) == half, "with efloat");
		static_assert(half == 0.5 && 0.5 == half && third < 1 && 0 < third && third > 0.333f && half <= 0.5 && half >= 0.5, "with native types");
		static_assert(half == 0.5L && third < 0.5L && 0.25L < third, "with long double, also where it is double");
		constexpr C minus_one = E(-1);
		static_assert(minus_one == char(-1) || !std::is_signed_v<char>, "a negative plain char is negative");
		static_assert(E(char(-1)) == E(-1) || !std::is_signed_v<char>, "efloat(char(-1)) is -1 where char is signed");
		static_assert(minus_one == static_cast<signed char>(-1) && minus_one < 0u, "signed char; -1 below an unsigned 0");
		static_assert(!(nan == nan) && nan != nan && !(nan < half) && !(nan > half) && !(nan <= half) && !(nan >= half), "a NaN is unordered");
		// efloat's own >= with a NaN, against efloat and against double in either order
		static_assert(!(E(nan) >= E(0.5)) && !(E(0.5) >= E(nan)) && !(E(nan) >= 0.5) && !(0.5 >= E(nan)), "efloat: NaN >= is false");
		static_assert(double(half) == 0.5 && float(third) == 1.0f / 3.0f, "to double and float");
		// a table of constants, and one passed to a constexpr function
		constexpr std::array<C, 3> table{ { C(E(1)), third, half } };
		static_assert(table[0] > table[2] && table[2] > table[1], "an array of constants");
		constexpr auto twice = [](const C& c) { return C(E(c) * 2); };
		static_assert(twice(half) == 1 && twice(third) > half, "into and out of a constexpr function");
		// arithmetic with efloat_constant operands returns the efloat result
		constexpr C two_thirds = third + third;
		static_assert(two_thirds == third * 2 && 2 * third == two_thirds && two_thirds - third == third, "+ - *");
		// (an efloat compares with an int ambiguously, between its long long and double overloads, so
		// the efloat results compare with efloat operands)
		static_assert(half / half == E(1) && third / 2 < third && 1 / half == E(2) && -half == E(-0.5) && (half - E(0.5)).iszero(), "/ and unary -");
		return 0;
	}

	// a long double past double's range takes efloat's long double constructor, at run time
	int VerifyWideLongDouble() {
#if LONG_DOUBLE_SUPPORT
		if constexpr (std::numeric_limits<long double>::max_exponent > std::numeric_limits<double>::max_exponent) {
			const efloat_constant<4> big = efloat<4>(std::numeric_limits<double>::max()) * 2;
			const long double wider = static_cast<long double>(std::numeric_limits<double>::max()) * 4;
			// an exact value, so that a long double turned into infinity would fail
			const efloat_constant<4> exact = efloat<4>(std::numeric_limits<double>::max()) * 4;
			if (!(exact == wider && wider == exact)) return 1;
			if (!(big < wider && wider > big && big != wider)) return 1;
		}
#endif
		return 0;
	}

	template<unsigned nlimbs> constexpr efloat_constant<nlimbs> pi_constant()  { return sw::universal::efloat_pi<nlimbs>(); }
	template<unsigned nlimbs> constexpr efloat_constant<nlimbs> e_constant()   { return sw::universal::efloat_e<nlimbs>(); }
	template<unsigned nlimbs> constexpr efloat_constant<nlimbs> phi_constant() { return sw::universal::efloat_phi<nlimbs>(); }

	// pi, e and phi from their literals at nlimbs * 32 bits (at most 3300): limb for limb the run time's
	template<unsigned nlimbs>
	int VerifyConstants(bool reportTestCases) {
		constexpr efloat_constant<nlimbs> pi  = pi_constant<nlimbs>();
		constexpr efloat_constant<nlimbs> e   = e_constant<nlimbs>();
		constexpr efloat_constant<nlimbs> phi = phi_constant<nlimbs>();
		static_assert(pi > 3.14159 && pi < 3.14160 && e > 2.71828 && e < 2.71829 && phi > 1.61803 && phi < 1.61804, "pi, e, phi");
		static_assert(pi.limb_data()[pi.limb_count() - 1] >> 32 == 0xC90F'DAA2u, "pi = 11.0010010000111111011...");
		using D = digest<efloat<nlimbs>>;
		int failures = 0;
		if (!(digest<efloat_constant<nlimbs>>(pi) == D(sw::universal::efloat_pi<nlimbs>())))   ++failures;
		if (!(digest<efloat_constant<nlimbs>>(e) == D(sw::universal::efloat_e<nlimbs>())))     ++failures;
		if (!(digest<efloat_constant<nlimbs>>(phi) == D(sw::universal::efloat_phi<nlimbs>()))) ++failures;
		if (failures != 0 && reportTestCases) std::cerr << "FAIL: pi, e or phi as efloat_constant<" << nlimbs << "> differs from the run time\n";
		return failures;
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

	std::string test_suite          = "efloat constexpr variables";
	std::string test_tag            = "efloat_constant";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyInline(), "efloat", "inline constexpr variables");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrips<32, 1024>(reportTestCases), "efloat", "round trips at 1024 bits");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyInline(), "efloat", "inline constexpr variables");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrips<1, 32>(reportTestCases), "efloat", "round trips at 32 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrips<4, 128>(reportTestCases), "efloat", "round trips at 128 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrips<17, 544>(reportTestCases), "efloat", "round trips at 544 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrips<32, 1024>(reportTestCases), "efloat", "round trips at 1024 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrips<104, 3300>(reportTestCases), "efloat", "round trips at 3300 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyRounding(reportTestCases), "efloat", "a longer significand rounded");
	nrOfFailedTestCases += ReportTestResult(VerifyComparisons(), "efloat", "comparisons and arithmetic");
	nrOfFailedTestCases += ReportTestResult(VerifyWideLongDouble(), "efloat", "a long double past double's range");
	nrOfFailedTestCases += ReportTestResult(VerifyConstants<32>(reportTestCases), "efloat", "pi, e, phi at 1024 bits");
#	endif

#	if REGRESSION_LEVEL_2
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
