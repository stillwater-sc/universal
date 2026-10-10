// api.cpp: application programming interface of the poxel
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#define POXEL_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/poxel/poxel.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

int expect(bool ok, const char* what, bool report) {
	if (ok) return 0;
	if (report) std::cerr << "FAIL: " << what << '\n';
	return 1;
}

// File-level helper for the constexpr smoke tests below: compares the raw
// bit pattern of two posits limb-by-limb. Used by both the integer and the
// IEEE-754 constexpr blocks.
template<typename Poxel>
bool poxel_same_bits(const Poxel& a, const Poxel& b) {
	auto               ab       = a.bits();
	auto               bb       = b.bits();
	constexpr unsigned nrBlocks = decltype(ab)::nrBlocks;
	for (unsigned i = 0; i < nrBlocks; ++i) {
		if (ab.block(i) != bb.block(i))
			return false;
	}
	return true;
}

// constexpr encoder: poxel<9,2> is the posit<8,2> lattice plus a ubit
using X9 = sw::universal::poxel<9, 2, std::uint16_t>;
static_assert(X9::encode(false, 0, 0ull, 0u, false) == 0x080, "1.0 is the exact tile 2 * 0x40");
static_assert(X9::encode(false, 0, 1ull, 8u, false) == 0x081, "a hair above 1.0 is the open tile above it");
static_assert(X9::encode(true, 0, 0ull, 0u, false) == 0x180, "-1.0 is the two's complement of 0x080");
static_assert(X9::encode(false, 200, 0ull, 0u, false) == 0x0FF, "above maxpos: (maxpos, inf)");
static_assert(X9::encode(false, -200, 0ull, 0u, false) == 0x001, "below minpos: (0, minpos)");
static_assert(X9::encode(true, -200, 0ull, 0u, false) == 0x1FF, "above -minpos: (-minpos, 0)");
static_assert(X9::encode(true, 200, 0ull, 0u, false) == 0x101, "below -maxpos: (-inf, -maxpos)");

}  // anonymous namespace

int main()
try {
	using namespace sw::universal;
	std::string test_suite  = "poxel API";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;
	ReportTestSuiteHeader(test_suite, reportTestCases);

	ReportTrivialityOfType<poxel33>();

	/////////////////////////////////////////////////////////////////////////////////////
	//// posit construction, initialization, assignment and comparisions

	std::cout << "+-----------------   constexpr integer construction\n";
	{
		// constexpr construction from integer literals must succeed at compile time
		// for nbits <= 64. The encoded bit pattern must match an INDEPENDENT
		// reference path (convert_ieee754 via double-cast) so that a bug in
		// encode_positive_uint64 cannot mask itself.
		// All test values fit exactly in double's 53-bit mantissa, so the double
		// cast is bit-exact and provides a valid reference.
		constexpr poxel<33, 2, std::uint64_t> cx_pos_42(42);
		constexpr poxel<33, 2>                cx_neg_42(-42);
		constexpr poxel<33, 2>                cx_zero(0);
		constexpr poxel<9, 0>                 cx_three(3);
		constexpr poxel<17, 1>                cx_kilo(1024);
		constexpr poxel<64, 3>                cx_big(123456789LL);
		constexpr poxel<9, 0>                 cx_sat_pos(1000);   // saturates to maxpos
		constexpr poxel<9, 0>                 cx_sat_neg(-1000);  // saturates to maxneg
		constexpr poxel<33, 2>                cx_int_min(int32_t(-2147483647 - 1));

		// Reference path: float literal constructor still routes through the
		// pre-existing convert_ieee754, independent of the new constexpr code.
		poxel<33, 2, std::uint64_t> ref_pos_42(42.0);
		poxel<33, 2>                ref_neg_42(-42.0);
		poxel<33, 2>                ref_zero(0.0);
		poxel<9, 0>                 ref_three(3.0);
		poxel<17, 1>                ref_kilo(1024.0);
		poxel<64, 3>                ref_big(123456789.0);
		poxel<9, 0>                 ref_sat_pos(1000.0);
		poxel<9, 0>                 ref_sat_neg(-1000.0);
		poxel<33, 2>                ref_int_min(static_cast<double>(int32_t(-2147483647 - 1)));

		int f = nrOfFailedTestCases;
		if (!poxel_same_bits(cx_pos_42,  ref_pos_42))  { ++f; std::cout << "FAIL constexpr poxel<33,2>(42)\n"; }
		if (!poxel_same_bits(cx_neg_42,  ref_neg_42))  { ++f; std::cout << "FAIL constexpr poxel<33,2>(-42)\n"; }
		if (!poxel_same_bits(cx_zero,    ref_zero))    { ++f; std::cout << "FAIL constexpr poxel<33,2>(0)\n"; }
		if (!poxel_same_bits(cx_three,   ref_three))   { ++f; std::cout << "FAIL constexpr poxel<9,0>(3)\n"; }
		if (!poxel_same_bits(cx_kilo,    ref_kilo))    { ++f; std::cout << "FAIL constexpr poxel<17,1>(1024)\n"; }
		if (!poxel_same_bits(cx_big,     ref_big))     { ++f; std::cout << "FAIL constexpr poxel<64,3>(123456789)\n"; }
		if (!poxel_same_bits(cx_sat_pos, ref_sat_pos)) { ++f; std::cout << "FAIL constexpr poxel<9,0>(1000) sat\n"; }
		if (!poxel_same_bits(cx_sat_neg, ref_sat_neg)) { ++f; std::cout << "FAIL constexpr poxel<9,0>(-1000) sat\n"; }
		if (!poxel_same_bits(cx_int_min, ref_int_min)) { ++f; std::cout << "FAIL constexpr poxel<33,2>(INT_MIN)\n"; }
		nrOfFailedTestCases += ReportTestResult(f, "poxel", "constexpr construction");
	}

		std::cout << "+-----------------   constexpr arithmetic + comparison\n";
#if BIT_CAST_IS_CONSTEXPR
	{
		// The #718 acceptance form: constexpr posit<32,2>(2.0) * posit<32,2>(3.0) etc.
		// Promotion of the entire decode/normalize/blocktriple chain to constexpr
		// makes posit a true plug-in for native float in constexpr contexts.
		constexpr poxel<32, 2> a(2.0);
		constexpr poxel<32, 2> b(3.0);
		constexpr auto         cx_sum  = a + b;
		constexpr auto         cx_diff = a - b;
		constexpr auto         cx_prod = a * b;
		constexpr auto         cx_quot = b / a;
		constexpr auto         cx_neg  = -a;

		// Compound forms via lambda
		constexpr poxel<32, 2> cx_addeq = []() {
			poxel<32, 2> t(2.0);
			t += poxel<32, 2>(3.0);
			return t;
		}();
		constexpr poxel<32, 2> cx_muleq = []() {
			poxel<32, 2> t(2.0);
			t *= poxel<32, 2>(3.0);
			return t;
		}();

		// Constexpr comparisons
		constexpr bool eq = (a == b);
		constexpr bool lt = (a < b);
		constexpr bool ge = (b >= a);
		static_assert(!eq, "constexpr a == b is false for 2 vs 3");
		static_assert(lt, "constexpr a < b is true for 2 < 3");
		static_assert(ge, "constexpr b >= a is true for 3 >= 2");

		// Cross-check constexpr-evaluated values match runtime-computed values bit-for-bit
		poxel<32, 2> ra(2.0), rb(3.0);
		poxel<32, 2> rsum  = ra + rb;
		poxel<32, 2> rprod = ra * rb;

		int start = nrOfFailedTestCases;
		if (!poxel_same_bits(cx_sum, rsum)) {
			++nrOfFailedTestCases;
			std::cout << "FAIL constexpr poxel + matches runtime\n";
		}
		if (!poxel_same_bits(cx_prod, rprod)) {
			++nrOfFailedTestCases;
			std::cout << "FAIL constexpr poxel * matches runtime\n";
		}
		if (!poxel_same_bits(cx_addeq, rsum)) {
			++nrOfFailedTestCases;
			std::cout << "FAIL constexpr poxel += matches runtime +\n";
		}
		if (!poxel_same_bits(cx_muleq, rprod)) {
			++nrOfFailedTestCases;
			std::cout << "FAIL constexpr posit *= matches runtime *\n";
		}

		// Exact arithmetic checks (these values fit poxel<32,2> exactly)
		poxel<32, 2> r5(5.0), r6(6.0);
		if (!poxel_same_bits(cx_sum, r5)) {
			++nrOfFailedTestCases;
			std::cout << "FAIL constexpr 2+3 == 5 exactly\n";
		}
		if (!poxel_same_bits(cx_prod, r6)) {
			++nrOfFailedTestCases;
			std::cout << "FAIL constexpr 2*3 == 6 exactly\n";
		}

		(void) cx_diff;
		(void) cx_quot;
		(void) cx_neg;  // suppress unused -- the constexpr eval IS the test
		if (nrOfFailedTestCases - start == 0)
			std::cout << "PASS constexpr arithmetic + comparison\n";
	}
#else
	{
		std::cout << "SKIP constexpr arithmetic + comparison (compiler lacks constexpr bit_cast support)\n";
	}
#endif

	std::cout << "+-----------------  type tag, is_poxel trait, to_binary, to_interval\n";
	{
		int f = 0;
		f += expect(type_tag(poxel17{}) == "poxel< 17, 2, uint32_t>", "type_tag", reportTestCases);
		f += expect(is_poxel<poxel9> && !is_poxel<double>, "is_poxel trait", reportTestCases);
		f += expect(to_binary(X9(1.0)) == "0b0.10.00.000|0", "to_binary of 1.0", reportTestCases);
		f += expect(to_interval(X9(0.1)) == "(0.09375, 0.1015625)", "to_interval of 0.1", reportTestCases);
		// a 60-fraction-bit lattice: decimal where long double holds it (x86 80-bit), exact
		// hexfloat where it does not (long double == double); either way the two ends of
		// the tile above 1.0 must print differently
		using W = poxel<64, 0, std::uint64_t>;
		W w(1);
		++w;
		const std::string wide = to_interval(w);
		const bool hex = (wide == "(0x1.000000000000000p+0, 0x1.000000000000001p+0)");
		const bool dec = (wide == "(1, 1.0000000000000000009)");
		f += expect(hex || dec, "to_interval of a 60-fraction-bit tile keeps both lattice points apart", reportTestCases);
		if (!(hex || dec) && reportTestCases) std::cerr << "  got " << wide << '\n';
		f += expect(to_interval(X9(SpecificValue::infpos)) == "(16777216, inf)" && to_interval(X9(SpecificValue::infneg)) == "(-inf, -16777216)", "to_interval of the end tiles", reportTestCases);
		nrOfFailedTestCases += ReportTestResult(f, "poxel", "type tag and text");
	}

	std::cout << "+-----------------  poxel<9,2> construction and special tiles\n";
	{
		int f = 0;
		const X9 a(3), b(3.0f), c(3.0), d(3.0l);
		f += expect(a == b && b == c && c == d && a.isexact(), "construction from int, float, double, long double", reportTestCases);
		f += expect(X9(SpecificValue::maxpos).upper<double>() == 16777216.0 && X9(SpecificValue::minpos).lower<double>() == 5.9604644775390625e-08, "maxpos and minpos of the posit<8,2> lattice", reportTestCases);
		f += expect(X9(SpecificValue::infpos).upper<double>() == INFINITY && X9(SpecificValue::infneg).lower<double>() == -INFINITY, "the end tiles", reportTestCases);
		f += expect(X9(SpecificValue::nar).isnar() && X9(SpecificValue::zero).iszero(), "nar and zero", reportTestCases);
		X9 one(1);
		++one;
		f += expect(!one.isexact() && one.lower<double>() == 1.0, "++ steps from 1.0 to the open tile above it", reportTestCases);
		nrOfFailedTestCases += ReportTestResult(f, "poxel<9,2>", "construction and special tiles");
	}

	{
		int f = 0;
		using L = std::numeric_limits<poxel17>;
		f += expect(L::epsilon().isexact() && double(L::epsilon()) == std::ldexp(1.0, -11), "epsilon is the exact lattice gap above 1.0", reportTestCases);
		f += expect(!L::has_infinity && L::quiet_NaN().isnar() && L::round_style == std::round_toward_neg_infinity, "no infinity; values are stored as their tile's lower end", reportTestCases);
		nrOfFailedTestCases += ReportTestResult(f, "poxel17", "numeric_limits");
	}

	std::cout << "+-----------------  arithmetic exceptions\n";
	{
		int f = 0;
		bool threw = false;
		try { poxel17 x = poxel17(1) / poxel17(0); (void)x; } catch (const poxel_divide_by_zero&) { threw = true; }
		f += expect(threw, "division by zero throws", reportTestCases);
		threw = false;
		try { poxel17 x = poxel17(1) / poxel17(SpecificValue::minpos).operator--(); (void)x; } catch (const poxel_divide_by_zero&) { threw = true; }
		f += expect(threw, "division by the (0, minpos) tile throws like division by zero", reportTestCases);
		threw = false;
		try { poxel17 x = poxel17(SpecificValue::nar) + poxel17(1); (void)x; } catch (const poxel_operand_is_nar&) { threw = true; }
		f += expect(threw, "a NaR operand throws", reportTestCases);
		nrOfFailedTestCases += ReportTestResult(f, "poxel17", "arithmetic exceptions");
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
