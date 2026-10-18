// logic.cpp: verify comparison operators of dfloat decimal floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/dfloat/dfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <algorithm>
#include <random>
#include <string>
#include <vector>

namespace {

using namespace sw::universal;

// The exact value of a non-NaN encoding of a narrow dfloat, as an integer in units of 10^emin,
// read from the decoded fields; the infinities map past every finite value.  It does not use the
// comparison operators, so it is an oracle for them (#1734).
template<typename Dfloat>
long long exact_value(const Dfloat& a) {
	static_assert(Dfloat::emax - Dfloat::emin <= 12 && Dfloat::ndigits <= 3, "the oracle needs value * 10^(emax - emin) to fit in 64 bits");
	if (a.isinf()) return a.sign() ? -(1ll << 62) : (1ll << 62);
	bool s; int e; typename Dfloat::significand_t sig;
	a.unpack(s, e, sig);
	long long v = 0;
	for (unsigned i = Dfloat::sig_bits; i-- > 0;) v = 2 * v + (sig.test(i) ? 1 : 0);
	for (int i = Dfloat::emin; i < e; ++i) v *= 10;
	return s ? -v : v;
}

// all six operators against the oracle's ordering of the two values
template<typename Dfloat>
bool comparisons_agree(const Dfloat& a, const Dfloat& b, int expected) {
	return (a == b) == (expected == 0) && (a != b) == (expected != 0) &&
	       (a < b) == (expected < 0) && (a > b) == (expected > 0) &&
	       (a <= b) == (expected <= 0) && (a >= b) == (expected >= 0);
}

// Every non-NaN encoding, grouped into classes of equal value: each encoding is compared with
// every member of its own class (the cohorts, and the zeros of every exponent and sign), and
// with the first members of the neighbouring classes; then random pairs.
template<typename Dfloat>
int VerifyComparisonsExhaustive(bool reportTestCases) {
	std::vector<std::pair<long long, Dfloat>> all;
	for (unsigned long long bits = 0; bits < (1ull << Dfloat::nbits); ++bits) {
		Dfloat a;
		a.setbits(bits);
		if (!a.isnan()) all.emplace_back(exact_value(a), a);
	}
	std::stable_sort(all.begin(), all.end(), [](const auto& x, const auto& y) { return x.first < y.first; });
	int fails = 0;
	auto check = [&](const Dfloat& a, const Dfloat& b, int expected) {
		if (!comparisons_agree(a, b, expected)) {
			if (reportTestCases && fails < 8) std::cerr << "FAIL: " << type_tag(a) << ' ' << to_binary(a) << " (" << a << ") vs " << to_binary(b) << " (" << b << ") expected " << expected << '\n';
			++fails;
		}
	};
	size_t begin = 0, classes = 0;
	while (begin < all.size()) {
		size_t end = begin;
		while (end < all.size() && all[end].first == all[begin].first) ++end;
		++classes;
		for (size_t i = begin; i < end; ++i) {
			for (size_t j = begin; j < end; ++j) check(all[i].second, all[j].second, 0);
			if (end < all.size()) {
				check(all[i].second, all[end].second, -1);
				check(all[end].second, all[i].second, 1);
			}
		}
		begin = end;
	}
	std::mt19937_64 rng(1734);
	std::uniform_int_distribution<size_t> pick(0, all.size() - 1);
	for (int n = 0; n < 20000; ++n) {
		const auto& x = all[pick(rng)];
		const auto& y = all[pick(rng)];
		check(x.second, y.second, (x.first > y.first) - (x.first < y.first));
	}
	if (classes < 2 || classes == all.size()) ++fails;   // the sweep must contain cohorts
	return fails;
}

// a decimal digit string doubled
std::string doubled(const std::string& digits) {
	std::string r(digits.size(), '0');
	int carry = 0;
	for (size_t i = digits.size(); i-- > 0;) {
		const int d = 2 * (digits[i] - '0') + carry;
		r[i] = static_cast<char>('0' + d % 10);
		carry = d / 10;
	}
	return carry ? "1" + r : r;
}

// In the wide formats the arithmetic makes the cohort members: x + x for an x whose last digit is
// 5 keeps x's exponent and ends in a 0, while the parser strips that 0 from the same value.
template<typename Dfloat>
int VerifyArithmeticCohorts(bool reportTestCases) {
	constexpr unsigned ndigits = Dfloat::ndigits;
	std::mt19937_64 rng(1734);
	std::uniform_int_distribution<int> digit(0, 9), lead(1, 4);
	std::uniform_int_distribution<int> exponent(Dfloat::emin + 2 * static_cast<int>(ndigits), Dfloat::emax - 2 * static_cast<int>(ndigits));
	int fails = 0, cohorts = 0;
	for (int n = 0; n < 500; ++n) {
		std::string digits(1, static_cast<char>('0' + lead(rng)));
		for (unsigned d = 1; d + 1 < ndigits; ++d) digits += static_cast<char>('0' + digit(rng));
		digits += '5';
		const std::string e = "e" + std::to_string(exponent(rng));
		const Dfloat x(digits + e);
		const Dfloat sum = x + x;
		const Dfloat parsed(doubled(digits) + e);
		if (to_binary(sum) != to_binary(parsed)) ++cohorts;
		Dfloat above(parsed), below(parsed);
		++above;
		--below;
		const Dfloat negsum = -sum, negparsed = -parsed;
		if (!comparisons_agree(sum, parsed, 0) || !comparisons_agree(parsed, sum, 0) ||
		    !comparisons_agree(negsum, negparsed, 0) || !comparisons_agree(sum, above, -1) ||
		    !comparisons_agree(sum, below, 1) || !comparisons_agree(negsum, -below, -1)) {
			if (reportTestCases && fails < 4) std::cerr << "FAIL: " << type_tag(x) << ' ' << digits << e << " + itself = " << sum << " vs " << parsed << '\n';
			++fails;
		}
	}
	if (cohorts == 0) ++fails;   // x + x must have produced encodings the parser does not
	return fails;
}

}  // anonymous namespace

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

	std::string test_suite = "dfloat<> comparison operator validation";
	std::string test_tag = "dfloat<> logic";
	bool reportTestCases = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using Decimal32 = dfloat<7, 6, DecimalEncoding::BID, uint32_t>;

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1

	// Test 1: Equality
	std::cout << "+---------    Equality tests\n";
	{
		Decimal32 a(42), b(42), c(43);
		if (!(a == b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 42 == 42\n";
		}
		if (a == c) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 42 != 43\n";
		}
	}

	// Test 2: Inequality
	std::cout << "+---------    Inequality tests\n";
	{
		Decimal32 a(42), b(43);
		if (!(a != b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 42 != 43\n";
		}
		Decimal32 c(42);
		if (a != c) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 42 == 42 via !=\n";
		}
	}

	// Test 3: Less than
	std::cout << "+---------    Less than tests\n";
	{
		Decimal32 a(10), b(20), c(-5);
		if (!(a < b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 10 < 20\n";
		}
		if (b < a) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 20 not < 10\n";
		}
		if (!(c < a)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: -5 < 10\n";
		}
	}

	// Test 4: Greater than
	std::cout << "+---------    Greater than tests\n";
	{
		Decimal32 a(100), b(50);
		if (!(a > b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 100 > 50\n";
		}
		if (b > a) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 50 not > 100\n";
		}
	}

	// Test 5: Less than or equal
	std::cout << "+---------    Less than or equal tests\n";
	{
		Decimal32 a(42), b(42), c(43);
		if (!(a <= b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 42 <= 42\n";
		}
		if (!(a <= c)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 42 <= 43\n";
		}
		if (c <= a) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 43 not <= 42\n";
		}
	}

	// Test 6: Greater than or equal
	std::cout << "+---------    Greater than or equal tests\n";
	{
		Decimal32 a(42), b(42), c(41);
		if (!(a >= b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 42 >= 42\n";
		}
		if (!(a >= c)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 42 >= 41\n";
		}
		if (c >= a) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 41 not >= 42\n";
		}
	}

	// Test 7: Zero comparisons
	std::cout << "+---------    Zero comparison tests\n";
	{
		Decimal32 zero(0), pos(1), neg(-1);
		if (!(zero == Decimal32(0))) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: zero == zero\n";
		}
		if (!(zero < pos)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 0 < 1\n";
		}
		if (!(neg < zero)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: -1 < 0\n";
		}
	}

	// Test 8: the members of a cohort, and the zeros of every exponent, compare by value (#1734)
	std::cout << "+---------    Cohort comparison tests\n";
	{
		const decimal64 sum = decimal64("1.25") + decimal64("1.25"), parsed("2.5");
		if (!(sum == parsed) || sum != parsed || sum < parsed || sum > parsed || !(sum <= parsed) || !(sum >= parsed) || !(sum == 2.5)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 1.25 + 1.25 == 2.5\n";
		}
	}
	nrOfFailedTestCases += ReportTestResult(VerifyComparisonsExhaustive<dfloat<3, 2, DecimalEncoding::BID, std::uint16_t>>(reportTestCases), "dfloat<3,2> BID", "exhaustive by value");
	nrOfFailedTestCases += ReportTestResult(VerifyComparisonsExhaustive<dfloat<3, 2, DecimalEncoding::DPD, std::uint16_t>>(reportTestCases), "dfloat<3,2> DPD", "exhaustive by value");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticCohorts<decimal32>(reportTestCases), "decimal32", "x + x cohorts");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticCohorts<decimal64>(reportTestCases), "decimal64", "x + x cohorts");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmeticCohorts<decimal128_dpd>(reportTestCases), "decimal128_dpd", "x + x cohorts");

	// Test 9: Negative number comparisons
	std::cout << "+---------    Negative number comparison tests\n";
	{
		Decimal32 a(-10), b(-5);
		if (!(a < b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: -10 < -5\n";
		}
		if (b < a) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: -5 not < -10\n";
		}
	}

	// Test 9: Zero-valued cohort encodings
	std::cout << "+---------    Zero-valued cohort encoding tests\n";
	{
		using TinyBID = dfloat<3, 2, DecimalEncoding::BID, uint16_t>;
		using TinyDPD = dfloat<3, 2, DecimalEncoding::DPD, uint16_t>;
		auto check_zero_encodings = [&](auto tag) {
			using Tiny = decltype(tag);
			for (uint64_t bits = 0; bits < (uint64_t(1) << Tiny::nbits); ++bits) {
				Tiny value;
				value.setbits(bits);
				bool sign;
				int exponent;
				typename Tiny::significand_t significand;
				value.unpack(sign, exponent, significand);
				const bool expected = !value.isinf() && !value.isnan() && significand.iszero();
				if (value.iszero() != expected) {
					++nrOfFailedTestCases;
					if (reportTestCases) std::cerr << "FAIL: zero classification for bits " << bits << '\n';
				}
				if (expected) {
					Tiny two(2);
#if DFLOAT_THROW_ARITHMETIC_EXCEPTION
					// the divide-by-zero path throws in this configuration
					bool threw = false;
					try {
						(void)(two / value);
					}
					catch (const sw::universal::dfloat_divide_by_zero&) {
						threw = true;
					}
					if (!threw) {
						++nrOfFailedTestCases;
						if (reportTestCases) std::cerr << "FAIL: division by encoded zero " << bits << " did not throw\n";
					}
#else
					if (!(two / value).isinf()) {
						++nrOfFailedTestCases;
						if (reportTestCases) std::cerr << "FAIL: division by encoded zero " << bits << '\n';
					}
#endif
				}
			}
		};
		check_zero_encodings(TinyBID{});
		check_zero_encodings(TinyDPD{});
	}

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
