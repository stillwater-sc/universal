#pragma once
// gfp.hpp: simplified floating-point to support generating fast decimal representations of floating-points
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <sstream>
#include <cassert>
#include <algorithm>

#include <universal/native/ieee754.hpp>

namespace sw {
	namespace universal {

		struct CachedPower {
			uint64_t significand;
			int binary_exponent;
			int decimal_exponent;
		};

		constexpr int kCachedPowersCount{87};
		constexpr int kCachedPowersMaxIdx{kCachedPowersCount - 1};

		static constexpr CachedPower CachedPowers[kCachedPowersCount] = {
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
				bool sign{ false };
				uint64_t biased{ 0 }, f64{ 0 }, bits{ 0 };
				extractFields(v, sign, biased, f64, bits);
				s = sign;
				e = static_cast<int>(biased) - ieee754_parameter<Real>::bias;
				q = ieee754_parameter<Real>::fbits;
				f = static_cast<UnsignedInt>(ieee754_parameter<Real>::hmask | f64); // add the hidden bit
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
				f = static_cast<UnsignedInt>(ac + (ad >> rightShift) + (bc >> rightShift) + (tmp >> rightShift));
				e = e + rhs.e + static_cast<int>(sizeOfUint);
				return *this;
			}

			explicit operator float() const noexcept { return to_native_ieee754<float>(); }
			explicit operator double() const noexcept { return to_native_ieee754<double>(); }

			/////////////////////////////////////////////////////
			// member functions
			
			int calculate_k(int alpha) {
				constexpr double oneoverlog2of10 = 0.30102999566398114;
				return static_cast<int>(std::ceil((alpha - e + (sizeOfUint - 1)) * oneoverlog2of10));
			}

			/////////////////////////////////////////////////////
			// modifiers

			// set raw components
			void set(bool sign, int exponent, uint64_t fraction, unsigned precision) noexcept {
				s = sign;
				e = exponent;
				f = static_cast<UnsignedInt>(fraction); // TODO: or is it better to push the UnsignedInt type in the argument?
				q = precision; // position of the radix point: half == 10, float == 23, double = 52
			}

			/////////////////////////////////////////////////////
			// selectors

			int exponent() const noexcept {
				return e;
			}

			UnsignedInt significant() const noexcept {
				return f;
			}

			unsigned radix() const noexcept {
				return q;
			}

		protected:

			template<typename Real>
			Real to_native_ieee754() const noexcept {
				Real v{ 0.0f };
				if constexpr (sizeof(Real) == 4) {
					union {
						uint32_t b;
						float f;
					} bits;
					bits.b = static_cast<uint64_t>(e + ieee754_parameter<Real>::bias) << ieee754_parameter<Real>::fbits;
					bits.b |= f & ~ieee754_parameter<Real>::hmask;
					bits.b |= (s ? (1ull << 31) : 0);
					v = bits.f;
				}
				else if constexpr (sizeof(Real) == 8) {
					union {
						uint64_t b;
						double d;
					} bits;
					bits.b = static_cast<uint64_t>(e + ieee754_parameter<Real>::bias) << ieee754_parameter<Real>::fbits;
					bits.b |= f & ~ieee754_parameter<Real>::hmask;
					bits.b |= (s ? (1ull << 63) : 0);
					v = bits.d;
				}
				return v;
			}

		private:
			bool        s;
			int         e;
			UnsignedInt f;
			unsigned    q;

			template<typename U>
			friend std::ostream& operator<<(std::ostream&, const gfp<U>&);
		};

		template<typename UnsignedInt>
		std::ostream& operator<<(std::ostream& ostr, const gfp<UnsignedInt>& v) {
			ostr << (v.s ? "-" : "+") << static_cast<std::uint64_t>(v.f) << "e" << v.e;
			return ostr;
		}

		// reflect the radix point after the hidden bit, which is explicity in gfp
		template<typename UnsignedInt>
		std::string to_binary(const gfp<UnsignedInt>& v) {
			std::stringstream s;

			constexpr unsigned nbits = sizeof(UnsignedInt) * 8;
			unsigned q = v.radix();
			std::uint64_t mask = (1ull << (nbits - 1ull));
			std::uint64_t significant = v.significant();
			for (int i = nbits - 1; i >= 0; --i) {
				s << ((mask & significant) ? '1' : '0');
				mask >>= 1ull;
				if (i == static_cast<int>(q)) s << '.';
				else if (i > 0 && ((i % 4) == 0)) s << '\'';
			}
			return s.str();
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

		int decimalScale(int binaryScale, int q, int alpha = 0 /*, int gamma = 3*/) noexcept {
			constexpr double oneoverlog2of10 = 0.30102999566398114;
			return static_cast<int>(std::ceil((alpha - binaryScale + (q - 1)) * oneoverlog2of10));
		}

		template<typename UnsignedInt = std::uint64_t>
		std::string grisu(double v) noexcept {
			gfp<UnsignedInt> w; w = v; // construction must normalize denormals
			int q = sizeof(UnsignedInt) * 8;
			int alpha{ 0 }; //, gamma{ 3 };

			int requested_dec_exp = decimalScale(/*binaryScale*/ w.exponent() + q,
                                    /*q*/ q,
                                    /*alpha*/ alpha);

			// Map decimal exponent to table index.
			constexpr int kCachedPowersOffset = 348;
			constexpr int kDecimalExponentDistance = 8;

			int idx = (requested_dec_exp + kCachedPowersOffset) / kDecimalExponentDistance;
			if (idx < 0 || idx >= kCachedPowersCount) {
			    std::cerr << "idx=" << idx << " requested_dec_exp=" << requested_dec_exp << "\n";
			    return "<bad cached power index>";
			}
			idx = std::clamp(idx, 0, kCachedPowersMaxIdx);
			// **FIXME**: ChatGPT says this is **wrong**, it's only adjusted to not crash
			CachedPower c_mk = CachedPowers[idx];

			gfp<UnsignedInt> p10;
			p10.set(true, c_mk.binary_exponent, c_mk.significand, 64);

			gfp<UnsignedInt> D = w * p10;
			std::stringstream s;
			s << D;
			return s.str();
		}
	}
} 
