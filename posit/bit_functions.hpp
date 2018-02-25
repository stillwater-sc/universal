#pragma once
// bit_functions.hpp: definitions of helper functions for bit operations on integers and floats
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <sstream>
#include <iomanip>
#include <limits>
#include <cmath>    // for frexpf/frexp/frexpl  float/double/long double fraction/exponent extraction

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

		// floating point component extractions
		inline void extract_fp_components(float fp, bool& _sign, int& _exponent, float& _fr, unsigned int& _fraction) {
			static_assert(sizeof(float) == 4, "This function only works when float is 32 bit.");
			_sign = fp < 0.0 ? true : false;
			_fr = frexpf(fp, &_exponent);
			_fraction = uint32_t(0x007FFFFFul) & reinterpret_cast<uint32_t&>(_fr);
		}
		inline void extract_fp_components(double fp, bool& _sign, int& _exponent, double& _fr, unsigned long long& _fraction) {
			static_assert(sizeof(double) == 8, "This function only works when double is 64 bit.");
			_sign = fp < 0.0 ? true : false;
			_fr = frexp(fp, &_exponent);
			_fraction = uint64_t(0x000FFFFFFFFFFFFFull) & reinterpret_cast<uint64_t&>(_fr);
		}
		inline void extract_fp_components(long double fp, bool& _sign, int& _exponent, long double& _fr, unsigned long long& _fraction) {
			static_assert(std::numeric_limits<long double>::digits <= 64, "This function only works when long double significant is <= 64 bit.");
			if (sizeof(long double) == 8) { // it is just a double
				_sign = fp < 0.0 ? true : false;
				_fr = frexp(double(fp), &_exponent);
				_fraction = uint64_t(0x000FFFFFFFFFFFFFull) & reinterpret_cast<uint64_t&>(_fr);
			}
			else if (sizeof(long double) == 16 && std::numeric_limits<long double>::digits <= 64) {
				_sign = fp < 0.0 ? true : false;
				_fr = frexpl(fp, &_exponent);
				_fraction = uint64_t(0x7FFFFFFFFFFFFFFFull) & reinterpret_cast<uint64_t&>(_fr); // 80bit extended format only has 63bits of fraction
			}
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
