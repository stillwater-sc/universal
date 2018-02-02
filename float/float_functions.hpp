#pragma once

// float_functions.hpp: simple math functions
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
	namespace ieee {

		// universal information functions to provide details regarding the properties of a posit configuration

#if TEMPLATIZED_TYPE
		// calculate exponential scale of maxpos
		template<typename Ty>
		int maxpos_scale() {
			return 0;
		}

		// calculate exponential scale of minpos
		template<typename Ty>
		int minpos_scale() {
			return 0;
		}

		// calculate the value of maxpos
		template<typename Ty>
		long double maxpos_value() {
			return 0;
		}

		// calculate the value of minpos
		template<typename Ty>
		long double minpos_value() {
			return 0;
		}
#else 
		// calculate exponential scale of maxpos
		template<size_t nbits, size_t es>
		int maxpos_scale() {
			return 0;
		}

		// calculate exponential scale of minpos
		template<size_t nbits, size_t es>
		int minpos_scale() {
			return 0;
		}

		// calculate the value of maxpos
		template<size_t nbits, size_t es>
		long double maxpos_value() {
			return 0;
		}

		// calculate the value of minpos
		template<size_t nbits, size_t es>
		long double minpos_value() {
			return 0;
		}
#endif // TEMPLATIZED_TYPE

	}
}
