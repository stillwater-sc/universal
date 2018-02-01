#pragma once
// bit_functions.hpp: definitions of helper functions for bit operations on integers and floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <iomanip>
#include <cmath>  // for frexp/frexpf

// This file contains functions that DO NOT use the posit type.
// If you have helpers that use the posit type, add them to the file posit_manipulators.hpp
namespace sw {
	namespace unum {

		inline uint64_t two_to_the_power(uint64_t n) {
			return (uint64_t(1) << n);
		}

		// find the most significant bit set: first bit is at position 1, so that no bits set returns 0
		inline unsigned int findMostSignificantBit(unsigned long long x) {
			// find the first non-zero bit
			static const unsigned int bval[] =
			{ 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };

			unsigned int base = 0;
			if (x & 0xFFFFFFFF00000000) { base += 32; x >>= 32; }
			if (x & 0x00000000FFFF0000) { base += 16; x >>= 16; }
			if (x & 0x000000000000FF00) { base += 8;  x >>= 8; }
			if (x & 0x00000000000000F0) { base += 4;  x >>= 4; }
			return base + bval[x];
		}

		inline unsigned int findMostSignificantBit(long long x) {
			// find the first non-zero bit
			static const unsigned int bval[] =
			{ 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };

			uint64_t tmp = x;
			unsigned int base = 0;
			if (tmp & 0xFFFFFFFF00000000) { base += 32; tmp >>= 32; }
			if (tmp & 0x00000000FFFF0000) { base += 16; tmp >>= 16; }
			if (tmp & 0x000000000000FF00) { base += 8;  tmp >>= 8; }
			if (tmp & 0x00000000000000F0) { base += 4;  tmp >>= 4; }
			return base + bval[tmp];
		}

		inline unsigned int findMostSignificantBit(int x) {
			// find the first non-zero bit
			static const unsigned int bval[] =
			{ 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };

			uint32_t tmp = x;
			unsigned int base = 0;
			if (tmp & 0xFFFF0000) { base += 16; tmp >>= 16; }
			if (tmp & 0x0000FF00) { base += 8;  tmp >>= 8; }
			if (tmp & 0x000000F0) { base += 4;  tmp >>= 4; }
			return base + bval[tmp];
		}

		inline unsigned int findMostSignificantBit(short x) {
			// find the first non-zero bit
			static const unsigned int bval[] =
			{ 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };

			uint16_t tmp = x;
			unsigned int base = 0;
			if (tmp & 0xFF00) { base += 8;  tmp >>= 8; }
			if (tmp & 0x00F0) { base += 4;  tmp >>= 4; }
			return base + bval[tmp];
		}

		inline unsigned int findMostSignificantBit(signed char x) {
			// find the first non-zero bit
			static const unsigned int bval[] =
			{ 0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };

			uint8_t tmp = x;
			unsigned int base = 0;
			if (tmp & 0xF0) { base += 4;  tmp >>= 4; }
			return base + bval[tmp];
		}

		// FLOAT component extractions
		inline bool extract_sign(float f) {	return f < 0.0f; }

		int extract_exponent(float f) {
			int exponent;
			frexpf(f, &exponent);
			return exponent;
		}

		inline uint32_t extract_fraction(float f) 
		{
				static_assert(sizeof(float) == 4, "This function only works when float is 32 bit.");
			int exponent;
			float fraction = frexpf(f, &exponent);
			return uint32_t(0x007FFFFFul) & reinterpret_cast<uint32_t&>(fraction);
		}

		// DOUBLE component extractions
		inline bool extract_sign(double f) { return f < 0.0; }

		inline int extract_exponent(double f) 
		{
			int exponent;
			frexp(f, &exponent);
			return exponent;
		}

		inline uint64_t extract_fraction(double f) 
		{
				static_assert(sizeof(double) == 8, "This function only works when double is 64 bit.");
			int exponent;
			double fraction = frexp(f, &exponent);
			return uint64_t(0x000FFFFFFFFFFFFFull) & reinterpret_cast<uint64_t&>(fraction);
		}

		// LONG DOUBLE component extractions
		inline bool extract_sign(long double f) { return f < 0.0; }

		inline int extract_exponent(long double f) 
		{
			int exponent;
			frexp(f, &exponent);
			return exponent;
		}

		// integral type to bitset transformations

		// we are using a full nbits sized bitset even though nbits-3 is the maximum fraction
		// a posit would contain. However, we need an extra bit after the cut-off to make the
		// round up/down decision. The <nbits-something> size created a lot of sw complexity
		// that isn't worth the trouble, so we are simplifying and simply manage a full nbits
		// of fraction bits.

		template<size_t nbits>
		std::bitset<nbits> extract_float_fraction(uint32_t _23b_fraction_without_hidden_bit) {
			std::bitset<nbits> _fraction;
			uint32_t mask = uint32_t(0x00400000ul);
			unsigned int ub = (nbits < 23 ? nbits : 23);
			for (unsigned int i = 0; i < ub; i++) {
				_fraction[nbits - 1 - i] = _23b_fraction_without_hidden_bit & mask;
				mask >>= 1;
			}
			return _fraction;
		}

		template<size_t nbits>
		std::bitset<nbits> extract_double_fraction(uint64_t _52b_fraction_without_hidden_bit) {
			std::bitset<nbits> _fraction;
			uint64_t mask = uint64_t(0x0008000000000000ull);
			unsigned int ub = (nbits < 52 ? nbits : 52);
			for (unsigned int i = 0; i < ub; i++) {
				_fraction[nbits - 1 - i] = _52b_fraction_without_hidden_bit & mask;
				mask >>= 1;
			}
			return _fraction;
		}

		// 128 bit unsigned int mapped to two uint64_t elements
		typedef struct __uint128 {
			uint64_t upper;
			uint64_t lower;
		} uint128;

		// take in a long double mapped to two uint64_t elements
		template<size_t nbits>
		std::bitset<nbits> extract_long_double_fraction(uint128* _112b_fraction_without_hidden_bit) {
			std::bitset<nbits> _fraction;
			int msb = nbits - 1;
			uint64_t mask = uint64_t(0x0000800000000000ull);
			unsigned int ub = (nbits < 48 ? nbits : 48); // 48 bits in the upper half
			for (unsigned int i = 0; i < ub; i++) {
				_fraction[msb--] = _112b_fraction_without_hidden_bit->upper & mask;
				mask >>= 1;
			}
			mask = uint64_t(0x8000000000000000ull);
			ub = (nbits < 112-48 ? nbits : 112-48); // max 64 bits in the lower half
			for (unsigned int i = 0; i < ub; i++) {
				_fraction[msb--] = _112b_fraction_without_hidden_bit->lower & mask;
				mask >>= 1;
			}
			return _fraction;
		}

		template<size_t nbits>
		std::bitset<nbits> copy_integer_fraction(unsigned long long _fraction_without_hidden_bit) {
			std::bitset<nbits> _fraction;
			uint64_t mask = uint64_t(0x8000000000000000ull);
			unsigned int ub = (nbits < 64 ? nbits : 64);
			for (unsigned int i = 0; i < ub; i++) {
				_fraction[nbits - 1 - i] = _fraction_without_hidden_bit & mask;
				mask >>= 1;
			}
			return _fraction;
		}

		// representation helpers

		// nbits binary representation of a signed 64-bit number
		template<size_t nbits>
		std::string to_binary_(long long int number) {
			std::stringstream ss;
			uint64_t mask = uint64_t(1) << (nbits - 1);                // parenthesis to avoid clang's prio warning
			for (int i = nbits - 1; i >= 0; --i) {
				ss << (mask & number ? "1" : "0");
				mask >>= 1;
			}
			return ss.str();
		}

		// full binary representation of a signed 64-bit number
		inline std::string to_binary(long long int number) {
			std::stringstream ss;
			unsigned int msb = findMostSignificantBit(number);
			if (msb == 0) {
				ss << "-";
			}
			else {
				uint64_t mask = (uint64_t(1) << msb);
				for (int i = msb; i >= 0; --i) {
					ss << (mask & number ? "1" : "0");
					mask >>= 1;
				}
			}
			return ss.str();
		}
		// full binary representation of a unsigned 64-bit number
		inline std::string to_binary(unsigned long long number) {
			std::stringstream ss;
			unsigned int msb = findMostSignificantBit(number);
			if (msb == 0) {
				ss << "-";
			}
			else {
				uint64_t mask = (uint64_t(1) << msb);
				for (int i = msb; i >= 0; --i) {
					ss << (mask & number ? "1" : "0");
					if (i > 0 && i % 4 == 0) ss << "_";
					mask >>= 1;
				}
			}
			return ss.str();
		}

		// full binary representation of a float number
		inline std::string to_binary(float number) {
			std::stringstream ss;

			uint32_t bits = *(uint32_t*) &number;
			unsigned int msb = 31;
			uint64_t mask = (uint64_t(1) << msb);
			ss << (mask & bits ? "1|" : "0|");
			msb--;
			for (int i = msb; i >= 0; --i) {
				if (i == 22) ss << "|";
				ss << (mask & bits ? "1" : "0");
				mask >>= 1;
			}

			return ss.str();
		}

		// full binary representation of a double number
		inline std::string to_binary(double number) {
			std::stringstream ss;

			uint64_t bits = *(uint64_t*)&number;
			unsigned int msb = 63;
			uint64_t mask = (uint64_t(1) << msb);
			ss << (mask & bits ? "1|" : "0|");
			msb--;
			for (int i = msb; i >= 0; --i) {
				if (i == 51) ss << "|";
				ss << (mask & bits ? "1" : "0");
				mask >>= 1;
			}

			return ss.str();
		}

	}  // namespace unum

}  // namespace sw
