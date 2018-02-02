#pragma once

// posit_functions.hpp: simple math functions
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
	namespace unum {

		// universal information functions to provide details regarding the properties of a posit configuration

		// calculate exponential scale of useed
		template<size_t nbits, size_t es>
		int useed_scale() {
			return (uint32_t(1) << es);
		}

		// calculate exponential scale of maxpos
		template<size_t nbits, size_t es>
		int maxpos_scale() {
			return (nbits - 2) * (1 << es);
		}

		// calculate exponential scale of minpos
		template<size_t nbits, size_t es>
		int minpos_scale() {
			return static_cast<int>(2 - int(nbits)) * (1 << es);
		}

		// calculate the constrained k value
		template<size_t nbits, size_t es>
		int calculate_k(int scale) {
			// constrain the scale to range [minpos, maxpos]
			if (scale < 0) {
				scale = scale > minpos_scale<nbits, es>() ? scale : minpos_scale<nbits, es>();
			}
			else {
				scale = scale < maxpos_scale<nbits, es>() ? scale : maxpos_scale<nbits, es>();
			}
			// bad int k = scale < 0 ? -(-scale >> es) - 1 : (scale >> es);
			// the scale of a posit is  2 ^ scale = useed ^ k * 2 ^ exp
			// -> (scale >> es) = (k*2^es + exp) >> es
			// -> (scale >> es) = k + (exp >> es) -> k = (scale >> es)
			int k = scale < 0 ? -(-scale >> es) : (scale >> es);
			if (k == 0 && scale < 0) {
				// project back to south-east quadrant
				k = -1;
			}
			return k;
		}

		// calculate the unconstrained k value
		template<size_t nbits, size_t es>
		int calculate_unconstrained_k(int scale) {
			// the scale of a posit is  2 ^ scale = useed ^ k * 2 ^ exp
			// -> (scale >> es) = (k*2^es + exp) >> es
			// -> (scale >> es) = k + (exp >> es) 
			// -> k = (scale >> es)
			int k = scale < 0 ? -(-scale >> es) : (scale >> es);
			if (k == 0 && scale < 0) {
				// project back to south-east quadrant
				k = -1;
			}
			return k;
		}

		// double value representation of the useed value of a posit<nbits, es>
		template<size_t nbits, size_t es>
		double useed() {
			return std::pow(2.0, std::pow(2.0, es));
		}

		// calculate the value of useed
		template<size_t nbits, size_t es>
		double useed_value() {
			return double(uint64_t(1) << useed_scale<nbits, es>());
		}

		// calculate the value of maxpos
		template<size_t nbits, size_t es>
		long double maxpos_value() {
			return std::pow((long double)(useed_value<nbits, es>()), (long double)(nbits - 2));
		}

		// calculate the value of minpos
		template<size_t nbits, size_t es>
		long double minpos_value() {
			return std::pow((long double)(useed_value<nbits, es>()), (long double)(static_cast<int>(2 - int(nbits))));
		}

		// this comparison is for a two's complement number only, for example, the raw bits of a posit
		template<size_t nbits>
		bool lessThan(const std::bitset<nbits>& lhs, const std::bitset<nbits>& rhs) {
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

	}
}
