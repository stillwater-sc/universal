#pragma once
// bitset_helpers.hpp: definitions of bitset operators and helpers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// number representation is sign-magnitude, sign is msb

#include <bitset>
#include <sstream>


namespace sw {
	namespace unum {

		// find the MSB, return position if found, return -1 if no bits are set
		template<size_t nbits>
		int findMostSignificantBit(const std::bitset<nbits>& bits) {
			int msb = -1; // indicative of no bits set
			for (int i = nbits - 1; i >= 0; i--) {
				if (bits.test(i)) {
					msb = i;
					break;
				}
			}
			return msb;
		}

		// calculate the 1's complement of a sign-magnitude encoded number
		template<size_t nbits>
		std::bitset<nbits> ones_complement(std::bitset<nbits> number) {
			std::bitset<nbits> complement;
			for (size_t i = 0; i < nbits; i++) {
				complement.set(i, !number[i]);
			}
			return complement;
		}

		// calculate the 2's complement of a 2's complement encoded number
		template<size_t nbits>
		std::bitset<nbits> twos_complement(std::bitset<nbits> number) {
			std::bitset<nbits> complement;
			uint8_t _slice = 0;
			uint8_t carry = 1;
			for (size_t i = 0; i < nbits; i++) {
				_slice = uint8_t(!number[i]) + carry;
				carry = _slice >> 1;
				complement[i] = (0x1 & _slice);
			}
			return complement;
		}

		// DANGER: this depends on the implicit type conversion of number to a uint64_t to sign extent a 2's complement number system
		// if nbits > 64 then this code breaks.
		template<size_t nbits, class Type>
		std::bitset<nbits> convert_to_bitset(Type number) {
			std::bitset<nbits> _Bits;
			uint64_t mask = uint64_t(1);
			for (std::size_t i = 0; i < nbits; i++) {
				_Bits[i] = mask & number;
				mask <<= 1;
			}
			return _Bits;
		}

		template<size_t nbits>
		std::string to_binary(std::bitset<nbits> bits) {
			std::stringstream ss;
			int msb = nbits; // compilation warning work-around for nbits = 0
			for (int i = msb - 1; i >= 0; --i) {
				ss << (bits[std::size_t(i)] ? "1" : "0");
			}
			return ss.str();
		}

		template<size_t nbits>
		std::string to_hex(std::bitset<nbits> bits) {
			char str[nbits];   // plenty of room
			const char* hexits = "0123456789ABCDEF";
			unsigned int max = (nbits >> 2) + (nbits % 4 ? 2 : 1);
			for (unsigned int i = 0; i < max; i++) {
				unsigned int hexit = (bits[3] << 3) + (bits[2] << 2) + (bits[1] << 1) + bits[0];
				str[max - 1 - i] = hexits[hexit];
				bits >>= 4;
			}
			str[max] = 0;
			return std::string(str);
		}

		// convert a sign/magnitude number to a string
		template<size_t nbits>
		std::string sign_magnitude_to_string(std::bitset<nbits> bits) {
			std::stringstream ss;
			ss << (bits[nbits - 1] ? "n-" : "p-");
			if (nbits < 2) return ss.str();
			for (int i = nbits - 2; i >= 0; --i) {
				ss << (bits[i] ? "1" : "0");
			}
			return ss.str();
		}

		// return a new bitset with the sign flipped as compared to the input bitset
		template<size_t nbits>
		std::bitset<nbits> flip_sign_bit(std::bitset<nbits> number) {
			number.flip(nbits - 1);
			return number;
		}

		// sticky bit representation of all the bits from [msb, lsb], that is, msb is included
		template<size_t nbits>
		bool anyAfter(const std::bitset<nbits>& bits, unsigned msb) {
			bool running = false;
			for (int i = msb; i >= 0; i--) {
				running |= bits.test(i);
			}
			return running;
		}

	} // namespace unum

} // namespace sw



