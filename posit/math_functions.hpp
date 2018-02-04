#pragma once

// math_functions.hpp: simple math functions for posits
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
	namespace unum {

		/*
		*/

		double babylonian(double v)	{
			double x_n = 0.5 * v; // initial guess
			const double eps = 1.0e-7;   // 
			do {
				x_n = (x_n + v / x_n) / 2.0;
			} while ( std::abs(x_n * x_n - v) > eps);
	
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

		template<size_t nbits, size_t es>
		posit<nbits, es> sqrt(const posit<nbits, es>& a) {
			if (a.isNegative() || a.isNaR()) {
				posit<nbits, es> p;
				p.setToNaR();
				return p;
			}
			return BabylonianMethod(a);
		}

	};  // namespace unum

};  // namespace sw
