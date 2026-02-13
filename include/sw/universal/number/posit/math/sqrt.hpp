#pragma once
// sqrt.hpp: sqrt functions for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/native/ieee754.hpp>
#include <universal/number/posit/math/sqrt_tables.hpp>

#ifndef POSIT_NATIVE_SQRT
#define POSIT_NATIVE_SQRT 0
#endif

namespace sw { namespace universal {

	// straight Babylonian
	template<unsigned nbits, unsigned es, typename bt>
	inline posit<nbits, es, bt> BabylonianMethod(const posit<nbits, es, bt>& v) {
		const double eps = 1.0e-5;
		posit<nbits, es, bt> half(0.5);
		posit<nbits, es, bt> x_next;
		posit<nbits, es, bt> x_n = half * v;
		posit<nbits, es, bt> diff;
		do {
			x_next = (x_n + v / x_n) * half;
			diff = x_next - x_n;
			if (_trace_sqrt) std::cout << " x_n+1: " << x_next << " x_n: " << x_n << " diff " << diff << std::endl;
			x_n = x_next;
		} while (double(sw::universal::abs(diff)) > eps);
		return x_n;
	}

	/*
	- Consider the function argument, x, in floating-point form, with a base
	(or radix) B, exponent e, and a fraction, f , such that 1/B <= f < 1.
	Then we have x = f Be. The number of bits in the exponent and
	fraction, and the value of the base, depends on the particular floating
	point arithmetic system chosen.

	- Use properties of the elementary function to range reduce the argument
	x to a small fixed interval.

	- Use a small polynomial approximation to produce an initial estimate,
	y0, of the function on the small interval. Such an estimate may
	be good to perhaps 5 to 10 bits.

	- Apply Newton iteration to refine the result. This takes the form 
	               yk = yk_1/2 + (f /2)/yk_1. 
	In base 2, the divisions by two can be done by exponent adjustments 
	in floating-point computation, or by bit shifting in fixed-point computation.

	Convergence of the Newton method is quadratic, so the number of
	correct bits doubles with each iteration. Thus, a starting point correct
	to 7 bits will produce iterates accurate to 14, 28, 56, ... bits. Since the
	number of iterations is very small, and known in advance, the loop is
	written as straight-line code.

	- Having computed the function value for the range-reduced argument,
	make whatever adjustments are necessary to produce the function value
	for the original argument; this step may involve a sign adjustment,
	and possibly a single multiplication and/or addition.
	*/

#if POSIT_NATIVE_SQRT
	// The value<>-based fast_sqrt and native sqrt path require internal::value<>.
	// These are only compiled when POSIT_NATIVE_SQRT is explicitly enabled.
#include <universal/internal/value/value.hpp>

	// fast sqrt at a given posit configuration.
	template<unsigned nbits, unsigned es, unsigned fbits>
	inline internal::value<fbits> fast_sqrt(internal::value<fbits>& v) {
		if (_trace_sqrt) std::cout << "---------------------------  SQRT -----------------------" << std::endl;
		//			static_assert(nbits >= 16, "fast_sqrt requires posit configurations nbits >= 16");
		posit<nbits, es> fr = v.fraction_value()*0.5;
		int e = v.scale() + 1;
		posit<nbits, es> y = posit<nbits, es>(0.41731f) + posit<nbits, es>(0.59016f) * fr;
		posit<nbits, es> z = y + fr / y;
		if (_trace_sqrt) {
			std::cout << "f          " << v << std::endl;
			std::cout << "e          " << e << std::endl;
			std::cout << "fr         " << fr << std::endl;
			std::cout << "y0         " << y << std::endl;
			std::cout << "y1         " << z << std::endl;
		}
		y = posit<nbits, es>(0.25f) * z + fr / z;
		if (_trace_sqrt) std::cout << "y2         " << y << std::endl;

		if (e % 2) {
			y *= posit<nbits, es>(0.707106781186547524400844362104);
			if (_trace_sqrt) std::cout << "y*sqrt0.5  " << y << std::endl;
			y = (y < posit<nbits, es>(0.5f) ? posit<nbits, es>(0.5f) : y);
			e += 1;
		}
		else {
			posit<nbits, es> one(1.0f), onemme;
			onemme = --one;
			y = (y < one ? y : onemme);
		}
		if (_trace_sqrt) std::cout << "y adjusted " << y << std::endl;

		internal::value<fbits> vsqrt = y.to_value();
		vsqrt.setscale((e >> 1) - 1);
		if (_trace_sqrt) std::cout << "vsqrt      " << vsqrt << std::endl;
		return vsqrt;
	}

	// sqrt for arbitrary posit
	template<unsigned nbits, unsigned es, typename bt>
	inline posit<nbits, es, bt> sqrt(const posit<nbits, es, bt>& a) {
		posit<nbits, es, bt> p;
		if (a.sign()) {
			p.setnar();
			return p;
		}

		// for small posits use a more precise posit to do the calculation while keeping the es config the same
		constexpr unsigned anbits = nbits > 33 ? nbits : 33;
		constexpr unsigned fbits = posit<anbits, es>::fbits;
		value<fbits> v;
		a.normalize_to(v);
		value<fbits> vsqrt = fast_sqrt<anbits, es, fbits>(v);
		convert(vsqrt, p);

		return p;
	}
#else
	template<unsigned nbits, unsigned es, typename bt>
	inline posit<nbits, es, bt> sqrt(const posit<nbits, es, bt>& a) {
		if (a.sign()) return posit<nbits, es, bt>(SpecificValue::nar);
		return posit<nbits, es, bt>(std::sqrt((double)a));
	}
#endif

	// reciprocal sqrt
	template<unsigned nbits, unsigned es, typename bt>
	inline posit<nbits, es, bt> rsqrt(const posit<nbits, es, bt>& a) {
		posit<nbits, es, bt> v = sqrt(a);
		return v.reciprocal();
	}

	///////////////////////////////////////////////////////////////////
	// specialized sqrt configurations


	// fast sqrt for posit<2,0>
	template<>
	inline posit<3, 0> sqrt(const posit<3, 0>& a) {
		posit<3, 0> p;
		if (a.isneg() || a.isnar()) {
			p.setnar();
			return p;
		}
		unsigned root = posit_3_0_roots[unsigned(a.bits())];
		p.setbits(root);
		return p;
	}

	// fast sqrt for posit<3,1>
	template<>
	inline posit<3, 1> sqrt(const posit<3, 1>& a) {
		posit<3, 1> p;
		if (a.isneg() || a.isnar()) {
			p.setnar();
			return p;
		}
		unsigned root = posit_3_1_roots[unsigned(a.bits())];
		p.setbits(root);
		return p;
	}

	// fast sqrt for posit<4,0>
	template<>
	inline posit<4, 0> sqrt(const posit<4, 0>& a) {
		posit<4, 0> p;
		if (a.isneg() || a.isnar()) {
			p.setnar();
			return p;
		}

		unsigned root = posit_4_0_roots[unsigned(a.bits())];
		p.setbits(root);
		return p;
	}

	// fast sqrt for posit<5,0>
	template<>
	inline posit<5, 0> sqrt(const posit<5, 0>& a) {
		posit<5, 0> p;
		if (a.isneg() || a.isnar()) {
			p.setnar();
			return p;
		}
		unsigned root = posit_5_0_roots[unsigned(a.bits())];
		p.setbits(root);
		return p;
	}

	// fast sqrt for posit<8,0>
	template<>
	inline posit<8, 0> sqrt(const posit<8, 0>& a) {
		posit<8, 0> p;
		if (a.isneg() || a.isnar()) {
			p.setnar();
			return p;
		}
		unsigned root = posit_8_0_roots[unsigned(a.bits())];
		p.setbits(root);
		return p;
	}

	// fast sqrt for posit<8,1>
	template<>
	inline posit<8, 1> sqrt(const posit<8, 1>& a) {
		posit<8, 1> p;
		if (a.isneg() || a.isnar()) {
			p.setnar();
			return p;
		}
		unsigned root = posit_8_1_roots[unsigned(a.bits())];
		p.setbits(root);
		return p;
	}

	// seed sqrt approximation
	const uint16_t approxRecipSqrt0[16] = {
		0xb4c9, 0xffab, 0xaa7d, 0xf11c, 0xa1c5, 0xe4c7, 0x9a43, 0xda29,
		0x93b5, 0xd0e5, 0x8ded, 0xc8b7, 0x88c6, 0xc16d, 0x8424, 0xbae1
	};
	const uint16_t approxRecipSqrt1[16] = {
		0xa5a5, 0xea42, 0x8c21, 0xc62d, 0x788f, 0xaa7f, 0x6928, 0x94b6,
		0x5cc7, 0x8335, 0x52a6, 0x74e2, 0x4a3e, 0x68fe, 0x432b, 0x5efd
	};

#if POSIT_FAST_POSIT_16_1

	// fast sqrt for posit<16,1>
	template<>
	inline posit<16, 1> sqrt(const posit<16, 1>& a) {
		posit<16, 1> p;
		if (a.isneg() || a.isnar()) {
			p.setnar();
			return p;
		}
		if (a.iszero()) {
			p.setzero();
			return p;
		}

		uint16_t raw = static_cast<uint16_t>(unsigned(a.bits()));
		int16_t scale;
		// Compute the square root. Here, kZ is the net power-of-2 scaling of the result.
		// Decode the regime and exponent bit; scale the input to be in the range 1 to 4:			
		if (raw & 0x4000) {
			scale = -1;
			while (raw & 0x4000) {
				++scale;
				raw = static_cast<uint16_t>((raw << 1) & 0xFFFF);
			}
		}
		else {
			scale = 0;
			while (!(raw & 0x4000)) {
				--scale;
				raw = static_cast<uint16_t>((raw << 1) & 0xFFFF);
			}

		}
		raw &= 0x3FFF;
		uint16_t exp = static_cast<uint16_t>(1 - (raw >> 13));
		uint16_t rhs_fraction = static_cast<uint16_t>((raw | 0x2000) >> 1);

		// Use table look-up of first four bits for piecewise linear approximation of 1/sqrt:
		uint16_t index = static_cast<uint16_t>(((rhs_fraction >> 8) & 0x000E) + exp);

		uint32_t r0 = approxRecipSqrt0[index] - ((uint32_t(approxRecipSqrt1[index])	* (rhs_fraction & 0x01FF)) >> 13);
		// Use Newton-Raphson refinement to get more accuracy for 1/sqrt:
		uint32_t eSqrR0 = ((uint_fast32_t)r0 * r0) >> 1;

		if (exp) eSqrR0 >>= 1;
		uint16_t sigma0 = 0xFFFF ^ (0xFFFF & (((uint64_t)eSqrR0 * (uint64_t)rhs_fraction) >> 18));
		uint32_t oneOverSqrt = (r0 << 2) + ((r0 * sigma0) >> 23);

		// We need 17 bits of accuracy for posit16 square root approximation.
		// Multiplying 16 bits and 18 bits needs 64-bit scratch before rounding.
		uint32_t result_fraction = (((uint64_t)rhs_fraction) * oneOverSqrt) >> 13;

		// Figure out the regime and the resulting right shift of the fraction
		uint16_t shift;
		if (scale < 0) {
			shift = static_cast<uint16_t>((-1 - scale) >> 1);
			raw = static_cast<uint16_t>(0x2000 >> shift);   // build up the raw bits of the result posit
		}
		else {
			shift = static_cast<uint16_t>(scale >> 1);
			raw = static_cast<uint16_t>(0x7FFF - (0x7FFF >> (shift + 1)));
		}
		// Set the exponent bit in the answer, if it is nonzero:
		if (scale & 1) raw |= (0x1000 >> shift);

		// Right-shift fraction bits, accounting for 1 <= a < 2 versus 2 <= a < 4:
		result_fraction = result_fraction >> (exp + shift);

		// Trick for eliminating off-by-one cases that only uses one multiply:
		result_fraction++;
		if (!(result_fraction & 0x0007)) {
			uint32_t shiftedFraction = result_fraction >> 1;
			uint32_t negRem = (shiftedFraction * shiftedFraction) & 0x0003FFFF;  // 0x0003'FFFF;
			if (negRem & 0x00020000) {
				result_fraction |= 1;
			}
			else {
				if (negRem) --result_fraction;
			}
		}
		// Strip off the hidden bit and round-to-nearest using last 4 bits.
		result_fraction -= (0x00010000 >> shift);
		bool bitNPlusOne = bool((result_fraction >> 3) & 0x1);
		if (bitNPlusOne) {
			if (((result_fraction >> 4) & 1) | (result_fraction & 7)) result_fraction += 0x0010;
		}
		// Assemble the result and return it.
		p.setbits(raw | (result_fraction >> 4));
		return p;
	}
#endif // POSIT_FAST_POSIT_16_1

#if POSIT_FAST_POSIT_16_2

	// fast sqrt for posit<16,2>
	template<>
	inline posit<16, 2> sqrt(const posit<16, 2>& a) {
		return posit<16, 2>(std::sqrt(double(a)));
	}

#endif  // POSIT_FAST_POSIT_16_2

#if POSIT_FAST_POSIT_32_2

	// fast sqrt for posit<32,2>
	template<>
	inline posit<32, 2> sqrt(const posit<32, 2>& a) {
		posit<32, 2> p;
		if (a.isneg() || a.isnar()) {
			p.setnar();
			return p;
		}
		if (a.iszero()) {
			p.setzero();
			return p;
		}

		uint32_t raw = static_cast<uint32_t>((unsigned long long)(a.bits()));
		int32_t scale;
		// Compute the square root; shiftZ is the power-of-2 scaling of the result.
		// Decode regime and exponent; scale the input to be in the range 1 to 4:
		if (raw & 0x40000000) {
			scale = -2;
			while (raw & 0x40000000) {
				scale += 2;
				raw = (raw << 1) & 0xFFFFFFFF;
			}
		}
		else {
			scale = 0;
			while (!(raw & 0x40000000)) {
				scale -= 2;
				raw = (raw << 1) & 0xFFFFFFFF;
			}
		}

		raw &= 0x3FFFFFFF;
		uint32_t exp = (raw >> 28);
		scale += (exp >> 1);
		exp = (0x1 ^ (exp & 0x1));
		raw &= 0x0FFFFFFF;
		uint32_t rhs_fraction = (raw | 0x10000000);

		// Use table look-up of first 4 bits for piecewise linear approx. of 1/sqrt:
		uint32_t index = ((rhs_fraction >> 24) & 0x000E) + exp;
		int32_t eps = static_cast<int32_t>(((rhs_fraction >> 9) & 0xFFFF));
		uint32_t r0 = approxRecipSqrt0[index] - ((uint64_t(approxRecipSqrt1[index]) * eps) >> 20);

		// Use Newton-Raphson refinement to get 33 bits of accuracy for 1/sqrt:
		uint64_t eSqrR0 = (uint64_t)r0 * r0;
		if (!exp) eSqrR0 <<= 1;
		uint64_t sigma0 = 0xFFFFFFFF & (0xFFFFFFFF ^ ((eSqrR0 * (uint64_t)rhs_fraction) >> 20));
		uint64_t recipSqrt = ((uint64_t)r0 << 20) + (((uint64_t)r0 * sigma0) >> 21);

		uint64_t sqrSigma0 = ((sigma0 * sigma0) >> 35);
		recipSqrt += (((recipSqrt + (recipSqrt >> 2) - ((uint64_t)r0 << 19)) * sqrSigma0) >> 46);


		uint64_t result_fraction = (((uint64_t)rhs_fraction) * recipSqrt) >> 31;
		if (exp) result_fraction = (result_fraction >> 1);

		// Find the exponent of Z and encode the regime bits
		uint32_t result_exp = static_cast<uint32_t>(scale & 0x3);
		uint32_t shift;
		if (scale < 0) {
			shift = static_cast<uint32_t>((-1 - scale) >> 2);
			raw = static_cast<uint32_t>(0x20000000u >> shift);     // build up the raw bits of the result posit
		}
		else {
			shift = static_cast<uint32_t>(scale >> 2);
			raw = static_cast<uint32_t>(0x7FFFFFFFu - (0x3FFFFFFFu >> shift));
		}

		// Trick for eliminating off-by-one cases that only uses one multiply:
		result_fraction++;
		if (!(result_fraction & 0x000Fu)) {
			uint64_t shiftedFraction = result_fraction >> 1;
			uint64_t negRem = (shiftedFraction * shiftedFraction) & 0x1FFFFFFFF; // 0x1'FFFF'FFFF;
			if (negRem & 0x100000000u) {  // 0x1'0000'0000 
				result_fraction |= 1;
			}
			else {
				if (negRem) --result_fraction;
			}
		}
		// Strip off the hidden bit and round-to-nearest using last shift+5 bits.
		result_fraction &= 0xFFFF'FFFFu;
		uint64_t mask = (1ull << (4ull + shift));
		if (mask & result_fraction) {
			if (((mask - 1) & result_fraction) | ((mask << 1ul) & result_fraction)) result_fraction += (mask << 1ul);
		}
		// Assemble the result and return it.
		p.setbits(uint64_t(raw) | (uint64_t(result_exp) << (27 - shift)) | uint64_t(result_fraction >> (5 + shift)));
		return p;
	}

#endif // POSIT_FAST_POSIT_32_2

#if POSIT_FAST_POSIT_64_3

	// fast sqrt for posit<64,3>
	template<>
	inline posit<64, 3> sqrt(const posit<64, 3>& a) {
		throw "fast sqrt(posit<64,3>) not yet implemented";
		return 0;
	}

#endif // POSIT_FAST_POSIT_64_3

#if POSIT_FAST_POSIT_128_4

	// fast sqrt for posit<128,4>
	template<>
	inline posit<128, 4> sqrt(const posit<128, 4>& a) {
		throw "fast sqrt(posit<128,4>) not yet implemented";
		return 0;
	}

#endif // POSIT_FAST_POSIT_128_4

#if POSIT_FAST_POSIT_256_5

	// fast sqrt for posit<256,5>
	template<>
	inline posit<256, 5> sqrt(const posit<256, 5>& a) {
		throw "fast sqrt(posit<256,5>) not yet implemented";
		return 0;
	}

#endif // POSIT_FAST_POSIT_256_5

}} // namespace sw::universal
