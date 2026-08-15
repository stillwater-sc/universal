// wide_arithmetic.cpp: correct-rounding verification for wide takum configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// A takum significand is 1 + p bits with p reaching nbits - 2 - rbits, so from
// nbits > 54 + rbits it no longer fits a double -- takum<64,3> carries 60 bits
// against a double's 53.  The operators used to evaluate in a double, which
// quantized BOTH OPERANDS before the operation, and this suite is the evidence
// that they no longer do (issue #1300).
//
// The sibling suites addition.cpp / multiplication.cpp / division.cpp check the
// narrow configurations against a double, which is the correct reference THERE
// and useless here: it is precisely the thing under test.
//
// So the reference is exact instead.  Every takum value is a dyadic rational
// S * 2^e with S < 2^62, so a + b, a - b and a * b are exact integers and a / b is
// an exact ratio of them.  Held in a 1024-bit integer<>, the reference is arrived
// at with no floating-point arithmetic at all, and the check is the one that
// matters: the produced encoding must be at least as close to the exact result as
// either of its neighbours.  Both neighbours are consulted, since consecutive
// encodings are consecutive in value (Prop. 4).
//
// Confirmed by mutation.  Reverting the operators to the double path fails this
// at 64 bits for ~75% of addition pairs -- matching the 75.60% measured
// independently against a 113-bit __float128 reference in #1300 -- and 0% at 16
// and 32 bits, which is the whole shape of the defect.  Disabling the round-to-odd
// step in the shared tail fails a further 834 cases, all of them at 64 bits.
//
// Exact ties are accepted either way: the check fails only on a strictly closer
// neighbour, so it never reports a correct implementation for a tie-break
// convention it was not told about.
//
// Saturated results -- zero, maxpos, maxneg -- are skipped.  Those are range
// decisions the format makes on the caller's behalf, not rounding decisions, and
// the nearest encoding to an out-of-range value is not what the type promises.
#include <universal/utility/directives.hpp>

#include <iostream>
#include <cstdint>
#include <universal/number/takum/takum.hpp>
#include <universal/number/takum/math/fma.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

// 1024 bits carries any comparison this suite forms at rbits = 3, where the
// characteristic spans [-255, 254] and the widest span between two operands is
// therefore about 630 bits.  Configurations whose span does not fit are skipped
// rather than mis-verified; see fits() below.
using BigInt = sw::universal::integer<1024, std::uint64_t>;
constexpr int64_t SPAN_LIMIT = 820;

enum class Op { add, sub, mul, div };

// |value| = S * 2^e, with S = 2^p + M the significand and its implicit bit.
struct fields {
	std::uint64_t S;
	std::int64_t  e;
	bool          sign;
	bool          zero;
};

template<typename T>
fields decode(const T& x) {
	if (x.iszero()) return fields{ 0ull, 0, x.sign(), true };
	auto d = T::Codec::decode(x.magnitude_bits());
	return fields{ (1ull << d.p) | d.M_bits,
	               d.c - static_cast<std::int64_t>(d.p),
	               x.sign(), false };
}

// S * 2^(e - base), signed.  Pre: e >= base, and the shift is within SPAN_LIMIT.
BigInt scaled(std::uint64_t S, std::int64_t e, bool sign, std::int64_t base) {
	BigInt v(S);
	v <<= static_cast<int>(e - base);
	return sign ? -v : v;
}

BigInt babs(const BigInt& a) { return (a < 0) ? -a : a; }

// The signed integer the encoding denotes, sign extended to 64 bits.  Encodings
// are ordered by value, so +/-1 on this is the neighbouring representable value.
template<typename T>
std::int64_t signed_bits(const T& x) {
	std::uint64_t raw = x.raw_bits();
	if constexpr (T::nbits < 64) {
		if (raw & (1ull << (T::nbits - 1))) raw |= ~((1ull << T::nbits) - 1);
	}
	return static_cast<std::int64_t>(raw);
}

// The neighbour v steps away, or false when that would leave the finite range.
// NaR is the most negative encoding and is excluded by construction.
template<typename T>
bool neighbour(std::int64_t v, T& out) {
	constexpr std::int64_t hi = (T::nbits >= 64) ? INT64_MAX
	                                            : ((std::int64_t(1) << (T::nbits - 1)) - 1);
	if (v < -hi || v > hi) return false;
	out.setbits(static_cast<std::uint64_t>(v) & T::Codec::nbits_mask());
	return true;
}

template<typename T>
bool saturated(const T& x) {
	T maxpos(sw::universal::SpecificValue::maxpos), maxneg(sw::universal::SpecificValue::maxneg);
	return x.iszero() || x.raw_bits() == maxpos.raw_bits() || x.raw_bits() == maxneg.raw_bits();
}

bool fits(std::int64_t e, std::int64_t base) { return (e - base) <= SPAN_LIMIT; }

// Is `got` at least as close to num/den as both of its neighbours?  num and den
// are exact, den > 0, and every value is scaled by the same 2^base, so comparing
// |num - value(candidate) * den| across candidates decides it outright.
template<typename T>
bool nearest(const T& got, const BigInt& num, const BigInt& den, std::int64_t base, bool& skipped) {
	auto distance = [&](const T& cand, bool& ok) -> BigInt {
		fields f = decode(cand);
		if (f.zero) { ok = true; return babs(num); }
		if (!fits(f.e, base)) { ok = false; return BigInt(0); }
		ok = true;
		return babs(num - scaled(f.S, f.e, f.sign, base) * den);
	};

	bool ok = false;
	const BigInt mine = distance(got, ok);
	if (!ok) { skipped = true; return true; }

	const std::int64_t g = signed_bits(got);
	for (int k = -1; k <= 1; k += 2) {
		T nb;
		if (!neighbour<T>(g + k, nb)) continue;
		const BigInt d = distance(nb, ok);
		if (!ok) { skipped = true; return true; }
		if (d < mine) return false;
	}
	return true;
}

// ---------------------------------------------------------------------------
// a OP b, correctly rounded
// ---------------------------------------------------------------------------
template<unsigned nbits, unsigned rbits>
int VerifyCorrectlyRounded(Op op, std::uint64_t stride, bool reportTestCases) {
	using T = sw::universal::takum<nbits, rbits, std::uint64_t>;
	int nrOfFailedTests = 0;
	long exercised = 0;
	// 1ull << nbits is undefined at nbits == 64, and 64 is the width this exists
	// for, so the bound avoids it.  stride samples, since the sweep is over PAIRS.
	const std::uint64_t NR = (nbits >= 64) ? ~0ull : (1ull << nbits);

	for (std::uint64_t i = 0; i < NR && i + stride > i; i += stride) {
		T a; a.setbits(i);
		if (a.isnar() || a.iszero()) continue;
		const fields fa = decode(a);

		for (std::uint64_t j = 0; j < NR && j + stride > j; j += stride) {
			T b; b.setbits(j);
			if (b.isnar() || b.iszero()) continue;
			const fields fb = decode(b);

			T got;
			switch (op) {
			case Op::add: got = a + b; break;
			case Op::sub: got = a - b; break;
			case Op::mul: got = a * b; break;
			default:      got = a / b; break;
			}
			if (got.isnar() || saturated(got)) continue;

			// Assemble the exact result as num/den, everything scaled by 2^base.
			// den is 1 except for division, where the quotient is not dyadic and
			// the comparison is cross-multiplied by the divisor's significand.
			const fields fg = decode(got);
			BigInt num(0), den(1);
			std::int64_t base = 0;
			bool skipped = false;

			if (op == Op::add || op == Op::sub) {
				base = std::min(std::min(fa.e, fb.e), fg.e) - 2;
				if (!fits(fa.e, base) || !fits(fb.e, base)) continue;
				const BigInt A = scaled(fa.S, fa.e, fa.sign, base);
				const BigInt B = scaled(fb.S, fb.e, (op == Op::sub) ? !fb.sign : fb.sign, base);
				num = A + B;
			}
			else if (op == Op::mul) {
				const std::int64_t ep = fa.e + fb.e;
				base = std::min(ep, fg.e) - 2;
				if (!fits(ep, base)) continue;
				BigInt P(fa.S);
				P = P * BigInt(fb.S);
				P <<= static_cast<int>(ep - base);
				num = (fa.sign != fb.sign) ? -P : P;
			}
			else {
				const std::int64_t eq = fa.e - fb.e;
				base = std::min(eq, fg.e) - 2;
				if (!fits(eq, base)) continue;
				num = scaled(fa.S, eq, fa.sign != fb.sign, base);
				den = BigInt(fb.S);
			}

			const bool ok = nearest(got, num, den, base, skipped);
			if (skipped) continue;
			++exercised;
			if (!ok) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cout << "FAIL not correctly rounded: op=" << int(op)
					          << " a=" << to_binary(a) << " b=" << to_binary(b)
					          << " got=" << to_binary(got) << '\n';
				}
			}
		}
	}
	if (exercised == 0) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL correctly-rounded check compared nothing\n";
	}
	return nrOfFailedTests;
}

// ---------------------------------------------------------------------------
// fma(a, b, c), correctly rounded
//
// The point of an fma is that the product is NOT rounded before the addend joins
// it, which is exactly what a double intermediate destroyed here: it rounded both
// factors first and then computed an exact product of the wrong numbers.
// ---------------------------------------------------------------------------
template<unsigned nbits, unsigned rbits>
int VerifyFmaCorrectlyRounded(std::uint64_t stride, bool reportTestCases) {
	using T = sw::universal::takum<nbits, rbits, std::uint64_t>;
	int nrOfFailedTests = 0;
	long exercised = 0;
	const std::uint64_t NR = (nbits >= 64) ? ~0ull : (1ull << nbits);
	// a third operand from the same sweep, offset so it is not a repeat of a or b
	const std::uint64_t skew = stride / 3ull + 1ull;

	for (std::uint64_t i = 0; i < NR && i + stride > i; i += stride) {
		T a; a.setbits(i);
		if (a.isnar() || a.iszero()) continue;
		const fields fa = decode(a);

		for (std::uint64_t j = 0; j < NR && j + stride > j; j += stride) {
			T b; b.setbits(j);
			if (b.isnar() || b.iszero()) continue;
			T c; c.setbits(i + j + skew);
			if (c.isnar() || c.iszero()) continue;
			const fields fb = decode(b);
			const fields fc = decode(c);

			T got = sw::universal::fma(a, b, c);
			if (got.isnar() || saturated(got)) continue;

			const fields fg = decode(got);
			const std::int64_t ep = fa.e + fb.e;
			const std::int64_t base = std::min(std::min(ep, fc.e), fg.e) - 2;
			if (!fits(ep, base) || !fits(fc.e, base)) continue;

			BigInt P(fa.S);
			P = P * BigInt(fb.S);
			P <<= static_cast<int>(ep - base);
			if (fa.sign != fb.sign) P = -P;
			const BigInt num = P + scaled(fc.S, fc.e, fc.sign, base);

			bool skipped = false;
			const bool ok = nearest(got, num, BigInt(1), base, skipped);
			if (skipped) continue;
			++exercised;
			if (!ok) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cout << "FAIL fma not correctly rounded: a=" << to_binary(a)
					          << " b=" << to_binary(b) << " c=" << to_binary(c)
					          << " got=" << to_binary(got) << '\n';
				}
			}
		}
	}
	if (exercised == 0) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL fma correctly-rounded check compared nothing\n";
	}
	return nrOfFailedTests;
}

// ---------------------------------------------------------------------------
// Exact identities
//
// These need no reference at all and they are what issue #1300 opened on: one
// encoding step above 1.0, takum<64,3> lost its low bit to `x + 0` because the
// operand was quantized on the way in.  Cheap enough to run at every width, and
// they discriminate -- the double path fails a + 0 at 64 bits.
// ---------------------------------------------------------------------------
template<unsigned nbits, unsigned rbits>
int VerifyExactIdentities(std::uint64_t stride, bool reportTestCases) {
	using T = sw::universal::takum<nbits, rbits, std::uint64_t>;
	int nrOfFailedTests = 0;
	const std::uint64_t NR = (nbits >= 64) ? ~0ull : (1ull << nbits);
	T one(1.0), zero; zero.setzero();
	T nar; nar.setnar();

	auto fail = [&](const char* what, std::uint64_t bits) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL " << what << " at bits=" << bits << '\n';
	};

	for (std::uint64_t i = 0; i < NR && i + stride > i; i += stride) {
		T a; a.setbits(i);
		if (a.isnar()) continue;

		if ((a + zero).raw_bits() != a.raw_bits()) fail("a + 0 != a", i);
		if ((a - zero).raw_bits() != a.raw_bits()) fail("a - 0 != a", i);
		if ((a * one).raw_bits() != a.raw_bits())  fail("a * 1 != a", i);
		if (!(a * zero).iszero())                  fail("a * 0 != 0", i);
		if (!(a - a).iszero())                     fail("a - a != 0", i);
		if (!(a + nar).isnar())                    fail("a + NaR must be NaR", i);
		if (!(a * nar).isnar())                    fail("a * NaR must be NaR", i);
		if (!(a / zero).isnar())                   fail("a / 0 must be NaR", i);
		if (!a.iszero()) {
			if ((a / one).raw_bits() != a.raw_bits()) fail("a / 1 != a", i);
			if ((a / a).raw_bits() != one.raw_bits()) fail("a / a != 1", i);
			if (!(zero / a).iszero())                 fail("0 / a != 0", i);
			if ((-a + a).raw_bits() != 0ull)          fail("-a + a != 0", i);
		}
	}
	return nrOfFailedTests;
}

// Addition and multiplication commute bit for bit.  Independent of any reference,
// and it reaches the operand-ordering logic in the wide path's alignment step.
template<unsigned nbits, unsigned rbits>
int VerifyCommutative(std::uint64_t stride, bool reportTestCases) {
	using T = sw::universal::takum<nbits, rbits, std::uint64_t>;
	int nrOfFailedTests = 0;
	const std::uint64_t NR = (nbits >= 64) ? ~0ull : (1ull << nbits);

	for (std::uint64_t i = 0; i < NR && i + stride > i; i += stride) {
		T a; a.setbits(i);
		if (a.isnar()) continue;
		for (std::uint64_t j = 0; j < NR && j + stride > j; j += stride) {
			T b; b.setbits(j);
			if (b.isnar()) continue;
			if ((a + b).raw_bits() != (b + a).raw_bits()) {
				++nrOfFailedTests;
				if (reportTestCases) std::cout << "FAIL addition not commutative at " << i << ',' << j << '\n';
			}
			if ((a * b).raw_bits() != (b * a).raw_bits()) {
				++nrOfFailedTests;
				if (reportTestCases) std::cout << "FAIL multiplication not commutative at " << i << ',' << j << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

// The reported symptom of #1300, verbatim: one encoding step above 1.0, then add
// zero.  A named regression so the report stays legible in the suite output.
int VerifyIssue1300Repro(bool reportTestCases) {
	using namespace sw::universal;
	using T64 = takum<64, 3, std::uint64_t>;
	int nrOfFailedTests = 0;

	T64 one(1.0), eps, zero;
	zero.setzero();
	eps.setbits(one.raw_bits() + 1ull);
	if ((eps + zero).raw_bits() != eps.raw_bits()) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL takum<64,3>: eps + 0 lost the low bit\n";
	}
	if ((eps - zero).raw_bits() != eps.raw_bits()) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL takum<64,3>: eps - 0 lost the low bit\n";
	}
	if ((eps * one).raw_bits() != eps.raw_bits()) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL takum<64,3>: eps * 1 lost the low bit\n";
	}
	if ((eps / one).raw_bits() != eps.raw_bits()) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL takum<64,3>: eps / 1 lost the low bit\n";
	}
	return nrOfFailedTests;
}

} // anonymous namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
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

	std::string test_suite  = "takum wide-configuration arithmetic verification";
	std::string test_tag    = "wide arithmetic";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// Strides are odd and coprime with the field structure so a sweep crosses
	// every DR rather than revisiting one layout.  maybe_unused because which of
	// them a build reaches depends on the regression level.
	[[maybe_unused]] constexpr std::uint64_t stride64 = 0x0123456789ABCDEFull;   // ~225 samples
	[[maybe_unused]] constexpr std::uint64_t stride32 = 0x01234567ull;           // ~225 samples
	[[maybe_unused]] constexpr std::uint64_t stride16 = 211ull;                  // ~310 samples

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyIssue1300Repro(true), "takum<64,3>", "issue 1300 repro");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<64, 3>(Op::add, stride64, true), "takum<64,3>", "correctly rounded add");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(
		VerifyIssue1300Repro(true), "takum<64,3>", "issue 1300 repro");
	nrOfFailedTestCases += ReportTestResult(
		VerifyExactIdentities<64, 3>(stride64, reportTestCases), "takum<64,3>", "exact identities");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<64, 3>(Op::add, stride64, reportTestCases), "takum<64,3>", "correctly rounded add");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<64, 3>(Op::mul, stride64, reportTestCases), "takum<64,3>", "correctly rounded mul");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<64, 3>(Op::sub, stride64, reportTestCases), "takum<64,3>", "correctly rounded sub");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<64, 3>(Op::div, stride64, reportTestCases), "takum<64,3>", "correctly rounded div");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCommutative<64, 3>(stride64, reportTestCases), "takum<64,3>", "commutativity");
	// The narrow configurations keep the double path, where the operands are exact
	// doubles and one rounding decides the result.  Verified against the SAME exact
	// reference, so the boundary the gate draws is measured rather than asserted.
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<32, 3>(Op::add, stride32, reportTestCases), "takum<32,3>", "correctly rounded add");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<32, 3>(Op::mul, stride32, reportTestCases), "takum<32,3>", "correctly rounded mul");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(
		VerifyFmaCorrectlyRounded<64, 3>(stride64, reportTestCases), "takum<64,3>", "correctly rounded fma");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<32, 3>(Op::sub, stride32, reportTestCases), "takum<32,3>", "correctly rounded sub");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<32, 3>(Op::div, stride32, reportTestCases), "takum<32,3>", "correctly rounded div");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<16, 3>(Op::add, stride16, reportTestCases), "takum<16,3>", "correctly rounded add");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<16, 3>(Op::div, stride16, reportTestCases), "takum<16,3>", "correctly rounded div");
#endif

#if REGRESSION_LEVEL_4
	// takum<58,3> is the narrowest configuration on the wide path (nbits > 54 + rbits)
	// and takum<64,1> the widest significand the format allows at 64 bits, p = 61,
	// which is what fixes the round-to-odd width at 63.
	nrOfFailedTestCases += ReportTestResult(
		VerifyExactIdentities<58, 3>(stride64, reportTestCases), "takum<58,3>", "exact identities");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<58, 3>(Op::add, stride64, reportTestCases), "takum<58,3>", "correctly rounded add");
	nrOfFailedTestCases += ReportTestResult(
		VerifyExactIdentities<64, 1>(stride64, reportTestCases), "takum<64,1>", "exact identities");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<64, 1>(Op::mul, stride64, reportTestCases), "takum<64,1>", "correctly rounded mul");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<64, 1>(Op::div, stride64, reportTestCases), "takum<64,1>", "correctly rounded div");
	// rbits = 5 pushes the characteristic past 2^31, an exponent range no binary
	// floating-point type can hold.  The exact integer path carries the exponent
	// as an int64_t outside the significand, so it is indifferent; the reference
	// above is not, and skips the pairs whose span exceeds its width, which is why
	// this configuration is covered structurally.
	nrOfFailedTestCases += ReportTestResult(
		VerifyExactIdentities<64, 5>(stride64, reportTestCases), "takum<64,5>", "exact identities");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCommutative<64, 5>(stride64, reportTestCases), "takum<64,5>", "commutativity");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
