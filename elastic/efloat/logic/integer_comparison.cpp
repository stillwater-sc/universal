// integer_comparison.cpp: efloat compared with an integer, exactly (#1852)
//
// An integer operand converts to an efloat exactly, all 64 bits, as it does in + - * /. These
// comparisons had gone wrong three ways: == did not compile (an int reached an undefined long long
// friend and the double overload at equal rank), == and != with a long long failed to link, and the
// other operators converted the integer to double, so 2^53 < 2^53 + 1 was false.
//
//   types       every integral type: the signed and unsigned integers, the character types and bool
//   operators   == != < > <= >=, efloat on the left and on the right
//   values      0, +-1, the 8-, 16-, 32- and 64-bit extremes and 2^53 - 1, 2^53, 2^53 + 1, each
//               against efloats holding those integers, those integers plus 1/2 and computed
//               through an addition (at 128 bits and more), +-0, +-inf and the NaNs
//   oracle      the integers compared as a sign and a 64-bit magnitude, not through efloat
//   constexpr   the comparisons in static_assert
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- every case above, at 32 and 128 bits.
//   LEVEL 2 -- as level 1, at 1024 bits as well.
//   LEVEL 3 -- as level 2.
//   LEVEL 4 -- as level 3.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using sw::universal::efloat;

	// an integer as a sign and a magnitude: every integral value fits
	struct wide {
		bool               negative;
		unsigned long long magnitude;
	};

	template<typename Integer>
	constexpr wide to_wide(Integer v) {
		if constexpr (std::is_signed_v<Integer>) {
			if (v < 0) return { true, 0ull - static_cast<unsigned long long>(static_cast<long long>(v)) };
		}
		return { false, static_cast<unsigned long long>(v) };
	}

	// -1, 0 or 1 as a < b, a == b, a > b
	constexpr int compare(wide a, wide b) {
		if (a.magnitude == 0 && b.magnitude == 0) return 0;
		if (a.negative != b.negative) return a.negative ? -1 : 1;
		const int m = (a.magnitude < b.magnitude) ? -1 : (a.magnitude > b.magnitude ? 1 : 0);
		return a.negative ? -m : m;
	}

	constexpr unsigned long long p53 = 1ull << 53;
	constexpr wide values[] = {
		{ false, 0ull }, { false, 1ull }, { true, 1ull }, { false, 2ull },
		{ false, 127ull }, { false, 128ull }, { true, 128ull }, { true, 129ull }, { false, 255ull }, { false, 256ull },
		{ false, 32767ull }, { true, 32768ull }, { false, 65535ull }, { false, 65536ull },
		{ false, 2147483647ull }, { true, 2147483648ull }, { false, 4294967295ull }, { false, 4294967296ull },
		{ false, p53 - 1 }, { false, p53 }, { false, p53 + 1 }, { true, p53 + 1 }, { true, p53 },
		{ false, 9223372036854775807ull }, { true, 9223372036854775808ull }, { false, 9223372036854775808ull },
		{ false, 18446744073709551615ull },
	};

	// the integer as an efloat, through the exact 64-bit constructors
	template<unsigned nlimbs>
	efloat<nlimbs> as_efloat(wide w) {
		if (w.negative) return efloat<nlimbs>(static_cast<long long>(0ull - w.magnitude));
		return efloat<nlimbs>(w.magnitude);
	}

	// whether w is a value of Integer, and that value
	template<typename Integer>
	bool representable(wide w) {
		using limits = std::numeric_limits<Integer>;
		if (w.negative) {
			if constexpr (!std::is_signed_v<Integer>) return false;
			else return w.magnitude - 1 <= static_cast<unsigned long long>(limits::max());
		}
		return w.magnitude <= static_cast<unsigned long long>(limits::max());
	}
	template<typename Integer>
	Integer narrow(wide w) {
		if (w.negative) return static_cast<Integer>(static_cast<long long>(0ull - w.magnitude));
		return static_cast<Integer>(w.magnitude);
	}

	// an efloat operand and how it compares with an integer: order(n) is -1, 0 or 1 as the efloat is
	// below, at or above n; unordered for a NaN
	struct operand {
		std::string name;
		bool        unordered;
		int         infinity;   // -1, 1, or 0 for a finite value
		wide        integer;    // the finite value's integer part, rounded toward -inf
		bool        half;       // the finite value is integer + 1/2
		int order(wide n) const {
			if (infinity != 0) return infinity;
			const int c = compare(integer, n);
			return half ? (c < 0 ? -1 : 1) : c;   // m + 1/2 against n: below when m < n, above otherwise
		}
	};

	template<unsigned nlimbs>
	struct case_set {
		std::vector<std::pair<efloat<nlimbs>, operand>> cases;
	};

	template<unsigned nlimbs>
	case_set<nlimbs> operands(bool with_fractions) {
		using Real = efloat<nlimbs>;
		case_set<nlimbs> s;
		for (const wide& w : values) {
			const std::string tag = std::string(w.negative ? "-" : "") + std::to_string(w.magnitude);
			const Real x = as_efloat<nlimbs>(w);
			s.cases.push_back({ x, operand{ tag, false, 0, w, false } });
			if (with_fractions) {
				// exact at 128 bits and more: m + 1/2, whose floor is m, and m computed through an
				// addition, with the limbs it leaves rather than those the constructor sets
				s.cases.push_back({ x + Real(0.5), operand{ tag + " + 1/2", false, 0, w, true } });
				s.cases.push_back({ (x + Real(0.25)) - Real(0.25), operand{ "(" + tag + " + 1/4) - 1/4", false, 0, w, false } });
			}
		}
		Real z; z.setzero();
		Real nz(-0.0);
		Real pinf; pinf.setinf(false);
		Real ninf; ninf.setinf(true);
		Real qnan; qnan.setnan(true);
		Real snan; snan.setnan(false);
		s.cases.push_back({ z,    operand{ "+0",   false, 0,  wide{ false, 0 }, false } });
		s.cases.push_back({ nz,   operand{ "-0",   false, 0,  wide{ false, 0 }, false } });
		s.cases.push_back({ pinf, operand{ "+inf", false, 1,  wide{ false, 0 }, false } });
		s.cases.push_back({ ninf, operand{ "-inf", false, -1, wide{ false, 0 }, false } });
		s.cases.push_back({ qnan, operand{ "qnan", true,  0,  wide{ false, 0 }, false } });
		s.cases.push_back({ snan, operand{ "snan", true,  0,  wide{ false, 0 }, false } });
		return s;
	}

	// the six operators, efloat on the left and on the right, against the oracle
	template<unsigned nlimbs, typename Integer>
	int VerifyType(const char* type, const case_set<nlimbs>& s, bool reportTestCases) {
		int fails = 0;
		for (const wide& w : values) {
			if (!representable<Integer>(w)) continue;
			const Integer n = narrow<Integer>(w);
			for (const auto& [x, op] : s.cases) {
				const bool un = op.unordered;
				const int  c  = op.unordered ? 0 : op.order(w);
				const bool got[12] = {
					x == n, x != n, x < n, x > n, x <= n, x >= n,
					n == x, n != x, n < x, n > x, n <= x, n >= x,
				};
				const bool want[12] = {
					!un && c == 0, un || c != 0, !un && c < 0, !un && c > 0, !un && c <= 0, !un && c >= 0,
					!un && c == 0, un || c != 0, !un && c > 0, !un && c < 0, !un && c >= 0, !un && c <= 0,
				};
				static const char* const names[12] = {
					"x == n", "x != n", "x < n", "x > n", "x <= n", "x >= n",
					"n == x", "n != x", "n < x", "n > x", "n <= x", "n >= x",
				};
				for (int i = 0; i < 12; ++i) {
					if (got[i] != want[i]) {
						++fails;
						if (reportTestCases) {
							std::cerr << "FAIL: efloat<" << nlimbs << "> x = " << op.name << ", " << type << " n = "
							          << (w.negative ? "-" : "") << w.magnitude << ": " << names[i] << " is " << got[i] << '\n';
						}
					}
				}
			}
		}
		return fails;
	}

	template<unsigned nlimbs>
	int VerifyIntegerComparisons(bool reportTestCases) {
		// at 32 bits an integer past 32 bits plus a fraction does not fit; the integers themselves do, exactly
		const case_set<nlimbs> s = operands<nlimbs>(nlimbs >= 4);
		int fails = 0;
		fails += VerifyType<nlimbs, signed char>("signed char", s, reportTestCases);
		fails += VerifyType<nlimbs, short>("short", s, reportTestCases);
		fails += VerifyType<nlimbs, int>("int", s, reportTestCases);
		fails += VerifyType<nlimbs, long>("long", s, reportTestCases);
		fails += VerifyType<nlimbs, long long>("long long", s, reportTestCases);
		fails += VerifyType<nlimbs, unsigned char>("unsigned char", s, reportTestCases);
		fails += VerifyType<nlimbs, unsigned short>("unsigned short", s, reportTestCases);
		fails += VerifyType<nlimbs, unsigned int>("unsigned int", s, reportTestCases);
		fails += VerifyType<nlimbs, unsigned long>("unsigned long", s, reportTestCases);
		fails += VerifyType<nlimbs, unsigned long long>("unsigned long long", s, reportTestCases);
		fails += VerifyType<nlimbs, char>("char", s, reportTestCases);
		fails += VerifyType<nlimbs, wchar_t>("wchar_t", s, reportTestCases);
#if defined(__cpp_char8_t)
		fails += VerifyType<nlimbs, char8_t>("char8_t", s, reportTestCases);
#endif
		fails += VerifyType<nlimbs, char16_t>("char16_t", s, reportTestCases);
		fails += VerifyType<nlimbs, char32_t>("char32_t", s, reportTestCases);
		fails += VerifyType<nlimbs, bool>("bool", s, reportTestCases);
		fails += VerifyType<nlimbs, std::int64_t>("int64_t", s, reportTestCases);
		fails += VerifyType<nlimbs, std::uint64_t>("uint64_t", s, reportTestCases);
		return fails;
	}

	// the comparisons in a constant evaluation
	int VerifyConstexpr() {
		using E = efloat<4>;
		constexpr long long n = 9007199254740993LL;   // 2^53 + 1: not a double
		constexpr E b(n - 1);
		static_assert(b < n && !(b >= n) && b != n && !(b == n) && b <= n && !(b > n), "2^53 against 2^53 + 1");
		static_assert(n > b && !(n <= b) && n != b && !(n == b) && n >= b && !(n < b), "2^53 + 1 against 2^53");
		static_assert(E(2) == 2 && 2 == E(2) && E(2) != 3 && 3 != E(2) && E(-1) < 0u && 0 > E(-1), "small integers");
		static_assert(E(1) == true && false == E(0) && E(0.5) < true && E(0.5) > false, "bool as 0 or 1");
		constexpr unsigned long long umax = std::numeric_limits<unsigned long long>::max();
		constexpr long long          smin = std::numeric_limits<long long>::min();
		static_assert(E(umax) == umax && E(umax) > smin && smin < E(umax) && E(smin) == smin && E(smin) < umax, "64-bit extremes");
		return 0;
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

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "efloat comparisons with integers";
	std::string test_tag            = "integer comparison";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyIntegerComparisons<4>(reportTestCases), "efloat<4>", test_tag);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyConstexpr(), "efloat<4>", "constexpr comparisons");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerComparisons<1>(reportTestCases), "efloat<1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerComparisons<4>(reportTestCases), "efloat<4>", test_tag);
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerComparisons<32>(reportTestCases), "efloat<32>", test_tag);
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
