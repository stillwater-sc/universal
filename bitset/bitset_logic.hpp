//  logic.cpp : bitset-based logic operators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <bitset>

#include "exceptions.hpp"

namespace sw {
	namespace unum {

		// this comparison is for a two's complement number only
		template<size_t nbits>
		bool lessThan_twoscomplement (const std::bitset<nbits>& lhs, const std::bitset<nbits>& rhs) {
			// comparison of the sign bit
			if (lhs[nbits - 1] == 0 && rhs[nbits - 1] == 1)	return false;
			if (lhs[nbits - 1] == 1 && rhs[nbits - 1] == 0) return true;
			// sign is equal, compare the remaining bits
			for (int i = nbits - 2; i >= 0; --i) {
				if (lhs[i] == 0 && rhs[i] == 1)	return true;
				if (lhs[i] == 1 && rhs[i] == 0) return false;
			}
			// numbers are equal
			return false;
		}

		// this comparison works for any number
		template<size_t nbits>
		bool operator==(const std::bitset<nbits>& lhs, const std::bitset<nbits>& rhs) {
			// compare remaining bits
			for (int i = nbits - 1; i >= 0; --i) {
				if (lhs[i] != rhs[i]) return false;
			}
			// numbers are equal
			return true;
		}

		// this comparison is for unsigned numbers only
		template<size_t nbits>
		bool operator< (const std::bitset<nbits>& lhs, const std::bitset<nbits>& rhs) {
			// compare remaining bits
			for (int i = nbits - 1; i >= 0; --i) {
				if (lhs[i] == 0 && rhs[i] == 1)	return true;
				if (lhs[i] == 1 && rhs[i] == 0) return false;
			}
			// numbers are equal
			return false;
		}

		// this comparison is for unsigned numbers only
		template<size_t nbits>
		bool operator<= (const std::bitset<nbits>& lhs, const std::bitset<nbits>& rhs) {
			// compare remaining bits
			for (int i = nbits - 1; i >= 0; --i) {
				if (lhs[i] == 0 && rhs[i] == 1)	return true;
				if (lhs[i] == 1 && rhs[i] == 0) return false;
			}
			// numbers are equal
			return true;
		}

		// this comparison is for unsigned numbers only
		template<size_t nbits>
		bool operator> (const std::bitset<nbits>& lhs, const std::bitset<nbits>& rhs) {
			// compare remaining bits
			for (int i = nbits - 1; i >= 0; --i) {
				if (lhs[i] == 0 && rhs[i] == 1)	return false;
				if (lhs[i] == 1 && rhs[i] == 0) return true;
			}
			// numbers are equal
			return false;
		}

		// this comparison is for unsigned numbers only
		template<size_t nbits>
		bool operator>= (const std::bitset<nbits>& lhs, const std::bitset<nbits>& rhs) {
			// compare remaining bits
			for (int i = nbits - 1; i >= 0; --i) {
				if (lhs[i] == 0 && rhs[i] == 1)	return false;
				if (lhs[i] == 1 && rhs[i] == 0) return true;
			}
			// numbers are equal
			return true;
		}

	} // namespace unum

} // namespace sw

