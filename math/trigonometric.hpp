#pragma once
// trigonometric.hpp: trigonometric functions for posits
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


namespace sw {
	namespace unum {
		// value representing an angle expressed in radians
		// One radian is equivalent to 180/PI degrees

		// sine of an angle of x radians
		template<size_t nbits, size_t es>
		posit<nbits,es> sin(posit<nbits,es> x) {
			return posit<nbits,es>(std::sin(double(x)));
		}

		// cosine of an angle of x radians
		template<size_t nbits, size_t es>
		posit<nbits,es> cos(posit<nbits,es> x) {
			return posit<nbits,es>(std::cos(double(x)));
		}

		// tangent of an angle of x radians
		template<size_t nbits, size_t es>
		posit<nbits,es> tan(posit<nbits,es> x) {
			return posit<nbits,es>(std::tan(double(x)));
		}

		// cotangent of an angle of x radians
		template<size_t nbits, size_t es>
		posit<nbits,es> atan(posit<nbits,es> x) {
			return posit<nbits,es>(std::atan(double(x)));
		}

		// cosecant of an angle of x radians
		template<size_t nbits, size_t es>
		posit<nbits,es> acos(posit<nbits,es> x) {
			return posit<nbits,es>(std::acos(double(x)));
		}

		// secant of an angle of x radians
		template<size_t nbits, size_t es>
		posit<nbits,es> asin(posit<nbits,es> x) {
			return posit<nbits,es>(std::asin(double(x)));
		}


	}  // namespace unum

}  // namespace sw
