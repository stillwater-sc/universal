// limb_kernels.cpp: efloat's multiplication, addition, division, square and cube root kernels against exact references (#1762, #1763, #1800)
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
//   cbrtrem  efloat_mpn's cube root must leave S^3 + R = N with R <= 3 S^2 + 3 S, the cube by the
//            schoolbook multiply, and without its remainder lie 0 to cbrt_overshoot units above
//            the root: the one-limb base case, the recursion's odd and even splits, the squaring's
//            Karatsuba and the division's divide-and-conquer thresholds, perfect cubes and their
//            neighbours, all-ones radicands, the smallest radicand, and top limbs within 1024 of
//            a perfect cube, where the base case's double estimate is tested.
//   mulhigh  efloat_mpn's short product must lie below the full product's high limbs by under n
//            units of r[n], and mul_truncated within one unit of the floor (#1807); x * y and
//            x * x rounded from the short product, at the operands' precision, must be the exact
//            product rounded once in every mode, also for all-ones operands, where only the
//            fallback to the full product is right.
//   div_appr, sqrt_approx  efloat_mpn's quotient and root without remainders (#1810) must lie
//            at or above the exact ones, by at most divappr_error and sqrt_approx_error units.
//   load_window  efloat_mpn's shifted copy, which places the fma and hypot kernels' terms in
//            their frames (#1802), against a bit at a time, and its flag for the bits it drops.
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- 300 random products and the threshold lengths, 2,000 sums per precision,
//              300 limb quotients and the threshold lengths, 300 rounded quotients per precision
//              (a twentieth of that at 40 and 60 limbs), 300 limb square roots and cube roots
//              and the lengths, 2,000 limb windows
//   LEVEL 2 -- 3,000, 20,000, 3,000, 3,000, 3,000 and 3,000; 20,000 windows
//   LEVEL 3 -- 30,000, 200,000, 30,000, 30,000, 30,000 and 30,000; 100,000 windows
//   LEVEL 4 -- 300,000, 2,000,000, 300,000, 300,000, 300,000 and 300,000; 300,000 windows
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

	// efloat_mpn::div_appr and sqrt_approx (#1810) against div_qr and sqrtrem: the approximate
	// quotient and root never below the exact ones and above them by at most divappr_error and
	// sqrt_approx_error units of the last limb. Quotients as long as the divisor and one to three
	// limbs shorter (sqrt's shape), across the divide-and-conquer threshold and its doublings,
	// all-ones operands, and roots of 1 to 200 limbs.
	int VerifyApproximateKernels(bool reportTestCases, unsigned nrSamples) {
		using namespace sw::universal;
		using efloat_mpn::limb_t;
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(18100);
		auto excess = [](const std::vector<limb_t>& approx, const std::vector<limb_t>& exact, std::size_t len, std::size_t bound) {
			std::vector<limb_t> d(len);
			const limb_t borrow = efloat_mpn::sub_n(d.data(), approx.data(), exact.data(), len);
			bool ok = (borrow == 0u) && d[0] <= bound;
			for (std::size_t i = 1; i < len; ++i) ok = ok && d[i] == 0u;
			return ok;
		};
		const std::size_t lengths[] = { 1, 2, 15, 16, 17, 31, 32, 33, 63, 64, 65, 128, 129, 130 };
		auto check_div = [&](std::size_t dn, std::size_t qn, bool ones) {
			const std::size_t nn = dn + qn;
			std::vector<limb_t> np(nn), np2, dp(dn), q(qn + 1), q2(qn + 1), sc(efloat_mpn::div_scratch_limbs(dn) + 8);
			for (auto& v : np) v = ones ? ~limb_t(0) : rng();
			for (auto& v : dp) v = ones ? ~limb_t(0) : rng();
			dp[dn - 1] |= limb_t(1) << 63;
			if (np[nn - 1] >= dp[dn - 1]) np[nn - 1] = dp[dn - 1] - 1u;
			np2 = np;
			q[qn]  = efloat_mpn::div_qr(q.data(), np.data(), nn, dp.data(), dn, sc.data());
			q2[qn] = efloat_mpn::div_appr(q2.data(), np2.data(), nn, dp.data(), dn, sc.data());
			if (!excess(q2, q, qn + 1, efloat_mpn::divappr_error(nn, dn))) {
				++nrOfFailedTestCases;
				if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: div_appr of " << nn << " by " << dn << " limbs is outside its bound\n";
			}
		};
		auto check_root = [&](std::size_t n, bool ones) {
			std::vector<limb_t> a(2 * n), b, sp(n), sp2(n), sc(efloat_mpn::sqrt_scratch_limbs(n) + 8);
			for (auto& v : a) v = ones ? ~limb_t(0) : rng();
			a[2 * n - 1] |= limb_t(1) << 62;
			b = a;
			efloat_mpn::sqrtrem(sp.data(), a.data(), n, sc.data());
			efloat_mpn::sqrt_approx(sp2.data(), b.data(), n, sc.data());
			if (!excess(sp2, sp, n, efloat_mpn::sqrt_approx_error(n))) {
				++nrOfFailedTestCases;
				if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: sqrt_approx of " << n << " limbs is outside its bound\n";
			}
		};
		// N = D (B^qn - 1) + D - 1: an all-ones quotient, whose low halves the truncated divisor
		// pushes to B^lo, where div_appr clamps them
		auto check_ones_quotient = [&](std::size_t dn, std::size_t qn) {
			const std::size_t nn = dn + qn;
			std::vector<limb_t> dp(dn), ones(qn, ~limb_t(0)), np(nn), np2, q(qn + 1), q2(qn + 1), sc(efloat_mpn::div_scratch_limbs(dn) + 8);
			std::vector<limb_t> sm(efloat_mpn::scratch_limbs(std::max(dn, qn)) + 8);
			for (auto& v : dp) v = rng();
			dp[dn - 1] |= limb_t(1) << 63;
			efloat_mpn::mul(np.data(), dp.data(), dn, ones.data(), qn, sm.data());
			std::vector<limb_t> dm1(dp);
			efloat_mpn::sub_1(dm1.data(), dn, 1u);
			efloat_mpn::add_n(np.data(), np.data(), dm1.data(), dn);   // no carry past nn: N < D B^qn
			np2 = np;
			q[qn]  = efloat_mpn::div_qr(q.data(), np.data(), nn, dp.data(), dn, sc.data());
			q2[qn] = efloat_mpn::div_appr(q2.data(), np2.data(), nn, dp.data(), dn, sc.data());
			if (!excess(q2, q, qn + 1, efloat_mpn::divappr_error(nn, dn))) {
				++nrOfFailedTestCases;
				if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: div_appr of an all-ones quotient of " << qn << " limbs is outside its bound\n";
			}
		};
		for (std::size_t n : lengths) {
			for (bool ones : { false, true }) {
				check_div(n, n, ones);
				if (n > 1) check_div(n, n - 1, ones);
				check_root(n, ones);
			}
			check_ones_quotient(n, n);
			if (n > 1) check_ones_quotient(n, n - 1);
		}
		for (unsigned i = 0; i < nrSamples; ++i) {
			const std::size_t dn = 1 + rng() % 150;
			const std::size_t qn = (i % 3 == 0 || dn < 4) ? dn : dn - 1 - rng() % 3;
			check_div(dn, qn, (i % 17) == 3);
			check_root(1 + rng() % 200, (i % 19) == 5);
		}
		return nrOfFailedTestCases;
	}

	// efloat_mpn::mulhigh_n and mul_truncated (#1807) against the full product: R = r[n - 1, 2n)
	// below the full product's limbs from n - 1 up by less than n units of r[n], and the
	// truncated product the floor of a b / B^drop or one unit below it. Lengths straddle the
	// basecase-to-Mulders threshold (32) and the Karatsuba ones (16, 48); squares, all-ones
	// operands (whose dropped partial products are largest) and unbalanced truncated products.
	// can_round on limbs is checked at the edges of the bits it reads.
	int VerifyShortProducts(bool reportTestCases, unsigned nrSamples) {
		using namespace sw::universal;
		using efloat_mpn::limb_t;
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(1807);
		const size_t lengths[] = { 1, 2, 3, 15, 16, 17, 31, 32, 33, 47, 48, 49, 63, 64, 65, 96, 128, 129, 200 };
		auto check_high = [&](size_t n, bool square, bool ones) {
			std::vector<limb_t> a(n), b(n), full(2 * n), r(2 * n), sc(efloat_mpn::scratch_limbs(n) + 8), d(n + 1);
			for (size_t i = 0; i < n; ++i) { a[i] = ones ? ~limb_t(0) : rng(); b[i] = ones ? ~limb_t(0) : rng(); }
			const limb_t* bp = square ? a.data() : b.data();
			efloat_mpn::mul_n(full.data(), a.data(), bp, n, sc.data(), square);
			efloat_mpn::mulhigh_n(r.data(), a.data(), bp, n, sc.data(), square);
			const limb_t borrow = efloat_mpn::sub_n(d.data(), full.data() + n - 1, r.data() + n - 1, n + 1);
			bool ok = (borrow == 0u) && (n == 1 ? d[1] == 0u : d[1] < n);
			for (size_t i = 2; i <= n; ++i) ok = ok && d[i] == 0u;
			if (!ok) {
				++nrOfFailedTestCases;
				if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: mulhigh_n of " << n << " limbs is off by n units or more\n";
			}
		};
		auto check_truncated = [&](size_t na, size_t nb, size_t drop, bool ones) {
			std::vector<limb_t> a(na), b(nb), full(na + nb), out(na + nb - drop), sc(efloat_mpn::truncated_scratch_limbs(na, nb, drop) + 8);
			std::vector<limb_t> sf(efloat_mpn::scratch_limbs(std::max(na, nb)) + 8), d(na + nb - drop);
			for (auto& v : a) v = ones ? ~limb_t(0) : rng();
			for (auto& v : b) v = ones ? ~limb_t(0) : rng();
			efloat_mpn::mul(full.data(), a.data(), na, b.data(), nb, sf.data());
			efloat_mpn::mul_truncated(out.data(), a.data(), na, b.data(), nb, drop, sc.data());
			const limb_t borrow = efloat_mpn::sub_n(d.data(), full.data() + drop, out.data(), na + nb - drop);
			bool ok = (borrow == 0u) && d[0] <= 1u;
			for (size_t i = 1; i < d.size(); ++i) ok = ok && d[i] == 0u;
			if (!ok) {
				++nrOfFailedTestCases;
				if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: mul_truncated of " << na << " x " << nb << " limbs dropping " << drop << " is not within one unit of the floor\n";
			}
		};
		for (size_t n : lengths) {
			for (bool square : { false, true }) { check_high(n, square, false); check_high(n, square, true); }
			check_truncated(n, n, n, false);
			check_truncated(n + 1, n, n, true);
			check_truncated(n, n, n > 1 ? n - 1 : 0, false);
		}
		for (unsigned i = 0; i < nrSamples; ++i) {
			const size_t n = 1 + rng() % 260;
			check_high(n, (i % 3) == 0, (i % 11) == 5);
			const size_t na = 1 + rng() % 200, nb = (i % 2) ? na : 1 + rng() % 200;
			check_truncated(na, nb, rng() % (na + nb), (i % 13) == 7);
		}
		// can_round: a value whose bits past the precision are 1 0 0 ... 0 up to bit `correct - 2`
		// sits on a midpoint as far as it can tell, and is not rounded; a 1 at bit correct - 2 is
		// the last one read, and one at bit correct - 1 is not
		{
			const unsigned precision = 100, correct = 160;
			auto frame = [](int p) {   // three limbs, leading bit at 191, bit p (from the leading one) set
				std::vector<limb_t> f(3, 0u);
				f[2] = limb_t(1) << 63;
				if (p >= 0) { const unsigned q = 191u - static_cast<unsigned>(p); f[q / 64] |= limb_t(1) << (q % 64); }
				return f;
			};
			const bool ok = !efloat_mpn::can_round(frame(-1).data(), 3, precision, correct)
			             && efloat_mpn::can_round(frame(static_cast<int>(correct) - 2).data(), 3, precision, correct)
			             && !efloat_mpn::can_round(frame(static_cast<int>(correct) - 1).data(), 3, precision, correct)
			             && !efloat_mpn::can_round(frame(static_cast<int>(precision)).data(), 3, precision, correct)
			             && efloat_mpn::can_round(frame(static_cast<int>(precision) + 1).data(), 3, precision, correct)
			             && !efloat_mpn::can_round(frame(static_cast<int>(precision) + 1).data(), 3, precision, precision + 3);
			if (!ok) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "    FAIL: efloat_mpn::can_round reads the wrong bits\n";
			}
		}
		return nrOfFailedTestCases;
	}

	// x * y and x * x at the operands' own precision from 16 64-bit limbs up, where the product
	// rounds from mulhigh_n's high half when Ziv's test passes (#1807), against the exact product
	// rounded once, in every mode. All-ones operands put the exact product one unit above a
	// representable value and the short product below it, which only the fallback to the full
	// product rounds right; 1 + 2^-k operands put it next to a boundary with nothing dropped.
	int VerifyRoundedProducts(bool reportTestCases, unsigned nrSamples) {
		using namespace sw::universal;
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(18070);
		const RoundingMode modes[] = { RoundingMode::RoundToNearest, RoundingMode::RoundToZero,
		                               RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative };
		const RoundingMode saved = efloat_rounding_mode;
		const size_t lengths[] = { 30, 31, 32, 33, 34, 63, 64, 65, 66, 96, 97, 98, 128, 129, 130, 256, 257, 258 };   // 32-bit limbs
		auto check = [&](const std::vector<uint32_t>& a, const std::vector<uint32_t>& b, const char* what) {
			const unsigned P = static_cast<unsigned>(32 * std::max(a.size(), b.size()));
			const int64_t ea = static_cast<int64_t>(rng() % 41) - 20, eb = static_cast<int64_t>(rng() % 41) - 20;
			const bool sa = (rng() & 1u) != 0u, sb = (rng() & 1u) != 0u;
			const Real x = make(a, ea, sa, P), y = make(b, eb, sb, P);
			std::vector<uint32_t> p, q;
			const int64_t e  = reference_product(a, ea, b, eb, p);
			const int64_t e2 = reference_product(a, ea, a, ea, q);
			for (RoundingMode mode : modes) {
				efloat_rounding_mode = RoundingMode::RoundToNearest;
				Real want  = make(p, e, sa != sb, static_cast<unsigned>(32 * p.size()));
				Real want2 = make(q, e2, false, static_cast<unsigned>(32 * q.size()));
				efloat_rounding_mode = mode;
				want.set_precision(P);    // the exact product rounded once, in the mode
				want2.set_precision(P);
				clear_efloat_exceptions();
				const Real xy = x * y;
				const bool inexact = has_efloat_exception(ExceptionFlag::Inexact);
				Real s = x;
				s *= s;
				const bool ok = identical(xy, want) && identical(x * x, want2) && identical(s, want2) && (inexact == !(p.size() <= P / 32));
				if (!ok) {
					++nrOfFailedTestCases;
					if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: " << what << " product of " << a.size() << " limbs at " << P << " bits, mode " << int(mode) << '\n';
				}
			}
		};
		for (size_t L : lengths) {
			check(random_limbs(rng, L), random_limbs(rng, L), "random");
			check(std::vector<uint32_t>(L, 0xFFFF'FFFFu), std::vector<uint32_t>(L, 0xFFFF'FFFFu), "all-ones");
			std::vector<uint32_t> ones_but_last(L, 0xFFFF'FFFFu);
			ones_but_last.back() = static_cast<uint32_t>(rng()) | 1u;
			check(ones_but_last, std::vector<uint32_t>(L, 0xFFFF'FFFFu), "all-ones");
			std::vector<uint32_t> near(L, 0u);
			near[0] = 0x8000'0000u;
			near.back() = 1u;
			check(near, near, "1 + 2^-k");
		}
		for (unsigned i = 0; i < nrSamples; ++i) {
			const size_t L = 30 + rng() % 300;
			check(random_limbs(rng, L), random_limbs(rng, (i % 4 == 0) ? 1 + rng() % L : L), "random");
		}
		efloat_rounding_mode = saved;
		return nrOfFailedTestCases;
	}

	// efloat_mpn::load_window (#1802) against a bit-at-a-time reference: r's bit j is a's bit
	// j - shift, and the flag says whether any of a's bits below -shift is set
	int VerifyLoadWindow(bool reportTestCases, unsigned nrSamples) {
		using namespace sw::universal;
		using efloat_mpn::limb_t;
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(1802);
		for (unsigned i = 0; i < nrSamples; ++i) {
			const std::size_t na = 1 + rng() % 9;
			const std::size_t n  = 1 + rng() % 12;
			std::vector<limb_t> a(na);
			for (limb_t& v : a) v = (i % 4 == 0) ? (limb_t(1) << (rng() % 64)) : rng();   // sparse ones find the flag's edges
			const int64_t shift = static_cast<int64_t>(rng() % (64 * 24)) - 64 * 12 + ((i % 8 == 0) ? 0 : static_cast<int64_t>(rng() % 3) - 1);
			std::vector<limb_t> r(n, ~limb_t(0));
			const bool lost = efloat_mpn::load_window(r.data(), n, a.data(), na, shift);
			auto bit = [&a, na](int64_t j) -> unsigned { return (j >= 0 && j < 64 * static_cast<int64_t>(na)) ? unsigned((a[static_cast<std::size_t>(j / 64)] >> (j % 64)) & 1u) : 0u; };
			bool ok = true;
			for (int64_t j = 0; ok && j < 64 * static_cast<int64_t>(n); ++j) {
				ok = (((r[static_cast<std::size_t>(j / 64)] >> (j % 64)) & 1u) == bit(j - shift));
			}
			bool want_lost = false;
			for (int64_t j = 0; j < -shift && !want_lost; ++j) want_lost = (bit(j) != 0u);
			if (!ok || lost != want_lost) {
				++nrOfFailedTestCases;
				if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: load_window of " << na << " limbs into " << n << " at shift " << shift << '\n';
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

	// S^3 + R = N with R <= 3 S^2 + 3 S for efloat_mpn::cbrtrem, the cube by the schoolbook
	// multiply; and without the remainder, S is the root plus 0 to cbrt_overshoot units
	int VerifyCubeRoot(bool reportTestCases, std::mt19937_64& rng, size_t n) {
		using namespace sw::universal::efloat_mpn;
		const unsigned style = static_cast<unsigned>(rng() % 7);
		std::vector<limb_t> a(3 * n);
		for (auto& x : a) {
			const uint64_t r = rng();
			x = (style == 1) ? ~limb_t(0) : (style == 2 && (r & 3u) != 0u) ? ((r & 4u) ? ~limb_t(0) : 0u) : rng();
		}
		if (style == 3) {   // a perfect cube, or one away from it
			std::vector<limb_t> b(n), b2(2 * n), work(scratch_limbs(2 * n));
			for (auto& x : b) x = rng();
			b[n - 1] |= limb_t(1) << 63;
			mul_basecase(b2.data(), b.data(), n, b.data(), n);
			mul_basecase(a.data(), b2.data(), 2 * n, b.data(), n);
			if (rng() & 1u) { if (rng() & 1u) add_1(a.data(), 3 * n, 1u); else sub_1(a.data(), 3 * n, 1u); }
		}
		if (style == 4) {   // the top limb at or near a perfect cube, where the base case's double estimate is tested
			const limb_t root = (limb_t(1) << 20) | (rng() & 0xF'FFFFu);   // root^3 >= 2^60
			a[3 * n - 1] = root * root * root + (rng() % 2048) - 1024;
		}
		if (style == 5) {   // the smallest normalized radicand, 2^(192 n - 3)
			for (auto& x : a) x = 0;
		}
		if (a[3 * n - 1] < (limb_t(1) << 61)) a[3 * n - 1] |= limb_t(1) << 61;   // normalized
		std::vector<limb_t> s(n), r(2 * n + 1), approx(n), scratch(cbrt_scratch_limbs(n));
		cbrtrem(s.data(), r.data(), a.data(), n, scratch.data(), true);
		cbrtrem(approx.data(), nullptr, a.data(), n, scratch.data(), false);

		// S^3 + R = N
		std::vector<limb_t> s2(2 * n), check(3 * n + 1, 0u);
		mul_basecase(s2.data(), s.data(), n, s.data(), n);
		mul_basecase(check.data(), s2.data(), 2 * n, s.data(), n);
		check[3 * n] = add_1(check.data() + 2 * n + 1, n - 1, add_n(check.data(), check.data(), r.data(), 2 * n + 1));
		bool ok = (check[3 * n] == 0u);
		for (size_t i = 0; ok && i < 3 * n; ++i) ok = (check[i] == a[i]);
		// R <= 3 S^2 + 3 S
		std::vector<limb_t> bound(2 * n + 1, 0u);
		bound[2 * n] = mul_1(bound.data(), s2.data(), 2 * n, 3u);
		add_1(bound.data() + n, n + 1, addmul_1(bound.data(), s.data(), n, 3u));
		ok = ok && geq_n(bound.data(), r.data(), 2 * n + 1);
		// 0 <= approx - S <= cbrt_overshoot
		std::vector<limb_t> d(n);
		const limb_t below = sub_n(d.data(), approx.data(), s.data(), n);
		bool small = (below == 0u) && d[0] <= cbrt_overshoot;
		for (size_t i = 1; small && i < n; ++i) small = (d[i] == 0u);
		if (!ok && reportTestCases) std::cout << "    FAIL: " << n << " limb cube root: S^3 + R != N or R > 3S^2 + 3S\n";
		if (!small && reportTestCases) std::cout << "    FAIL: " << n << " limb cube root without remainder: more than " << cbrt_overshoot << " units from the root\n";
		return (ok && small) ? 0 : 1;
	}

	int VerifyCubeRootKernel(bool reportTestCases, unsigned nrSamples) {
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(1800);
		// one limb (the base case), the recursion's odd splits, the squaring's Karatsuba threshold
		// in the half-length squares, and the division's divide-and-conquer threshold
		for (size_t n : { size_t(1), size_t(2), size_t(3), size_t(4), size_t(5), size_t(7), size_t(16), size_t(17), size_t(31), size_t(32), size_t(33), size_t(64), size_t(65), size_t(129) }) {
			for (int k = 0; k < 7; ++k) nrOfFailedTestCases += VerifyCubeRoot(reportTestCases && nrOfFailedTestCases < 4, rng, n);
		}
		for (unsigned i = 0; i < nrSamples; ++i) {
			nrOfFailedTestCases += VerifyCubeRoot(reportTestCases && nrOfFailedTestCases < 4, rng, 1 + rng() % 150);
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

	std::string test_suite          = "efloat multiplication, addition, division, square and cube root kernels";
	std::string test_tag            = "limb kernels";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyExactProducts(reportTestCases, 30), "efloat", "exact products");
	nrOfFailedTestCases += ReportTestResult(VerifyAlignedSums(reportTestCases, 100), "efloat", "rounded sums");
	nrOfFailedTestCases += ReportTestResult(VerifyLoadWindow(reportTestCases, 2000), "efloat", "limb windows");
	nrOfFailedTestCases += ReportTestResult(VerifyShortProducts(reportTestCases, 100), "efloat", "short products");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedProducts(reportTestCases, 20), "efloat", "products from short products");
	nrOfFailedTestCases += ReportTestResult(VerifyApproximateKernels(reportTestCases, 100), "efloat", "approximate quotients and roots");
	nrOfFailedTestCases += ReportTestResult(VerifyQuotientKernel(reportTestCases, 100), "efloat", "limb quotients");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedQuotients(reportTestCases, 100), "efloat", "rounded quotients");
	nrOfFailedTestCases += ReportTestResult(VerifySquareRootKernel(reportTestCases, 100), "efloat", "limb square roots");
	nrOfFailedTestCases += ReportTestResult(VerifyCubeRootKernel(reportTestCases, 100), "efloat", "limb cube roots");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExactProducts(reportTestCases, 300), "efloat", "exact products");
	nrOfFailedTestCases += ReportTestResult(VerifyAlignedSums(reportTestCases, 2000), "efloat", "rounded sums");
	nrOfFailedTestCases += ReportTestResult(VerifyLoadWindow(reportTestCases, 2000), "efloat", "limb windows");
	nrOfFailedTestCases += ReportTestResult(VerifyShortProducts(reportTestCases, 300), "efloat", "short products");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedProducts(reportTestCases, 60), "efloat", "products from short products");
	nrOfFailedTestCases += ReportTestResult(VerifyApproximateKernels(reportTestCases, 3000), "efloat", "approximate quotients and roots");
	nrOfFailedTestCases += ReportTestResult(VerifyQuotientKernel(reportTestCases, 300), "efloat", "limb quotients");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedQuotients(reportTestCases, 300), "efloat", "rounded quotients");
	nrOfFailedTestCases += ReportTestResult(VerifySquareRootKernel(reportTestCases, 300), "efloat", "limb square roots");
	nrOfFailedTestCases += ReportTestResult(VerifyCubeRootKernel(reportTestCases, 300), "efloat", "limb cube roots");
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyExactProducts(reportTestCases, 3000), "efloat", "exact products");
	nrOfFailedTestCases += ReportTestResult(VerifyAlignedSums(reportTestCases, 20000), "efloat", "rounded sums");
	nrOfFailedTestCases += ReportTestResult(VerifyLoadWindow(reportTestCases, 20000), "efloat", "limb windows");
	nrOfFailedTestCases += ReportTestResult(VerifyShortProducts(reportTestCases, 3000), "efloat", "short products");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedProducts(reportTestCases, 300), "efloat", "products from short products");
	nrOfFailedTestCases += ReportTestResult(VerifyApproximateKernels(reportTestCases, 20000), "efloat", "approximate quotients and roots");
	nrOfFailedTestCases += ReportTestResult(VerifyQuotientKernel(reportTestCases, 3000), "efloat", "limb quotients");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedQuotients(reportTestCases, 3000), "efloat", "rounded quotients");
	nrOfFailedTestCases += ReportTestResult(VerifySquareRootKernel(reportTestCases, 3000), "efloat", "limb square roots");
	nrOfFailedTestCases += ReportTestResult(VerifyCubeRootKernel(reportTestCases, 3000), "efloat", "limb cube roots");
#	endif

#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyExactProducts(reportTestCases, 30000), "efloat", "exact products");
	nrOfFailedTestCases += ReportTestResult(VerifyAlignedSums(reportTestCases, 200000), "efloat", "rounded sums");
	nrOfFailedTestCases += ReportTestResult(VerifyLoadWindow(reportTestCases, 100000), "efloat", "limb windows");
	nrOfFailedTestCases += ReportTestResult(VerifyShortProducts(reportTestCases, 30000), "efloat", "short products");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedProducts(reportTestCases, 3000), "efloat", "products from short products");
	nrOfFailedTestCases += ReportTestResult(VerifyApproximateKernels(reportTestCases, 30000), "efloat", "approximate quotients and roots");
	nrOfFailedTestCases += ReportTestResult(VerifyQuotientKernel(reportTestCases, 30000), "efloat", "limb quotients");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedQuotients(reportTestCases, 30000), "efloat", "rounded quotients");
	nrOfFailedTestCases += ReportTestResult(VerifySquareRootKernel(reportTestCases, 30000), "efloat", "limb square roots");
	nrOfFailedTestCases += ReportTestResult(VerifyCubeRootKernel(reportTestCases, 30000), "efloat", "limb cube roots");
#	endif

#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyExactProducts(reportTestCases, 300000), "efloat", "exact products");
	nrOfFailedTestCases += ReportTestResult(VerifyAlignedSums(reportTestCases, 2000000), "efloat", "rounded sums");
	nrOfFailedTestCases += ReportTestResult(VerifyLoadWindow(reportTestCases, 300000), "efloat", "limb windows");
	nrOfFailedTestCases += ReportTestResult(VerifyShortProducts(reportTestCases, 100000), "efloat", "short products");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedProducts(reportTestCases, 10000), "efloat", "products from short products");
	nrOfFailedTestCases += ReportTestResult(VerifyApproximateKernels(reportTestCases, 100000), "efloat", "approximate quotients and roots");
	nrOfFailedTestCases += ReportTestResult(VerifyQuotientKernel(reportTestCases, 300000), "efloat", "limb quotients");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedQuotients(reportTestCases, 300000), "efloat", "rounded quotients");
	nrOfFailedTestCases += ReportTestResult(VerifySquareRootKernel(reportTestCases, 300000), "efloat", "limb square roots");
	nrOfFailedTestCases += ReportTestResult(VerifyCubeRootKernel(reportTestCases, 300000), "efloat", "limb cube roots");
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
