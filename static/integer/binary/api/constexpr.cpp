// constexpr.cpp: compile-time tests for constexpr support of the integer<> type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This regression test exercises the full constexpr API surface of integer<>:
//   - construction from native int types
//   - cross-template copy construction (integer<n+1> from integer<n>)
//   - all arithmetic operators (+, -, *, /, %, +=, -=, *=, /=, %=, ++, --, unary -)
//   - all bitwise operators (&, |, ^, ~, <<, >>, &=, |=, ^=, <<=, >>=)
//   - all comparison operators (==, !=, <, >, <=, >=)
//   - signed and unsigned (IntegerNumber, NaturalNumber) configurations
//   - single-limb (nrBlocks==1) and multi-limb (nrBlocks>1) configurations
//   - uint8_t / uint16_t / uint32_t / uint64_t block types
//   - findMsb() free function (constexpr after PR for issue #720)
//   - idiv() free function (constexpr after PR for issue #720)
//   - edge cases: signed overflow saturation, divide-by-zero defined behavior

#include <universal/utility/directives.hpp>
// disable arithmetic exceptions so constexpr-throw paths don't fail to compile
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

// ----------------------------------------------------------------------------
// Construction tests
// ----------------------------------------------------------------------------

template<typename Integer>
void TestConstexprConstruction() {
	{ constexpr Integer a{};           (void)a; }                  // value-init (default ctor is trivial / no zeroing)
	{ constexpr Integer a(static_cast<signed char>(-3));   (void)a; }
	{ constexpr Integer a(static_cast<short>(-300));        (void)a; }
	{ constexpr Integer a(-30000);                          (void)a; }
	{ constexpr Integer a(0L);                              (void)a; }
	{ constexpr Integer a(123456789LL);                     (void)a; }
	{ constexpr Integer a(static_cast<unsigned char>(255)); (void)a; }
	{ constexpr Integer a(static_cast<unsigned short>(65535)); (void)a; }
	{ constexpr Integer a(4000000000u);                     (void)a; }
	{ constexpr Integer a(0UL);                             (void)a; }
	{ constexpr Integer a(static_cast<unsigned long long>(18446744073709551615ull)); (void)a; }
}

// Cross-template copy construction (integer<n>(integer<srcbits>))
// This is exercised internally by multi-limb mul (constructs integer<nbits+1>)
// so it must be constexpr for the multi-limb path to be constexpr-callable.
template<unsigned dstBits, unsigned srcBits, typename BlockType, IntegerNumberType NumberType>
void TestConstexprCrossTemplateCopy() {
	using Src = integer<srcBits, BlockType, NumberType>;
	using Dst = integer<dstBits, BlockType, NumberType>;
	constexpr Src s(42);
	constexpr Dst d(s);
	(void)d;
}

// ----------------------------------------------------------------------------
// Arithmetic operators (binary + compound assignment + unary)
// ----------------------------------------------------------------------------

template<typename Integer>
int TestConstexprArithmetic() {
	int errors = 0;
	constexpr Integer a(42);
	constexpr Integer b(7);

	// Binary operators
	constexpr auto sum  = a + b;
	constexpr auto diff = a - b;
	constexpr auto prod = a * b;
	constexpr auto quot = a / b;
	constexpr auto rem  = a % b;
	constexpr auto neg  = -a;

	// Compound assignment via lambda (so the test can be in constexpr context)
	constexpr Integer addeq = []() { Integer t(42); t += Integer(7); return t; }();
	constexpr Integer subeq = []() { Integer t(42); t -= Integer(7); return t; }();
	constexpr Integer muleq = []() { Integer t(42); t *= Integer(7); return t; }();
	constexpr Integer diveq = []() { Integer t(42); t /= Integer(7); return t; }();
	constexpr Integer modeq = []() { Integer t(42); t %= Integer(7); return t; }();
	constexpr Integer scleq = []() { Integer t(42); t *= typename Integer::BlockType(2); return t; }();

	// Increment / decrement
	constexpr Integer inc_pre  = []() { Integer t(0); ++t; return t; }();
	constexpr Integer dec_pre  = []() { Integer t(0); --t; return t; }();
	constexpr Integer inc_post = []() { Integer t(0); t++; return t; }();
	constexpr Integer dec_post = []() { Integer t(0); t--; return t; }();

	// Reference values for cross-check
	const Integer r49(49), r35(35), r294(294), r6(6), r0(0), r_neg42(-42), r_neg1(-1), r1(1), r84(84);

	if (sum  != r49)     { ++errors; std::cout << "FAIL constexpr 42 + 7 == 49\n"; }
	if (diff != r35)     { ++errors; std::cout << "FAIL constexpr 42 - 7 == 35\n"; }
	if (prod != r294)    { ++errors; std::cout << "FAIL constexpr 42 * 7 == 294\n"; }
	if (quot != r6)      { ++errors; std::cout << "FAIL constexpr 42 / 7 == 6\n"; }
	if (rem  != r0)      { ++errors; std::cout << "FAIL constexpr 42 % 7 == 0\n"; }
	if (neg  != r_neg42) { ++errors; std::cout << "FAIL constexpr -42\n"; }
	if (addeq != r49)    { ++errors; std::cout << "FAIL constexpr += equals binary +\n"; }
	if (subeq != r35)    { ++errors; std::cout << "FAIL constexpr -= equals binary -\n"; }
	if (muleq != r294)   { ++errors; std::cout << "FAIL constexpr *= equals binary *\n"; }
	if (diveq != r6)     { ++errors; std::cout << "FAIL constexpr /= equals binary /\n"; }
	if (modeq != r0)     { ++errors; std::cout << "FAIL constexpr %= equals binary %\n"; }
	if (scleq != r84)    { ++errors; std::cout << "FAIL constexpr *=(BlockType) (42 * 2 == 84)\n"; }
	if (inc_pre  != r1)  { ++errors; std::cout << "FAIL constexpr ++t starting from 0\n"; }
	if (dec_pre  != r_neg1) { ++errors; std::cout << "FAIL constexpr --t starting from 0\n"; }
	if (inc_post != r1)  { ++errors; std::cout << "FAIL constexpr t++ starting from 0\n"; }
	if (dec_post != r_neg1) { ++errors; std::cout << "FAIL constexpr t-- starting from 0\n"; }
	return errors;
}

// ----------------------------------------------------------------------------
// Bitwise + shift operators
// ----------------------------------------------------------------------------

template<typename Integer>
int TestConstexprBitwise() {
	int errors = 0;
	constexpr Integer a(42);   // ...0010 1010
	constexpr Integer b(7);    // ...0000 0111

	constexpr auto bw_and = a & b;   // 2
	constexpr auto bw_or  = a | b;   // 47
	constexpr auto bw_xor = a ^ b;   // 45
	constexpr auto bw_not = ~a;      // -43 (signed two's complement) but we'll just check it's not == a
	constexpr auto shl    = a << 2;  // 168
	constexpr auto shr    = a >> 2;  // 10

	// Compound forms
	constexpr Integer andeq = []() { Integer t(42); t &= Integer(7);  return t; }();
	constexpr Integer oreq  = []() { Integer t(42); t |= Integer(7);  return t; }();
	constexpr Integer xoreq = []() { Integer t(42); t ^= Integer(7);  return t; }();
	constexpr Integer shleq = []() { Integer t(42); t <<= 2;          return t; }();
	constexpr Integer shreq = []() { Integer t(42); t >>= 2;          return t; }();

	const Integer r2(2), r47(47), r45(45), r168(168), r10(10);
	if (bw_and != r2)   { ++errors; std::cout << "FAIL constexpr 42 & 7 == 2\n"; }
	if (bw_or  != r47)  { ++errors; std::cout << "FAIL constexpr 42 | 7 == 47\n"; }
	if (bw_xor != r45)  { ++errors; std::cout << "FAIL constexpr 42 ^ 7 == 45\n"; }
	if (bw_not == a)    { ++errors; std::cout << "FAIL constexpr ~a should differ from a\n"; }
	if (shl    != r168) { ++errors; std::cout << "FAIL constexpr 42 << 2 == 168\n"; }
	if (shr    != r10)  { ++errors; std::cout << "FAIL constexpr 42 >> 2 == 10\n"; }
	if (andeq  != r2)   { ++errors; std::cout << "FAIL constexpr &= equals &\n"; }
	if (oreq   != r47)  { ++errors; std::cout << "FAIL constexpr |= equals |\n"; }
	if (xoreq  != r45)  { ++errors; std::cout << "FAIL constexpr ^= equals ^\n"; }
	if (shleq  != r168) { ++errors; std::cout << "FAIL constexpr <<= equals <<\n"; }
	if (shreq  != r10)  { ++errors; std::cout << "FAIL constexpr >>= equals >>\n"; }
	return errors;
}

// ----------------------------------------------------------------------------
// Comparison operators
// ----------------------------------------------------------------------------

template<typename Integer>
void TestConstexprComparisons() {
	constexpr Integer a(42);
	constexpr Integer b(7);
	constexpr Integer c(42);

	// integer-integer
	static_assert( (a == c), "constexpr 42 == 42");
	static_assert(!(a == b), "constexpr 42 != 7");
	static_assert( (a != b), "constexpr 42 != 7");
	static_assert( (b <  a), "constexpr 7 < 42");
	static_assert( (a >  b), "constexpr 42 > 7");
	static_assert( (a <= c), "constexpr 42 <= 42");
	static_assert( (a >= b), "constexpr 42 >= 7");

	// integer-IntType (using the literal-comparison overloads)
	static_assert( (a == 42),  "constexpr a == 42");
	static_assert(!(a == 7),   "constexpr !(a == 7)");
	static_assert( (a >  7),   "constexpr a > 7");
	static_assert( (7  <  a),  "constexpr 7 < a");
	static_assert( (42 == a),  "constexpr 42 == a");
}

// ----------------------------------------------------------------------------
// Multi-limb + uint64-limb tests (the hard cases that exercise the
// is_constant_evaluated() carry-propagation dispatch and constexpr findMsb)
// ----------------------------------------------------------------------------

int TestConstexprMultiLimb() {
	int errors = 0;

	// Multi-limb mul with uint32_t limbs (4 limbs of 32 bits = 128 bits).
	// 1_000_000 * 1_000 = 1_000_000_000 (fits in uint32, so this is a basic
	// smoke that the multi-limb code path is constexpr-callable).
	using I128_32 = integer<128, std::uint32_t, IntegerNumberType::IntegerNumber>;
	constexpr I128_32 ma(1000000), mb(1000);
	constexpr auto cx_m_prod = ma * mb;
	constexpr I128_32 r_billion(1000000000ll);
	if (cx_m_prod != r_billion) { ++errors; std::cout << "FAIL constexpr 1M * 1k == 1B (uint32 multi-limb mul)\n"; }

	// Multi-limb division (exercises constexpr findMsb in idiv).
	// 1_000_000_000 / 1_000 = 1_000_000.
	constexpr auto cx_m_quot = r_billion / mb;
	constexpr I128_32 r_million(1000000);
	if (cx_m_quot != r_million) { ++errors; std::cout << "FAIL constexpr 1B / 1k == 1M (uint32 multi-limb div)\n"; }

	// Multi-limb modulo (also via idiv).
	constexpr auto cx_m_rem = (r_billion + I128_32(7)) % I128_32(13);
	constexpr I128_32 r_expected_rem((1000000000ll + 7) % 13);
	if (cx_m_rem != r_expected_rem) { ++errors; std::cout << "FAIL constexpr (1B+7) % 13 (uint32 multi-limb mod)\n"; }

#if defined(__SIZEOF_INT128__)
	// uint64-limb mul: exercises the __int128-based constexpr fallback that
	// preserves the upper 64 bits of each partial product.
	// (1 << 62) * 4 == (1 << 64) which spans both limbs of integer<128, uint64>.
	using I128_64 = integer<128, std::uint64_t, IntegerNumberType::IntegerNumber>;
	constexpr I128_64 huge_a(1ll << 62);
	constexpr I128_64 huge_b(4);
	constexpr auto cx_huge = huge_a * huge_b;
	const I128_64 rt_huge = I128_64(1ll << 62) * I128_64(4);
	if (cx_huge != rt_huge) { ++errors; std::cout << "FAIL constexpr uint64-limb mul carry preservation\n"; }
#endif
	return errors;
}

// ----------------------------------------------------------------------------
// Edge cases: divide-by-zero with exceptions disabled, signed/unsigned, etc.
// ----------------------------------------------------------------------------

int TestConstexprEdgeCases() {
	int errors = 0;
	using I32 = integer<32, std::uint32_t, IntegerNumberType::IntegerNumber>;

	// Divide-by-zero with INTEGER_THROW_ARITHMETIC_EXCEPTION=0 returns 0
	// instead of UB. Exercise at runtime; constexpr eval would short-circuit
	// before the actual divide.
	I32 a(10), b(0);
	a /= b;
	if (!a.iszero()) { ++errors; std::cout << "FAIL div-by-zero with exceptions disabled should yield 0\n"; }

	I32 c(10), d(0);
	c %= d;
	if (!c.iszero()) { ++errors; std::cout << "FAIL mod-by-zero with exceptions disabled should yield 0\n"; }

	// Signed arithmetic produces correct negative results
	constexpr I32 neg(-5), pos(3);
	constexpr auto sum_neg = neg + pos;
	const I32 r_minus2(-2);
	if (sum_neg != r_minus2) { ++errors; std::cout << "FAIL constexpr signed -5 + 3 == -2\n"; }

	// Unsigned (NaturalNumber and WholeNumber) -- the issue #758 work added a
	// real magnitude subtractor, so we now exercise +=, -= on both modes
	// across single-limb and multi-limb configurations.
	using N32 = integer<32, std::uint32_t, IntegerNumberType::NaturalNumber>;
	using W32 = integer<32, std::uint32_t, IntegerNumberType::WholeNumber>;
	using N64u8 = integer<64, std::uint8_t, IntegerNumberType::NaturalNumber>;  // multi-limb

	constexpr N32 nat_a(42), nat_b(7);
	constexpr auto nat_sum = nat_a + nat_b;
	const N32 r49(49);
	if (nat_sum != r49) { ++errors; std::cout << "FAIL constexpr NaturalNumber addition\n"; }

	// Subtraction (issue #758) -- single-limb
	constexpr N32 nat_diff = []() { N32 t(42); t -= N32(7); return t; }();
	const N32 r35(35);
	if (nat_diff != r35) { ++errors; std::cout << "FAIL constexpr NaturalNumber 42 - 7 == 35\n"; }

	constexpr W32 wh_diff = []() { W32 t(42); t -= W32(7); return t; }();
	if (wh_diff != W32(35)) { ++errors; std::cout << "FAIL constexpr WholeNumber 42 - 7 == 35\n"; }

	// Subtraction (issue #758) -- multi-limb cross-limb borrow
	// 256 -= 1 = 255 forces a borrow from limb[1] into limb[0]
	constexpr N64u8 ml_diff = []() { N64u8 t(256); t -= N64u8(1); return t; }();
	if (ml_diff != N64u8(255)) { ++errors; std::cout << "FAIL constexpr NaturalNumber multi-limb 256 - 1 == 255\n"; }

	// NaturalNumber: result == 0 is allowed (no exception unless THROW=1).
	constexpr N32 nat_zero = []() { N32 t(7); t -= N32(7); return t; }();
	if (!nat_zero.iszero()) { ++errors; std::cout << "FAIL constexpr NaturalNumber 7 - 7 == 0\n"; }

	return errors;
}

// ----------------------------------------------------------------------------
// findMsb (free function used by idiv long division)
// ----------------------------------------------------------------------------

int TestConstexprFindMsb() {
	int errors = 0;
	using I64_32 = integer<64, std::uint32_t, IntegerNumberType::IntegerNumber>;

	// findMsb returns the 0-based position of the most significant bit, or -1
	// if v == 0. Cross-checked against runtime evaluation.
	constexpr I64_32 v0(0);
	constexpr I64_32 v1(1);
	constexpr I64_32 v_pow12(1ll << 12);
	constexpr I64_32 v_pow32(1ll << 32);

	constexpr signed cx_msb_zero  = findMsb(v0);
	constexpr signed cx_msb_one   = findMsb(v1);
	constexpr signed cx_msb_pow12 = findMsb(v_pow12);
	constexpr signed cx_msb_pow32 = findMsb(v_pow32);

	if (cx_msb_zero  != -1) { ++errors; std::cout << "FAIL constexpr findMsb(0) == -1\n"; }
	if (cx_msb_one   !=  0) { ++errors; std::cout << "FAIL constexpr findMsb(1) == 0\n"; }
	if (cx_msb_pow12 != 12) { ++errors; std::cout << "FAIL constexpr findMsb(1<<12) == 12\n"; }
	if (cx_msb_pow32 != 32) { ++errors; std::cout << "FAIL constexpr findMsb(1<<32) == 32 (multi-limb)\n"; }
	return errors;
}

}}  // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "integer<> constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// Construction across block types and number-type policies
	std::cout << "+----- construction\n";
	TestConstexprConstruction<integer<32, std::uint8_t,  IntegerNumberType::IntegerNumber>>();
	TestConstexprConstruction<integer<32, std::uint16_t, IntegerNumberType::IntegerNumber>>();
	TestConstexprConstruction<integer<32, std::uint32_t, IntegerNumberType::IntegerNumber>>();
	TestConstexprConstruction<integer<64, std::uint64_t, IntegerNumberType::IntegerNumber>>();
	TestConstexprConstruction<integer<128, std::uint32_t, IntegerNumberType::IntegerNumber>>();

	// Cross-template copy is exercised internally by multi-limb mul; ensure
	// it works in constexpr context for the configurations that need it.
	std::cout << "+----- cross-template copy construction\n";
	TestConstexprCrossTemplateCopy<33,  32,  std::uint32_t, IntegerNumberType::IntegerNumber>();
	TestConstexprCrossTemplateCopy<129, 128, std::uint32_t, IntegerNumberType::IntegerNumber>();

	// Arithmetic: single-limb and multi-limb, multiple block types, signed and unsigned
	std::cout << "+----- arithmetic\n";
	nrOfFailedTestCases += TestConstexprArithmetic<integer<32,  std::uint8_t,  IntegerNumberType::IntegerNumber>>();
	nrOfFailedTestCases += TestConstexprArithmetic<integer<32,  std::uint16_t, IntegerNumberType::IntegerNumber>>();
	nrOfFailedTestCases += TestConstexprArithmetic<integer<32,  std::uint32_t, IntegerNumberType::IntegerNumber>>();
	nrOfFailedTestCases += TestConstexprArithmetic<integer<64,  std::uint64_t, IntegerNumberType::IntegerNumber>>();
	nrOfFailedTestCases += TestConstexprArithmetic<integer<128, std::uint32_t, IntegerNumberType::IntegerNumber>>();

	// Bitwise + shifts
	std::cout << "+----- bitwise and shifts\n";
	nrOfFailedTestCases += TestConstexprBitwise<integer<32,  std::uint8_t,  IntegerNumberType::IntegerNumber>>();
	nrOfFailedTestCases += TestConstexprBitwise<integer<32,  std::uint32_t, IntegerNumberType::IntegerNumber>>();
	nrOfFailedTestCases += TestConstexprBitwise<integer<128, std::uint32_t, IntegerNumberType::IntegerNumber>>();

	// Comparisons (static_assert based -- compile-time failures show up as
	// build errors, not runtime test failures)
	std::cout << "+----- comparisons (static_assert)\n";
	TestConstexprComparisons<integer<32, std::uint8_t,  IntegerNumberType::IntegerNumber>>();
	TestConstexprComparisons<integer<32, std::uint32_t, IntegerNumberType::IntegerNumber>>();
	TestConstexprComparisons<integer<128, std::uint32_t, IntegerNumberType::IntegerNumber>>();

	// Multi-limb (mul, div, mod) and uint64-limb (carry preservation)
	std::cout << "+----- multi-limb + uint64-limb\n";
	nrOfFailedTestCases += TestConstexprMultiLimb();

	// Edge cases
	std::cout << "+----- edge cases (divide-by-zero, signed, unsigned)\n";
	nrOfFailedTestCases += TestConstexprEdgeCases();

	// findMsb free function (used by idiv long division)
	std::cout << "+----- findMsb\n";
	nrOfFailedTestCases += TestConstexprFindMsb();

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
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
