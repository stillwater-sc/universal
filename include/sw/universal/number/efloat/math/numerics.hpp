// numerics.hpp: numeric support functions (frexp, ldexp, scalbn, logb, ilogb, fma) for efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>    // FP_ILOGB0, FP_ILOGBNAN
#include <cstdint>
#include <climits>  // INT_MAX
#include <algorithm>
#include <initializer_list>
#include <universal/number/efloat/math/exponent.hpp>   // exact_product, exact_sum, round_result

namespace sw {
namespace universal {

// ldexp(x, exp) = x * 2^exp. Exact: it only shifts efloat's binary exponent, so
// no rounding occurs and the mantissa is untouched, unless the result leaves the exponent
// range, where it overflows or underflows as a product does (#1786). Zero/inf/nan pass through.
template<unsigned nlimbs>
constexpr efloat<nlimbs> ldexp(const efloat<nlimbs>& x, int exp) {
	if (exp == 0 || x.iszero() || x.isinf() || x.isnan())
		return x;
	efloat<nlimbs> result(x);
	result.setexponent(x.scale() + static_cast<std::int64_t>(exp));
	result.normalize();
	return result;
}

// frexp(x, exp) splits x into a normalized fraction m in [0.5, 1) and an integer
// power of two, such that x == m * 2^(*exp). The binade of x is [2^k, 2^(k+1))
// with k = scale(), so m = x * 2^-(k+1) (leading bit at 2^-1) and *exp = k + 1.
// Exact: only the exponent moves. frexp(0) -> 0 with *exp = 0; inf/nan pass
// through with *exp = 0 (matching <cmath>).
template<unsigned nlimbs>
constexpr efloat<nlimbs> frexp(const efloat<nlimbs>& x, int* exp) {
	if (x.iszero() || x.isinf() || x.isnan()) {
		if (exp != nullptr)
			*exp = 0;
		return x;
	}
	if (exp != nullptr)
		*exp = static_cast<int>(x.scale() + 1);
	efloat<nlimbs> mantissa(x);
	mantissa.setexponent(-1);  // leading bit at 2^-1 -> |mantissa| in [0.5, 1)
	return mantissa;
}

// scalbn(x, n) = x * 2^n -- identical to ldexp for a radix-2 type.
template<unsigned nlimbs>
constexpr efloat<nlimbs> scalbn(const efloat<nlimbs>& x, int n) {
	return ldexp(x, n);
}

// logb(x): the unbiased radix-2 exponent of x as a floating-point value,
// floor(log2(|x|)) -- exactly scale() for a normalized efloat. logb(0) = -inf
// (raises DivisionByZero), logb(+/-inf) = +inf, logb(nan) = nan.
template<unsigned nlimbs>
constexpr efloat<nlimbs> logb(const efloat<nlimbs>& x) {
	if (x.isnan())
		return x;
	if (x.isinf()) {
		efloat<nlimbs> inf;
		inf.setinf(false);
		return inf;
	}
	if (x.iszero()) {
		efloat<nlimbs> ninf;
		ninf.setinf(true);
		if (!std::is_constant_evaluated())
			efloat_exception_flags.set(ExceptionFlag::DivisionByZero);
		return ninf;
	}
	return efloat<nlimbs>(static_cast<long long>(x.scale()));
}

// ilogb(x): logb as an int, with the <cmath> special values.
template<unsigned nlimbs>
inline int ilogb(const efloat<nlimbs>& x) {
	if (x.isnan())
		return FP_ILOGBNAN;
	if (x.isinf())
		return INT_MAX;
	if (x.iszero())
		return FP_ILOGB0;
	return static_cast<int>(x.scale());
}

namespace efloat_detail {

	// x y + z for finite nonzero x, y and z with |scale(x) + scale(y)| < 2^61, rounded once at
	// `precision` bits (#1802). The product X Y of the significands is exact on limbs, and so is
	// its sum with Z in a frame wide enough for both, unless the term with the lower leading bit
	// lies wholly more than 66 bits below both the other's last bit and 64 bits past the rounding
	// point. It is then replaced by a proxy of its sign at the frame's last bit: the sum and the
	// proxied sum lie within two frame units of the larger term, a multiple of 2^64 of them whose
	// rounding boundaries are coarser still, on the same side of it, and so round alike. No
	// efloat is formed on the way: before, the widened operands, the product and the sum were
	// each one, and an fma at 128 bits took the time of about four multiplications.
	template<unsigned nlimbs>
	inline void fma_limbs(efloat<nlimbs>& result, const efloat<nlimbs>& x, const efloat<nlimbs>& y, const efloat<nlimbs>& z, unsigned precision) {
		using efloat_mpn::limb_t;
		const std::size_t nx = (x.limb_vector().size() + 1) / 2;
		const std::size_t ny = (y.limb_vector().size() + 1) / 2;
		const std::size_t nz = (z.limb_vector().size() + 1) / 2;
		const std::size_t np = nx + ny;
		const int64_t B  = 32 * static_cast<int64_t>((precision + 31) / 32);   // the rounding precision
		const int64_t R  = x.scale() + y.scale();
		// weights relative to 2^R: the product's limbs P from 2^(2 - 64 np) to 2^1, z's from
		// 2^(dz - 64 nz + 1) to 2^dz, with |dz| < 2^63 since |R| < 2^61
		const int64_t dz     = z.scale() - R;
		const int64_t p_low  = 2 - 64 * static_cast<int64_t>(np);
		const int64_t z_low  = dz - 64 * static_cast<int64_t>(nz) + 1;
		const bool    z_big  = dz > 1;
		const int64_t big_top   = z_big ? dz : 1;
		const int64_t big_low   = z_big ? z_low : p_low;
		const int64_t small_top = z_big ? 1 : dz;
		const int64_t bottom    = std::min(big_low, big_top - B - 64);
		const bool    proxied   = small_top < bottom - 66;
		const int64_t w0 = proxied ? bottom - 64 : std::min(p_low, z_low);   // the frame's last bit
		const std::size_t w = static_cast<std::size_t>((std::max<int64_t>(big_top, small_top) + 2 - w0 + 63) / 64);

		limb_buffer buffer;
		limb_t* px   = buffer.get(nx + ny + nz + np + w + nz + 1 + efloat_mpn::scratch_limbs(std::max(nx, ny)));
		limb_t* py   = px + nx;
		limb_t* pz   = py + ny;
		limb_t* prod = pz + nz;
		limb_t* f    = prod + np;      // the frame, w limbs
		limb_t* g    = f + w;          // z shifted to its place in the frame, nz + 1 limbs
		limb_t* work = g + nz + 1;
		pack_significand(px, x);
		pack_significand(py, y);
		pack_significand(pz, z);
		const bool p_neg = (x.sign() == -1) != (y.sign() == -1);
		const bool z_neg = (z.sign() == -1);
		bool negative = p_neg;
		if (proxied && z_big) {   // the product is the proxy
			efloat_mpn::load_window(f, w, pz, nz, z_low - w0);
			negative = z_neg;
		}
		else {
			// the product in place: its limbs from limb op of the frame on, shifted up cp bits
			const std::size_t op = static_cast<std::size_t>((p_low - w0) / 64);
			const unsigned    cp = static_cast<unsigned>((p_low - w0) % 64);
			for (std::size_t k = 0; k < op; ++k) f[k] = 0u;
			std::size_t end = op + np;
			if (cp == 0u) {
				efloat_mpn::mul(f + op, px, nx, py, ny, work);
			}
			else {
				efloat_mpn::mul(prod, px, nx, py, ny, work);
				f[end] = efloat_mpn::lshift(f + op, prod, np, cp);
				++end;
			}
			for (std::size_t k = end; k < w; ++k) f[k] = 0u;
		}
		if (proxied) {
			if (p_neg == z_neg) efloat_mpn::add_1(f, w, 1u); else efloat_mpn::sub_1(f, w, 1u);
		}
		else {
			// z's limbs from limb oz of the frame on: added, or subtracted with the borrow out of the
			// top saying z was the larger, when the frame holds the difference's two's complement
			const std::size_t oz = static_cast<std::size_t>((z_low - w0) / 64);
			const std::size_t lz = std::min(nz + 1, w - oz);
			efloat_mpn::load_window(g, lz, pz, nz, (z_low - w0) % 64);
			if (p_neg == z_neg) {
				const limb_t carry = efloat_mpn::add_n(f + oz, f + oz, g, lz);
				efloat_mpn::add_1(f + oz + lz, w - oz - lz, carry);
			}
			else {
				const limb_t borrow = efloat_mpn::sub_n(f + oz, f + oz, g, lz);
				if (efloat_mpn::sub_1(f + oz + lz, w - oz - lz, borrow) != 0u) {
					limb_t c = 1;
					for (std::size_t k = 0; k < w; ++k) {
						const limb_t v = ~f[k] + c;
						c = (v < c) ? 1u : 0u;
						f[k] = v;
					}
					negative = z_neg;
				}
				bool zero = true;
				for (std::size_t k = 0; zero && k < w; ++k) zero = (f[k] == 0u);
				if (zero) {   // x y = -z: +0, or -0 rounding toward -inf (IEEE-754 6.3)
					result = efloat<nlimbs>(0.0);
					result.setsign(efloat_rounding_mode == RoundingMode::RoundTowardNegative);
					return;
				}
			}
		}
		// the frame's top bit weighs 2^(R + w0 + 64 w - 1); with z the larger, that is z's scale
		// plus a bounded offset, so the sum stays in int64_t
		std::size_t used = w;
		while (f[used - 1] == 0u) --used;   // the leading zero limbs, the carry room among them; f != 0
		const int64_t top = (z_big ? z.scale() + (w0 - dz) : R + w0) + 64 * static_cast<int64_t>(used) - 1;
		result.assign_rounded_limbs(negative, f, used, top, false, static_cast<unsigned>(B / 32));
	}

}  // namespace efloat_detail

// fma(x, y, z) = x y + z, correctly rounded in the current mode at the wider of the precisions
// x, y and z carry (#1783): the product is exact, and the sum rounds once. Before, x * y rounded
// to the operands' precision before z was added, and the result rounded twice. The special
// values come from efloat's own multiply and add, as IEEE's do: 0 inf is NaN.
template<unsigned nlimbs>
inline efloat<nlimbs> fma(const efloat<nlimbs>& x, const efloat<nlimbs>& y, const efloat<nlimbs>& z) {
	if (x.isnan() || y.isnan() || z.isnan() || x.isinf() || y.isinf()) return x * y + z;
	if (z.isinf()) return z;   // x y is finite, though x * y may overflow: inf - inf would be NaN
	const bool carries = x.carries_precision() || y.carries_precision() || z.carries_precision();
	unsigned precision = x.get_precision();
	if (carries) {
		precision = 0;
		for (const efloat<nlimbs>* v : { &x, &y, &z }) {
			if (v->carries_precision()) precision = std::max(precision, v->get_precision());
		}
	}
	using E = efloat<nlimbs>;
	const int64_t s = x.iszero() || y.iszero() ? 0 : x.scale() + y.scale();   // the product's scale is s or s + 1
	constexpr int64_t limbs_range = int64_t(1) << 61;
	if (!x.iszero() && !y.iszero() && !z.iszero() && s > -limbs_range && s < limbs_range) {
		// a copy of the longest operand holds the result's limbs without growing in most cases
		const std::size_t lx = x.limb_vector().size(), ly = y.limb_vector().size(), lz = z.limb_vector().size();
		E result((lx >= ly && lx >= lz) ? x : (ly >= lz ? y : z));
		efloat_detail::fma_limbs(result, x, y, z, precision);   // rounded, at the precision
		if (!carries || result.iszero()) efloat_detail::round_result(result, precision, carries);
		return result;
	}
	if (s >= E::min_scale && s < E::max_scale) {
		E result = efloat_detail::exact_sum(efloat_detail::exact_product(x, y), z, precision + 2);
		efloat_detail::round_result(result, precision, carries);
		return result;
	}
	// The exact product leaves the exponent range (#1786), though x y + z may not. Scale both
	// terms by 2^-t, t the larger one's scale, so the larger lies in [1, 4): a term more than 2^62
	// binades below it is negligible at any precision, and is raised to 2^(min_scale + 2), where it
	// still lies far below the larger's last bit and exact_sum proxies it alike. The sum rounds at
	// the precision, and 2^t then scales it back, overflowing or underflowing as a product would.
	const int64_t t = z.iszero() ? s : std::max(s, z.scale());
	auto offset = [t](int64_t e) -> int64_t {   // e - t, or min_scale + 2 if that is lower
		if (e == t) return 0;
		return (e < t + E::min_scale + 2) ? E::min_scale + 2 : e - t;   // here t is z's scale, in range
	};
	E xs(x), ys(y), zs(z);
	xs.setexponent(0);
	ys.setexponent(offset(s));
	if (!zs.iszero()) zs.setexponent(offset(z.scale()));
	E result = efloat_detail::exact_sum(efloat_detail::exact_product(xs, ys), zs, precision + 2);
	efloat_detail::round_result(result, precision, carries);
	if (!result.iszero()) {
		const int64_t e = result.scale();   // e in [min_scale, 2], t in [2 min_scale, 2 max_scale]
		int64_t scaled = 0;
		if (t > E::max_scale - e)          scaled = E::max_scale + 1;   // overflows
		else if (t < E::min_scale - 1 - e) scaled = E::min_scale - 2;   // below min / 2: underflows
		else                               scaled = e + t;
		result.setexponent(scaled);
		result.normalize();
	}
	return result;
}

}  // namespace universal
}  // namespace sw
