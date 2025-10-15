#pragma once
// grisu.hpp: Grisu3 algorithm for floating-point to decimal string conversion
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The Grisu3 algorithm (Loitsch, 2010: "Printing Floating-Point Numbers Quickly and Accurately with Integers")
// provides fast conversion of binary floating-point numbers to decimal strings using cached powers of 10.
//
// Grisu3 is typically faster than Dragon4 and produces the shortest decimal representation that
// rounds back to the original value. It falls back to Dragon4 in rare cases (~0.5% of inputs).
//
// Algorithm overview:
// 1. Normalize input to (f, e) where value = f × 2^e, f is odd
// 2. Find cached power c_k ≈ 10^(-k) stored as (c, q) where c × 2^q ≈ 10^(-k)
// 3. Multiply (f, e) × (c, q) to get scaled value
// 4. Generate digits from scaled value
// 5. Check boundaries to ensure shortest representation

#include <cstdint>
#include <cmath>
#include <cstring>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <universal/number/support/decimal.hpp>
#include <universal/number/support/dragon.hpp>

namespace sw { namespace universal {

// Forward declarations
namespace internal {
	template<unsigned fbits> class value;
}

namespace grisu {

// DiyFp represents a floating-point number as (significand, exponent)
// Value = significand × 2^exponent
// Significand is a 64-bit unsigned integer
struct DiyFp {
	uint64_t f;  // significand
	int e;       // binary exponent

	DiyFp() : f(0), e(0) {}
	DiyFp(uint64_t f_, int e_) : f(f_), e(e_) {}

	// Subtract exponents (assumes f values are already aligned)
	DiyFp operator-(const DiyFp& rhs) const {
		return DiyFp(f - rhs.f, e);
	}

	// Multiply two DiyFp values
	// Result: (this->f × rhs.f) × 2^(this->e + rhs.e)
	DiyFp operator*(const DiyFp& rhs) const {
		// Compute 64-bit × 64-bit = 128-bit product
		// We need the upper 64 bits plus rounding information

		uint64_t a = f >> 32;
		uint64_t b = f & 0xFFFFFFFF;
		uint64_t c = rhs.f >> 32;
		uint64_t d = rhs.f & 0xFFFFFFFF;

		uint64_t ac = a * c;
		uint64_t bc = b * c;
		uint64_t ad = a * d;
		uint64_t bd = b * d;

		uint64_t tmp = (bd >> 32) + (ad & 0xFFFFFFFF) + (bc & 0xFFFFFFFF);
		tmp += 1U << 31;  // Round to nearest

		uint64_t result_f = ac + (ad >> 32) + (bc >> 32) + (tmp >> 32);
		int result_e = e + rhs.e + 64;

		return DiyFp(result_f, result_e);
	}

	// Normalize: ensure the most significant bit is set
	void normalize() {
		while ((f & 0xF000000000000000ULL) == 0) {
			f <<= 4;
			e -= 4;
		}
		while ((f & 0x8000000000000000ULL) == 0) {
			f <<= 1;
			e -= 1;
		}
	}
};

// Cached powers of 10: 10^k ≈ c × 2^q
// These are precomputed normalized DiyFp values
struct CachedPower {
	uint64_t significand;
	int binary_exponent;
	int decimal_exponent;
};

// Cached powers table (subset for testing - full table has ~80 entries)
// Each entry represents 10^k ≈ significand × 2^binary_exponent
static const CachedPower kCachedPowers[] = {
	{ 0xfa8fd5a0081c0288ULL, -1220, -348 },
	{ 0xbaaee17fa23ebf76ULL, -1193, -340 },
	{ 0x8b16fb203055ac76ULL, -1166, -332 },
	{ 0xcf42894a5dce35eaULL, -1140, -324 },
	{ 0x9a6bb0aa55653b2dULL, -1113, -316 },
	{ 0xe61acf033d1a45dfULL, -1087, -308 },
	{ 0xab70fe17c79ac6caULL, -1060, -300 },
	{ 0xff77b1fcbebcdc4fULL, -1034, -292 },
	{ 0xbe5691ef416bd60cULL, -1007, -284 },
	{ 0x8dd01fad907ffc3cULL, -980, -276 },
	{ 0xd3515c2831559a83ULL, -954, -268 },
	{ 0x9d71ac8fada6c9b5ULL, -927, -260 },
	{ 0xea9c227723ee8bcbULL, -901, -252 },
	{ 0xaecc49914078536dULL, -874, -244 },
	{ 0x823c12795db6ce57ULL, -847, -236 },
	{ 0xc21094364dfb5637ULL, -821, -228 },
	{ 0x9096ea6f3848984fULL, -794, -220 },
	{ 0xd77485cb25823ac7ULL, -768, -212 },
	{ 0xa086cfcd97bf97f4ULL, -741, -204 },
	{ 0xef340a98172aace5ULL, -715, -196 },
	{ 0xb23867fb2a35b28eULL, -688, -188 },
	{ 0x84c8d4dfd2c63f3bULL, -661, -180 },
	{ 0xc5dd44271ad3cdbaULL, -635, -172 },
	{ 0x936b9fcebb25c996ULL, -608, -164 },
	{ 0xdbac6c247d62a584ULL, -582, -156 },
	{ 0xa3ab66580d5fdaf6ULL, -555, -148 },
	{ 0xf3e2f893dec3f126ULL, -529, -140 },
	{ 0xb5b5ada8aaff80b8ULL, -502, -132 },
	{ 0x87625f056c7c4a8bULL, -475, -124 },
	{ 0xc9bcff6034c13053ULL, -449, -116 },
	{ 0x964e858c91ba2655ULL, -422, -108 },
	{ 0xdff9772470297ebdULL, -396, -100 },
	{ 0xa6dfbd9fb8e5b88fULL, -369, -92 },
	{ 0xf8a95fcf88747d94ULL, -343, -84 },
	{ 0xb94470938fa89bcfULL, -316, -76 },
	{ 0x8a08f0f8bf0f156bULL, -289, -68 },
	{ 0xcdb02555653131b6ULL, -263, -60 },
	{ 0x993fe2c6d07b7facULL, -236, -52 },
	{ 0xe45c10c42a2b3b06ULL, -210, -44 },
	{ 0xaa242499697392d3ULL, -183, -36 },
	{ 0xfd87b5f28300ca0eULL, -157, -28 },
	{ 0xbce5086492111aebULL, -130, -20 },
	{ 0x8cbccc096f5088ccULL, -103, -12 },
	{ 0xd1b71758e219652cULL, -77, -4 },
	{ 0x9c40000000000000ULL, -50, 4 },
	{ 0xe8d4a51000000000ULL, -24, 12 },
	{ 0xad78ebc5ac620000ULL, 3, 20 },
	{ 0x813f3978f8940984ULL, 30, 28 },
	{ 0xc097ce7bc90715b3ULL, 56, 36 },
	{ 0x8f7e32ce7bea5c70ULL, 83, 44 },
	{ 0xd5d238a4abe98068ULL, 109, 52 },
	{ 0x9f4f2726179a2245ULL, 136, 60 },
	{ 0xed63a231d4c4fb27ULL, 162, 68 },
	{ 0xb0de65388cc8ada8ULL, 189, 76 },
	{ 0x83c7088e1aab65dbULL, 216, 84 },
	{ 0xc45d1df942711d9aULL, 242, 92 },
	{ 0x924d692ca61be758ULL, 269, 100 },
	{ 0xda01ee641a708deaULL, 295, 108 },
	{ 0xa26da3999aef774aULL, 322, 116 },
	{ 0xf209787bb47d6b85ULL, 348, 124 },
	{ 0xb454e4a179dd1877ULL, 375, 132 },
	{ 0x865b86925b9bc5c2ULL, 402, 140 },
	{ 0xc83553c5c8965d3dULL, 428, 148 },
	{ 0x952ab45cfa97a0b3ULL, 455, 156 },
	{ 0xde469fbd99a05fe3ULL, 481, 164 },
	{ 0xa59bc234db398c25ULL, 508, 172 },
	{ 0xf6c69a72a3989f5cULL, 534, 180 },
	{ 0xb7dcbf5354e9beceULL, 561, 188 },
	{ 0x88fcf317f22241e2ULL, 588, 196 },
	{ 0xcc20ce9bd35c78a5ULL, 614, 204 },
	{ 0x98165af37b2153dfULL, 641, 212 },
	{ 0xe2a0b5dc971f303aULL, 667, 220 },
	{ 0xa8d9d1535ce3b396ULL, 694, 228 },
	{ 0xfb9b7cd9a4a7443cULL, 720, 236 },
	{ 0xbb764c4ca7a44410ULL, 747, 244 },
	{ 0x8bab8eefb6409c1aULL, 774, 252 },
	{ 0xd01fef10a657842cULL, 800, 260 },
	{ 0x9b10a4e5e9913129ULL, 827, 268 },
	{ 0xe7109bfba19c0c9dULL, 853, 276 },
	{ 0xac2820d9623bf429ULL, 880, 284 },
	{ 0x80444b5e7aa7cf85ULL, 907, 292 },
	{ 0xbf21e44003acdd2dULL, 933, 300 },
	{ 0x8e679c2f5e44ff8fULL, 960, 308 },
	{ 0xd433179d9c8cb841ULL, 986, 316 },
	{ 0x9e19db92b4e31ba9ULL, 1013, 324 },
	{ 0xeb96bf6ebadf77d9ULL, 1039, 332 },
	{ 0xaf87023b9bf0ee6bULL, 1066, 340 }
};

static const int kCachedPowersSize = sizeof(kCachedPowers) / sizeof(kCachedPowers[0]);

// Get cached power for a given exponent
inline CachedPower GetCachedPower(int e, int* K) {
	// Find k such that 10^k ≈ 2^e
	double dk = (-61 - e) * 0.30102999566398114 + 347;  // dk = ceil((-61 - e) * log10(2)) + 347
	int k = static_cast<int>(dk);
	if (dk - k > 0.0) k++;

	int index = (k >> 3) + 1;
	*K = -(-348 + (index << 3));  // decimal exponent

	return kCachedPowers[index];
}

// Generate digits using Grisu3
inline bool DigitGen(const DiyFp& W, const DiyFp& Mp, uint64_t delta, char* buffer, int* len, int* K) {
	const DiyFp one(uint64_t{1} << -Mp.e, Mp.e);
	//const DiyFp wp_w = Mp - W;

	uint32_t p1 = static_cast<uint32_t>(Mp.f >> -one.e);
	uint64_t p2 = Mp.f & (one.f - 1);

	// Generate digits for the integer part
	int kappa = 10;  // Maximum digits we might generate
	char* p = buffer;

	// Divide p1 by powers of 10
	while (kappa > 0) {
		uint32_t d = 0;
		switch (kappa) {
			case 10: if (p1 >= 1000000000) { d = p1 / 1000000000; p1 %= 1000000000; } break;
			case  9: if (p1 >=  100000000) { d = p1 /  100000000; p1 %=  100000000; } break;
			case  8: if (p1 >=   10000000) { d = p1 /   10000000; p1 %=   10000000; } break;
			case  7: if (p1 >=    1000000) { d = p1 /    1000000; p1 %=    1000000; } break;
			case  6: if (p1 >=     100000) { d = p1 /     100000; p1 %=     100000; } break;
			case  5: if (p1 >=      10000) { d = p1 /      10000; p1 %=      10000; } break;
			case  4: if (p1 >=       1000) { d = p1 /       1000; p1 %=       1000; } break;
			case  3: if (p1 >=        100) { d = p1 /        100; p1 %=        100; } break;
			case  2: if (p1 >=         10) { d = p1 /         10; p1 %=         10; } break;
			case  1:                       { d = p1;              p1 =           0; } break;
			default: break;
		}

		if (d > 0 || p != buffer) {
			*p++ = '0' + d;
		}

		kappa--;
		uint64_t tmp = (static_cast<uint64_t>(p1) << -one.e) + p2;
		if (tmp <= delta) {
			*K += kappa;
			*len = static_cast<int>(p - buffer);

			// Round
			uint64_t rest = (static_cast<uint64_t>(p1) << -one.e) + p2;
			if (rest > delta) {
				return false;  // Failed
			}
			if (2 * rest > delta && (rest > delta - rest || (rest == delta - rest && (d & 1)))) {
				// Round up
				while (*len > 0 && buffer[*len - 1] == '9') {
					(*len)--;
				}
				if (*len == 0) {
					buffer[0] = '1';
					*len = 1;
					(*K)++;
				} else {
					buffer[*len - 1]++;
				}
			}
			return true;
		}
	}

	// Generate fractional digits
	for (;;) {
		p2 *= 10;
		delta *= 10;
		char d = static_cast<char>(p2 >> -one.e);
		if (d > 0 || p != buffer) {
			*p++ = '0' + d;
		}
		p2 &= one.f - 1;
		kappa--;
		if (p2 < delta) {
			*K += kappa;
			*len = static_cast<int>(p - buffer);

			// Round
			if (2 * p2 > delta && (p2 > delta - p2 || ((delta - p2) == p2 && (d & 1)))) {
				// Round up
				while (*len > 0 && buffer[*len - 1] == '9') {
					(*len)--;
				}
				if (*len == 0) {
					buffer[0] = '1';
					*len = 1;
					(*K)++;
				} else {
					buffer[*len - 1]++;
				}
			}
			return true;
		}
	}
}

// Main Grisu3 algorithm
inline bool Grisu3(uint64_t significand, int exponent, char* buffer, int* length, int* K) {
	// Normalize input
	DiyFp v(significand, exponent);
	v.normalize();

	// Boundaries: v- and v+
	DiyFp w_minus(v.f - 1, v.e);
	DiyFp w_plus(v.f + 1, v.e);

	w_minus.normalize();
	w_plus.normalize();

	// Get cached power
	const CachedPower cached = GetCachedPower(v.e, K);
	const DiyFp c(cached.significand, cached.binary_exponent);

	// Scale
	const DiyFp W = v * c;
	DiyFp Wp = w_plus * c;
	DiyFp Wm = w_minus * c;

	Wm.f++;
	Wp.f--;

	// Generate digits
	return DigitGen(W, Wp, Wp.f - Wm.f, buffer, length, K);
}

/// <summary>
/// grisu_context holds the configuration for formatting
/// </summary>
struct grisu_context {
	std::ios_base::fmtflags flags;
	std::streamsize precision;
	bool use_scientific;
	bool use_fixed;
	bool show_pos;
	bool uppercase;

	grisu_context(std::ios_base::fmtflags f = std::ios_base::dec, std::streamsize prec = 6)
		: flags(f), precision(prec)
	{
		use_scientific = (flags & std::ios_base::scientific) != 0;
		use_fixed = (flags & std::ios_base::fixed) != 0;
		show_pos = (flags & std::ios_base::showpos) != 0;
		uppercase = (flags & std::ios_base::uppercase) != 0;

		if (use_scientific && use_fixed) use_fixed = false;
		if (precision <= 0) precision = 6;
	}
};

// Format the output string
inline std::string FormatGrisu3(bool sign, const char* digits, int len, int K, const grisu_context& ctx) {
	std::stringstream ss;

	if (sign) {
		ss << '-';
	} else if (ctx.show_pos) {
		ss << '+';
	}

	int exp = K + len - 1;

	if (ctx.use_fixed) {
		// Fixed-point notation
		int integer_digits = exp + 1;

		if (integer_digits > 0) {
			// Number >= 1
			if (integer_digits <= len) {
				for (int i = 0; i < integer_digits; i++) {
					ss << digits[i];
				}
				if (ctx.precision > 0) {
					ss << '.';
					for (int i = 0; i < ctx.precision; i++) {
						if (integer_digits + i < len) {
							ss << digits[integer_digits + i];
						} else {
							ss << '0';
						}
					}
				}
			} else {
				for (int i = 0; i < len; i++) {
					ss << digits[i];
				}
				for (int i = 0; i < integer_digits - len; i++) {
					ss << '0';
				}
				if (ctx.precision > 0) {
					ss << '.';
					for (int i = 0; i < ctx.precision; i++) {
						ss << '0';
					}
				}
			}
		} else {
			// Number < 1
			ss << "0.";
			for (int i = 0; i < -integer_digits; i++) {
				ss << '0';
			}
			int frac_digits = len;
			if (-integer_digits + frac_digits > ctx.precision) {
				frac_digits = ctx.precision + integer_digits;
			}
			for (int i = 0; i < frac_digits; i++) {
				ss << digits[i];
			}
			for (int i = frac_digits; i < ctx.precision + integer_digits; i++) {
				ss << '0';
			}
		}
	} else {
		// Scientific notation
		ss << digits[0];
		if (ctx.precision > 0) {
			ss << '.';
			for (int i = 0; i < ctx.precision; i++) {
				if (i + 1 < len) {
					ss << digits[i + 1];
				} else {
					ss << '0';
				}
			}
		}

		ss << (ctx.uppercase ? 'E' : 'e');
		ss << (exp >= 0 ? '+' : '-');
		int abs_exp = (exp >= 0) ? exp : -exp;
		if (abs_exp < 10) ss << '0';
		ss << abs_exp;
	}

	return ss.str();
}

// ==================== MathGeoLib Grisu3 Implementation ====================
// Complete, working implementation from MathGeoLib by Jukka Jylänki
// Based on "Printing Floating-Point Numbers Quickly and Accurately with Integers"
// by Florian Loitsch (2010)

// Constants for IEEE-754 double precision
constexpr uint64_t D64_SIGN         = 0x8000000000000000ULL;
constexpr uint64_t D64_EXP_MASK     = 0x7FF0000000000000ULL;
constexpr uint64_t D64_FRACT_MASK   = 0x000FFFFFFFFFFFFFULL;
constexpr uint64_t D64_IMPLICIT_ONE = 0x0010000000000000ULL;
constexpr int D64_EXP_POS      = 52;
constexpr int D64_EXP_BIAS     = 1075;
constexpr int DIYFP_FRACT_SIZE = 64;
constexpr double D_1_LOG2_10   = 0.30102999566398114; // 1 / log10(2)
constexpr int MIN_TARGET_EXP   = -60;
constexpr uint64_t MASK32      = 0xFFFFFFFFULL;
constexpr int MIN_CACHED_EXP = -348;
constexpr int CACHED_EXP_STEP = 8;

// pow10_cache[i] = 10^(i-1)
static const uint32_t pow10_cache[] = {
	0, 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000
};

// Cached power lookup for Grisu3
inline int cached_pow_grisu(int exp, DiyFp *p) {
	int k = static_cast<int>(std::ceil((exp + DIYFP_FRACT_SIZE - 1) * D_1_LOG2_10));
	int i = (k - MIN_CACHED_EXP - 1) / CACHED_EXP_STEP + 1;
	p->f = kCachedPowers[i].significand;
	p->e = kCachedPowers[i].binary_exponent;
	return kCachedPowers[i].decimal_exponent;
}

// Subtract two DiyFp (assumes same exponent)
inline DiyFp minus_diyf(DiyFp x, DiyFp y) {
	DiyFp d;
	d.f = x.f - y.f;
	d.e = x.e;
	return d;
}

// Multiply two DiyFp
inline DiyFp multiply_diyf(DiyFp x, DiyFp y) {
	uint64_t a = x.f >> 32;
	uint64_t b = x.f & MASK32;
	uint64_t c = y.f >> 32;
	uint64_t d = y.f & MASK32;
	uint64_t ac = a * c;
	uint64_t bc = b * c;
	uint64_t ad = a * d;
	uint64_t bd = b * d;
	uint64_t tmp = (bd >> 32) + (ad & MASK32) + (bc & MASK32);
	tmp += 1U << 31; // round
	DiyFp r;
	r.f = ac + (ad >> 32) + (bc >> 32) + (tmp >> 32);
	r.e = x.e + y.e + 64;
	return r;
}

// Normalize DiyFp
inline DiyFp normalize_diy_fp(DiyFp n) {
	while (!(n.f & 0xFFC0000000000000ULL)) { n.f <<= 10; n.e -= 10; }
	while (!(n.f & D64_SIGN)) { n.f <<= 1; --n.e; }
	return n;
}

// Convert IEEE-754 double to DiyFp
inline DiyFp double2diy_fp(double d) {
	uint64_t u64;
	std::memcpy(&u64, &d, sizeof(double));

	DiyFp fp;
	if (!(u64 & D64_EXP_MASK)) {
		// Subnormal
		fp.f = u64 & D64_FRACT_MASK;
		fp.e = 1 - D64_EXP_BIAS;
	} else {
		// Normal
		fp.f = (u64 & D64_FRACT_MASK) + D64_IMPLICIT_ONE;
		fp.e = static_cast<int>((u64 & D64_EXP_MASK) >> D64_EXP_POS) - D64_EXP_BIAS;
	}
	return fp;
}

// Find largest power of 10 that fits in n
inline int largest_pow10(uint32_t n, int n_bits, uint32_t *power) {
	int guess = ((n_bits + 1) * 1233 >> 12) + 1;  // +1 to skip first entry
	if (guess >= 10) guess = 9;
	if (n < pow10_cache[guess]) --guess;
	*power = pow10_cache[guess];
	return guess;
}

// Round and weed out digits
inline bool round_weed(char *buffer, int len, uint64_t wp_W, uint64_t delta,
                       uint64_t rest, uint64_t ten_kappa, uint64_t ulp) {
	uint64_t wp_Wup = wp_W - ulp;
	uint64_t wp_Wdown = wp_W + ulp;

	while (rest < wp_Wup && delta - rest >= ten_kappa &&
	       (rest + ten_kappa < wp_Wup || wp_Wup - rest >= rest + ten_kappa - wp_Wup)) {
		--buffer[len - 1];
		rest += ten_kappa;
	}

	if (rest < wp_Wdown && delta - rest >= ten_kappa &&
	    (rest + ten_kappa < wp_Wdown || wp_Wdown - rest > rest + ten_kappa - wp_Wdown))
		return false;

	return 2 * ulp <= rest && rest <= delta - 4 * ulp;
}

// Generate digits using MathGeoLib's digit_gen
inline bool digit_gen_grisu(DiyFp low, DiyFp w, DiyFp high, char *buffer, int *length, int *kappa) {
	uint64_t unit = 1;
	DiyFp too_low = { low.f - unit, low.e };
	DiyFp too_high = { high.f + unit, high.e };
	DiyFp unsafe_interval = minus_diyf(too_high, too_low);
	DiyFp one = { 1ULL << -w.e, w.e };
	uint32_t p1 = static_cast<uint32_t>(too_high.f >> -one.e);
	uint64_t p2 = too_high.f & (one.f - 1);
	uint32_t div;

	*kappa = largest_pow10(p1, DIYFP_FRACT_SIZE + one.e, &div);
	*length = 0;

	// Generate integer part digits
	while (*kappa > 0) {
		int digit = p1 / div;
		buffer[*length] = static_cast<char>('0' + digit);
		++*length;
		p1 %= div;
		--*kappa;
		uint64_t rest = (static_cast<uint64_t>(p1) << -one.e) + p2;
		if (rest < unsafe_interval.f) {
			return round_weed(buffer, *length, minus_diyf(too_high, w).f,
			                  unsafe_interval.f, rest,
			                  static_cast<uint64_t>(div) << -one.e, unit);
		}
		div /= 10;
	}

	// Generate fractional part digits
	for (;;) {
		p2 *= 10;
		unit *= 10;
		unsafe_interval.f *= 10;
		int digit = static_cast<int>(p2 >> -one.e);
		buffer[*length] = static_cast<char>('0' + digit);
		++*length;
		p2 &= one.f - 1;
		--*kappa;
		if (p2 < unsafe_interval.f) {
			return round_weed(buffer, *length, minus_diyf(too_high, w).f * unit,
			                  unsafe_interval.f, p2, one.f, unit);
		}
	}
}

// Main MathGeoLib Grisu3 algorithm
inline bool grisu3_mathgeolib(double v, char *buffer, int *length, int *d_exp) {
	DiyFp dfp = double2diy_fp(v);
	DiyFp w = normalize_diy_fp(dfp);

	// Normalize boundaries
	DiyFp t = { (dfp.f << 1) + 1, dfp.e - 1 };
	DiyFp b_plus = normalize_diy_fp(t);
	DiyFp b_minus;

	uint64_t u64;
	std::memcpy(&u64, &v, sizeof(double));

	// Lower boundary is closer for powers of 2
	if (!(u64 & D64_FRACT_MASK) && (u64 & D64_EXP_MASK) != 0) {
		b_minus.f = (dfp.f << 2) - 1;
		b_minus.e = dfp.e - 2;
	} else {
		b_minus.f = (dfp.f << 1) - 1;
		b_minus.e = dfp.e - 1;
	}
	b_minus.f = b_minus.f << (b_minus.e - b_plus.e);
	b_minus.e = b_plus.e;

	// Get cached power of 10
	DiyFp c_mk;
	int mk = cached_pow_grisu(MIN_TARGET_EXP - DIYFP_FRACT_SIZE - w.e, &c_mk);

	// Scale
	w = multiply_diyf(w, c_mk);
	b_minus = multiply_diyf(b_minus, c_mk);
	b_plus = multiply_diyf(b_plus, c_mk);

	// Generate digits
	int kappa;
	bool success = digit_gen_grisu(b_minus, w, b_plus, buffer, length, &kappa);
	*d_exp = kappa - mk;
	return success;
}

// Format Grisu3 output (MathGeoLib style)
inline std::string format_grisu3_output(bool negative, const char* buffer, int length, int d_exp) {
	std::string result;
	if (negative) result = "-";

	// Copy buffer to working array
	char s2[64];
	std::memcpy(s2, buffer, length);

	int decimals = std::min(-d_exp, std::max(1, length - 1));
	int len = length;

	if (d_exp < 0 && len > 1) {
		// Add decimal point
		for (int i = 0; i < decimals; ++i) {
			s2[len - i] = s2[len - i - 1];
		}
		s2[len++ - decimals] = '.';
		d_exp += decimals;
		if (d_exp != 0) {
			s2[len++] = 'e';
			int exp_len = std::snprintf(s2 + len, sizeof(s2) - len, "%d", d_exp);
			len += exp_len;
		}
	} else if (d_exp < 0 && d_exp >= -3) {
		// Add decimal point for numbers like 0.001
		for (int i = 0; i < len; ++i) {
			s2[len - d_exp - 1 - i] = s2[len - i - 1];
		}
		s2[0] = '.';
		for (int i = 1; i < -d_exp; ++i) {
			s2[i] = '0';
		}
		len += -d_exp;
	} else if (d_exp < 0 || d_exp > 2) {
		// Scientific notation
		s2[len++] = 'e';
		int exp_len = std::snprintf(s2 + len, sizeof(s2) - len, "%d", d_exp);
		len += exp_len;
	} else if (d_exp > 0) {
		// Add trailing zeros
		while (d_exp-- > 0) s2[len++] = '0';
	}

	s2[len] = '\0';
	result += s2;
	return result;
}

// ==================== End MathGeoLib Implementation ====================

// Helper: Convert arbitrary-precision decimal mantissa to normalized 64-bit significand
inline uint64_t decimal_to_uint64_normalized(const support::decimal& mantissa, int& shift_out) {
	// Convert decimal digits to uint64_t, taking most significant bits
	std::stringstream ss;
	ss << mantissa;
	std::string mant_str = ss.str();

	// Parse decimal digits into uint64_t (up to 19 digits fit)
	uint64_t sig = 0;
	int digit_count = 0;
	for (size_t i = 0; i < mant_str.length() && digit_count < 19; i++) {
		if (mant_str[i] >= '0' && mant_str[i] <= '9') {
			sig = sig * 10 + (mant_str[i] - '0');
			digit_count++;
		}
	}

	// Convert to binary and normalize
	// First, find the MSB position
	int msb_pos = -1;
	for (int i = 63; i >= 0; --i) {
		if (sig & (1ULL << i)) {
			msb_pos = i;
			break;
		}
	}

	if (msb_pos < 0) {
		// Zero significand
		shift_out = 0;
		return 0;
	}

	// Normalize so MSB is at bit 63
	int shift = 63 - msb_pos;
	sig <<= shift;
	shift_out = shift;

	return sig;
}

// Convert from Universal's internal representation to Grisu3 or Dragon4
inline std::string to_decimal_string(bool sign, int scale, const support::decimal& mantissa, int fbits,
                                      std::ios_base::fmtflags flags = std::ios_base::dec,
                                      std::streamsize precision = 6) {
	// NOTE: Grisu3 is optimized for IEEE-754 standard sizes (23 or 52 fraction bits).
	// For arbitrary-precision values, we fall back to Dragon4.

	// Check if we can use Grisu3 (only for IEEE-754 float and double)
	if (fbits == 23 || fbits == 52) {
		// TODO: For now, still fall back to Dragon4 until we implement value<> to IEEE-754 conversion
		// The challenge is that value<> stores (sign, scale, decimal_mantissa)
		// but we need (sign, binary_significand, binary_exponent) for Grisu3
		return dragon::to_decimal_string(sign, scale, mantissa, fbits, flags, precision);
	}

	// For non-standard precisions (quad, octo, etc.), use Dragon4
	return dragon::to_decimal_string(sign, scale, mantissa, fbits, flags, precision);
}

} // namespace grisu

}} // namespace sw::universal
