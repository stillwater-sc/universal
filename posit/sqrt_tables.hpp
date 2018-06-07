#pragma once

// sqrt_tables.hpp: specialized posit configurations to support efficient sqrt for small posits
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
	namespace unum {

		// need a better code generator for the small posits up to nbits = 8
		// TODO: find if there is any structure in these tables across nbits and es

		template<size_t nbits, size_t es>
		void GenerateSqrtTable() {
			constexpr unsigned int NR_POSITS = (unsigned(1) << (nbits - 1)); // no need for negative posits

			std::cout << std::setprecision(20);
			posit<nbits, es> p;
			for (unsigned int i = 0; i < NR_POSITS; i++) {
				p.set_raw_bits(i);
				double ref = std::sqrt(double(p));
				posit<nbits, es> psqrt(ref);
				std::cout << p.get() << " " << psqrt.get() << "      " << p << " " << psqrt << " ref: " << ref << std::endl;
			}
			std::cout << std::setprecision(5);
		}

		// roots for posit<3,0>
		//   v   r       v   r          high precision root
		//  000 000      0   0     ref : 0
		//  001 001      0.5 0.5   ref : 0.70710678118654757274
		//	010 010      1   1     ref : 1
		//	011 010      2   1     ref : 1.4142135623730951455
		inline posit<3, 0> sqrt(const posit<3, 0>& a) {
			posit<3, 0> p;
			if (a.isNegative() || a.isNaR()) {
				p.setToNaR();
				return p;
			}
			unsigned roots[4] = { 0, 1, 2, 2 };
			unsigned root = roots[a.get_encoding_as_integer()];
			p.set_raw_bits(root);
			return p;
		}

		// roots for posit<3,1>
		//   v   r       v   r          high precision root
		//  000 000      0    0     ref : 0
		//  001 001      0.25 0.5   ref : 0.5
		//	010 010      1   1      ref : 1
		//	011 010      4   1      ref : 1
		inline posit<3, 1> sqrt(const posit<3, 1>& a) {
			posit<3, 1> p;
			if (a.isNegative() || a.isNaR()) {
				p.setToNaR();
				return p;
			}
			unsigned roots[4] = { 0, 1, 2, 2 };
			unsigned root = roots[a.get_encoding_as_integer()];
			p.set_raw_bits(root);
			return p;
		}

		// roots for posit<4,0>
		//   v    r        v    r        high precision root
		//	0000 0000      0    0        ref: 0
		//	0001 0010      0.25 0.5      ref: 0.5
		//	0010 0011      0.5  0.75     ref : 0.70710678118654757274
		//	0011 0011      0.75 0.75     ref : 0.86602540378443859659
		//	0100 0100      1    1        ref : 1
		//	0101 0100      1.5  1        ref : 1.2247448713915889407
		//	0110 0101      2    1.5      ref : 1.4142135623730951455
		//	0111 0110      4    2        ref : 2
		inline posit<4, 0> sqrt(const posit<4, 0>& a) {
			posit<4, 0> p;
			if (a.isNegative() || a.isNaR()) {
				p.setToNaR();
				return p;
			}
			unsigned roots[8] = { 0, 2, 3, 3, 4, 4, 5, 6 };
			unsigned root = roots[a.get_encoding_as_integer()];
			p.set_raw_bits(root);
			return p;
		}

		// roots for posit<4,0>
		//   v     r         v    r        high precision root
		//	00000 00000      0     0       ref: 0
		//	00001 00011      0.125 0.375   ref : 0.35355339059327378637
		//	00010 00100      0.25  0.5     ref : 0.5
		//	00011 00101      0.375 0.625   ref : 0.61237243569579447033
		//	00100 00110      0.5   0.75    ref : 0.70710678118654757274
		//	00101 00110      0.625 0.75    ref : 0.7905694150420948807
		//	00110 00111      0.75  0.875   ref : 0.86602540378443859659
		//	00111 00111      0.875 0.875   ref : 0.93541434669348533237
		//	01000 01000      1     1       ref : 1
		//	01001 01000      1.25  1       ref : 1.1180339887498949025
		//	01010 01001      1.5   1.25    ref : 1.2247448713915889407
		//	01011 01001      1.75  1.25    ref : 1.3228756555322953581
		//	01100 01010      2     1.5     ref : 1.4142135623730951455
		//	01101 01011      3     1.75    ref : 1.7320508075688771932
		//	01110 01100      4     2       ref : 2
		//	01111 01101      8     3       ref : 2.8284271247461902909
		inline posit<5, 0> sqrt(const posit<5, 0>& a) {
			posit<5, 0> p;
			if (a.isNegative() || a.isNaR()) {
				p.setToNaR();
				return p;
			}
			unsigned roots[16] = { 0, 3, 4, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12, 13 };
			unsigned root = roots[a.get_encoding_as_integer()];
			p.set_raw_bits(root);
			return p;
		}

	}  // namespace unum

}  // namespace sw
