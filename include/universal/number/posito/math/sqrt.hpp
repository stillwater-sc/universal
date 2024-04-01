#pragma once
// sqrt.hpp: sqrt functions for positos
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/native/ieee754.hpp>
#include <universal/number/posito/math/sqrt_tables.hpp>

#ifndef POSITO_NATIVE_SQRT
#define POSITO_NATIVE_SQRT 0
#endif

namespace sw { namespace universal {

	// straight Babylonian
	template<unsigned nbits, unsigned es>
	inline posito<nbits, es> BabylonianMethod(const posito<nbits, es>& v) {
		const double eps = 1.0e-5;
		posito<nbits, es> half(0.5);
		posito<nbits, es> x_next;
		posito<nbits, es> x_n = half * v;
		posito<nbits, es> diff;
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

	// fast sqrt at a given posito configuration.
	template<unsigned nbits, unsigned es, unsigned fbits>
	inline internal::value<fbits> fast_posito_sqrt(internal::value<fbits>& v) {
		if (_trace_sqrt) std::cout << "---------------------------  SQRT -----------------------" << std::endl;
		//			static_assert(nbits >= 16, "fast_sqrt requires posito configurations nbits >= 16");
		posito<nbits, es> fr = v.fraction_value()*0.5;
		int e = v.scale() + 1;
		posito<nbits, es> y = posito<nbits, es>(0.41731f) + posito<nbits, es>(0.59016f) * fr;
		posito<nbits, es> z = y + fr / y;
		if (_trace_sqrt) {
			std::cout << "f          " << v << std::endl;
			std::cout << "e          " << e << std::endl;
			std::cout << "fr         " << fr << std::endl;
			std::cout << "y0         " << y << std::endl;
			std::cout << "y1         " << z << std::endl;
		}
		y = posito<nbits, es>(0.25f) * z + fr / z;
		if (_trace_sqrt) std::cout << "y2         " << y << std::endl;

		if (e % 2) {
			y *= posito<nbits, es>(0.707106781186547524400844362104);
			if (_trace_sqrt) std::cout << "y*sqrt0.5  " << y << std::endl;
			y = (y < posito<nbits, es>(0.5f) ? posito<nbits, es>(0.5f) : y);
			e += 1;
		}
		else {
			posito<nbits, es> one(1.0f), onemme;
			onemme = --one;
			y = (y < one ? y : onemme);
		}
		if (_trace_sqrt) std::cout << "y adjusted " << y << std::endl;

		internal::value<fbits> vsqrt = y.to_value();
		vsqrt.setscale((e >> 1) - 1);
		if (_trace_sqrt) std::cout << "vsqrt      " << vsqrt << std::endl;
		return vsqrt;
	}

#if POSITO_NATIVE_SQRT
	// sqrt for arbitrary posito
	template<unsigned nbits, unsigned es>
	inline posito<nbits, es> sqrt(const posito<nbits, es>& a) {
		posito<nbits, es> p;
		if (a.sign()) {
			p.setnar();
			return p;
		}

		// for small positos use a more precise posito to do the calculation while keeping the es config the same
		constexpr unsigned anbits = nbits > 33 ? nbits : 33;
		constexpr unsigned fbits = posito<anbits, es>::fbits;
		value<fbits> v;
		a.normalize_to(v);
		value<fbits> vsqrt = fast_sqrt<anbits, es, fbits>(v);
		convert(vsqrt, p);

		return p;
	}
#else
	template<unsigned nbits, unsigned es>
	inline posito<nbits, es> sqrt(const posito<nbits, es>& a) {
		if (a.sign()) return posito<nbits, es>(SpecificValue::nar);
		return posito<nbits, es>(std::sqrt((double)a));
	}
#endif

	// reciprocal sqrt
	template<unsigned nbits, unsigned es>
	inline posito<nbits, es> rsqrt(const posito<nbits, es>& a) {
		posito<nbits, es> v = sqrt(a);
		return v.reciprocate();
	}

	///////////////////////////////////////////////////////////////////
	// specialized sqrt configurations


	// fast sqrt for posito<2,0>
	template<>
	inline posito<3, 0> sqrt(const posito<3, 0>& a) {
		posito<3, 0> p;
		if (a.isneg() || a.isnar()) {
			p.setnar();
			return p;
		}
		unsigned root = posito_3_0_roots[a.bits()];
		p.setbits(root);
		return p;
	}

	// fast sqrt for posito<3,1>
	template<>
	inline posito<3, 1> sqrt(const posito<3, 1>& a) {
		posito<3, 1> p;
		if (a.isneg() || a.isnar()) {
			p.setnar();
			return p;
		}
		unsigned root = posito_3_1_roots[a.bits()];
		p.setbits(root);
		return p;
	}

	// fast sqrt for posito<4,0>
	template<>
	inline posito<4, 0> sqrt(const posito<4, 0>& a) {
		posito<4, 0> p;
		if (a.isneg() || a.isnar()) {
			p.setnar();
			return p;
		}

		unsigned root = posito_4_0_roots[a.bits()];
		p.setbits(root);
		return p;
	}

	// fast sqrt for posito<5,0>
	template<>
	inline posito<5, 0> sqrt(const posito<5, 0>& a) {
		posito<5, 0> p;
		if (a.isneg() || a.isnar()) {
			p.setnar();
			return p;
		}
		unsigned root = posito_5_0_roots[a.bits()];
		p.setbits(root);
		return p;
	}

	// fast sqrt for posito<8,0>
	template<>
	inline posito<8, 0> sqrt(const posito<8, 0>& a) {
		posito<8, 0> p;
		if (a.isneg() || a.isnar()) {
			p.setnar();
			return p;
		}
		unsigned root = posito_8_0_roots[a.bits()];
		p.setbits(root);
		return p;
	}

	// fast sqrt for posito<8,1>
	template<>
	inline posito<8, 1> sqrt(const posito<8, 1>& a) {
		posito<8, 1> p;
		if (a.isneg() || a.isnar()) {
			p.setnar();
			return p;
		}
		unsigned root = posito_8_1_roots[a.bits()];
		p.setbits(root);
		return p;
	}

}} // namespace sw::universal
