// limb_kernels.cpp: efloat's multiplication, addition, division and square root kernels against exact references (#1762, #1763)
//
//   x * y    at a precision wide enough to hold the whole product, so the result is exact: its
//            limbs must be the schoolbook product's, normalized, and no Inexact flag is raised.
//            Operands of 1 to 300 limbs reach the schoolbook kernel, Karatsuba (from 24 64-bit
//            limbs, 48 of efloat's), its odd splits, and the block loop for unbalanced operands;
//            odd limb counts exercise the zero limb the packing pads them with.
//   x * x    and x *= x take the squaring kernel.
//   x + y    and x - y align the operand with the smaller exponent to the other, keep two limbs
//            past the precision and fold whatever falls below them into a sticky bit.  The
//            result must be the exact sum, formed at a precision that holds all of it, rounded
//            once to the target, in every rounding mode: exponent gaps of 0 to 40 limbs, either
//            operand the longer, and near-cancellations, including one by an operand that carries
//            no precision and is longer than the target across a one-bit gap (#1767: it lost its
//            last bits to the sticky bit, and 2 - (2 - 2^-127) came out 2^-126 - 2^-158).
//   div_qr   efloat_mpn's division on 64-bit limbs must leave N = Q D + R with R < D, the product
//            by the schoolbook multiply: one- and two-limb divisors, the divide-and-conquer
//            threshold and its doublings, quotients shorter and longer than the divisor, and
//            dividends whose top limbs equal the divisor's.
//   x / y    must be the exact quotient, from a bit-serial reference, rounded once to the target
//            in every rounding mode: dividends and divisors longer than the quotient, exact
//            quotients, and exact quotients but for a dividend limb that only the sticky bit sees.
//   sqrtrem  efloat_mpn's square root must leave S^2 + R = N with R <= 2 S, the square by the
//            schoolbook multiply: the one- and two-limb base cases, the recursion's odd splits,
//            perfect squares and their neighbours, all-ones radicands, and top limbs within
//            1024 of a perfect square, where the base case's double estimate can be one low.
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- 300 random products and the threshold lengths, 2,000 sums per precision,
//              300 limb quotients and the threshold lengths, 300 rounded quotients per precision
//              (a twentieth of that at 40 and 60 limbs), 300 limb square roots and the lengths
//   LEVEL 2 -- 3,000, 20,000, 3,000, 3,000 and 3,000
//   LEVEL 3 -- 30,000, 200,000, 30,000, 30,000 and 30,000
//   LEVEL 4 -- 300,000, 2,000,000, 300,000, 300,000 and 300,000
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using Real = sw::universal::efloat<64>;

	// a normalized significand of n limbs, most significant first, with runs of all-ones and
	// all-zero limbs, which carry and borrow the furthest
	std::vector<uint32_t> random_limbs(std::mt19937_64& rng, size_t n) {
		std::vector<uint32_t> l(n);
		const unsigned style = static_cast<unsigned>(rng() % 4);
		for (auto& x : l) {
			const uint64_t r = rng();
			x = (style == 1) ? 0xFFFF'FFFFu : (style == 2 && (r & 3u) != 0u) ? ((r & 4u) ? 0xFFFF'FFFFu : 0u) : static_cast<uint32_t>(r >> 8);
		}
		l[0] |= 0x8000'0000u;
		if (rng() % 2) l.back() |= 1u;   // the last limb counts
		return l;
	}

	Real make(const std::vector<uint32_t>& limbs, int64_t exponent, bool negative, unsigned precision_bits) {
		Real v(1);
		v.set_precision(precision_bits);   // before the limbs: set_precision rounds to it
		for (size_t i = 0; i < limbs.size(); ++i) v.setlimb(static_cast<unsigned>(i), limbs[i]);
		v.setexponent(exponent);
		v.setsign(negative);
		return v;
	}

	// the exact product of a and b, most significant limb first, as efloat holds it: shifted to
	// its leading bit, trailing zero limbs dropped; returns the exponent of the leading bit
	int64_t reference_product(const std::vector<uint32_t>& a, int64_t ea, const std::vector<uint32_t>& b, int64_t eb, std::vector<uint32_t>& p) {
		const size_t m = a.size(), n = b.size();
		std::vector<uint32_t> r(m + n, 0u);   // least significant first
		for (size_t i = 0; i < m; ++i) {
			uint64_t carry = 0;
			for (size_t j = 0; j < n; ++j) {
				const uint64_t t = uint64_t(a[m - 1 - i]) * b[n - 1 - j] + r[i + j] + carry;
				r[i + j] = static_cast<uint32_t>(t);
				carry    = t >> 32;
			}
			r[i + n] = static_cast<uint32_t>(carry);
		}
		p.assign(r.rbegin(), r.rend());
		int64_t exponent = ea + eb + 1;
		if ((p[0] & 0x8000'0000u) == 0u) {   // a product of normalized significands has 0 or 1 leading zero bits
			for (size_t i = 0; i + 1 < p.size(); ++i) p[i] = (p[i] << 1) | (p[i + 1] >> 31);
			p.back() <<= 1;
			--exponent;
		}
		while (p.size() > 1 && p.back() == 0u) p.pop_back();
		return exponent;
	}

	bool matches(const Real& v, const std::vector<uint32_t>& p, int64_t exponent, bool negative) {
		return !v.iszero() && !v.isnan() && !v.isinf() && v.bits() == p && v.scale() == exponent && (v.sign() == -1) == negative;
	}

	int VerifyProduct(bool reportTestCases, std::mt19937_64& rng, size_t m, size_t n) {
		using namespace sw::universal;
		const auto     a  = random_limbs(rng, m);
		const auto     b  = random_limbs(rng, n);
		const int64_t  ea = static_cast<int64_t>(rng() % 201) - 100, eb = static_cast<int64_t>(rng() % 201) - 100;
		const bool     sa = (rng() & 1u) != 0u, sb = (rng() & 1u) != 0u;
		const unsigned bits = static_cast<unsigned>(64 * std::max(m, n));   // room for x * y and x * x
		const Real x = make(a, ea, sa, bits), y = make(b, eb, sb, bits);
		int failures = 0;

		std::vector<uint32_t> p;
		int64_t e = reference_product(a, ea, b, eb, p);
		clear_efloat_exceptions();
		const Real xy = x * y;
		const bool exact = !has_efloat_exception(ExceptionFlag::Inexact);
		if (!matches(xy, p, e, sa != sb) || !exact) {
			++failures;
			if (reportTestCases) std::cout << "    FAIL: " << m << " x " << n << " limb product differs from the schoolbook reference\n";
		}

		e = reference_product(a, ea, a, ea, p);
		Real s = x;
		s *= s;
		if (!matches(x * x, p, e, false) || !matches(s, p, e, false)) {
			++failures;
			if (reportTestCases) std::cout << "    FAIL: " << m << " limb square differs from the schoolbook reference\n";
		}
		return failures;
	}

	int VerifyExactProducts(bool reportTestCases, unsigned nrSamples) {
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(1762);
		// the Karatsuba thresholds, 24 and 32 64-bit limbs, and twice them, with odd neighbours
		const size_t lengths[] = { 1, 2, 3, 4, 7, 8, 9, 46, 47, 48, 49, 50, 63, 64, 65, 66, 95, 96, 97, 98, 128, 129, 255, 256 };
		for (size_t m : lengths) {
			nrOfFailedTestCases += VerifyProduct(reportTestCases, rng, m, m);
			nrOfFailedTestCases += VerifyProduct(reportTestCases, rng, m, 1 + rng() % m);
		}
		for (unsigned i = 0; i < nrSamples; ++i) {
			const size_t m = 1 + rng() % 300;
			const size_t n = (i % 3 == 0) ? m : 1 + rng() % 300;
			nrOfFailedTestCases += VerifyProduct(reportTestCases && nrOfFailedTestCases < 4, rng, m, n);
		}
		return nrOfFailedTestCases;
	}

	bool identical(const Real& a, const Real& b) {
		return a.sign() == b.sign() && a.scale() == b.scale() && a.bits() == b.bits()
		    && a.get_precision() == b.get_precision() && a.carries_precision() == b.carries_precision();
	}

	int VerifyAlignedSums(bool reportTestCases, unsigned nrSamples) {
		using namespace sw::universal;
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(17621);
		const RoundingMode modes[] = { RoundingMode::RoundToNearest, RoundingMode::RoundToZero,
		                               RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative };
		const unsigned precisions[] = { 1, 2, 5, 16 };   // in limbs
		const RoundingMode saved = efloat_rounding_mode;
		for (unsigned p : precisions) {
			for (unsigned i = 0; i < nrSamples; ++i) {
				const unsigned target = 32 * p;
				const size_t   nx     = 1 + rng() % p;
				size_t         ny     = 1 + rng() % p;
				const int64_t  ex     = static_cast<int64_t>(rng() % 41) - 20;
				int64_t        gap    = (i % 4 == 0) ? static_cast<int64_t>(rng() % 3) : static_cast<int64_t>(rng() % (32 * 40));
				const bool     sx     = (rng() & 1u) != 0u;
				const bool     sy     = (i % 3 == 0) ? !sx : ((rng() & 1u) != 0u);   // a third subtract
				Real x = make(random_limbs(rng, nx), ex, sx, target);
				Real y = make(random_limbs(rng, ny), ex - gap, sy, target);
				if (i % 5 == 0) {
					// a Y that carries no precision and is longer than the target, a gap of 0 to 2
					// bits, and x = 2^ex against y = 2^ex - 2^-k: the difference cancels nearly all
					// of y's bits
					const size_t     wide_ny = p + 1 + rng() % 4;
					std::vector<uint32_t> ones(wide_ny, 0xFFFF'FFFFu);
					if (rng() & 1u) ones.back() = static_cast<uint32_t>(rng()) | 1u;
					gap = static_cast<int64_t>(rng() % 3);
					x = make(std::vector<uint32_t>{ 0x8000'0000u }, ex, sx, target);
					y = make((rng() & 1u) ? ones : random_limbs(rng, wide_ny), ex - gap, !sx, 32 * static_cast<unsigned>(wide_ny));
					y.clear_precision();
					ny = wide_ny;
				}
				// the exact sum: at this precision neither operand loses a bit to the alignment
				const unsigned wide = static_cast<unsigned>(32 * (nx + ny + gap / 32 + 4));
				Real X = x, Y = y;
				X.set_precision(wide);
				Y.set_precision(wide);
				efloat_rounding_mode = modes[i % 4];
				Real sum = X + Y, difference = X - Y;
				sum.set_precision(target);          // rounds once, in the current mode
				difference.set_precision(target);
				const bool ok = identical(x + y, sum) && identical(y + x, sum) && identical(x - y, difference);
				efloat_rounding_mode = saved;
				if (!ok) {
					++nrOfFailedTestCases;
					if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: x + y with an exponent gap of " << gap << " bits at " << target << " bits is not the rounded exact sum\n";
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// N = Q D + R with R < D for efloat_mpn::div_qr: the product by the schoolbook multiply,
	// which shares nothing with the division
	int VerifyQuotient(bool reportTestCases, std::mt19937_64& rng, size_t dn, size_t qn) {
		using namespace sw::universal::efloat_mpn;
		auto random64 = [&rng](size_t n, unsigned style) {
			std::vector<limb_t> v(n);
			for (auto& x : v) {
				const uint64_t r = rng();
				x = (style == 1) ? ~limb_t(0) : (style == 2 && (r & 3u) != 0u) ? ((r & 4u) ? ~limb_t(0) : 0u) : rng();
			}
			return v;
		};
		const size_t nn = dn + qn;
		std::vector<limb_t> d = random64(dn, static_cast<unsigned>(rng() % 3));
		d[dn - 1] |= limb_t(1) << 63;
		std::vector<limb_t> n = random64(nn, static_cast<unsigned>(rng() % 3));
		if (rng() % 4 == 0) {   // the dividend's top limbs equal the divisor's: the estimate B - 1
			for (size_t i = 0; i < dn; ++i) n[qn + i] = d[i];
			if (qn > 0 && (rng() & 1u)) n[nn - 2] = d[dn - 1];
		}
		std::vector<limb_t> r = n, q(qn + 1), scratch(div_scratch_limbs(dn));
		q[qn] = div_qr(q.data(), r.data(), nn, d.data(), dn, scratch.data());

		std::vector<limb_t> check(qn + 1 + dn);
		mul_basecase(check.data(), q.data(), qn + 1, d.data(), dn);
		const limb_t carry = add_1(check.data() + dn, qn + 1, add_n(check.data(), check.data(), r.data(), dn));
		bool ok = (carry == 0u) && (check[nn] == 0u) && !geq_n(r.data(), d.data(), dn);
		for (size_t i = 0; ok && i < nn; ++i) ok = (check[i] == n[i]);
		if (!ok && reportTestCases) std::cout << "    FAIL: " << qn << " by " << dn << " limb division: N != Q D + R or R >= D\n";
		return ok ? 0 : 1;
	}

	int VerifyQuotientKernel(bool reportTestCases, unsigned nrSamples) {
		using sw::universal::efloat_mpn::dc_div_threshold;
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(1763);
		// one-limb and two-limb divisors, the divide-and-conquer threshold and its doublings,
		// quotients shorter than, as long as, and several blocks longer than the divisor
		const size_t t = dc_div_threshold;
		const size_t lengths[] = { 1, 2, 3, t - 1, t, t + 1, 2 * t - 1, 2 * t, 2 * t + 1, 4 * t, 4 * t + 3, 128, 129 };
		for (size_t dn : lengths) {
			for (size_t qn : { size_t(0), size_t(1), t - 1, t, dn - 1, dn, dn + 1, 2 * dn, 3 * dn + t + 2 }) {
				nrOfFailedTestCases += VerifyQuotient(reportTestCases && nrOfFailedTestCases < 4, rng, dn, qn);
			}
		}
		for (unsigned i = 0; i < nrSamples; ++i) {
			const size_t dn = 1 + rng() % 200;
			const size_t qn = (i % 4 == 0) ? rng() % 8 : rng() % 300;
			nrOfFailedTestCases += VerifyQuotient(reportTestCases && nrOfFailedTestCases < 4, rng, dn, qn);
		}
		return nrOfFailedTestCases;
	}

	// S^2 + R = N with R <= 2 S for efloat_mpn::sqrtrem: the square by the schoolbook multiply
	int VerifySquareRoot(bool reportTestCases, std::mt19937_64& rng, size_t n) {
		using namespace sw::universal::efloat_mpn;
		const unsigned style = static_cast<unsigned>(rng() % 6);
		std::vector<limb_t> a(2 * n);
		for (auto& x : a) {
			const uint64_t r = rng();
			x = (style == 1) ? ~limb_t(0) : (style == 2 && (r & 3u) != 0u) ? ((r & 4u) ? ~limb_t(0) : 0u) : rng();
		}
		if (style == 3) {   // a perfect square, or one away from it
			std::vector<limb_t> b(n), work(scratch_limbs(n));
			for (auto& x : b) x = rng();
			b[n - 1] |= limb_t(1) << 63;
			mul_n(a.data(), b.data(), b.data(), n, work.data(), true);
			if (rng() & 1u) { if (rng() & 1u) add_1(a.data(), 2 * n, 1u); else sub_1(a.data(), 2 * n, 1u); }
		}
		if (style == 4) {   // the top limb at or near a perfect square, where the base case's double estimate can be one low
			const limb_t root = (limb_t(1) << 31) | (rng() & 0x7FFF'FFFFu);
			a[2 * n - 1] = root * root + (rng() % 2048) - 1024;
		}
		if (a[2 * n - 1] < (limb_t(1) << 62)) a[2 * n - 1] |= limb_t(1) << 62;   // normalized
		std::vector<limb_t> r = a, sq(n), scratch(sqrt_scratch_limbs(n));
		const limb_t carry = sqrtrem(sq.data(), r.data(), n, scratch.data());

		std::vector<limb_t> check(2 * n + 1, 0u), twice(n + 1, 0u);
		mul_basecase(check.data(), sq.data(), n, sq.data(), n);
		check[2 * n] = add_1(check.data() + n, n, add_n(check.data(), check.data(), r.data(), n));
		check[2 * n] += add_1(check.data() + n, n, carry);
		bool ok = (check[2 * n] == 0u);
		for (size_t i = 0; ok && i < 2 * n; ++i) ok = (check[i] == a[i]);
		// R <= 2 S: compare carry B^n + r with 2 S
		twice[n] = add_n(twice.data(), sq.data(), sq.data(), n);
		std::vector<limb_t> rem(n + 1);
		for (size_t i = 0; i < n; ++i) rem[i] = r[i];
		rem[n] = carry;
		ok = ok && geq_n(twice.data(), rem.data(), n + 1) && carry <= 1u;
		if (!ok && reportTestCases) std::cout << "    FAIL: " << n << " limb square root: S^2 + R != N or R > 2S\n";
		return ok ? 0 : 1;
	}

	int VerifySquareRootKernel(bool reportTestCases, unsigned nrSamples) {
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(17632);
		// one and two limbs (the base cases), the recursion's odd splits, and the division's
		// divide-and-conquer threshold in the half-length divisions
		for (size_t n : { size_t(1), size_t(2), size_t(3), size_t(4), size_t(5), size_t(7), size_t(31), size_t(32), size_t(33), size_t(64), size_t(65), size_t(129) }) {
			for (int k = 0; k < 5; ++k) nrOfFailedTestCases += VerifySquareRoot(reportTestCases && nrOfFailedTestCases < 4, rng, n);
		}
		for (unsigned i = 0; i < nrSamples; ++i) {
			nrOfFailedTestCases += VerifySquareRoot(reportTestCases && nrOfFailedTestCases < 4, rng, 1 + rng() % 150);
		}
		return nrOfFailedTestCases;
	}

	// floor(A 2^k / B) as MSB-first limbs without leading zero limbs, a bit at a time; returns
	// whether a remainder is left.  A and B are the MSB-first limb vectors as integers.
	bool reference_quotient(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b, size_t k, std::vector<uint32_t>& q) {
		const size_t nb = b.size();
		std::vector<uint32_t> d(b.rbegin(), b.rend()), r(nb + 1, 0u);   // least significant first
		const size_t total = 32 * a.size() + k;
		std::vector<uint32_t> bits((total + 31) / 32, 0u);   // the quotient, least significant first
		for (size_t t = total; t-- > 0;) {
			uint32_t in = 0;
			if (t >= k) {
				const size_t j = t - k;   // bit j of A
				in = (a[a.size() - 1 - j / 32] >> (j % 32)) & 1u;
			}
			for (size_t i = nb + 1; i-- > 1;) r[i] = (r[i] << 1) | (r[i - 1] >> 31);
			r[0] = (r[0] << 1) | in;
			bool geq = (r[nb] != 0u);
			if (!geq) {
				geq = true;
				for (size_t i = nb; i-- > 0;) {
					if (r[i] != d[i]) { geq = (r[i] > d[i]); break; }
				}
			}
			if (geq) {
				int64_t borrow = 0;
				for (size_t i = 0; i <= nb; ++i) {
					const int64_t v = int64_t(r[i]) - (i < nb ? int64_t(d[i]) : 0) - borrow;
					r[i]   = static_cast<uint32_t>(v);
					borrow = (v < 0) ? 1 : 0;
				}
				bits[t / 32] |= 1u << (t % 32);
			}
		}
		q.assign(bits.rbegin(), bits.rend());
		while (q.size() > 1 && q[0] == 0u) q.erase(q.begin());
		for (uint32_t w : r) if (w != 0u) return true;
		return false;
	}

	// x / y must be the exact quotient rounded once to the target in every rounding mode: the
	// reference quotient with enough bits and a sticky limb, rounded by set_precision.  Operands
	// of 1 to p limbs, dividends and divisors that carry no precision and are longer than the
	// quotient (their extra bits reach the result only through the sticky bit), exact quotients
	// (x y) / y, and exact quotients but for one limb of the dividend below the quotient's reach.
	int VerifyRoundedQuotients(bool reportTestCases, unsigned nrSamples) {
		using namespace sw::universal;
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(17631);
		const RoundingMode modes[] = { RoundingMode::RoundToNearest, RoundingMode::RoundToZero,
		                               RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative };
		const unsigned precisions[] = { 1, 2, 5, 16, 40, 60 };   // in limbs: 60 reaches divide-and-conquer, 64 is Real's cap
		const RoundingMode saved = efloat_rounding_mode;
		for (unsigned p : precisions) {
			const unsigned samples = (p >= 40) ? 1 + nrSamples / 20 : nrSamples;
			for (unsigned i = 0; i < samples; ++i) {
				const unsigned target = 32 * p;
				// 0, 1: within the precision, 2: a long dividend, 3: a long divisor, 4: exact, 5: exact but
				// for a limb below the quotient's last bit, which only the sticky bit can see
				const unsigned shape  = i % 6;
				// a long operand must still fit Real's 64 limbs, or clear_precision() rounds it
				const size_t cap = Real::maxNrLimbs;
				size_t nx = 1 + rng() % p, ny = 1 + rng() % p;
				if (shape == 2 && p + 3 <= cap) {   // more dividend bits than the quotient can use
					ny = 1 + rng() % std::min<size_t>(p, cap - p - 2);   // y is rounded to p limbs
					nx = p + ny + 1 + rng() % (cap - p - ny);
				}
				if (shape == 3) ny = p + 1 + rng() % std::min<size_t>(8, cap - p);
				std::vector<uint32_t> a = random_limbs(rng, nx), b = random_limbs(rng, ny);
				const int64_t ex = static_cast<int64_t>(rng() % 201) - 100, ey = static_cast<int64_t>(rng() % 201) - 100;
				const bool    sx = (rng() & 1u) != 0u, sy = (rng() & 1u) != 0u;
				Real x = make(a, ex, sx, 32 * static_cast<unsigned>(nx)), y = make(b, ey, sy, 32 * static_cast<unsigned>(ny));
				int64_t eax = ex;
				bool    nax = sx;   // the dividend's exponent and sign
				if (shape >= 4) {   // x = (an operand) * y, exact at a precision that holds it
					const size_t room = std::min<size_t>(p, Real::maxNrLimbs - ny);   // x must fit Real, which holds 64 limbs
					std::vector<uint32_t> c = random_limbs(rng, 1 + rng() % room);
					const Real cx = make(c, ex, sx, 32 * static_cast<unsigned>(c.size() + ny));
					Real yy = y;
					yy.set_precision(32 * static_cast<unsigned>(c.size() + ny));
					x   = cx * yy;
					a   = x.bits();
					eax = x.scale();
					nax = (x.sign() == -1);
					if (shape == 5 && p + ny + 4 <= cap) {
						a.resize(p + ny + 2 + rng() % 3, 0u);   // past the quotient's last bit, then nonzero
						a.back() = static_cast<uint32_t>(rng()) | 1u;
						nx = a.size();
						x  = make(a, eax, nax, 32 * static_cast<unsigned>(nx));
					}
					nx  = a.size();
				}
				if (shape == 2 || shape >= 4) { x.clear_precision(); y.set_precision(target); }   // x keeps every bit
				else if (shape == 3) { y.clear_precision(); x.set_precision(target); }
				else { x.set_precision(target); y.set_precision(target); }

				// the quotient with at least target + 2 limbs of bits, the leading one weighing
				// 2^(ex - ey + 32 (ny - nx) - k + L - 1) for L bits
				const size_t k = 32 * (p + 2 + ((ny > nx) ? ny - nx : 0));
				std::vector<uint32_t> q;
				const bool sticky = reference_quotient(a, b, k, q);
				const int64_t length = 32 * static_cast<int64_t>(q.size()) - static_cast<int64_t>(sw::universal::clz(q[0]));
				const int64_t eq = eax - ey + 32 * (static_cast<int64_t>(ny) - static_cast<int64_t>(nx)) - static_cast<int64_t>(k) + length - 1;
				const unsigned sh = static_cast<unsigned>(sw::universal::clz(q[0]));
				if (sh != 0u) {
					for (size_t j = 0; j + 1 < q.size(); ++j) q[j] = (q[j] << sh) | (q[j + 1] >> (32u - sh));
					q.back() <<= sh;
				}
				if (sticky) q.push_back(1u);
				Real reference = make(q, eq, nax != sy, 32 * static_cast<unsigned>(q.size()));
				efloat_rounding_mode = modes[i % 4];
				reference.set_precision(target);   // rounds once, in the current mode
				const Real quotient = x / y;
				Real compound = x;
				compound /= y;
				const bool ok = identical(quotient, reference) && identical(compound, reference);
				efloat_rounding_mode = saved;
				if (!ok) {
					++nrOfFailedTestCases;
					if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: " << nx << " by " << ny << " limb x / y (shape " << shape << ") at " << target << " bits is not the rounded exact quotient\n";
				}
			}
		}
		return nrOfFailedTestCases;
	}

}  // anonymous namespace

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

	std::string test_suite          = "efloat multiplication, addition, division and square root kernels";
	std::string test_tag            = "limb kernels";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyExactProducts(reportTestCases, 30), "efloat", "exact products");
	nrOfFailedTestCases += ReportTestResult(VerifyAlignedSums(reportTestCases, 100), "efloat", "rounded sums");
	nrOfFailedTestCases += ReportTestResult(VerifyQuotientKernel(reportTestCases, 100), "efloat", "limb quotients");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedQuotients(reportTestCases, 100), "efloat", "rounded quotients");
	nrOfFailedTestCases += ReportTestResult(VerifySquareRootKernel(reportTestCases, 100), "efloat", "limb square roots");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExactProducts(reportTestCases, 300), "efloat", "exact products");
	nrOfFailedTestCases += ReportTestResult(VerifyAlignedSums(reportTestCases, 2000), "efloat", "rounded sums");
	nrOfFailedTestCases += ReportTestResult(VerifyQuotientKernel(reportTestCases, 300), "efloat", "limb quotients");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedQuotients(reportTestCases, 300), "efloat", "rounded quotients");
	nrOfFailedTestCases += ReportTestResult(VerifySquareRootKernel(reportTestCases, 300), "efloat", "limb square roots");
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyExactProducts(reportTestCases, 3000), "efloat", "exact products");
	nrOfFailedTestCases += ReportTestResult(VerifyAlignedSums(reportTestCases, 20000), "efloat", "rounded sums");
	nrOfFailedTestCases += ReportTestResult(VerifyQuotientKernel(reportTestCases, 3000), "efloat", "limb quotients");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedQuotients(reportTestCases, 3000), "efloat", "rounded quotients");
	nrOfFailedTestCases += ReportTestResult(VerifySquareRootKernel(reportTestCases, 3000), "efloat", "limb square roots");
#	endif

#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyExactProducts(reportTestCases, 30000), "efloat", "exact products");
	nrOfFailedTestCases += ReportTestResult(VerifyAlignedSums(reportTestCases, 200000), "efloat", "rounded sums");
	nrOfFailedTestCases += ReportTestResult(VerifyQuotientKernel(reportTestCases, 30000), "efloat", "limb quotients");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedQuotients(reportTestCases, 30000), "efloat", "rounded quotients");
	nrOfFailedTestCases += ReportTestResult(VerifySquareRootKernel(reportTestCases, 30000), "efloat", "limb square roots");
#	endif

#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyExactProducts(reportTestCases, 300000), "efloat", "exact products");
	nrOfFailedTestCases += ReportTestResult(VerifyAlignedSums(reportTestCases, 2000000), "efloat", "rounded sums");
	nrOfFailedTestCases += ReportTestResult(VerifyQuotientKernel(reportTestCases, 300000), "efloat", "limb quotients");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedQuotients(reportTestCases, 300000), "efloat", "rounded quotients");
	nrOfFailedTestCases += ReportTestResult(VerifySquareRootKernel(reportTestCases, 300000), "efloat", "limb square roots");
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
