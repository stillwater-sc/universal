// limb_kernels.cpp: efloat's multiplication, addition, division, square and cube root kernels against exact references (#1762, #1763, #1800)
//
//   x * y    at a precision wide enough to hold the whole product, so the result is exact: its
//            limbs must be the schoolbook product's, normalized, and no Inexact flag is raised.
//            Operands of 1 to 300 limbs reach the schoolbook kernel, Karatsuba (from 24 64-bit
//            limbs, 48 of efloat's), its odd splits, and the block loop for unbalanced operands;
//            odd limb counts exercise the zero limb the packing pads them with.
//   x * x    and x *= x take the squaring kernel.
//   mul_n    by Toom-3 (#1814), from 240 limbs (352 for squares), against the schoolbook
//            product: lengths either side of the thresholds, every n mod 3, nested Toom-3 from
//            720 limbs, and operands that stress the evaluations' signs and carries.
//   x + y    and x - y align the operand with the smaller exponent to the other, keep two limbs
//            past the precision and fold whatever falls below them into a sticky bit.  The
//            result must be the exact sum, formed at a precision that holds all of it, rounded
//            once to the target, in every rounding mode: exponent gaps of 0 to 40 limbs, either
//            operand the longer, and near-cancellations, including one by an operand that carries
//            no precision and is longer than the target across a one-bit gap (#1767: it lost its
//            last bits to the sticky bit, and 2 - (2 - 2^-127) came out 2^-126 - 2^-158).
//            Operands both as long as the precision take a shorter path (#1822, #1826), sums and
//            differences, at precisions of whole 64-bit limbs and of an odd number of 32-bit
//            ones: gaps of 0 to 64 m - 1 bits, every whole number of limbs and its neighbours
//            among them, and deep cancellations, against the same exact reference.
//   x * y    of one to four limbs at their full precision (#1826): the short products in
//            fixed-size code, against the exact product rounded once, in every mode.
//   sqrt     at 32 to 256 bits (#1826): the exact root with its remainder, and the approximate
//            one, bracketed by exact squares, Inexact included; squares one unit off, whose roots
//            lie next to a rounding boundary (#1828).
//   short    the division kernels for quotients of a few limbs, which estimate in hardware
//            (#1826), against the ones that divide by a reciprocal.
//   2-by-1   udiv_qr_2by1, divrem_1's step by a reciprocal (#1830), and udiv_qrnnd on dividends
//            built as q d + r, remainders and quotients at both ends of their ranges.
//   div_qr   limb_kernels's division on 64-bit limbs must leave N = Q D + R with R < D, the product
//            by the schoolbook multiply: one- and two-limb divisors, the divide-and-conquer
//            threshold and its doublings, quotients shorter and longer than the divisor, and
//            dividends whose top limbs equal the divisor's.
//   x / y    at 128 bits with a remainder a unit either side of half the divisor (#1828).
//   x / y    must be the exact quotient, from a bit-serial reference, rounded once to the target
//            in every rounding mode: dividends and divisors longer than the quotient, exact
//            quotients, and exact quotients but for a dividend limb that only the sticky bit sees.
//   sqrtrem4 the four-limb root in registers (#1828) against the general one: random radicands,
//            perfect squares and their neighbours, top limbs at a perfect square, the smallest.
//   twice_sub_n  2 a - b in one pass (#1828), checked with the kernels below.
//   sqrtrem  limb_kernels's square root must leave S^2 + R = N with R <= 2 S, the square by the
//            schoolbook multiply: the one- and two-limb base cases, the recursion's odd splits,
//            perfect squares and their neighbours, all-ones radicands, and top limbs within
//            1024 of a perfect square, where the base case's double estimate can be one low.
//   cbrtrem  limb_kernels's cube root must leave S^3 + R = N with R <= 3 S^2 + 3 S, the cube by the
//            schoolbook multiply, and without its remainder lie 0 to cbrt_overshoot units above
//            the root: the one-limb base case, the recursion's odd and even splits, the squaring's
//            Karatsuba and the division's divide-and-conquer thresholds, perfect cubes and their
//            neighbours, all-ones radicands, the smallest radicand, and top limbs within 1024 of
//            a perfect cube, where the base case's double estimate is tested.
//   mulhigh  limb_kernels's short product must lie below the full product's high limbs by under n
//            units of r[n], and mul_truncated within one unit of the floor (#1807); x * y and
//            x * x rounded from the short product, at the operands' precision, must be the exact
//            product rounded once in every mode, also for all-ones operands, where only the
//            fallback to the full product is right.
//   x / y    next to and on exact quotients from 1088-bit operands, where the quotient comes
//            from div_appr and only the exact fallback rounds right (#1811).
//   x / n    for integers of 2 to 64 bits, divided in place by one limb (#1811), against the
//            general long division at a width that holds the quotient, rounded once.
//   div_appr, sqrt_approx  limb_kernels's quotient and root without remainders (#1810) must lie
//            at or above the exact ones, by at most divappr_error and sqrt_approx_error units.
//   mul_1, addmul_1, submul_1, add_n, sub_n  limb_kernels's x86-64 assembly (#1816) against the
//            portable loops it replaces, where the processor has it: lengths 1 to 300, every
//            n mod 4, all-ones and all-zero runs, all-ones multipliers, the carries and borrows
//            out of the top, and destinations that are a source.  ASan and UBSan do not see
//            inside inline assembly, so this comparison is the check on it.
//   load_window  limb_kernels's shifted copy, which places the fma and hypot kernels' terms in
//            their frames (#1802), against a bit at a time, and its flag for the bits it drops.
//   constexpr  the kernels evaluated by the compiler (#1819), on their portable loops, integer
//            root seeds and new/delete scratch, against the same calls at run time: random
//            operands, a perfect cube, and the carries out of an all-ones limb and equal limbs.
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- every limb kernel at lengths 1 to 300 and 3,000 more (30,000, 300,000 and
//              1,000,000 at the levels below), 300 random products and the threshold lengths,
//              2,000 sums per precision,
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
#include <array>
#include <cstdint>
#include <random>
#include <vector>
#include <universal/number/efloat/efloat.hpp>
#include <universal/number/efloat/efloat_mpn.hpp>   // the deprecated name, kept for one release (#1823)
#include <universal/verification/test_suite.hpp>
#include <type_traits>

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
		// the Karatsuba thresholds, 24 and 64 64-bit limbs (48 and 128 of efloat's), and twice them, with odd neighbours
		const size_t lengths[] = { 1, 2, 3, 4, 7, 8, 9, 46, 47, 48, 49, 50, 63, 64, 65, 66, 95, 96, 97, 98, 127, 128, 129, 130, 255, 256, 257 };
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

	// x + y and x - y for operands both as long as a precision of m 64-bit limbs, 64 m bits or
	// 64 m - 32, at gaps of 0 to 64 m - 1 bits (#1820, #1826): the sum and the difference that
	// shift y into the result's limbs, with a guard bit of y's bit gap - 1, in the limb below gap /
	// 64 when the gap is a whole number of limbs, a carry out of the top that shifts a sum down, and
	// a difference that cancels one leading bit or, at a gap of one bit, any number of them. One to
	// four limbs run in fixed-size code, longer operands in loops. The result must be the exact sum
	// rounded once, in every rounding mode.
	int VerifyFullLengthSums(bool reportTestCases, unsigned nrSamples) {
		using namespace sw::universal;
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(18203);
		const RoundingMode modes[] = { RoundingMode::RoundToNearest, RoundingMode::RoundToZero,
		                               RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative };
		const RoundingMode saved = efloat_rounding_mode;
		for (unsigned m : { 1u, 2u, 3u, 4u, 8u, 17u }) {
			for (unsigned short_by : { 0u, 32u }) {   // a precision of 64 m bits, or of an odd number of 32-bit limbs
				const unsigned target = 64 * m - short_by;
				std::vector<int64_t> gaps = { 0, 1, 2, 31, 32, 33, 63 };
				for (unsigned k = 1; k < m; ++k) {
					gaps.push_back(64 * k - 1);
					gaps.push_back(64 * k);
					gaps.push_back(64 * k + 1);
				}
				gaps.push_back(64 * static_cast<int64_t>(m) - 1);
				for (unsigned i = 0; i < nrSamples; ++i) {
					int64_t    gap = (i < 4 * gaps.size()) ? gaps[i % gaps.size()] : static_cast<int64_t>(rng() % target);
					const bool sx = (rng() & 1u) != 0u;
					bool       sy = (i % 2 == 0) ? !sx : ((rng() & 1u) != 0u);   // half are differences
					std::vector<uint32_t> lx = random_limbs(rng, target / 32), ly = random_limbs(rng, target / 32);
					lx.back() |= 1u;   // both as long as the precision
					ly.back() |= 1u;
					if (i % 7 == 3) {   // y next to 2 x: a gap of one bit that cancels deeply
						ly = lx;
						ly.back() ^= static_cast<uint32_t>(rng()) | 1u;
					}
					if (i % 13 == 6) {
						// a difference one bit apart that cancels a limb or more, which twice the
						// difference, 2 X - Y, leaves to the general rounding (#1828): x just above a
						// power of 2 and y, its significand all ones but for its last bits, just below it
						std::fill(lx.begin(), lx.end(), 0u);
						lx.front() = 0x8000'0000u;
						lx.back() |= 1u + (static_cast<uint32_t>(rng()) & 6u);
						std::fill(ly.begin(), ly.end(), 0xFFFF'FFFFu);
						ly.back() &= ~(static_cast<uint32_t>(rng()) & 0xFFFEu);
						gap = 1;
						sy  = !sx;
					}
					if (i % 11 == 5 && m >= (short_by != 0u ? 3u : 2u)) {
						// a sum that carries out of the top while the limb shifted out of y holds only its
						// lowest bit: x all ones but for its last two bits, 01, so that the sum carries and its
						// shift leaves a last bit of 0 and a guard bit of 1, a tie to nearest but for the bit
						// below; y its top bit and bit gap - 64 of its 64-bit limbs, which is in
						// its last 32-bit limb for a gap of 64 to 95 bits, or 96 to 127 bits when the precision is
						// an odd number of 32-bit limbs and the 64-bit limbs are padded at the bottom; the
						// carry's shift must keep that bit in the sticky bit
						const int64_t g = 64 + static_cast<int64_t>(short_by) + static_cast<int64_t>(rng() % 32);
						std::fill(lx.begin(), lx.end(), 0xFFFF'FFFFu);
						lx.back() = 0xFFFF'FFFDu;
						std::fill(ly.begin(), ly.end(), 0u);
						ly.front() = 0x8000'0000u;
						ly.back() = 1u << ((g - 64 - static_cast<int64_t>(short_by)) % 32);
						const int64_t ex2 = static_cast<int64_t>(rng() % 41) - 20;
						const Real x2 = make(lx, ex2, sx, target);
						const Real y2 = make(ly, ex2 - g, sx, target);
						Real X2 = x2, Y2 = y2;
						X2.set_precision(2 * target + static_cast<unsigned>(g) + 64);
						Y2.set_precision(2 * target + static_cast<unsigned>(g) + 64);
						efloat_rounding_mode = modes[i % 4];
						clear_efloat_exceptions();
						const Real r2 = x2 + y2;
						const uint32_t flags = get_efloat_exceptions();
						Real sum2 = X2 + Y2;
						clear_efloat_exceptions();
						sum2.set_precision(target);
						const bool ok2 = identical(r2, sum2) && flags == get_efloat_exceptions();
						efloat_rounding_mode = saved;
						if (!ok2) {
							++nrOfFailedTestCases;
							if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: a carrying sum at " << target << " bits with a gap of " << g << " bits loses the bit below its shifted-out limb\n";
						}
					}
					const int64_t ex = static_cast<int64_t>(rng() % 41) - 20;
					const Real x = make(lx, ex, sx, target);
					const Real y = make(ly, ex - gap, sy, target);
					// the exact sum and difference, at a precision that holds all of them
					Real X = x, Y = y;
					X.set_precision(2 * target + static_cast<unsigned>(gap) + 64);
					Y.set_precision(2 * target + static_cast<unsigned>(gap) + 64);
					efloat_rounding_mode = modes[i % 4];
					Real sum = X + Y, difference = X - Y, reverse = Y - X;
					sum.set_precision(target);   // round once, in the current mode
					difference.set_precision(target);
					reverse.set_precision(target);
					Real accumulated = x, decremented = x;
					accumulated += y;
					decremented -= y;
					const bool ok = identical(x + y, sum) && identical(y + x, sum) && identical(accumulated, sum)
					             && identical(x - y, difference) && identical(decremented, difference) && identical(y - x, reverse);
					efloat_rounding_mode = saved;
					if (!ok) {
						++nrOfFailedTestCases;
						if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: x + y or x - y at " << target << " bits, both full length, with a gap of " << gap << " bits is not the rounded exact result\n";
					}
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// x * y and x * x for operands both as long as a precision of one to four 64-bit limbs, 32 to
	// 256 bits (#1826): the products of one to three limbs that multiply and round in fixed-size
	// code, and the general product past them. The result must be the exact product rounded once,
	// in every mode, with all-ones operands, whose products round up out of the top, among them.
	int VerifySmallProducts(bool reportTestCases, unsigned nrSamples) {
		using namespace sw::universal;
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(18264);
		const RoundingMode modes[] = { RoundingMode::RoundToNearest, RoundingMode::RoundToZero,
		                               RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative };
		const RoundingMode saved = efloat_rounding_mode;
		for (unsigned target : { 32u, 64u, 96u, 128u, 160u, 192u, 224u, 256u }) {
			for (unsigned i = 0; i < nrSamples; ++i) {
				std::vector<uint32_t> lx = random_limbs(rng, target / 32), ly = random_limbs(rng, target / 32);
				if (i % 3 != 2) { lx.back() |= 1u; ly.back() |= 1u; }   // mostly full length
				const Real x = make(lx, static_cast<int64_t>(rng() % 41) - 20, (rng() & 1u) != 0u, target);
				const Real y = make(ly, static_cast<int64_t>(rng() % 41) - 20, (rng() & 1u) != 0u, target);
				Real X = x, Y = y;
				X.set_precision(2 * target + 64);
				Y.set_precision(2 * target + 64);
				efloat_rounding_mode = modes[i % 4];
				Real product = X * Y, square = X * X;   // exact
				product.set_precision(target);
				square.set_precision(target);
				Real accumulated = x;
				accumulated *= y;
				const bool ok = identical(x * y, product) && identical(accumulated, product) && identical(x * x, square);
				efloat_rounding_mode = saved;
				if (!ok) {
					++nrOfFailedTestCases;
					if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: x * y at " << target << " bits is not the rounded exact product\n";
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// sqrt(x) at 32 to 256 bits (#1826): the exact root with its remainder, at a precision of whole
	// 64-bit limbs up to 256 bits, and the approximate root otherwise. Each root is checked by
	// bracketing with exact squares: r^2 <= x < (r + u)^2 rounding down, (r - u')^2 < x <= r^2
	// rounding up, (r - u' / 2)^2 < x < (r + u / 2)^2 to nearest, u the unit of r's last bit and u'
	// the one below r (u / 2 when r is a power of 2); and Inexact must say whether r^2 != x. Exact
	// squares, whose roots end in zeros, are among the arguments.
	int VerifySmallSquareRoots(bool reportTestCases, unsigned nrSamples) {
		using namespace sw::universal;
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(18265);
		const RoundingMode modes[] = { RoundingMode::RoundToNearest, RoundingMode::RoundToZero,
		                               RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative };
		const RoundingMode saved = efloat_rounding_mode;
		for (unsigned target : { 32u, 64u, 96u, 128u, 160u, 192u, 256u }) {
			const unsigned wide = 4 * target + 64;
			for (unsigned i = 0; i < nrSamples; ++i) {
				std::vector<uint32_t> lx = random_limbs(rng, target / 32);
				Real x = make(lx, static_cast<int64_t>(rng() % 41) - 20, false, target);
				if (i % 4 == 3) {   // an exact square of a root of half the precision
					std::vector<uint32_t> lc = random_limbs(rng, std::max(1u, target / 64));
					const Real c = make(lc, static_cast<int64_t>(rng() % 21) - 10, false, target);
					x = c * c;   // exact: 2 (target / 2) bits
					if (i % 8 == 7) {
						// a unit of the last bit off it: the root lies just above or just below an exact
						// one, within the approximate root's error of a rounding boundary at 96 bits,
						// where only the exact root decides (#1828)
						Real u(1);
						u.set_precision(target);
						u.setexponent(x.scale() - static_cast<int64_t>(target) + 1);
						x = (rng() & 1u) ? x + u : x - u;
					}
				}
				efloat_rounding_mode = modes[i % 4];
				clear_efloat_exceptions();
				const Real r = sqrt(x);
				const bool inexact = has_efloat_exception(ExceptionFlag::Inexact);
				efloat_rounding_mode = saved;
				Real R = r, X = x;
				R.set_precision(wide);
				X.set_precision(wide);
				Real u(1), ul(1);
				u.set_precision(wide);
				u.setexponent(r.scale() - static_cast<int64_t>(target) + 1);
				ul = u;
				if (r.bits().size() == 1 && r.bits()[0] == 0x8000'0000u) ul.setexponent(u.scale() - 1);   // r a power of 2
				const Real r2 = R * R;
				bool ok = (inexact == (r2 != X));
				switch (modes[i % 4]) {
				case RoundingMode::RoundToNearest: {
					Real h = u, hl = ul;
					h.setexponent(u.scale() - 1);
					hl.setexponent(ul.scale() - 1);
					const Real lo = R - hl, hi = R + h;
					ok = ok && (lo * lo <= X) && (X <= hi * hi);
					break;
				}
				case RoundingMode::RoundToZero:
				case RoundingMode::RoundTowardNegative: {
					const Real hi = R + u;
					ok = ok && (r2 <= X) && (X < hi * hi);
					break;
				}
				case RoundingMode::RoundTowardPositive: {
					const Real lo = R - ul;
					ok = ok && (lo * lo < X) && (X <= r2);
					break;
				}
				}
				if (!ok) {
					++nrOfFailedTestCases;
					if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: sqrt(x) at " << target << " bits is not correctly rounded\n";
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// the division kernels for quotients of a few limbs (#1826): divrem_1_short and
	// sb_div_qr_short, which estimate each quotient limb by the processor's division where the
	// build has one, against divrem_1 and sbpi1_div_qr, which divide by a reciprocal, on the same
	// operands: random limbs, all-ones and all-zero runs, a top equal to the divisor's and a
	// dividend just below it, and dividends whose first estimate is two too large.
	int VerifyShortDivisionKernels(bool reportTestCases, unsigned nrSamples) {
		using namespace sw::universal::limb_kernels;
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(18266);
		auto pick = [&rng](int style) -> limb_t {
			switch (style) {
			case 1:  return ~limb_t(0);
			case 2:  return (rng() & 1u) ? ~limb_t(0) : 0u;
			case 3:  return rng() & 0xFFu;
			default: return rng();
			}
		};
		for (unsigned it = 0; it < nrSamples; ++it) {
			const std::size_t dn = 1 + rng() % 8, qn = rng() % 5, nn = dn + qn;
			const int style = static_cast<int>(rng() % 4);
			std::vector<limb_t> d(dn), a(nn), q1(qn + 1), q2(qn + 1);
			for (auto& x : d) x = pick(style);
			d[dn - 1] |= limb_t(1) << 63;
			for (auto& x : a) x = pick(static_cast<int>(rng() % 4));
			if (it % 4 == 1 && qn > 0) for (std::size_t i = 0; i < dn; ++i) a[qn + i] = d[i];   // the top equals the divisor
			if (it % 4 == 2 && nn >= 2) { a[nn - 1] = d[dn - 1]; if (dn >= 2) a[nn - 2] = d[dn - 2]; }
			if (it % 8 == 3) {   // just below the divisor
				for (std::size_t i = 0; i < dn; ++i) a[qn + i] = d[i];
				std::size_t k = qn;
				while (k < nn && a[k] == 0u) ++k;
				if (k < nn) --a[k];
			}
			if (it % 16 == 7 && dn >= 2 && qn >= 1) {
				// a first estimate two too large (Knuth's bound): d1 = 2^63 + small, d0 near B, and the
				// dividend's top limbs d1 - 1 and near B, which the estimate by d1 alone overshoots
				d[dn - 1] = (limb_t(1) << 63) + (rng() & 3u);
				d[dn - 2] = ~limb_t(0) - (rng() & 3u);
				a[nn - 1] = d[dn - 1] - 1u;
				a[nn - 2] = ~limb_t(0) - (rng() & 3u);
			}
			std::vector<limb_t> b = a;
			q1[qn] = (dn == 1) ? divrem_1_short(q1.data(), a.data(), nn, d[0]) : sb_div_qr_short(q1.data(), a.data(), nn, d.data(), dn);
			q2[qn] = (dn == 1) ? divrem_1(q2.data(), b.data(), nn, d[0], invert_limb(d[0]))
			                   : sbpi1_div_qr(q2.data(), b.data(), nn, d.data(), dn, invert_pi1(d[dn - 1], d[dn - 2]));
			bool same = (q1 == q2);
			for (std::size_t i = 0; same && i < dn; ++i) same = (a[i] == b[i]);
			if (!same) {
				++nrOfFailedTestCases;
				if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: a short division by " << dn << " limbs differs from the reciprocal division\n";
			}
		}
		return nrOfFailedTestCases;
	}

	// udiv_qr_2by1, the step of divrem_1 by a reciprocal, written for its dependency chain in
	// #1830: N = q d + r built from a chosen q and r, so that q and r are known, for normalized
	// d: remainders of 0, 1, d - 2, d - 1 and random, quotients of 0, 1, B - 1 and random, and
	// divisors of 2^63, B - 1 and random; both corrections are reached
	int VerifyTwoByOneSteps(bool reportTestCases, unsigned nrSamples) {
		using namespace sw::universal::limb_kernels;
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(18301);
		for (unsigned it = 0; it < nrSamples; ++it) {
			limb_t d = rng() | (limb_t(1) << 63);
			if (it % 7 == 1) d = limb_t(1) << 63;
			if (it % 7 == 2) d = ~limb_t(0);
			if (it % 7 == 3) d = (limb_t(1) << 63) + (rng() & 0xFFu);
			limb_t q = rng();
			if (it % 5 == 1) q = 0u;
			if (it % 5 == 2) q = 1u;
			if (it % 5 == 3) q = ~limb_t(0) - (rng() & 3u);
			limb_t r = rng() % d;
			switch (it % 6) {
			case 1: r = 0u; break;
			case 2: r = 1u; break;
			case 3: r = d - 2u; break;
			case 4: r = d - 1u; break;
			default: break;
			}
			limb_t n1 = 0, n0 = 0;
			umul(q, d, n1, n0);   // N = q d + r < d B
			n0 += r;
			n1 += (n0 < r) ? 1u : 0u;
			limb_t q2 = 0, r2 = 0;
			udiv_qr_2by1(q2, r2, n1, n0, d, invert_limb(d));
			limb_t q3 = 0, r3 = 0;
			udiv_qrnnd(q3, r3, n1, n0, d);
			if (q2 != q || r2 != r || q3 != q || r3 != r) {
				++nrOfFailedTestCases;
				if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: a 2-by-1 division step gives the wrong quotient or remainder\n";
			}
		}
		return nrOfFailedTestCases;
	}

	// x / y where the quotient comes from div_appr (#1811), operands of 16 64-bit limbs and more,
	// on and next to exact quotients: x = y q exactly, y of P bits and q of P / 2, at 2P + 64
	// bits, so that the quotient must be q itself, with no Inexact flag; and x = y q plus or minus
	// a unit of x's last bit, whose quotient lies an ulp or so off q. The approximate quotient
	// lands on q or a few units above it, which only the exact fallback rounds right. The near
	// cases are checked by bracketing with exact products: r y <= x < (r + ulp) y rounding down,
	// (r - ulp) y < x <= r y rounding up, and the nearer of the two to nearest, ties to even.
	int VerifyQuotientsNextToExact(bool reportTestCases, unsigned nrSamples) {
		using namespace sw::universal;
		using E = efloat<256>;
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(18111);
		const RoundingMode modes[] = { RoundingMode::RoundToNearest, RoundingMode::RoundToZero,
		                               RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative };
		const RoundingMode saved = efloat_rounding_mode;
		auto full = [&rng](unsigned bits, int64_t e) {
			E v(1);
			v.set_precision(bits);
			for (unsigned j = 0; j < bits / 32; ++j) v.setlimb(j, static_cast<uint32_t>(rng()) | (j == 0 ? 0x8000'0000u : 0u));
			v.set_precision(bits);
			v.setexponent(e);
			return v;
		};
		for (unsigned i = 0; i < nrSamples; ++i) {
			efloat_rounding_mode = RoundingMode::RoundToNearest;
			const unsigned P  = 32 * (34 + static_cast<unsigned>(rng() % 60));   // 1088 to 3008 bits
			const unsigned Wx = 2 * P + 64;                                      // x's precision, and the quotient's
			const E y = full(P, static_cast<int64_t>(rng() % 21) - 10);
			const E q = full(P / 2 - (P / 2) % 32, static_cast<int64_t>(rng() % 21) - 10);
			E x(y);
			x.set_precision(Wx);
			x *= q;   // exact: P + P / 2 bits
			const int which = static_cast<int>(i % 3);   // y q, y q + an ulp of x, y q - an ulp of x
			if (which != 0) {
				E unit(1);
				unit.setexponent(x.scale() - static_cast<int64_t>(Wx) + 1);
				x = (which == 1) ? x + unit : x - unit;
			}
			x.set_precision(Wx);
			for (RoundingMode mode : modes) {
				efloat_rounding_mode = mode;
				clear_efloat_exceptions();
				const E r = x / y;
				const bool inexact = has_efloat_exception(ExceptionFlag::Inexact);
				efloat_rounding_mode = RoundingMode::RoundToNearest;
				bool ok = (r.get_precision() == Wx) && (inexact == (which != 0));
				if (which == 0) {
					ok = ok && (r == q);
				}
				else {
					// exact products at a width that holds them: r y - x and its neighbours' (r +- ulp) y - x
					const unsigned wide = 2 * Wx + 2 * P + 128;
					E ulp(1);
					ulp.setexponent(r.scale() - static_cast<int64_t>(Wx) + 1);
					E rw(r), yw(y), xw(x), uw(ulp);
					rw.set_precision(wide); yw.set_precision(wide); xw.set_precision(wide); uw.set_precision(wide);
					const E d0 = rw * yw - xw, dup = (rw + uw) * yw - xw, ddn = (rw - uw) * yw - xw;
					const bool down_ok = !(d0 > E(0)) && dup > E(0);    // r y <= x < (r + ulp) y
					const bool up_ok   = !(d0 < E(0)) && ddn < E(0);    // (r - ulp) y < x <= r y
					if (mode == RoundingMode::RoundTowardNegative || mode == RoundingMode::RoundToZero) ok = ok && down_ok;
					else if (mode == RoundingMode::RoundTowardPositive) ok = ok && up_ok;
					else {
						E a0 = d0; a0.setsign(false);
						E au = dup; au.setsign(false);
						E ad = ddn; ad.setsign(false);
						ok = ok && !(a0 > au) && !(a0 > ad);   // no neighbour nearer (a tie cannot occur: x / y is not a binary midpoint here)
					}
				}
				if (!ok) {
					++nrOfFailedTestCases;
					if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: a quotient " << (which == 0 ? "on" : "next to") << " an exact one at " << Wx << " bits, mode " << int(mode) << '\n';
				}
			}
		}
		efloat_rounding_mode = saved;
		return nrOfFailedTestCases;
	}

	// x / n and x /= n for integers of 2 to 64 bits (#1811), which divide x's significand by one
	// limb in place, against the general long division by efloat(n) carried at 4P + 512 bits and
	// rounded once: a directed rounding repeated in its direction is the single rounding, and to
	// nearest the wide quotient would have to lie within 2^-(4P + 512) of a midpoint, which a
	// quotient that is not a binary fraction does not; an exact one is exact in both. Divisors of
	// 3, 7, near 2^32 and 2^64, both signs, exact quotients, in every mode.
	int VerifyIntegerDivisors(bool reportTestCases, unsigned nrSamples) {
		using namespace sw::universal;
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(18110);
		const RoundingMode modes[] = { RoundingMode::RoundToNearest, RoundingMode::RoundToZero,
		                               RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative };
		const RoundingMode saved = efloat_rounding_mode;
		const uint64_t fixed[] = { 3u, 7u, 1000003u, 0xFFFF'FFFFull, 0x1'0000'0001ull, 0xFFFF'FFFF'FFFF'FFFFull, 0x8000'0000'0000'0001ull };
		for (unsigned i = 0; i < nrSamples; ++i) {
			const unsigned p   = 1 + static_cast<unsigned>(rng() % 20);   // in limbs
			const unsigned P   = 32 * p;
			const size_t   nx  = (i % 3 == 0) ? 1 + rng() % p : p;
			uint64_t n = (i < sizeof(fixed) / sizeof(fixed[0])) ? fixed[i] : (rng() >> (rng() % 62));
			if ((n & (n - 1u)) == 0u) n += 3u;   // not a power of 2, which takes the general path
			Real x = make(random_limbs(rng, nx), static_cast<int64_t>(rng() % 41) - 20, (rng() & 1u) != 0u, P);
			if (i % 5 == 4) {   // an exact quotient: x = n m
				Real m = make(random_limbs(rng, 1 + rng() % 2), static_cast<int64_t>(rng() % 21) - 10, false, P);
				x = m * Real(n);
			}
			const bool negate = (i % 2 == 1) && n < (uint64_t(1) << 63);
			for (RoundingMode mode : modes) {
				efloat_rounding_mode = mode;
				Real wide(x);
				wide.set_precision(4 * P + 512);
				Real want = negate ? wide / Real(-static_cast<int64_t>(n)) : wide / Real(n);
				want.set_precision(x.get_precision());
				Real t(x);
				if (negate) t /= -static_cast<int64_t>(n); else t /= n;
				const Real q = negate ? x / -static_cast<int64_t>(n) : x / n;
				if (!identical(t, want) || !identical(q, want)) {
					++nrOfFailedTestCases;
					if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: x / " << (negate ? "-" : "") << n << " at " << P << " bits, mode " << int(mode) << '\n';
				}
			}
		}
		efloat_rounding_mode = saved;
		return nrOfFailedTestCases;
	}

	// limb_kernels::mul_n by Toom-3 (#1814) against the schoolbook product: lengths either side of
	// the thresholds (240 for products, 352 for squares), every n mod 3, products whose parts
	// recurse into Toom-3 again (from 720 limbs), and operands that stress the evaluations'
	// signs and carries: all ones, a zero middle or low third, a top third of all ones against
	// zeros, sparse operands, and squares, into a destination of garbage; and the exact division
	// by 3 on its own, on values whose zero limbs meet a carry, where it borrows
	// limb_kernels's kernels, which take the x86-64 assembly where the processor has BMI2 and ADX
	// (#1816), against the portable loops; elsewhere both sides are the portable loop
	int VerifyLimbKernels(bool reportTestCases, unsigned nrSamples) {
		using namespace sw::universal;
		using limb_kernels::limb_t;
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(18160);
		const char* names[] = { "mul_1", "addmul_1", "submul_1", "add_n", "sub_n", "twice_sub_n" };
		auto check = [&](std::size_t n, int pattern, int op, bool in_place) {
			auto value = [&]() -> limb_t {
				if (pattern == 1) return ~limb_t(0);
				if (pattern == 2) return (rng() & 1u) ? ~limb_t(0) : limb_t(0);
				return rng();
			};
			std::vector<limb_t> a(n), b(n), r1(n), r2(n);
			for (auto& v : a) v = value();
			for (auto& v : b) v = value();
			for (auto& v : r1) v = value();
			const limb_t m = (pattern != 0 || rng() % 4 == 0) ? ~limb_t(0) : rng();
			if (in_place) r1 = a;   // the destination is the first source
			r2 = r1;
			const limb_t* a1 = in_place ? r1.data() : a.data();
			const limb_t* a2 = in_place ? r2.data() : a.data();
			limb_t c1 = 0, c2 = 0;
			switch (op) {
			case 0: c1 = limb_kernels::mul_1_portable(r1.data(), a1, n, m);         c2 = limb_kernels::mul_1(r2.data(), a2, n, m);         break;
			case 1: c1 = limb_kernels::addmul_1_portable(r1.data(), a1, n, m);      c2 = limb_kernels::addmul_1(r2.data(), a2, n, m);      break;
			case 2: c1 = limb_kernels::submul_1_portable(r1.data(), a1, n, m);      c2 = limb_kernels::submul_1(r2.data(), a2, n, m);      break;
			case 3: c1 = limb_kernels::add_n_portable(r1.data(), a1, b.data(), n);  c2 = limb_kernels::add_n(r2.data(), a2, b.data(), n);  break;
			case 4: c1 = limb_kernels::sub_n_portable(r1.data(), a1, b.data(), n); c2 = limb_kernels::sub_n(r2.data(), a2, b.data(), n); break;
			default:   // 2 a - b mod B^n (#1828), against a + a and less b on the portable loops
				limb_kernels::add_n_portable(r1.data(), a1, a1, n);
				limb_kernels::sub_n_portable(r1.data(), r1.data(), b.data(), n);
				limb_kernels::twice_sub_n(r2.data(), a2, b.data(), n);
				break;
			}
			if (c1 != c2 || r1 != r2) {
				++nrOfFailedTestCases;
				if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: " << names[op] << " of " << n << " limbs, pattern " << pattern << (in_place ? ", in place" : "") << '\n';
			}
		};
		for (std::size_t n = 1; n <= 300; ++n) {
			for (int op = 0; op < 6; ++op) {
				for (int pattern = 0; pattern < 3; ++pattern) check(n, pattern, op, false);
				check(n, 0, op, true);
			}
		}
		for (unsigned i = 0; i < nrSamples; ++i) check(1 + rng() % 300, static_cast<int>(rng() % 3), static_cast<int>(rng() % 6), rng() % 4 == 0);
		return nrOfFailedTestCases;
	}

	int VerifyToom3(bool reportTestCases, unsigned nrSamples) {
		using namespace sw::universal;
		using limb_kernels::limb_t;
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(18140);
		auto check = [&](std::size_t n, int pattern, bool square) {
			std::vector<limb_t> a(n), b(n), r1(2 * n), r2(2 * n), sc(limb_kernels::scratch_limbs(n) + 8);
			const std::size_t k = (n + 2) / 3;
			for (std::size_t i = 0; i < n; ++i) { a[i] = rng(); b[i] = rng(); }
			if (pattern == 1) for (std::size_t i = 0; i < n; ++i) a[i] = b[i] = ~limb_t(0);
			if (pattern == 2) for (std::size_t i = k; i < std::min(n, 2 * k); ++i) a[i] = 0u;
			if (pattern == 3) for (std::size_t i = 0; i < std::min(n, k); ++i) b[i] = 0u;
			if (pattern == 4) for (std::size_t i = 2 * k; i < n; ++i) { a[i] = ~limb_t(0); b[i] = 0u; }
			if (pattern == 5) {   // sparse: zero limbs throughout the coefficients, where the exact division by 3 borrows
				for (std::size_t i = 0; i < n; ++i) { a[i] = (rng() % 16 == 0) ? (rng() & 3u) : 0u; b[i] = (rng() % 16 == 0) ? (rng() & 3u) : 0u; }
				a[n - 1] = 1u; b[n - 1] = 1u;
			}
			for (auto& v : r2) v = rng();   // the product must not rely on a zeroed destination
			const limb_t* bp = square ? a.data() : b.data();
			limb_kernels::mul_basecase(r1.data(), a.data(), n, bp, n);
			limb_kernels::mul_n(r2.data(), a.data(), bp, n, sc.data(), square);
			if (r1 != r2) {
				++nrOfFailedTestCases;
				if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: Toom-3 " << (square ? "square" : "product") << " of " << n << " limbs, pattern " << pattern << '\n';
			}
		};
		// 1056 and 1060: squares whose thirds, of 352 limbs or more, square by Toom-3 again
		const std::size_t lengths[] = { 239, 240, 241, 242, 243, 351, 352, 353, 354, 480, 481, 482, 720, 721, 722, 1025, 1056, 1060 };
		for (std::size_t n : lengths) {
			for (int pattern = 0; pattern < 6; ++pattern) {
				check(n, pattern, false);
				check(n, pattern, true);
			}
		}
		for (unsigned i = 0; i < nrSamples; ++i) check(240 + rng() % 900, static_cast<int>(rng() % 6), (i % 3) == 0);
		// the interpolation's exact division by 3, on its own: N = 3 Q mod B^w with Q built limb
		// by limb so that N has zero limbs where a carry arrives, the case in which it borrows,
		// and Q's negations, read as two's complement
		for (unsigned i = 0; i < 200 + nrSamples; ++i) {
			const std::size_t w = 2 + rng() % 40;
			std::vector<limb_t> q(w), nq(w);
			limb_t carry = 0;   // the carry of 3 Q into limb j
			for (std::size_t j = 0; j < w; ++j) {
				q[j] = rng();
				if (carry != 0u && rng() % 3 == 0) q[j] = (0u - carry) * 0xAAAA'AAAA'AAAA'AAABull;   // 3 q_j + carry = 0 mod 2^64
				limb_t lo = 0;
				limb_t hi = limb_kernels::mul_1(&lo, &q[j], 1, 3u);   // 3 q_j, without a 128-bit type (MSVC has none)
				const limb_t sum = lo + carry;
				hi += (sum < lo) ? 1u : 0u;
				nq[j] = sum;
				carry = hi;
			}
			if (i % 2 == 1) { limb_kernels::toom::negate(q.data(), w); limb_kernels::toom::negate(nq.data(), w); }
			limb_kernels::toom::third(nq.data(), w);
			if (nq != q) {
				++nrOfFailedTestCases;
				if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: Toom-3's exact division by 3 of " << w << " limbs\n";
			}
		}
		return nrOfFailedTestCases;
	}

	// limb_kernels::div_appr and sqrt_approx (#1810) against div_qr and sqrtrem: the approximate
	// quotient and root never below the exact ones and above them by at most divappr_error and
	// sqrt_approx_error units of the last limb. Quotients as long as the divisor, one to three
	// limbs shorter (sqrt's shape) and longer than it (the division's, #1811), across the
	// divide-and-conquer threshold and its doublings, all-ones operands and all-ones quotients,
	// and roots of 1 to 200 limbs.
	int VerifyApproximateKernels(bool reportTestCases, unsigned nrSamples) {
		using namespace sw::universal;
		using limb_kernels::limb_t;
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(18100);
		auto excess = [](const std::vector<limb_t>& approx, const std::vector<limb_t>& exact, std::size_t len, std::size_t bound) {
			std::vector<limb_t> d(len);
			const limb_t borrow = limb_kernels::sub_n(d.data(), approx.data(), exact.data(), len);
			bool ok = (borrow == 0u) && d[0] <= bound;
			for (std::size_t i = 1; i < len; ++i) ok = ok && d[i] == 0u;
			return ok;
		};
		const std::size_t lengths[] = { 1, 2, 15, 16, 17, 31, 32, 33, 63, 64, 65, 128, 129, 130 };
		auto check_div = [&](std::size_t dn, std::size_t qn, bool ones) {
			const std::size_t nn = dn + qn;
			std::vector<limb_t> np(nn), np2, dp(dn), q(qn + 1), q2(qn + 1), sc(limb_kernels::div_scratch_limbs(dn) + 8);
			for (auto& v : np) v = ones ? ~limb_t(0) : rng();
			for (auto& v : dp) v = ones ? ~limb_t(0) : rng();
			dp[dn - 1] |= limb_t(1) << 63;
			if (np[nn - 1] >= dp[dn - 1]) np[nn - 1] = dp[dn - 1] - 1u;
			np2 = np;
			q[qn]  = limb_kernels::div_qr(q.data(), np.data(), nn, dp.data(), dn, sc.data());
			q2[qn] = limb_kernels::div_appr(q2.data(), np2.data(), nn, dp.data(), dn, sc.data());
			if (!excess(q2, q, qn + 1, limb_kernels::divappr_error(nn, dn))) {
				++nrOfFailedTestCases;
				if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: div_appr of " << nn << " by " << dn << " limbs is outside its bound\n";
			}
		};
		auto check_root = [&](std::size_t n, bool ones) {
			std::vector<limb_t> a(2 * n), b, sp(n), sp2(n), sc(limb_kernels::sqrt_scratch_limbs(n) + 8);
			for (auto& v : a) v = ones ? ~limb_t(0) : rng();
			a[2 * n - 1] |= limb_t(1) << 62;
			b = a;
			limb_kernels::sqrtrem(sp.data(), a.data(), n, sc.data());
			limb_kernels::sqrt_approx(sp2.data(), b.data(), n, sc.data());
			if (!excess(sp2, sp, n, limb_kernels::sqrt_approx_error(n))) {
				++nrOfFailedTestCases;
				if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: sqrt_approx of " << n << " limbs is outside its bound\n";
			}
		};
		// N = D (B^qn - 1) + D - 1: an all-ones quotient, whose low halves the truncated divisor
		// pushes to B^lo, where div_appr clamps them
		auto check_ones_quotient = [&](std::size_t dn, std::size_t qn) {
			const std::size_t nn = dn + qn;
			std::vector<limb_t> dp(dn), ones(qn, ~limb_t(0)), np(nn), np2, q(qn + 1), q2(qn + 1), sc(limb_kernels::div_scratch_limbs(dn) + 8);
			std::vector<limb_t> sm(limb_kernels::scratch_limbs(std::max(dn, qn)) + 8);
			for (auto& v : dp) v = rng();
			dp[dn - 1] |= limb_t(1) << 63;
			limb_kernels::mul(np.data(), dp.data(), dn, ones.data(), qn, sm.data());
			std::vector<limb_t> dm1(dp);
			limb_kernels::sub_1(dm1.data(), dn, 1u);
			const limb_t carry = limb_kernels::add_n(np.data(), np.data(), dm1.data(), dn);
			limb_kernels::add_1(np.data() + dn, nn - dn, carry);   // N < D B^qn: nothing carries past nn
			np2 = np;
			q[qn]  = limb_kernels::div_qr(q.data(), np.data(), nn, dp.data(), dn, sc.data());
			q2[qn] = limb_kernels::div_appr(q2.data(), np2.data(), nn, dp.data(), dn, sc.data());
			if (!excess(q2, q, qn + 1, limb_kernels::divappr_error(nn, dn))) {
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
			// quotients longer than the divisor, as the division forms them (#1811): exact blocks
			// above an approximate lowest one
			for (std::size_t extra : { std::size_t(1), std::size_t(2), n + 3 }) {
				check_div(n, n + extra, false);
				check_ones_quotient(n, n + extra);
			}
		}
		for (unsigned i = 0; i < nrSamples; ++i) {
			const std::size_t dn = 1 + rng() % 150;
			const std::size_t qn = (i % 4 == 0 || dn < 4) ? dn : (i % 4 == 1) ? dn + 1 + rng() % (2 * dn) : dn - 1 - rng() % 3;
			check_div(dn, qn, (i % 17) == 3);
			check_root(1 + rng() % 200, (i % 19) == 5);
		}
		return nrOfFailedTestCases;
	}

	// limb_kernels::mulhigh_n and mul_truncated (#1807) against the full product: R = r[n - 1, 2n)
	// below the full product's limbs from n - 1 up by less than n units of r[n], and the
	// truncated product the floor of a b / B^drop or one unit below it. Lengths straddle the
	// basecase-to-Mulders threshold (64) and the Karatsuba ones (24, 64); squares, all-ones
	// operands (whose dropped partial products are largest) and unbalanced truncated products.
	// can_round on limbs is checked at the edges of the bits it reads.
	int VerifyShortProducts(bool reportTestCases, unsigned nrSamples) {
		using namespace sw::universal;
		using limb_kernels::limb_t;
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(1807);
		const size_t lengths[] = { 1, 2, 3, 15, 16, 17, 23, 24, 25, 31, 32, 33, 47, 48, 49, 63, 64, 65, 96, 127, 128, 129, 200 };
		auto check_high = [&](size_t n, bool square, bool ones) {
			std::vector<limb_t> a(n), b(n), full(2 * n), r(2 * n), sc(limb_kernels::scratch_limbs(n) + 8), d(n + 1);
			for (size_t i = 0; i < n; ++i) { a[i] = ones ? ~limb_t(0) : rng(); b[i] = ones ? ~limb_t(0) : rng(); }
			const limb_t* bp = square ? a.data() : b.data();
			limb_kernels::mul_n(full.data(), a.data(), bp, n, sc.data(), square);
			limb_kernels::mulhigh_n(r.data(), a.data(), bp, n, sc.data(), square);
			const limb_t borrow = limb_kernels::sub_n(d.data(), full.data() + n - 1, r.data() + n - 1, n + 1);
			bool ok = (borrow == 0u) && (n == 1 ? d[1] == 0u : d[1] < n);
			for (size_t i = 2; i <= n; ++i) ok = ok && d[i] == 0u;
			if (!ok) {
				++nrOfFailedTestCases;
				if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: mulhigh_n of " << n << " limbs is off by n units or more\n";
			}
		};
		auto check_truncated = [&](size_t na, size_t nb, size_t drop, bool ones) {
			std::vector<limb_t> a(na), b(nb), full(na + nb), out(na + nb - drop), sc(limb_kernels::truncated_scratch_limbs(na, nb, drop) + 8);
			std::vector<limb_t> sf(limb_kernels::scratch_limbs(std::max(na, nb)) + 8), d(na + nb - drop);
			for (auto& v : a) v = ones ? ~limb_t(0) : rng();
			for (auto& v : b) v = ones ? ~limb_t(0) : rng();
			limb_kernels::mul(full.data(), a.data(), na, b.data(), nb, sf.data());
			limb_kernels::mul_truncated(out.data(), a.data(), na, b.data(), nb, drop, sc.data());
			const limb_t borrow = limb_kernels::sub_n(d.data(), full.data() + drop, out.data(), na + nb - drop);
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
			const bool ok = !limb_kernels::can_round(frame(-1).data(), 3, precision, correct)
			             && limb_kernels::can_round(frame(static_cast<int>(correct) - 2).data(), 3, precision, correct)
			             && !limb_kernels::can_round(frame(static_cast<int>(correct) - 1).data(), 3, precision, correct)
			             && !limb_kernels::can_round(frame(static_cast<int>(precision)).data(), 3, precision, correct)
			             && limb_kernels::can_round(frame(static_cast<int>(precision) + 1).data(), 3, precision, correct)
			             && !limb_kernels::can_round(frame(static_cast<int>(precision) + 1).data(), 3, precision, precision + 3);
			if (!ok) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "    FAIL: limb_kernels::can_round reads the wrong bits\n";
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

	// limb_kernels::load_window (#1802) against a bit-at-a-time reference: r's bit j is a's bit
	// j - shift, and the flag says whether any of a's bits below -shift is set
	int VerifyLoadWindow(bool reportTestCases, unsigned nrSamples) {
		using namespace sw::universal;
		using limb_kernels::limb_t;
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(1802);
		for (unsigned i = 0; i < nrSamples; ++i) {
			const std::size_t na = 1 + rng() % 9;
			const std::size_t n  = 1 + rng() % 12;
			std::vector<limb_t> a(na);
			for (limb_t& v : a) v = (i % 4 == 0) ? (limb_t(1) << (rng() % 64)) : rng();   // sparse ones find the flag's edges
			const int64_t shift = static_cast<int64_t>(rng() % (64 * 24)) - 64 * 12 + ((i % 8 == 0) ? 0 : static_cast<int64_t>(rng() % 3) - 1);
			std::vector<limb_t> r(n, ~limb_t(0));
			const bool lost = limb_kernels::load_window(r.data(), n, a.data(), na, shift);
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

	// N = Q D + R with R < D for limb_kernels::div_qr: the product by the schoolbook multiply,
	// which shares nothing with the division
	int VerifyQuotient(bool reportTestCases, std::mt19937_64& rng, size_t dn, size_t qn) {
		using namespace sw::universal::limb_kernels;
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
		using sw::universal::limb_kernels::dc_div_threshold;
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

	// S^2 + R = N with R <= 2 S for limb_kernels::sqrtrem: the square by the schoolbook multiply
	int VerifySquareRoot(bool reportTestCases, std::mt19937_64& rng, size_t n) {
		using namespace sw::universal::limb_kernels;
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

	// limb_kernels::sqrtrem4, the root of four limbs in registers (#1828), against dc_sqrtrem, and
	// sqrt_approx4 within 2^15 units of it: random radicands, all-ones runs, perfect squares and
	// their neighbours on either side (one below a square takes the correction), the top limbs at
	// or next to a perfect square, and the smallest radicand, 2^254
	int VerifyFourLimbSquareRoots(bool reportTestCases, unsigned nrSamples) {
		using namespace sw::universal::limb_kernels;
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(18280);
		for (unsigned it = 0; it < nrSamples; ++it) {
			const unsigned style = it % 6;
			limb_t n[4];
			for (auto& x : n) x = (style == 1) ? ~limb_t(0) : (style == 2) ? ((rng() & 1u) ? ~limb_t(0) : 0u) : rng();
			if (style == 3) {   // S^2 - 1, S^2 or S^2 + 1
				limb_t s[2] = { rng(), rng() | (limb_t(1) << 63) }, work[16];
				mul_n(n, s, s, 2, work, true);
				const unsigned k = static_cast<unsigned>(rng() % 3);
				if (k == 0) sub_1(n, 4, 1u);
				if (k == 2) add_1(n, 4, 1u);
			}
			if (style == 4) {   // the top two limbs a perfect square, or one off it
				limb_t s1 = rng() | (limb_t(1) << 63), hi = 0, lo = 0;
				umul(s1, s1, hi, lo);
				n[3] = hi;
				n[2] = lo + (rng() % 3) - 1u;
			}
			if (style == 5) { n[3] = limb_t(1) << 62; n[2] = 0u; n[1] = 0u; n[0] = (it % 12 == 5) ? 0u : rng(); }
			if (n[3] < (limb_t(1) << 62)) n[3] |= limb_t(1) << 62;
			limb_t s4[2] = {}, r4[2] = {};
			const limb_t c4 = sqrtrem4(s4, r4, n);
			limb_t np[4] = { n[0], n[1], n[2], n[3] }, sq[2] = {}, work[sqrt_scratch_limbs(2)];
			const limb_t c = dc_sqrtrem(sq, np, 2, work);
			if (s4[0] != sq[0] || s4[1] != sq[1] || r4[0] != np[0] || r4[1] != np[1] || c4 != c) {
				++nrOfFailedTestCases;
				if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: sqrtrem4 differs from dc_sqrtrem (style " << style << ")\n";
			}
			// sqrt_approx4 within 2^15 units of the root, which the 96-bit root's window of 2^16
			// relies on: |S' - S| as a two-limb difference
			limb_t sa[2] = {};
			if (sqrt_approx4(sa, n)) {
				const bool   above = (sa[1] > sq[1]) || (sa[1] == sq[1] && sa[0] >= sq[0]);
				const limb_t lo    = above ? sa[0] - sq[0] : sq[0] - sa[0];
				const limb_t hi    = above ? sa[1] - sq[1] - ((sa[0] < sq[0]) ? 1u : 0u) : sq[1] - sa[1] - ((sq[0] < sa[0]) ? 1u : 0u);
				if (hi != 0u || lo >= (limb_t(1) << 15)) {
					++nrOfFailedTestCases;
					if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: sqrt_approx4 is more than 2^15 units off the root (style " << style << ")\n";
				}
			}
		}
		return nrOfFailedTestCases;
	}

	int VerifySquareRootKernel(bool reportTestCases, unsigned nrSamples) {
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(17632);
		// one and two limbs (the base cases), the recursion's odd splits, and the division's
		// divide-and-conquer threshold in the half-length divisions
		for (size_t n : { size_t(1), size_t(2), size_t(3), size_t(4), size_t(5), size_t(7), size_t(31), size_t(32), size_t(33), size_t(63), size_t(64), size_t(65), size_t(129) }) {
			for (int k = 0; k < 5; ++k) nrOfFailedTestCases += VerifySquareRoot(reportTestCases && nrOfFailedTestCases < 4, rng, n);
		}
		for (unsigned i = 0; i < nrSamples; ++i) {
			nrOfFailedTestCases += VerifySquareRoot(reportTestCases && nrOfFailedTestCases < 4, rng, 1 + rng() % 150);
		}
		return nrOfFailedTestCases;
	}

	// S^3 + R = N with R <= 3 S^2 + 3 S for limb_kernels::cbrtrem, the cube by the schoolbook
	// multiply; and without the remainder, S is the root plus 0 to cbrt_overshoot units
	int VerifyCubeRoot(bool reportTestCases, std::mt19937_64& rng, size_t n) {
		using namespace sw::universal::limb_kernels;
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
		const unsigned precisions[] = { 1, 2, 3, 4, 5, 6, 8, 16, 40, 60 };   // in limbs: 60 reaches divide-and-conquer, 64 is Real's cap
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

	// x / y at 128 bits with a remainder of (D - 1) / 2 or (D + 1) / 2 for an odd divisor D, a unit
	// either side of half of it (#1828): the short quotient's guard bit is whether 2 R >= D, its
	// comparison of R with D - R the closest it can be, where D - R borrows across the limbs. With
	// Q chosen, R fixes A B^2 = Q D + R modulo B^2: Q = -R D^-1 mod B^2, and A is the quotient's
	// dividend whenever its top bit is set. Against the rounded exact quotient, in every mode.
	int VerifyShortQuotientHalves(bool reportTestCases, unsigned nrSamples) {
		using namespace sw::universal;
		using limb_kernels::limb_t;
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(18281);
		const RoundingMode modes[] = { RoundingMode::RoundToNearest, RoundingMode::RoundToZero,
		                               RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative };
		const RoundingMode saved = efloat_rounding_mode;
		// the low two limbs of a b, and of its negation
		auto mul2 = [](const limb_t* a, const limb_t* b, limb_t* r) {
			limb_t hi = 0, lo = 0, h1 = 0, l1 = 0, h2 = 0, l2 = 0;
			limb_kernels::umul(a[0], b[0], hi, lo);
			limb_kernels::umul(a[0], b[1], h1, l1);
			limb_kernels::umul(a[1], b[0], h2, l2);
			r[0] = lo;
			r[1] = hi + l1 + l2;
		};
		auto to32 = [](const limb_t* w) {   // two limbs as four 32-bit limbs, most significant first
			return std::vector<uint32_t>{ static_cast<uint32_t>(w[1] >> 32), static_cast<uint32_t>(w[1]), static_cast<uint32_t>(w[0] >> 32), static_cast<uint32_t>(w[0]) };
		};
		unsigned done = 0;
		while (done < nrSamples) {
			limb_t d[2] = { rng() | 1u, rng() | (limb_t(1) << 63) };   // odd and normalized
			limb_t inv[2] = { d[0], d[1] };                             // D^-1 mod B^2 by Newton: 3 bits, then doubling
			for (int k = 0; k < 6; ++k) {
				limb_t dx[2], two[2];
				mul2(d, inv, dx);
				two[0] = limb_t(0) - dx[0];   // 2 - D x
				two[1] = ~dx[1] + ((dx[0] == 0u) ? 1u : 0u);
				two[0] += 2u;
				two[1] += (two[0] < 2u) ? 1u : 0u;
				mul2(inv, two, inv);
			}
			const bool above = (rng() & 1u) != 0u;
			limb_t r[2] = { (d[0] >> 1) | (d[1] << 63), d[1] >> 1 };   // (D - 1) / 2
			if (above) { ++r[0]; if (r[0] == 0u) ++r[1]; }              // (D + 1) / 2
			limb_t q[2];
			mul2(r, inv, q);
			q[0] = limb_t(0) - q[0];   // -R D^-1
			q[1] = ~q[1] + ((q[0] == 0u) ? 1u : 0u);
			limb_t p[4];
			limb_kernels::mul_basecase(p, q, 2, d, 2);
			limb_t carry = limb_kernels::add_n(p, p, r, 2);
			carry = limb_kernels::add_1(p + 2, 2, carry);
			if (p[0] != 0u || p[1] != 0u || carry != 0u) {
				++nrOfFailedTestCases;
				if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: the crafted dividend is not Q D + R\n";
				continue;
			}
			const limb_t a[2] = { p[2], p[3] };
			if ((a[1] >> 63) == 0u) continue;   // A must be normalized
			++done;
			const std::vector<uint32_t> la = to32(a), lb = to32(d);
			const int64_t ex = static_cast<int64_t>(rng() % 41) - 20, ey = static_cast<int64_t>(rng() % 41) - 20;
			const bool    sx = (rng() & 1u) != 0u, sy = (rng() & 1u) != 0u;
			const Real x = make(la, ex, sx, 128), y = make(lb, ey, sy, 128);
			std::vector<uint32_t> qq;
			const bool sticky = reference_quotient(la, lb, 32 * 6, qq);
			const int64_t length = 32 * static_cast<int64_t>(qq.size()) - static_cast<int64_t>(sw::universal::clz(qq[0]));
			const int64_t eq = ex - ey - 32 * 6 + length - 1;
			const unsigned sh = static_cast<unsigned>(sw::universal::clz(qq[0]));
			if (sh != 0u) {
				for (size_t j = 0; j + 1 < qq.size(); ++j) qq[j] = (qq[j] << sh) | (qq[j + 1] >> (32u - sh));
				qq.back() <<= sh;
			}
			if (sticky) qq.push_back(1u);
			for (RoundingMode m : modes) {
				efloat_rounding_mode = m;
				Real reference = make(qq, eq, sx != sy, 32 * static_cast<unsigned>(qq.size()));
				reference.set_precision(128);
				const Real quotient = x / y;
				efloat_rounding_mode = saved;
				if (!identical(quotient, reference)) {
					++nrOfFailedTestCases;
					if (reportTestCases && nrOfFailedTestCases <= 3) std::cout << "    FAIL: a 128-bit quotient " << (above ? "above" : "below") << " half a unit is not the rounded exact quotient\n";
				}
			}
		}
		return nrOfFailedTestCases;
	}

}  // anonymous namespace

namespace {
	// The kernels in constant evaluation (#1819), where they take the portable loops, an integer
	// seed for the roots and new and delete for scratch: their results by the compiler and at run
	// time, where the assembly and the double estimates run, the same limbs. Lengths below and
	// past the Karatsuba and divide-and-conquer thresholds, a cube root of one limb, which seeds
	// by a binary search there, and the carry and borrow chains of the short sums.
	constexpr std::uint64_t ce_next(std::uint64_t& x) { x ^= x << 13; x ^= x >> 7; x ^= x << 17; return x; }

	template<std::size_t N>
	constexpr std::array<std::uint64_t, 10 * N + 24> constant_evaluated_kernels(std::uint64_t seed) {
		using namespace sw::universal::limb_kernels;
		std::array<std::uint64_t, 10 * N + 24> out{};
		std::size_t o = 0;
		limb_t a[N]{}, b[N]{}, p[2 * N]{};
		for (auto& v : a) v = ce_next(seed);
		for (auto& v : b) v = ce_next(seed);
		b[N - 1] |= limb_t(1) << 63;
		limb_t* s = new limb_t[div_scratch_limbs(N) + cbrt_scratch_limbs(N) + sqrt_scratch_limbs(N)]{};
		mul_n(p, a, b, N, s, false);
		for (std::size_t i = 0; i < 2 * N; ++i) out[o++] = p[i];
		limb_t sq[2 * N]{};
		mul_n(sq, a, a, N, s, true);
		out[o++] = sq[0] ^ sq[N] ^ sq[2 * N - 1];
		limb_t q[N + 1]{}, np[2 * N]{};
		for (std::size_t i = 0; i < 2 * N; ++i) np[i] = p[i];
		q[N] = div_qr(q, np, 2 * N, b, N, s);
		for (std::size_t i = 0; i <= N; ++i) out[o++] = q[i];
		for (std::size_t i = 0; i < N; ++i) out[o++] = np[i];   // the remainder
		limb_t root[N]{}, rad[2 * N]{};
		for (std::size_t i = 0; i < 2 * N; ++i) rad[i] = p[i];
		rad[2 * N - 1] |= limb_t(1) << 62;
		out[o++] = sqrtrem(root, rad, N, s);
		for (std::size_t i = 0; i < N; ++i) out[o++] = root[i] ^ rad[i];
		limb_t cube[3 * N]{}, croot[N]{}, crem[2 * N + 1]{};
		for (std::size_t i = 0; i < 3 * N; ++i) cube[i] = ce_next(seed);
		cube[3 * N - 1] |= limb_t(1) << 61;
		cbrtrem(croot, crem, cube, N, s, true);
		for (std::size_t i = 0; i < N; ++i) out[o++] = croot[i];
		for (std::size_t i = 0; i < 2 * N + 1; ++i) out[o++] = crem[i];
		limb_t c = 0, d = 1;
		for (std::size_t i = 0; i < N; ++i) out[o++] = add_carry(a[i], b[i], c) ^ sub_borrow(a[i], b[i], d);
		out[o++] = c + 2 * d;
		// the carry in that makes an all-ones limb wrap, and the borrow in from equal limbs
		limb_t c1 = 1, d1 = 1;
		out[o++] = add_carry(~limb_t(0), a[0], c1) ^ sub_borrow(a[0], a[0], d1);
		out[o++] = c1 + 2 * d1;
		// a perfect cube, whose root the one-limb base case must not step past
		limb_t pc[3]{}, pr[1]{}, prem[3]{};
		cube_1(pc, a[0] | (limb_t(1) << 63));
		cbrtrem(pr, prem, pc, 1, s, true);
		out[o++] = pr[0]; out[o++] = prem[0] | prem[1] | prem[2];
		limb_t qq = 0, rr = 0;
		udiv_qrnnd(qq, rr, b[N - 1] >> 1, a[0], b[N - 1]);
		out[o++] = qq; out[o++] = rr;
		twice_sub_n(p, a, b, N);
		out[o++] = p[0] ^ p[N - 1];
		out[o++] = submul_1(a, b, N, ce_next(seed)) ^ addmul_1(b, a, N, ce_next(seed));
		delete[] s;
		return out;
	}

	template<std::size_t N, std::uint64_t seed>
	int VerifyConstantEvaluatedKernel(bool reportTestCases) {
		constexpr auto compiled = constant_evaluated_kernels<N>(seed);
		const auto run = constant_evaluated_kernels<N>(seed);
		if (compiled == run) return 0;
		if (reportTestCases) std::cout << "    FAIL: the kernels at " << N << " limbs differ in constant evaluation\n";
		return 1;
	}
	int VerifyConstantEvaluatedKernels(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		nrOfFailedTestCases += VerifyConstantEvaluatedKernel<1, 88172645463325252ull>(reportTestCases);
		nrOfFailedTestCases += VerifyConstantEvaluatedKernel<2, 362436069ull>(reportTestCases);
		nrOfFailedTestCases += VerifyConstantEvaluatedKernel<5, 521288629ull>(reportTestCases);
		nrOfFailedTestCases += VerifyConstantEvaluatedKernel<16, 88675123ull>(reportTestCases);
		nrOfFailedTestCases += VerifyConstantEvaluatedKernel<40, 5783321ull>(reportTestCases);   // Karatsuba, divide and conquer
		return nrOfFailedTestCases;
	}

	// the kernels' names before #1823: the forwarding header aliases the namespace, so code that
	// names efloat_mpn still compiles, against the same functions
	static_assert(std::is_same_v<decltype(&sw::universal::efloat_mpn::add_n), decltype(&sw::universal::limb_kernels::add_n)>);
	static_assert(std::is_same_v<sw::universal::efloat_mpn::limb_t, sw::universal::limb_kernels::limb_t>);
}

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
	nrOfFailedTestCases += ReportTestResult(VerifyLimbKernels(reportTestCases, 1000), "efloat", "limb kernels against the portable loops");
	nrOfFailedTestCases += ReportTestResult(VerifyToom3(reportTestCases, 5), "efloat", "Toom-3 products");
	nrOfFailedTestCases += ReportTestResult(VerifyAlignedSums(reportTestCases, 100), "efloat", "rounded sums");
	nrOfFailedTestCases += ReportTestResult(VerifyFullLengthSums(reportTestCases, 60), "efloat", "full-length sums and differences");
	nrOfFailedTestCases += ReportTestResult(VerifySmallProducts(reportTestCases, 60), "efloat", "small products");
	nrOfFailedTestCases += ReportTestResult(VerifySmallSquareRoots(reportTestCases, 60), "efloat", "small square roots");
	nrOfFailedTestCases += ReportTestResult(VerifyShortDivisionKernels(reportTestCases, 1000), "efloat", "short division kernels");
	nrOfFailedTestCases += ReportTestResult(VerifyTwoByOneSteps(reportTestCases, 10000), "efloat", "2-by-1 division steps");
	nrOfFailedTestCases += ReportTestResult(VerifyLoadWindow(reportTestCases, 2000), "efloat", "limb windows");
	nrOfFailedTestCases += ReportTestResult(VerifyShortProducts(reportTestCases, 100), "efloat", "short products");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedProducts(reportTestCases, 20), "efloat", "products from short products");
	nrOfFailedTestCases += ReportTestResult(VerifyApproximateKernels(reportTestCases, 100), "efloat", "approximate quotients and roots");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerDivisors(reportTestCases, 200), "efloat", "integer divisors");
	nrOfFailedTestCases += ReportTestResult(VerifyQuotientsNextToExact(reportTestCases, 20), "efloat", "quotients next to exact ones");
	nrOfFailedTestCases += ReportTestResult(VerifyQuotientKernel(reportTestCases, 100), "efloat", "limb quotients");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedQuotients(reportTestCases, 100), "efloat", "rounded quotients");
	nrOfFailedTestCases += ReportTestResult(VerifyShortQuotientHalves(reportTestCases, 1000), "efloat", "quotients next to half a unit");
	nrOfFailedTestCases += ReportTestResult(VerifySquareRootKernel(reportTestCases, 100), "efloat", "limb square roots");
	nrOfFailedTestCases += ReportTestResult(VerifyFourLimbSquareRoots(reportTestCases, 10000), "efloat", "four-limb square roots");
	nrOfFailedTestCases += ReportTestResult(VerifyCubeRootKernel(reportTestCases, 100), "efloat", "limb cube roots");
	nrOfFailedTestCases += ReportTestResult(VerifyConstantEvaluatedKernels(reportTestCases), "efloat", "kernels in constant evaluation");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExactProducts(reportTestCases, 300), "efloat", "exact products");
	nrOfFailedTestCases += ReportTestResult(VerifyLimbKernels(reportTestCases, 3000), "efloat", "limb kernels against the portable loops");
	nrOfFailedTestCases += ReportTestResult(VerifyToom3(reportTestCases, 20), "efloat", "Toom-3 products");
	nrOfFailedTestCases += ReportTestResult(VerifyAlignedSums(reportTestCases, 2000), "efloat", "rounded sums");
	nrOfFailedTestCases += ReportTestResult(VerifyFullLengthSums(reportTestCases, 300), "efloat", "full-length sums and differences");
	nrOfFailedTestCases += ReportTestResult(VerifySmallProducts(reportTestCases, 300), "efloat", "small products");
	nrOfFailedTestCases += ReportTestResult(VerifySmallSquareRoots(reportTestCases, 300), "efloat", "small square roots");
	nrOfFailedTestCases += ReportTestResult(VerifyShortDivisionKernels(reportTestCases, 20000), "efloat", "short division kernels");
	nrOfFailedTestCases += ReportTestResult(VerifyTwoByOneSteps(reportTestCases, 20000), "efloat", "2-by-1 division steps");
	nrOfFailedTestCases += ReportTestResult(VerifyLoadWindow(reportTestCases, 2000), "efloat", "limb windows");
	nrOfFailedTestCases += ReportTestResult(VerifyShortProducts(reportTestCases, 300), "efloat", "short products");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedProducts(reportTestCases, 60), "efloat", "products from short products");
	nrOfFailedTestCases += ReportTestResult(VerifyApproximateKernels(reportTestCases, 3000), "efloat", "approximate quotients and roots");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerDivisors(reportTestCases, 1000), "efloat", "integer divisors");
	nrOfFailedTestCases += ReportTestResult(VerifyQuotientsNextToExact(reportTestCases, 60), "efloat", "quotients next to exact ones");
	nrOfFailedTestCases += ReportTestResult(VerifyQuotientKernel(reportTestCases, 300), "efloat", "limb quotients");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedQuotients(reportTestCases, 300), "efloat", "rounded quotients");
	nrOfFailedTestCases += ReportTestResult(VerifyShortQuotientHalves(reportTestCases, 3000), "efloat", "quotients next to half a unit");
	nrOfFailedTestCases += ReportTestResult(VerifySquareRootKernel(reportTestCases, 300), "efloat", "limb square roots");
	nrOfFailedTestCases += ReportTestResult(VerifyFourLimbSquareRoots(reportTestCases, 30000), "efloat", "four-limb square roots");
	nrOfFailedTestCases += ReportTestResult(VerifyCubeRootKernel(reportTestCases, 300), "efloat", "limb cube roots");
	nrOfFailedTestCases += ReportTestResult(VerifyConstantEvaluatedKernels(reportTestCases), "efloat", "kernels in constant evaluation");
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyExactProducts(reportTestCases, 3000), "efloat", "exact products");
	nrOfFailedTestCases += ReportTestResult(VerifyLimbKernels(reportTestCases, 30000), "efloat", "limb kernels against the portable loops");
	nrOfFailedTestCases += ReportTestResult(VerifyToom3(reportTestCases, 200), "efloat", "Toom-3 products");
	nrOfFailedTestCases += ReportTestResult(VerifyAlignedSums(reportTestCases, 20000), "efloat", "rounded sums");
	nrOfFailedTestCases += ReportTestResult(VerifyFullLengthSums(reportTestCases, 3000), "efloat", "full-length sums and differences");
	nrOfFailedTestCases += ReportTestResult(VerifySmallProducts(reportTestCases, 3000), "efloat", "small products");
	nrOfFailedTestCases += ReportTestResult(VerifySmallSquareRoots(reportTestCases, 3000), "efloat", "small square roots");
	nrOfFailedTestCases += ReportTestResult(VerifyShortDivisionKernels(reportTestCases, 200000), "efloat", "short division kernels");
	nrOfFailedTestCases += ReportTestResult(VerifyTwoByOneSteps(reportTestCases, 200000), "efloat", "2-by-1 division steps");
	nrOfFailedTestCases += ReportTestResult(VerifyLoadWindow(reportTestCases, 20000), "efloat", "limb windows");
	nrOfFailedTestCases += ReportTestResult(VerifyShortProducts(reportTestCases, 3000), "efloat", "short products");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedProducts(reportTestCases, 300), "efloat", "products from short products");
	nrOfFailedTestCases += ReportTestResult(VerifyApproximateKernels(reportTestCases, 20000), "efloat", "approximate quotients and roots");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerDivisors(reportTestCases, 10000), "efloat", "integer divisors");
	nrOfFailedTestCases += ReportTestResult(VerifyQuotientsNextToExact(reportTestCases, 600), "efloat", "quotients next to exact ones");
	nrOfFailedTestCases += ReportTestResult(VerifyQuotientKernel(reportTestCases, 3000), "efloat", "limb quotients");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedQuotients(reportTestCases, 3000), "efloat", "rounded quotients");
	nrOfFailedTestCases += ReportTestResult(VerifyShortQuotientHalves(reportTestCases, 30000), "efloat", "quotients next to half a unit");
	nrOfFailedTestCases += ReportTestResult(VerifySquareRootKernel(reportTestCases, 3000), "efloat", "limb square roots");
	nrOfFailedTestCases += ReportTestResult(VerifyFourLimbSquareRoots(reportTestCases, 300000), "efloat", "four-limb square roots");
	nrOfFailedTestCases += ReportTestResult(VerifyCubeRootKernel(reportTestCases, 3000), "efloat", "limb cube roots");
#	endif

#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyExactProducts(reportTestCases, 30000), "efloat", "exact products");
	nrOfFailedTestCases += ReportTestResult(VerifyLimbKernels(reportTestCases, 300000), "efloat", "limb kernels against the portable loops");
	nrOfFailedTestCases += ReportTestResult(VerifyToom3(reportTestCases, 1000), "efloat", "Toom-3 products");
	nrOfFailedTestCases += ReportTestResult(VerifyAlignedSums(reportTestCases, 200000), "efloat", "rounded sums");
	nrOfFailedTestCases += ReportTestResult(VerifyFullLengthSums(reportTestCases, 30000), "efloat", "full-length sums and differences");
	nrOfFailedTestCases += ReportTestResult(VerifySmallProducts(reportTestCases, 30000), "efloat", "small products");
	nrOfFailedTestCases += ReportTestResult(VerifySmallSquareRoots(reportTestCases, 30000), "efloat", "small square roots");
	nrOfFailedTestCases += ReportTestResult(VerifyShortDivisionKernels(reportTestCases, 2000000), "efloat", "short division kernels");
	nrOfFailedTestCases += ReportTestResult(VerifyTwoByOneSteps(reportTestCases, 2000000), "efloat", "2-by-1 division steps");
	nrOfFailedTestCases += ReportTestResult(VerifyLoadWindow(reportTestCases, 100000), "efloat", "limb windows");
	nrOfFailedTestCases += ReportTestResult(VerifyShortProducts(reportTestCases, 30000), "efloat", "short products");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedProducts(reportTestCases, 3000), "efloat", "products from short products");
	nrOfFailedTestCases += ReportTestResult(VerifyApproximateKernels(reportTestCases, 30000), "efloat", "approximate quotients and roots");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerDivisors(reportTestCases, 50000), "efloat", "integer divisors");
	nrOfFailedTestCases += ReportTestResult(VerifyQuotientsNextToExact(reportTestCases, 3000), "efloat", "quotients next to exact ones");
	nrOfFailedTestCases += ReportTestResult(VerifyQuotientKernel(reportTestCases, 30000), "efloat", "limb quotients");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedQuotients(reportTestCases, 30000), "efloat", "rounded quotients");
	nrOfFailedTestCases += ReportTestResult(VerifyShortQuotientHalves(reportTestCases, 300000), "efloat", "quotients next to half a unit");
	nrOfFailedTestCases += ReportTestResult(VerifySquareRootKernel(reportTestCases, 30000), "efloat", "limb square roots");
	nrOfFailedTestCases += ReportTestResult(VerifyFourLimbSquareRoots(reportTestCases, 3000000), "efloat", "four-limb square roots");
	nrOfFailedTestCases += ReportTestResult(VerifyCubeRootKernel(reportTestCases, 30000), "efloat", "limb cube roots");
#	endif

#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyExactProducts(reportTestCases, 300000), "efloat", "exact products");
	nrOfFailedTestCases += ReportTestResult(VerifyLimbKernels(reportTestCases, 1000000), "efloat", "limb kernels against the portable loops");
	nrOfFailedTestCases += ReportTestResult(VerifyToom3(reportTestCases, 4000), "efloat", "Toom-3 products");
	nrOfFailedTestCases += ReportTestResult(VerifyAlignedSums(reportTestCases, 2000000), "efloat", "rounded sums");
	nrOfFailedTestCases += ReportTestResult(VerifyFullLengthSums(reportTestCases, 100000), "efloat", "full-length sums and differences");
	nrOfFailedTestCases += ReportTestResult(VerifySmallProducts(reportTestCases, 100000), "efloat", "small products");
	nrOfFailedTestCases += ReportTestResult(VerifySmallSquareRoots(reportTestCases, 100000), "efloat", "small square roots");
	nrOfFailedTestCases += ReportTestResult(VerifyShortDivisionKernels(reportTestCases, 10000000), "efloat", "short division kernels");
	nrOfFailedTestCases += ReportTestResult(VerifyTwoByOneSteps(reportTestCases, 10000000), "efloat", "2-by-1 division steps");
	nrOfFailedTestCases += ReportTestResult(VerifyLoadWindow(reportTestCases, 300000), "efloat", "limb windows");
	nrOfFailedTestCases += ReportTestResult(VerifyShortProducts(reportTestCases, 100000), "efloat", "short products");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedProducts(reportTestCases, 10000), "efloat", "products from short products");
	nrOfFailedTestCases += ReportTestResult(VerifyApproximateKernels(reportTestCases, 100000), "efloat", "approximate quotients and roots");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerDivisors(reportTestCases, 200000), "efloat", "integer divisors");
	nrOfFailedTestCases += ReportTestResult(VerifyQuotientsNextToExact(reportTestCases, 10000), "efloat", "quotients next to exact ones");
	nrOfFailedTestCases += ReportTestResult(VerifyQuotientKernel(reportTestCases, 300000), "efloat", "limb quotients");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedQuotients(reportTestCases, 300000), "efloat", "rounded quotients");
	nrOfFailedTestCases += ReportTestResult(VerifyShortQuotientHalves(reportTestCases, 3000000), "efloat", "quotients next to half a unit");
	nrOfFailedTestCases += ReportTestResult(VerifySquareRootKernel(reportTestCases, 300000), "efloat", "limb square roots");
	nrOfFailedTestCases += ReportTestResult(VerifyFourLimbSquareRoots(reportTestCases, 30000000), "efloat", "four-limb square roots");
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
