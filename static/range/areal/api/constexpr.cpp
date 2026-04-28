// constexpr.cpp: compile time tests for areal constexpr
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the areal template environment
// first: enable general or specialized fixed-point configurations
#define AREAL_FAST_SPECIALIZATION
// second: enable/disable areal arithmetic exceptions
#define AREAL_THROW_ARITHMETIC_EXCEPTION 1

#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_reporters.hpp> 

// stylistic constexpr of pi that we'll assign constexpr to an areal
constexpr double pi = 3.14159265358979323846;

template<typename Real>
void TestConstexprConstruction() {
	// decorated constructors
	{
		Real a(1l);  // signed long
		std::cout << a << '\n';
	}
	{
		constexpr Real a(1ul);  // unsigned long
		std::cout << a << '\n';
	}
	{
		BIT_CAST_CONSTEXPR Real a(1.0f);  // float
		std::cout << a << '\n';
	}
	{
		BIT_CAST_CONSTEXPR Real a(pi);   // double
		std::cout << a << '\n';
	}
	{
		BIT_CAST_CONSTEXPR Real a(1.0l);  // long double
		std::cout << a << '\n';
	}
}

template<typename Real>
void TestConstexprAssignment() {
	// decorated constructors
	{
		Real a = 1l;  // signed long
		std::cout << a << '\n';
	}
	{
		constexpr Real a = 1ul;  // unsigned long
		std::cout << a << '\n';
	}
	{
		BIT_CAST_CONSTEXPR Real a = 1.0f;  // float
		std::cout << a << '\n';
	}
	{
		BIT_CAST_CONSTEXPR Real a = pi;   // double
		std::cout << a << '\n';
	}
	{
		BIT_CAST_CONSTEXPR Real a = 1.0l;  // long double
		std::cout << a << '\n';
	}
}

//
template<typename Real>
void TestConstexprSpecificValues() {
	{
		constexpr Real positiveMax(sw::universal::SpecificValue::maxpos);
		std::cout << "maxpos  : " << to_binary(positiveMax) << " : " << positiveMax << '\n';
	}
	{
		constexpr Real positiveMin(sw::universal::SpecificValue::minpos);
		std::cout << "minpos  : " << to_binary(positiveMin) << " : " << positiveMin << '\n';
	}
	{
		constexpr Real zero(sw::universal::SpecificValue::zero);
		std::cout << "zero    : " << to_binary(zero) << " : " << zero << '\n';
	}
	{
		constexpr Real negativeMin(sw::universal::SpecificValue::minneg);
		std::cout << "minneg  : " << to_binary(negativeMin) << " : " << negativeMin << '\n';
	}
	{
		constexpr Real negativeMax(sw::universal::SpecificValue::maxneg);
		std::cout << "maxneg  : " << to_binary(negativeMax) << " : " << negativeMax << '\n';
	}
}

// =============================================================================
// static_assert smoke tests for the constexpr contract (issue #724)
// =============================================================================
// Lock in the operator-level constexpr promotion: construction (integer +
// IEEE-754 via BIT_CAST_CONSTEXPR), arithmetic, comparison, unary negation,
// and increment / decrement must all evaluate at compile time.
//
// Gated on BIT_CAST_IS_CONSTEXPR so the file still compiles on older
// compilers that lack std::bit_cast or __builtin_bit_cast (the runtime
// tests below still exercise the same operators in those builds).

#if BIT_CAST_IS_CONSTEXPR
namespace areal_constexpr_contract {

	using sw::universal::areal;
	using SmokeReal = areal<12, 2, std::uint8_t>;

	// Construction: integer (already constexpr in current code) + IEEE-754
	// (via BIT_CAST_CONSTEXPR -> __builtin_bit_cast).
	constexpr           SmokeReal one_int(1);          // signed int ctor
	BIT_CAST_CONSTEXPR  SmokeReal two_dbl(2.0);        // double via bit_cast
	BIT_CAST_CONSTEXPR  SmokeReal three_dbl(3.0);

	// Comparison: must be constexpr in same-sign and mixed-sign cases.
	static_assert( two_dbl == two_dbl,   "operator== should be constexpr");
	static_assert(!(two_dbl != two_dbl), "operator!= should be constexpr");
	static_assert( two_dbl <  three_dbl, "operator<  should be constexpr (2.0 < 3.0)");
	static_assert( three_dbl > two_dbl,  "operator>  should be constexpr (3.0 > 2.0)");
	static_assert( two_dbl <= three_dbl, "operator<= should be constexpr");
	static_assert( three_dbl >= two_dbl, "operator>= should be constexpr");

	// Unary negation: must be constexpr and yield a strictly smaller value.
	BIT_CAST_CONSTEXPR  SmokeReal neg_two = -two_dbl;
	static_assert( neg_two <  two_dbl,   "unary operator- should be constexpr (-2 < 2)");
	static_assert(-neg_two == two_dbl,   "double negation should round-trip");

	// Compound arithmetic: wrap in immediately-invoked constexpr lambdas so
	// the mutation produces a single constexpr result.
	BIT_CAST_CONSTEXPR  SmokeReal sum  = []() { SmokeReal x(2.0); x += SmokeReal(3.0); return x; }();
	BIT_CAST_CONSTEXPR  SmokeReal diff = []() { SmokeReal x(5.0); x -= SmokeReal(3.0); return x; }();
	BIT_CAST_CONSTEXPR  SmokeReal prod = []() { SmokeReal x(2.0); x *= SmokeReal(3.0); return x; }();
	BIT_CAST_CONSTEXPR  SmokeReal quot = []() { SmokeReal x(6.0); x /= SmokeReal(3.0); return x; }();

	// The headline acceptance from #724: constexpr arithmetic on constexpr
	// operands must produce a constexpr result.
	static_assert( sum  >  two_dbl,      "constexpr 2 + 3 must be > 2");
	static_assert( diff <  three_dbl,    "constexpr 5 - 3 must be < 3");
	static_assert( prod >  three_dbl,    "constexpr 2 * 3 must be > 3");
	static_assert( quot <  three_dbl,    "constexpr 6 / 3 must be < 3");

	// Increment / decrement: prefix forms must be constexpr; the
	// freshly-implemented operator-- must be the symmetric inverse of
	// operator++ (as far as encoding ordering goes).
	BIT_CAST_CONSTEXPR  SmokeReal next = []() { SmokeReal x(1.0); ++x; return x; }();
	BIT_CAST_CONSTEXPR  SmokeReal prev = []() { SmokeReal x(1.0); --x; return x; }();
	BIT_CAST_CONSTEXPR  SmokeReal one_dbl(1.0);
	static_assert( next != one_dbl,      "++x must move x");
	static_assert( prev != one_dbl,      "--x must move x");
	BIT_CAST_CONSTEXPR  SmokeReal roundtrip = []() { SmokeReal x(1.0); ++x; --x; return x; }();
	static_assert( roundtrip == one_dbl, "++x; --x must round-trip");

	// Postfix forms: lock in that operator++(int) and operator--(int) are
	// constexpr AND that the postfix return-value contract holds (the
	// returned value is the prior encoding; only the captured copy of x
	// after the call is mutated).
	BIT_CAST_CONSTEXPR  SmokeReal post_next       = []() { SmokeReal x(1.0); x++; return x; }();
	BIT_CAST_CONSTEXPR  SmokeReal post_prev       = []() { SmokeReal x(1.0); x--; return x; }();
	BIT_CAST_CONSTEXPR  SmokeReal post_next_value = []() { SmokeReal x(1.0); return x++; }();
	BIT_CAST_CONSTEXPR  SmokeReal post_prev_value = []() { SmokeReal x(1.0); return x--; }();
	static_assert( post_next       == next,    "x++ and ++x must produce the same final encoding");
	static_assert( post_prev       == prev,    "x-- and --x must produce the same final encoding");
	static_assert( post_next_value == one_dbl, "x++ must RETURN the prior encoding (not the new one)");
	static_assert( post_prev_value == one_dbl, "x-- must RETURN the prior encoding (not the new one)");

	// Zero-wrap path: the operator-- implementation has a special case where
	// 000...000 wraps to 111...111 (mirror of operator++'s all-ones-wrap).
	// This path does not get exercised by --1.0 above (which decrements
	// within the normal range), so test it directly via setbits.
	BIT_CAST_CONSTEXPR  SmokeReal dec_wrap = []() { SmokeReal x; x.setbits(0); --x; return x; }();
	BIT_CAST_CONSTEXPR  SmokeReal all_ones = []() {
		SmokeReal x;
		// All nbits set: ((1 << nbits) - 1). For SmokeReal = areal<12,2>, that's
		// 0xFFF -- representable as a uint64_t for nbits <= 64.
		x.setbits((uint64_t(1) << SmokeReal::nbits) - 1u);
		return x;
	}();
	static_assert( dec_wrap == all_ones, "--0 must wrap to the all-ones encoding (operator-- borrow path)");

	// Binary (non-mutating) arithmetic operators. These are free functions
	// that delegate to the compound forms; promoting them to constexpr lets
	// expressions like `a + b` produce a constexpr result without an
	// intermediate lambda.
	BIT_CAST_CONSTEXPR  SmokeReal bin_sum  = two_dbl + three_dbl;
	BIT_CAST_CONSTEXPR  SmokeReal bin_diff = SmokeReal(5.0) - three_dbl;
	BIT_CAST_CONSTEXPR  SmokeReal bin_prod = two_dbl * three_dbl;
	BIT_CAST_CONSTEXPR  SmokeReal bin_quot = SmokeReal(6.0) / three_dbl;
	static_assert( bin_sum  == sum,      "binary +  should equal compound +=");
	static_assert( bin_diff == diff,     "binary -  should equal compound -=");
	static_assert( bin_prod == prod,     "binary *  should equal compound *=");
	static_assert( bin_quot == quot,     "binary /  should equal compound /=");

	// Specific values are constexpr (already exercised by the runtime
	// tests; static_assert here on the encoding equality just locks in
	// that the SpecificValue constructor stays constexpr-clean).
	constexpr SmokeReal zero_sv(sw::universal::SpecificValue::zero);
	static_assert( zero_sv == zero_sv,   "SpecificValue ctor + ==  must be constexpr");

	// Mixed equality / ordering convention (locked in by logic/logic.cpp):
	//   - operator== is bit-pattern equality (so +0 != -0)
	//   - operator<  uses IEEE-style ordering (so neither +0 < -0 nor -0 < +0,
	//     because zero values compare equal *as ordered quantities*)
	// These constexpr asserts lock both halves of the convention.
	BIT_CAST_CONSTEXPR  SmokeReal pos_zero(0.0);
	BIT_CAST_CONSTEXPR  SmokeReal neg_zero = -pos_zero;
	static_assert( pos_zero != neg_zero,   "areal operator== is bit-pattern: +0 != -0");
	static_assert(!(pos_zero <  neg_zero), "ordering: +0 not < -0");
	static_assert(!(neg_zero <  pos_zero), "ordering: -0 not < +0");

	// NaN convention. operator== is bit-pattern (so qNaN == qNaN with the
	// same encoding is true; qNaN with one encoding != qNaN with another).
	// operator< / <= / >= / > are IEEE-style (any comparison involving NaN
	// is false). Construct distinct qNaN and sNaN encodings via setnan(int)
	// rather than SpecificValue::qnan/snan (which both delegate to the
	// default-NaN-type setnan() and produce identical encodings).
	constexpr SmokeReal qnan = []() { SmokeReal x; x.setnan(sw::universal::NAN_TYPE_QUIET);       return x; }();
	constexpr SmokeReal snan = []() { SmokeReal x; x.setnan(sw::universal::NAN_TYPE_SIGNALLING);  return x; }();
	static_assert( qnan == qnan,           "bit-pattern: qNaN == qNaN (same encoding)");
	static_assert(!(qnan != qnan),         "bit-pattern: !(qNaN != qNaN) for same encoding");
	static_assert( qnan != snan,           "bit-pattern: qNaN != sNaN (different encodings)");
	static_assert(!(qnan <  two_dbl),      "ordering: NaN < x is false");
	static_assert(!(two_dbl <  qnan),      "ordering: x < NaN is false");
	static_assert(!(qnan <= two_dbl),      "ordering: NaN <= x is false");
	static_assert(!(two_dbl <= qnan),      "ordering: x <= NaN is false");
	static_assert(!(qnan >  two_dbl),      "ordering: NaN > x is false");
	static_assert(!(two_dbl >  qnan),      "ordering: x > NaN is false");
	static_assert(!(qnan >= two_dbl),      "ordering: NaN >= x is false");
	static_assert(!(two_dbl >= qnan),      "ordering: x >= NaN is false");

}  // namespace areal_constexpr_contract
#endif  // BIT_CAST_IS_CONSTEXPR

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "areal constexpr ";
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

	using Real = areal<12, 2>;
	Real a;
	a.constexprClassParameters();

	TestConstexprConstruction<Real>();
	TestConstexprAssignment<Real>();
	TestConstexprSpecificValues<Real>();

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
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
