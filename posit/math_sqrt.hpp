#pragma once
// sqrt.hpp: sqrt functions for posits
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


/*

Mathematical 	C++ Symbol	Decimal Representation
Expression
pi				M_PI		3.14159265358979323846
pi/2			M_PI_2		1.57079632679489661923
pi/4			M_PI_4		0.785398163397448309616
1/pi			M_1_PI		0.318309886183790671538
2/pi			M_2_PI		0.636619772367581343076
2/sqrt(pi)		M_2_SQRTPI	1.12837916709551257390
sqrt(2)			M_SQRT2		1.41421356237309504880
1/sqrt(2)		M_SQRT1_2	0.707106781186547524401
e				M_E			2.71828182845904523536
log_2(e)		M_LOG2E		1.44269504088896340736
log_10(e)		M_LOG10E	0.434294481903251827651
log_e(2)		M_LN2		0.693147180559945309417
log_e(10)		M_LN10		2.30258509299404568402

*/

#include "sqrt_tables.hpp"

namespace sw {
	namespace unum {

		// straight Babylonian
		inline double babylonian(double v) {
			double x_n = 0.5 * v; // initial guess
			const double eps = 1.0e-7;   // 
			do {
				x_n = (x_n + v / x_n) / 2.0;
			} while (std::abs(x_n * x_n - v) > eps);

			return x_n;
		}

		template<size_t nbits, size_t es>
		inline posit<nbits, es> BabylonianMethod(const posit<nbits, es>& v) {
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
		Then we have x = �f � Be. The number of bits in the exponent and
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

		// reference for fast direct sqrt method
		inline float my_test_sqrt(float a) {
			if (_trace_sqrt) std::cout << "----------------------- TEST SQRT -----------------------" << std::endl;

			bool s;
			int e;
			float fr;
			unsigned int _fraction;
			extract_fp_components(a, s, e, fr, _fraction);
			if (_trace_sqrt) std::cout << "f          " << a << std::endl;
			if (_trace_sqrt) std::cout << "e          " << e << std::endl;
			if (_trace_sqrt) std::cout << "fr         " << fr << std::endl;
			// onemme = 1.0 - machine epsilon
			union {
				float f;
				unsigned i;
			} m;
			m.i = 0x3f7fffff;
			float onemme = m.f;

			// y0 to 7.04 bits
			double y = 0.41731 + 0.59016 * fr;
			if (_trace_sqrt) std::cout << "y0         " << y << std::endl;

			// y1 to 15.08 bits
			double z = y + fr / y;
			if (_trace_sqrt) std::cout << "y1         " << z << std::endl;

			// y2 to 31.16 bits
			y = 0.25*z + fr / z;
			if (_trace_sqrt) std::cout << "y2         " << y << std::endl;

			// Include sqrt(2) factor for odd exponents, and
			// ensure(0.5 <= y) and (y < 1.0).
			// Otherwise, exponent calculation is incorrect
			if (e % 2) {
				y = y * 0.707106781186547524400844362104;
				if (_trace_sqrt) std::cout << "y*sqrt0.5  " << y << std::endl;
				y = (y < 0.5 ? 0.5 : y);  // max(y, 0.5)
				e = e + 1;
			}
			else {
				y = (y < onemme ? y : onemme); //  min(y, onemme);
			}
			if (_trace_sqrt) std::cout << "y adjusted " << y << std::endl;
			// update exponent to undo range reduction.
			value<23> v(y);
			v.setExponent((e >> 1) - 1);
			return v.to_float();
		}

		// fast sqrt at a given posit configuration.
		template<size_t nbits, size_t es, size_t fbits> 
		inline value<fbits> fast_sqrt(value<fbits>& v) {
			if (_trace_sqrt) std::cout << "---------------------------  SQRT -----------------------" << std::endl;
//			static_assert(nbits >= 16, "fast_sqrt requires posit configurations nbits >= 16");
			posit<nbits, es> fr = v.fraction_value()*0.5;
			int e = v.scale()+1;
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

			value<fbits> vsqrt = y.convert_to_scientific_notation();
			vsqrt.setExponent((e >> 1) - 1);
			if (_trace_sqrt) std::cout << "vsqrt      " << vsqrt << std::endl;
			return vsqrt;
		}

		// sqrt for arbitrary posit
		template<size_t nbits, size_t es>
		inline posit<nbits, es> sqrt(const posit<nbits, es>& a) {
			posit<nbits, es> p;
			if (a.isNegative() || a.isNaR()) {
				p.setToNaR();
				return p;
			}

			// for small posits use a more precise posit to do the calculation while keeping the es config the same
			constexpr size_t anbits = nbits > 33 ? nbits : 33;
			constexpr size_t fbits = posit<anbits,es>::fbits;
			value<fbits> v;
			a.normalize_to(v);
			value<fbits> vsqrt = fast_sqrt<anbits, es, fbits>(v);
			p.convert(vsqrt);

			return p;
		}

		// reciprocal sqrt
		template<size_t nbits, size_t es>
		inline posit<nbits, es> rsqrt(const posit<nbits,es>& a) {
			posit<nbits,es> v = sqrt(a);
			return v.reciprocate();
		}

	}  // namespace unum

}  // namespace sw
