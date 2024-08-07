#pragma once
// f2s.hpp: simplified floating-point to support generating fast decimal representations of floating-points
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <sstream>
#include <cassert>
#include <universal/native/ieee754.hpp>

namespace sw {
	namespace universal {

		template<typename UInt> class f2s;

		struct CachedPower {
			uint64_t significand;
			int binary_exponent;
			int decimal_exponent;
		};

		static constexpr CachedPower CachedPowers[] = {
		  {(0xfa8fd5a0081c0288), -1220, -348},
		  {(0xbaaee17fa23ebf76), -1193, -340},
		  {(0x8b16fb203055ac76), -1166, -332},
		  {(0xcf42894a5dce35ea), -1140, -324},
		  {(0x9a6bb0aa55653b2d), -1113, -316},
		  {(0xe61acf033d1a45df), -1087, -308},
		  {(0xab70fe17c79ac6ca), -1060, -300},
		  {(0xff77b1fcbebcdc4f), -1034, -292},
		  {(0xbe5691ef416bd60c), -1007, -284},
		  {(0x8dd01fad907ffc3c), -980, -276},
		  {(0xd3515c2831559a83), -954, -268},
		  {(0x9d71ac8fada6c9b5), -927, -260},
		  {(0xea9c227723ee8bcb), -901, -252},
		  {(0xaecc49914078536d), -874, -244},
		  {(0x823c12795db6ce57), -847, -236},
		  {(0xc21094364dfb5637), -821, -228},
		  {(0x9096ea6f3848984f), -794, -220},
		  {(0xd77485cb25823ac7), -768, -212},
		  {(0xa086cfcd97bf97f4), -741, -204},
		  {(0xef340a98172aace5), -715, -196},
		  {(0xb23867fb2a35b28e), -688, -188},
		  {(0x84c8d4dfd2c63f3b), -661, -180},
		  {(0xc5dd44271ad3cdba), -635, -172},
		  {(0x936b9fcebb25c996), -608, -164},
		  {(0xdbac6c247d62a584), -582, -156},
		  {(0xa3ab66580d5fdaf6), -555, -148},
		  {(0xf3e2f893dec3f126), -529, -140},
		  {(0xb5b5ada8aaff80b8), -502, -132},
		  {(0x87625f056c7c4a8b), -475, -124},
		  {(0xc9bcff6034c13053), -449, -116},
		  {(0x964e858c91ba2655), -422, -108},
		  {(0xdff9772470297ebd), -396, -100},
		  {(0xa6dfbd9fb8e5b88f), -369, -92},
		  {(0xf8a95fcf88747d94), -343, -84},
		  {(0xb94470938fa89bcf), -316, -76},
		  {(0x8a08f0f8bf0f156b), -289, -68},
		  {(0xcdb02555653131b6), -263, -60},
		  {(0x993fe2c6d07b7fac), -236, -52},
		  {(0xe45c10c42a2b3b06), -210, -44},
		  {(0xaa242499697392d3), -183, -36},
		  {(0xfd87b5f28300ca0e), -157, -28},
		  {(0xbce5086492111aeb), -130, -20},
		  {(0x8cbccc096f5088cc), -103, -12},
		  {(0xd1b71758e219652c), -77, -4},
		  {(0x9c40000000000000), -50, 4},
		  {(0xe8d4a51000000000), -24, 12},
		  {(0xad78ebc5ac620000), 3, 20},
		  {(0x813f3978f8940984), 30, 28},
		  {(0xc097ce7bc90715b3), 56, 36},
		  {(0x8f7e32ce7bea5c70), 83, 44},
		  {(0xd5d238a4abe98068), 109, 52},
		  {(0x9f4f2726179a2245), 136, 60},
		  {(0xed63a231d4c4fb27), 162, 68},
		  {(0xb0de65388cc8ada8), 189, 76},
		  {(0x83c7088e1aab65db), 216, 84},
		  {(0xc45d1df942711d9a), 242, 92},
		  {(0x924d692ca61be758), 269, 100},
		  {(0xda01ee641a708dea), 295, 108},
		  {(0xa26da3999aef774a), 322, 116},
		  {(0xf209787bb47d6b85), 348, 124},
		  {(0xb454e4a179dd1877), 375, 132},
		  {(0x865b86925b9bc5c2), 402, 140},
		  {(0xc83553c5c8965d3d), 428, 148},
		  {(0x952ab45cfa97a0b3), 455, 156},
		  {(0xde469fbd99a05fe3), 481, 164},
		  {(0xa59bc234db398c25), 508, 172},
		  {(0xf6c69a72a3989f5c), 534, 180},
		  {(0xb7dcbf5354e9bece), 561, 188},
		  {(0x88fcf317f22241e2), 588, 196},
		  {(0xcc20ce9bd35c78a5), 614, 204},
		  {(0x98165af37b2153df), 641, 212},
		  {(0xe2a0b5dc971f303a), 667, 220},
		  {(0xa8d9d1535ce3b396), 694, 228},
		  {(0xfb9b7cd9a4a7443c), 720, 236},
		  {(0xbb764c4ca7a44410), 747, 244},
		  {(0x8bab8eefb6409c1a), 774, 252},
		  {(0xd01fef10a657842c), 800, 260},
		  {(0x9b10a4e5e9913129), 827, 268},
		  {(0xe7109bfba19c0c9d), 853, 276},
		  {(0xac2820d9623bf429), 880, 284},
		  {(0x80444b5e7aa7cf85), 907, 292},
		  {(0xbf21e44003acdd2d), 933, 300},
		  {(0x8e679c2f5e44ff8f), 960, 308},
		  {(0xd433179d9c8cb841), 986, 316},
		  {(0x9e19db92b4e31ba9), 1013, 324},
		  {(0xeb96bf6ebadf77d9), 1039, 332},
		  {(0xaf87023b9bf0ee6b), 1066, 340},
		};

		// Not all powers of ten are cached. The decimal exponent of two neighboring
		// cached numbers will differ by kDecimalExponentDistance.
		static constexpr int kDecimalExponentDistance = 8;

		// The minimal and maximal target exponent define the range of w's binary
		// exponent, where 'w' is the result of multiplying the input by a cached power
		// of ten.
		//
		// A different range might be chosen on a different platform, to optimize digit
		// generation, but a smaller range requires more powers of ten to be cached.
		static const int kMinimalTargetExponent = -60;
		static const int kMaximalTargetExponent = -32;

		static constexpr int kMinDecimalExponent = -348;
		static constexpr int kMaxDecimalExponent = 340;
		static constexpr int kCachedPowersOffset = 348;  // -1 * the first decimal_exponent.
		static constexpr double kD_1_LOG2_10 = 0.30102999566398114;  //  1 / lg(10)

		template<typename UnsignedInt>
		void GetCachedPowerForBinaryExponentRange(const int min_exponent, const int max_exponent, f2s<UnsignedInt>& power, int& decimal_exponent) {
			//using F2S = f2s<UnsignedInt>;
			int kQ = sizeof(UnsignedInt) * 4;
			double k = std::ceil((min_exponent + kQ - 1) * kD_1_LOG2_10);
			int index =	(kCachedPowersOffset + static_cast<int>(k) - 1) / kDecimalExponentDistance + 1;
			assert(0 <= index && index < static_cast<int>(sizeof(CachedPowers)));
			CachedPower cached_power = CachedPowers[index];
			assert(min_exponent <= cached_power.binary_exponent);
			(void)max_exponent;  // Mark variable as used.
			assert(cached_power.binary_exponent <= max_exponent);
			decimal_exponent = cached_power.decimal_exponent;
			power.set(false,  cached_power.binary_exponent, cached_power.significand, sizeof(UnsignedInt)*8);
		}

		template<typename UnsignedInt>
		void GetCachedPowerForDecimalExponent(const int requested_exponent, f2s<UnsignedInt>& power, int& found_exponent) {
			assert(kMinDecimalExponent <= requested_exponent);
			assert(requested_exponent < kMaxDecimalExponent + kDecimalExponentDistance);
			int index =	(requested_exponent + kCachedPowersOffset) / kDecimalExponentDistance;
			CachedPower cached_power = CachedPowers[index];
			power.set(false, cached_power.binary_exponent, cached_power.significand, sizeof(UnsignedInt)*8);
			found_exponent = cached_power.decimal_exponent;
			assert(found_exponent <= requested_exponent);
			assert(requested_exponent < found_exponent + kDecimalExponentDistance);
		}

		template<typename UnsignedInt>
		class f2s {
			// BlockType can be one of uint8_t, uint16_t, uint32_t, uint64_t. TODO: how do we support an arbitrary precision unsigned integer
			static_assert (
				std::is_same<UnsignedInt, std::uint8_t>::value  || 
				std::is_same<UnsignedInt, std::uint16_t>::value ||
				std::is_same<UnsignedInt, std::uint32_t>::value ||
				std::is_same<UnsignedInt, std::uint64_t>::value, "UnsignedInt must be one of [uint8_t, uint16_t, uint32_t, uint64_t]");
		public:
			static constexpr unsigned sizeOfSignificant = sizeof(UnsignedInt) * 8;
			static constexpr unsigned rightShift = sizeof(UnsignedInt) * 4;

			/// trivial constructor
			f2s() noexcept = default;

			f2s(const f2s&) noexcept = default;
			f2s(f2s&&) noexcept = default;

			f2s(float rhs) noexcept { *this = rhs; }
			f2s(double rhs) noexcept { *this = rhs; }

			f2s& operator=(const f2s&) noexcept = default;
			f2s& operator=(f2s&&) noexcept = default;

			f2s& operator=(float rhs) noexcept { return convert_ieee754(rhs); }
			f2s& operator=(double rhs) noexcept { return convert_ieee754(rhs); }

			f2s& operator+=(const f2s& rhs) {
				assert(e_ == rhs.e_ && f_ >= rhs.f_);
				f_ += rhs.f_;
				return *this;
			}
			f2s& operator-=(const f2s& rhs) {
				assert(e_ == rhs.e_ && f_ >= rhs.f_);
				f_ -= rhs.f_;
				return *this;
			}
			f2s& operator*=(const f2s& rhs) {
				std::uint64_t mask = (~0ull) >> rightShift;
				std::uint64_t a = f_ >> rightShift;
				std::uint64_t b = f_ & mask;
				std::uint64_t c = rhs.f_ >> rightShift;
				std::uint64_t d = rhs.f_ & mask;
				std::uint64_t ac = a * c;
				std::uint64_t bc = b * c;
				std::uint64_t ad = a * d;
				std::uint64_t bd = b * d;
				std::uint64_t tmp = (bd >> rightShift) + (ad & mask) + (bc & mask);
				tmp += (1ull << (rightShift - 1));  // round
				f_ = static_cast<UnsignedInt>(ac + (ad >> rightShift) + (bc >> rightShift) + (tmp >> rightShift));
				e_ = e_ + rhs.e_ + static_cast<int>(sizeOfSignificant);
				return *this;
			}

			explicit operator float() const noexcept { return to_native_ieee754<float>(); }
			explicit operator double() const noexcept { return to_native_ieee754<double>(); }

			/////////////////////////////////////////////////////
			// member functions

			// Computes the two boundaries of a double value.
			// The bigger boundary (m_plus) is normalized. The lower boundary has the same exponent as m_plus.
			// Precondition: the f2s is a raw (not-normalized) copy of a double  
			void normalizedBoundaries(f2s& m_minus, f2s& m_plus) const {
				m_plus.set(false, e_ - 1, (f_ << 1) + 1, q_);
				m_plus.normalize();
				if (lowerBoundaryIsCloser()) {
					m_minus.set(false, e_ - 2, (f_ << 2) - 1, q_);
				}
				else {
					m_minus.set(false, e_ - 1, (f_ << 1) - 1, q_);
				}
				UnsignedInt frac = m_minus.f() << (m_minus.e() - m_plus.e());
				int exp = m_plus.e();
				m_minus.set(false, exp, frac, m_plus.q());
			}
			/////////////////////////////////////////////////////
			// modifiers

			// set raw components, do not implicitely normalize the f2s
			void set(bool sign, int exponent, uint64_t fraction, unsigned precision) noexcept {
				assert(precision < sizeOfSignificant);
				s_ = sign;
				e_ = exponent;
				f_ = static_cast<UnsignedInt>(fraction);
				q_ = precision;
			}

			bool lowerBoundaryIsCloser() const {
				// The boundary is closer if the significand is of the form f == 2^p-1 then
				// the lower boundary is closer.
				// Think of v = 1000e10 and v- = 9999e9.
				// Then the boundary (== (v - v-)/2) is not just at a distance of 1e9 but
				// at a distance of 1e8.
				// The only exception is for the smallest normal: the largest denormal is
				// at the same distance as its successor.
				// Note: denormals have the same exponent as the smallest normals.
				//bool physical_significand_is_zero = ((AsUint64() & kSignificandMask) == 0);
				//return physical_significand_is_zero && (Exponent() != kDenormalExponent);
				return false;  // TODO
			}

			void normalize() {
				assert(f_ != 0);
				uint64_t significand = f_;
				int32_t exponent = e_;

				// This method is mainly called for normalizing boundaries. In general,
				// boundaries need to be shifted by 10 bits, and we optimize for this case.
				constexpr uint64_t NOTFRACTION_MASK = 0xFFC0'0000'0000'0000ull;
				while ((significand & NOTFRACTION_MASK) == 0) {
					significand <<= 10;
					exponent -= 10;
				}
				constexpr uint64_t MSB_MASK = 0x8000'0000'0000'0000ull;
				while ((significand & MSB_MASK) == 0) {
					significand <<= 1;
					exponent--;
				}
				f_ = significand;
				e_ = exponent;
				q_ = sizeOfSignificant - 1;
			}
			/////////////////////////////////////////////////////
			// selectors

			bool s() const noexcept {
				return s_;
			}
			int e() const noexcept {
				return e_;
			}
			UnsignedInt f() const noexcept {
				return f_;
			}
			unsigned q() const noexcept {
				return q_;
			}

		protected:

			template<typename Real>
			f2s& convert_ieee754(Real v) noexcept {
				bool sign{ false };
				uint64_t biased{ 0 }, f64{ 0 }, bits{ 0 };
				extractFields(v, sign, biased, f64, bits);
				s_ = sign;
				e_ = static_cast<int>(biased) - ieee754_parameter<Real>::bias;
				f_ = static_cast<UnsignedInt>(ieee754_parameter<Real>::hmask | f64); // add the hidden bit
				q_ = ieee754_parameter<Real>::fbits;
				// do not automatically normalize
				// constexpr unsigned storageAdjustment = sizeOfSignificant - 32;
				// constexpr unsigned normingShift = (sizeof(Real) == 4) ? (storageAdjustment + 8) : 11;
				// f_ <<= normingShift;
				// e_ -= (sizeOfSignificant - 1);
				e_ -= q_; // fbits or fhbits - 1
				return *this;
			}

			template<typename Real>
			Real to_native_ieee754() const noexcept {
				Real v{ 0.0f };
				if constexpr (sizeof(Real) == 4) {
					union _floatbits {
						uint32_t b;
						float f;
					} bits;
					constexpr unsigned floatOffset = sizeOfSignificant - 32;
					constexpr unsigned denormShift = sizeOfSignificant - floatOffset - 8; // msb of internal f is explicit hidden bit
					bits.b = static_cast<uint64_t>(e_ + ieee754_parameter<Real>::bias) << ieee754_parameter<Real>::fbits;
					bits.b |= (f_ >> denormShift) & ~ieee754_parameter<Real>::hmask; // denormalize and remove the hidden bit
					bits.b |= (s_ ? (1ull << 31) : 0);
					v = bits.f;
				}
				else if constexpr (sizeof(Real) == 8) {
					union _doublebits {
						uint64_t b;
						double d;
					} bits;
					constexpr unsigned denormShift = sizeOfSignificant - 10; // msb of internal f is explicit hidden bit
					bits.b = static_cast<uint64_t>(e_ + ieee754_parameter<Real>::bias) << ieee754_parameter<Real>::fbits;
					bits.b |= (f_ >> denormShift) & ~ieee754_parameter<Real>::hmask; // denormalize and remove the hidden bit
					bits.b |= (s_ ? (1ull << 63) : 0);
					v = bits.d;
				}
				return v;
			}

		private:
			bool        s_;
			int         e_;
			UnsignedInt f_;
			unsigned    q_; // the radix point

			template<typename U>
			friend std::ostream& operator<<(std::ostream&, const f2s<U>&);
		};

		template<typename UnsignedInt>
		std::ostream& operator<<(std::ostream& ostr, const f2s<UnsignedInt>& v) {
			ostr << (v.s() ? "-" : "+") << static_cast<std::uint64_t>(v.f()) << "e" << v.e();
			return ostr;
		}

		// reflect the radix point after the hidden bit, which is explicit in f2s
		template<typename UnsignedInt>
		std::string to_binary(const f2s<UnsignedInt>& v) {
			std::stringstream s;

			constexpr unsigned nbits = sizeof(UnsignedInt) * 8;
			std::uint64_t mask = (1ull << (nbits - 1ull));
			std::uint64_t significant = v.f();
			for (int i = nbits - 1; i >= 0; --i) {
				s << ((mask & significant) ? '1' : '0');
				mask >>= 1ull;
				if (i == static_cast<int>(nbits - 1)) s << '.';
				else if (i > 0 && ((i % 4) == 0)) s << '\'';
			}
			return s.str();
		}

		template<typename UnsignedInt>
		std::string to_triple(const f2s<UnsignedInt>& v) {
			std::stringstream s;
			s << '(' << (v.s() ? "-, " : "+, ");
			s << to_hex(v.f(), 0, true) << ", ";
			s << v.e() << '(' << to_hex(v.e(), true, true) << ')' << ')';
			return s.str();
		}

		template<typename UnsignedInt>
		f2s<UnsignedInt> operator+(const f2s<UnsignedInt>& a, const f2s<UnsignedInt>& b) {
			f2s<UnsignedInt> sum(a);
			sum += b;
			return sum;
		}

		template<typename UnsignedInt>
		f2s<UnsignedInt> operator-(const f2s<UnsignedInt>& a, const f2s<UnsignedInt>& b) {
			f2s<UnsignedInt> difference(a);
			difference -= b;
			return difference;
		}

		template<typename UnsignedInt>
		f2s<UnsignedInt> operator*(const f2s<UnsignedInt>& a, const f2s<UnsignedInt>& b) {
			f2s<UnsignedInt> product(a);
			product *= b;
			return product;
		}

		int calculate_k(int alpha, int e, unsigned q) {
			constexpr double oneoverlog2of10 = 0.30102999566398114;
			return static_cast<int>(std::ceil((alpha - e + (q - 1)) * oneoverlog2of10));
		}

		int decimalScale(int binaryScale, int q, int alpha = 0 /*, int gamma = 3*/) noexcept {
			constexpr double oneoverlog2of10 = 0.30102999566398114;
			return static_cast<int>(std::ceil((alpha - binaryScale + (q - 1)) * oneoverlog2of10));
		}

		template<typename UnsignedInt = std::uint64_t>
		std::string grisu(double v) noexcept {
			f2s<UnsignedInt> w; w = v; // construction must normalize denormals
			int q = sizeof(UnsignedInt) * 8;
			int alpha{ 0 };
			//int gamma{ 3 };
			int mk = decimalScale(w.exponent() + q, alpha);
			CachedPower c_mk = CachedPowers[mk];
			f2s<UnsignedInt> p10;
			p10.set(false, c_mk.binary_exponent, c_mk.significand);

			f2s<UnsignedInt> D = w * p10;
			std::stringstream s;
			s << D;
			return s.str();
		}
	}
} 
