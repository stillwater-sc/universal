#pragma once
// sqrt.hpp: sqrt functions for adaptive precision decimal rationals
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cerrno>
#include <cstdint>
#include <universal/number/edecimal/math/sqrt.hpp>   // the exact integer square root
#include <universal/number/erational/numeric_limits.hpp>

namespace sw { namespace universal {

	namespace erational_sqrt_detail {

		// 2^k by binary exponentiation: O(log k) multiplications
		inline edecimal pow2(unsigned k) {
			edecimal result(1), base(2);
			while (k != 0) {
				if (k & 1u) result *= base;
				k >>= 1;
				if (k != 0) base *= base;
			}
			return result;
		}

		// the number of bits of n > 0: the b with 2^(b-1) <= n < 2^b.  A d-digit n is at least
		// 10^(d-1) >= 2^floor((d-1) 3.3219), as 3.3219 < log2(10), so the search starts there
		// and needs at most four doublings instead of one per bit.
		inline unsigned bit_length(const edecimal& n) {
			const std::uint64_t d = n.size();
			unsigned b = static_cast<unsigned>(((d > 0 ? d - 1 : 0) * 33219ull) / 10000ull);
			edecimal p = pow2(b);                     // p = 2^b <= n
			const edecimal two(2);
			while (!(n < p)) { p *= two; ++b; }
			return b;
		}

	}  // namespace erational_sqrt_detail

	// sqrt for erational (#1725).
	//
	// The root of a rational is rational exactly when the reduced numerator and denominator are
	// both perfect squares, and then sqrt returns it exactly.  Otherwise it is irrational, and
	// sqrt(x, bits) returns the nearest multiple of 2^-k to it, ties to even, with k chosen so
	// that the result carries `bits` significant bits: k = bits - 1 - floor(floor(log2 x) / 2),
	// from exact bit lengths, so 2^(bits-1) <= sqrt(x) 2^k < 2^bits.  The default, 106 bits, is
	// a double-double's precision.
	//
	// With x = p/q reduced, sqrt(x) 2^k = sqrt(p 4^k / q) (or sqrt(p / (q 4^-k)) for k < 0), so
	// the result is R / 2^k for the integer R nearest sqrt(A/B): R0 = floor(sqrt(floor(A/B))) is
	// floor(sqrt(A/B)), and the root rounds up exactly when 4A > (2 R0 + 1)^2 B; equality is the
	// tie.  The floor roots are edecimal's exact integer square roots.
	//
	// This used to be erational(std::sqrt(double(x))), 53 bits at most, and wrote
	// "erational_negative_sqrt_arg" to std::cerr on EVERY call when the throwing configuration
	// was off (the message sat outside the sign test).  The native branch of the
	// ERATIONAL_NATIVE_SQRT switch referenced names that do not exist and never compiled; the
	// switch is gone.  A negative argument now throws erational_negative_sqrt_arg under
	// ERATIONAL_THROW_ARITHMETIC_EXCEPTION, and otherwise sets errno = EDOM and returns NaN.
	inline erational sqrt(const erational& x, unsigned bits = 106) {
		using namespace erational_sqrt_detail;
		if (x.isnan()) return x;
		if (x.iszero()) return x;
		if (x.isneg()) {
#if ERATIONAL_THROW_ARITHMETIC_EXCEPTION
			throw erational_negative_sqrt_arg();
#else
			errno = EDOM;                             // as std::sqrt: EDOM and a NaN, with no message
			erational nan;
			nan.setnumerator(edecimal(0));
			nan.setdenominator(edecimal(0));
			return nan;
#endif
		}
		if (bits == 0) bits = 1;
		// x need not be reduced: erational(8, 18) keeps 8/18, whose root 2/3 is exact.  Reduce
		// local copies, by Euclid, so the result does not depend on how x was constructed.
		edecimal p = x.top(), q = x.bottom();
		{
			edecimal a = p, b = q;
			while (!b.iszero()) { edecimal t = a % b; a = b; b = t; }
			p /= a;
			q /= a;
		}
		const edecimal rp = sqrt(p), rq = sqrt(q);
		erational r;
		if (rp * rp == p && rq * rq == q) {             // a rational root: exact, and already reduced
			r.setnumerator(rp);
			r.setdenominator(rq);
			return r;
		}
		// e = floor(log2(p/q)): the bit-length difference, or one less when p < q 2^diff
		const int bp = static_cast<int>(bit_length(p)), bq = static_cast<int>(bit_length(q));
		int e = bp - bq;
		if (e >= 0 ? (p < q * pow2(static_cast<unsigned>(e))) : (p * pow2(static_cast<unsigned>(-e)) < q)) --e;
		const int half = (e >= 0) ? e / 2 : -((1 - e) / 2);           // floor(e / 2)
		const int k = static_cast<int>(bits) - 1 - half;
		edecimal A = p, B = q;
		if (k >= 0) A *= pow2(2u * static_cast<unsigned>(k)); else B *= pow2(2u * static_cast<unsigned>(-k));
		edecimal R = sqrt(A / B);                       // floor(sqrt(A/B))
		const edecimal twoR1 = edecimal(2) * R + edecimal(1);
		const edecimal lhs = edecimal(4) * A, rhs = twoR1 * twoR1 * B;
		if (rhs < lhs || (lhs == rhs && !((R % edecimal(2)).iszero()))) R += edecimal(1);   // ties to even
		// R / 2^k, reduced: R > 0 shares only factors of two with 2^k
		int kk = k;
		const edecimal two(2);
		while (kk > 0 && (R % two).iszero()) { R /= two; --kk; }
		if (kk >= 0) {
			r.setnumerator(R);
			r.setdenominator(pow2(static_cast<unsigned>(kk)));
		}
		else {
			r.setnumerator(R * pow2(static_cast<unsigned>(-kk)));
			r.setdenominator(edecimal(1));
		}
		return r;
	}

	// reciprocal sqrt
//	inline rational rsqrt(const rational& f) {
//		rational rsqrt = sqrt(f);
//		return rsqrt.reciprocate();
//	}

	///////////////////////////////////////////////////////////////////
	// specialized sqrt configurations

}} // namespace sw::universal
