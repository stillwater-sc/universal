#pragma once
// gfp.hpp: simplified floating-point to support generating fast decimal representations of floating-points
//
// Copyright (C) 2022-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <sstream>

// should be defined by calling environment, catching it here just in case it is not
#ifndef LONG_DOUBLE_SUPPORT
#pragma message("LONG_DOUBLE_SUPPORT is not defined")
#define LONG_DOUBLE_SUPPORT 0
#endif
#include <universal/native/ieee754.hpp>

namespace sw {
	namespace universal {

		template<typename UnsignedInt>
		class gfp {
			// BlockType can be one of uint8_t, uint16_t, uint32_t, uint64_t, or an arbitrary precision unsigned integer
			static_assert (
				std::is_same<UnsignedInt, std::uint8_t>::value  || 
				std::is_same<UnsignedInt, std::uint16_t>::value ||
				std::is_same<UnsignedInt, std::uint32_t>::value ||
				std::is_same<UnsignedInt, std::uint64_t>::value, "UnsignedInt must be one of [uint8_t, uint16_t, uint32_t, uint64_t]");
		public:
			static constexpr unsigned sizeOfUint = sizeof(UnsignedInt) * 8;
			static constexpr unsigned rightShift = sizeof(UnsignedInt) * 4;

			/// trivial constructor
			gfp() noexcept = default;

			gfp(const gfp&) noexcept = default;
			gfp(gfp&&) noexcept = default;

			gfp& operator=(const gfp&) noexcept = default;
			gfp& operator=(gfp&&) noexcept = default;

			template<typename Real>
			gfp& operator=(Real v) {
				uint64_t biased{ 0 }, f64{ 0 };
				extractFields<Real>(v, s, biased, f64);
				e = static_cast<int>(biased) - ieee754_parameter<Real>::bias;
				f = static_cast<UnsignedInt>(f64);
				return *this;
			}
			gfp& operator+=(const gfp& rhs) {
				assert(e == rhs.e && f >= rhs.f);
				f += rhs.f;
				return *this;
			}
			gfp& operator-=(const gfp& rhs) {
				assert(e == rhs.e && f >= rhs.f);
				f -= rhs.f;
				return *this;
			}
			gfp& operator*=(const gfp& rhs) {
				std::uint64_t mask = (~0ull) >> rightShift;
				std::uint64_t a = f >> rightShift;
				std::uint64_t b = f & mask;
				std::uint64_t c = rhs.f >> rightShift;
				std::uint64_t d = rhs.f & mask;
				std::uint64_t ac = a * c;
				std::uint64_t bc = b * c;
				std::uint64_t ad = a * d;
				std::uint64_t bd = b * d;
				std::uint64_t tmp = (bd >> rightShift) + (ad & mask) + (bc & mask);
				tmp += (1ull << (rightShift - 1));  // round
				f = ac + (ad >> rightShift) + (bc >> rightShift) + (tmp >> rightShift);
				e = e + rhs.e + sizeOfUint;
				return *this;
			}

			void set(bool sign, int exponent, uint64_t fraction) noexcept {
				s = sign;
				e = exponent;
				f = fraction;
			}
		protected:

		private:
			bool        s;
			int         e;
			UnsignedInt f;

			template<typename U>
			friend std::ostream& operator<<(std::ostream&, const gfp<U>&);
		};

		template<typename UnsignedInt>
		std::ostream& operator<<(std::ostream& ostr, const gfp<UnsignedInt>& v) {
			ostr << (v.s ? "-" : "+") << static_cast<std::uint64_t>(v.f) << "e" << v.e;
			return ostr;
		}

		template<typename UnsignedInt>
		gfp<UnsignedInt> operator+(const gfp<UnsignedInt>& a, const gfp<UnsignedInt>& b) {
			gfp<UnsignedInt> sum(a);
			sum += b;
			return sum;
		}

		template<typename UnsignedInt>
		gfp<UnsignedInt> operator-(const gfp<UnsignedInt>& a, const gfp<UnsignedInt>& b) {
			gfp<UnsignedInt> difference(a);
			difference -= b;
			return difference;
		}

		template<typename UnsignedInt>
		gfp<UnsignedInt> operator*(const gfp<UnsignedInt>& a, const gfp<UnsignedInt>& b) {
			gfp<UnsignedInt> product(a);
			product *= b;
			return product;
		}
	}
} 
