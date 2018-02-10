#pragma once

// math_functions.hpp: simple math functions for posits
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
	namespace unum {

		// straight Babylonian
		double babylonian(double v) {
			double x_n = 0.5 * v; // initial guess
			const double eps = 1.0e-7;   // 
			do {
				x_n = (x_n + v / x_n) / 2.0;
			} while (std::abs(x_n * x_n - v) > eps);

			return x_n;
		}

		template<size_t nbits, size_t es>
		posit<nbits, es> BabylonianMethod(const posit<nbits, es>& v) {
			const double eps = 1.0e-5;
			posit<nbits, es> half(0.5);
			posit<nbits, es> x_next;
			posit<nbits, es> x_n = half * v;
			posit<nbits, es> diff;
			do {
				x_next = (x_n + v / x_n) * half;
				diff = x_next - x_n;
				if (_trace_sqrt) std::cout << " x_n+1: " << x_next << " x_n: " << x_n << " diff " << diff << std::endl;
				x_n = x_next;
			} while (double(sw::unum::abs(diff)) > eps);
			return x_n;
		}

		/*
		- Consider the function argument, x, in floating-point form, with a base
		(or radix) B, exponent e, and a fraction, f , such that 1/B ? f < 1.
		Then we have x = ±f × Be. The number of bits in the exponent and
		fraction, and the value of the base, depends on the particular floating 
		point arithmetic system chosen.
		
		- Use properties of the elementary function to range reduce the argument
		x to a small fixed interval. 

		- Use a small polynomial approximation to produce an initial estimate,
		y0, of the function on the small interval. Such an estimate may
		be good to perhaps 5 to 10 bits.

		- Apply Newton iteration to refine the result. This takes the form yk =
		yk?1/2 + (f /2)/yk?1. In base 2, the divisions by two can be done by
		exponent adjustments in floating-point computation, or by bit shifting
		in fixed-point computation.

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


		float my_test_sqrt(float a) {
			bool s;
			int e;
			float f;
			unsigned int _fraction;
			extract_fp_components(a, s, e, f, _fraction);

			// onemme = 1.0 - machine epsilon
			union {
				float f;
				unsigned i;
			} m;
			m.i = 0x3f7fffff;
			float onemme = m.f;

			// y0 to 7.04 bits
			double y = 0.41731 + 0.59016 * f;

			// y1 to 15.08 bits
			double z = y + f / y;

			// y2 to 31.16 bits
			y = 0.25*z + f / z;

			// Include sqrt(2) factor for odd exponents, and
			// ensure(0.5 <= y) and (y < 1.0).
			// Otherwise, exponent calculation is incorrect
			if (e % 2) {
				y = y * 0.707106781186547524400844362104;
				y = (y < 0.5 ? 0.5 : y);  // max(y, 0.5)
				e = e + 1;
			}
			else {
				y = (y < onemme ? y : onemme); //  min(y, onemme);
			}

			// update exponent to undo range reduction.
			value<23> v(y);
			v.setExponent((e >> 1) - 1);
			return v.to_float();
		}

		// fast sqrt at a given posit configuration. Does not work for small posits
		template<size_t nbits, size_t es, size_t fbits> 
		value<fbits> fast_sqrt(value<fbits>& v) {
			static_assert(nbits >= 16, "fast_sqrt requires posit configurations nbits >= 16");
			posit<nbits, es> f = v.fraction_value();
			posit<nbits, es> y = posit<nbits, es>(0.41731f) + posit<nbits, es>(0.59016f) * f;
			posit<nbits, es> z = y + f / y;
			y = posit<nbits, es>(0.25f) * z + f / z;
			int e = v.scale();
			if (e % 2) {
				y *= posit<nbits, es>(0.707106781186547524400844362104);
				y = (y < posit<nbits, es>(0.5f) ? posit<nbits, es>(0.5f) : y);
				e += 1;
			}
			else {
				posit<nbits, es> one(1.0f), onemme;
				onemme = --one;
				y = (y < one ? y : onemme);
			}
			value<fbits> vsqrt = y.convert_to_scientific_notation();
			vsqrt.setExponent((e >> 1) - 1);
			return vsqrt;
		}

		template<size_t nbits, size_t es>
		posit<nbits, es> sqrt(const posit<nbits, es>& a) {
			posit<nbits, es> p;
			if (a.isNegative() || a.isNaR()) {
				p.setToNaR();
				return p;
			}

// TODO: we could also do lookup tables for small posits: seems more appropriate

				// for small posits use 16bit posits to do the calculation while keeping the es config the same
				constexpr size_t anbits = nbits > 16 ? nbits : 16;
				constexpr size_t fbits = posit<anbits,es>::fbits;
				value<fbits> v(a.get_fraction().value());
				value<fbits> vsqrt = fast_sqrt<anbits, es, fbits>(v);
				p.convert(v);

			return p;
		}

	};  // namespace unum

};  // namespace sw
