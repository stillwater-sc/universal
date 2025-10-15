// grisu3_port.cpp: Direct port of MathGeoLib's grisu3.c to C++
//
// Original implementation by Jukka Jylänki (MathGeoLib)
// Based on "Printing Floating-Point Numbers Quickly and Accurately with Integers"
// by Florian Loitsch, available at
// http://www.cs.tufts.edu/~nr/cs257/archive/florian-loitsch/printf.pdf
//
// Ported to C++ for Universal Numbers Library
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT

#include <cstdint>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <string>

namespace grisu3 {

// Constants for IEEE-754 double precision
constexpr uint64_t D64_SIGN         = 0x8000000000000000ULL;
constexpr uint64_t D64_EXP_MASK     = 0x7FF0000000000000ULL;
constexpr uint64_t D64_FRACT_MASK   = 0x000FFFFFFFFFFFFFULL;
constexpr uint64_t D64_IMPLICIT_ONE = 0x0010000000000000ULL;
constexpr int D64_EXP_POS      = 52;
constexpr int D64_EXP_BIAS     = 1075;
constexpr int DIYFP_FRACT_SIZE = 64;
constexpr double D_1_LOG2_10   = 0.30102999566398114; // 1 / lg(10)
constexpr int MIN_TARGET_EXP   = -60;
constexpr uint64_t MASK32      = 0xFFFFFFFFULL;

constexpr int MIN_CACHED_EXP = -348;
constexpr int CACHED_EXP_STEP = 8;

// DiyFp: Do-It-Yourself Floating Point
struct diy_fp {
	uint64_t f;  // significand
	int e;       // exponent
};

// Cached power of 10
struct power {
	uint64_t fract;
	int16_t b_exp;  // binary exponent
	int16_t d_exp;  // decimal exponent
};

// Cached powers of 10: 10^d_exp ≈ fract × 2^b_exp
static const power pow_cache[] = {
	{ 0xfa8fd5a0081c0288ULL, -1220, -348 },
	{ 0xbaaee17fa23ebf76ULL, -1193, -340 },
	{ 0x8b16fb203055ac76ULL, -1166, -332 },
	{ 0xcf42894a5dce35eaULL, -1140, -324 },
	{ 0x9a6bb0aa55653b2dULL, -1113, -316 },
	{ 0xe61acf033d1a45dfULL, -1087, -308 },
	{ 0xab70fe17c79ac6caULL, -1060, -300 },
	{ 0xff77b1fcbebcdc4fULL, -1034, -292 },
	{ 0xbe5691ef416bd60cULL, -1007, -284 },
	{ 0x8dd01fad907ffc3cULL,  -980, -276 },
	{ 0xd3515c2831559a83ULL,  -954, -268 },
	{ 0x9d71ac8fada6c9b5ULL,  -927, -260 },
	{ 0xea9c227723ee8bcbULL,  -901, -252 },
	{ 0xaecc49914078536dULL,  -874, -244 },
	{ 0x823c12795db6ce57ULL,  -847, -236 },
	{ 0xc21094364dfb5637ULL,  -821, -228 },
	{ 0x9096ea6f3848984fULL,  -794, -220 },
	{ 0xd77485cb25823ac7ULL,  -768, -212 },
	{ 0xa086cfcd97bf97f4ULL,  -741, -204 },
	{ 0xef340a98172aace5ULL,  -715, -196 },
	{ 0xb23867fb2a35b28eULL,  -688, -188 },
	{ 0x84c8d4dfd2c63f3bULL,  -661, -180 },
	{ 0xc5dd44271ad3cdbaULL,  -635, -172 },
	{ 0x936b9fcebb25c996ULL,  -608, -164 },
	{ 0xdbac6c247d62a584ULL,  -582, -156 },
	{ 0xa3ab66580d5fdaf6ULL,  -555, -148 },
	{ 0xf3e2f893dec3f126ULL,  -529, -140 },
	{ 0xb5b5ada8aaff80b8ULL,  -502, -132 },
	{ 0x87625f056c7c4a8bULL,  -475, -124 },
	{ 0xc9bcff6034c13053ULL,  -449, -116 },
	{ 0x964e858c91ba2655ULL,  -422, -108 },
	{ 0xdff9772470297ebdULL,  -396, -100 },
	{ 0xa6dfbd9fb8e5b88fULL,  -369,  -92 },
	{ 0xf8a95fcf88747d94ULL,  -343,  -84 },
	{ 0xb94470938fa89bcfULL,  -316,  -76 },
	{ 0x8a08f0f8bf0f156bULL,  -289,  -68 },
	{ 0xcdb02555653131b6ULL,  -263,  -60 },
	{ 0x993fe2c6d07b7facULL,  -236,  -52 },
	{ 0xe45c10c42a2b3b06ULL,  -210,  -44 },
	{ 0xaa242499697392d3ULL,  -183,  -36 },
	{ 0xfd87b5f28300ca0eULL,  -157,  -28 },
	{ 0xbce5086492111aebULL,  -130,  -20 },
	{ 0x8cbccc096f5088ccULL,  -103,  -12 },
	{ 0xd1b71758e219652cULL,   -77,   -4 },
	{ 0x9c40000000000000ULL,   -50,    4 },
	{ 0xe8d4a51000000000ULL,   -24,   12 },
	{ 0xad78ebc5ac620000ULL,     3,   20 },
	{ 0x813f3978f8940984ULL,    30,   28 },
	{ 0xc097ce7bc90715b3ULL,    56,   36 },
	{ 0x8f7e32ce7bea5c70ULL,    83,   44 },
	{ 0xd5d238a4abe98068ULL,   109,   52 },
	{ 0x9f4f2726179a2245ULL,   136,   60 },
	{ 0xed63a231d4c4fb27ULL,   162,   68 },
	{ 0xb0de65388cc8ada8ULL,   189,   76 },
	{ 0x83c7088e1aab65dbULL,   216,   84 },
	{ 0xc45d1df942711d9aULL,   242,   92 },
	{ 0x924d692ca61be758ULL,   269,  100 },
	{ 0xda01ee641a708deaULL,   295,  108 },
	{ 0xa26da3999aef774aULL,   322,  116 },
	{ 0xf209787bb47d6b85ULL,   348,  124 },
	{ 0xb454e4a179dd1877ULL,   375,  132 },
	{ 0x865b86925b9bc5c2ULL,   402,  140 },
	{ 0xc83553c5c8965d3dULL,   428,  148 },
	{ 0x952ab45cfa97a0b3ULL,   455,  156 },
	{ 0xde469fbd99a05fe3ULL,   481,  164 },
	{ 0xa59bc234db398c25ULL,   508,  172 },
	{ 0xf6c69a72a3989f5cULL,   534,  180 },
	{ 0xb7dcbf5354e9beceULL,   561,  188 },
	{ 0x88fcf317f22241e2ULL,   588,  196 },
	{ 0xcc20ce9bd35c78a5ULL,   614,  204 },
	{ 0x98165af37b2153dfULL,   641,  212 },
	{ 0xe2a0b5dc971f303aULL,   667,  220 },
	{ 0xa8d9d1535ce3b396ULL,   694,  228 },
	{ 0xfb9b7cd9a4a7443cULL,   720,  236 },
	{ 0xbb764c4ca7a44410ULL,   747,  244 },
	{ 0x8bab8eefb6409c1aULL,   774,  252 },
	{ 0xd01fef10a657842cULL,   800,  260 },
	{ 0x9b10a4e5e9913129ULL,   827,  268 },
	{ 0xe7109bfba19c0c9dULL,   853,  276 },
	{ 0xac2820d9623bf429ULL,   880,  284 },
	{ 0x80444b5e7aa7cf85ULL,   907,  292 },
	{ 0xbf21e44003acdd2dULL,   933,  300 },
	{ 0x8e679c2f5e44ff8fULL,   960,  308 },
	{ 0xd433179d9c8cb841ULL,   986,  316 },
	{ 0x9e19db92b4e31ba9ULL,  1013,  324 },
	{ 0xeb96bf6ebadf77d9ULL,  1039,  332 },
	{ 0xaf87023b9bf0ee6bULL,  1066,  340 }
};

// Get cached power of 10 for given exponent
// Returns the decimal exponent
static int cached_pow(int exp, diy_fp *p) {
	int k = static_cast<int>(std::ceil((exp + DIYFP_FRACT_SIZE - 1) * D_1_LOG2_10));
	int i = (k - MIN_CACHED_EXP - 1) / CACHED_EXP_STEP + 1;
	p->f = pow_cache[i].fract;
	p->e = pow_cache[i].b_exp;
	return pow_cache[i].d_exp;
}

// Subtract two diy_fp numbers (assumes same exponent)
static diy_fp minus(diy_fp x, diy_fp y) {
	assert(x.e == y.e && x.f >= y.f);
	diy_fp d;
	d.f = x.f - y.f;
	d.e = x.e;
	return d;
}

// Multiply two diy_fp numbers
static diy_fp multiply(diy_fp x, diy_fp y) {
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
	diy_fp r;
	r.f = ac + (ad >> 32) + (bc >> 32) + (tmp >> 32);
	r.e = x.e + y.e + 64;
	return r;
}

// Normalize diy_fp (shift left until MSB is set)
static diy_fp normalize_diy_fp(diy_fp n) {
	assert(n.f != 0);
	while (!(n.f & 0xFFC0000000000000ULL)) { n.f <<= 10; n.e -= 10; }
	while (!(n.f & D64_SIGN)) { n.f <<= 1; --n.e; }
	return n;
}

// Convert IEEE-754 double to diy_fp
static diy_fp double2diy_fp(double d) {
	uint64_t u64;
	std::memcpy(&u64, &d, sizeof(double));

	diy_fp fp;
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

// pow10_cache[i] = 10^(i-1)
static const uint32_t pow10_cache[] = {
	0, 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000
};

// Find largest power of 10 that fits in n
static int largest_pow10(uint32_t n, int n_bits, uint32_t *power) {
	int guess = ((n_bits + 1) * 1233 >> 12) + 1;  // +1 to skip first entry (which is 1)
	if (guess >= 10) guess = 9;  // Clamp to valid range
	if (n < pow10_cache[guess]) --guess;
	*power = pow10_cache[guess];
	return guess;
}

// Round and weed out digits
static bool round_weed(char *buffer, int len, uint64_t wp_W, uint64_t delta,
                       uint64_t rest, uint64_t ten_kappa, uint64_t ulp) {
	uint64_t wp_Wup = wp_W - ulp;
	uint64_t wp_Wdown = wp_W + ulp;

	// Try to decrement last digit
	while (rest < wp_Wup && delta - rest >= ten_kappa &&
	       (rest + ten_kappa < wp_Wup || wp_Wup - rest >= rest + ten_kappa - wp_Wup)) {
		--buffer[len - 1];
		rest += ten_kappa;
	}

	// Check if we can round
	if (rest < wp_Wdown && delta - rest >= ten_kappa &&
	    (rest + ten_kappa < wp_Wdown || wp_Wdown - rest > rest + ten_kappa - wp_Wdown))
		return false;

	return 2 * ulp <= rest && rest <= delta - 4 * ulp;
}

// Generate digits
static bool digit_gen(diy_fp low, diy_fp w, diy_fp high, char *buffer, int *length, int *kappa) {
	uint64_t unit = 1;
	diy_fp too_low = { low.f - unit, low.e };
	diy_fp too_high = { high.f + unit, high.e };
	diy_fp unsafe_interval = minus(too_high, too_low);
	diy_fp one = { 1ULL << -w.e, w.e };
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
			return round_weed(buffer, *length, minus(too_high, w).f,
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
			return round_weed(buffer, *length, minus(too_high, w).f * unit,
			                  unsafe_interval.f, p2, one.f, unit);
		}
	}
}

// Main Grisu3 algorithm
static bool grisu3(double v, char *buffer, int *length, int *d_exp) {
	diy_fp dfp = double2diy_fp(v);
	diy_fp w = normalize_diy_fp(dfp);

	// Normalize boundaries
	diy_fp t = { (dfp.f << 1) + 1, dfp.e - 1 };
	diy_fp b_plus = normalize_diy_fp(t);
	diy_fp b_minus;

	uint64_t u64;
	std::memcpy(&u64, &v, sizeof(double));
	assert(v > 0 && v <= 1.7976931348623157e308);

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
	diy_fp c_mk;
	int mk = cached_pow(MIN_TARGET_EXP - DIYFP_FRACT_SIZE - w.e, &c_mk);

	// Scale
	w = multiply(w, c_mk);
	b_minus = multiply(b_minus, c_mk);
	b_plus = multiply(b_plus, c_mk);

	// Generate digits
	int kappa;
	bool success = digit_gen(b_minus, w, b_plus, buffer, length, &kappa);
	*d_exp = kappa - mk;
	return success;
}

// Convert double to string (main API)
std::string dtoa_grisu3(double v) {
	uint64_t u64;
	std::memcpy(&u64, &v, sizeof(double));

	// Handle special cases
	if ((u64 << 1) > 0xFFE0000000000000ULL) return "nan";

	bool negative = (u64 & D64_SIGN) != 0;
	if (negative) {
		v = -v;
		u64 ^= D64_SIGN;  // Clear sign bit
	}

	if (!u64) return "0";  // Check after handling sign
	if (u64 == D64_EXP_MASK) return negative ? "-inf" : "inf";

	char buffer[32];
	int length = 0;
	int d_exp = 0;

	bool success = grisu3(v, buffer, &length, &d_exp);
	if (!success) {
		// Fallback for cases where Grisu3 can't guarantee shortest representation
		char temp[32];
		std::snprintf(temp, sizeof(temp), "%.17g", negative ? -v : v);
		return std::string(temp);
	}

	// Debug output (commented out for production)
	// std::cerr << "v=" << v << " buffer=\"" << std::string(buffer, length) << "\" length=" << length << " d_exp=" << d_exp << "\n";

	// Format the result - port of original dtoa_grisu3 formatting logic
	std::string result;
	if (negative) result = "-";

	// Copy buffer to a working array
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
		// Need scientific notation as well?
		if (d_exp != 0) {
			s2[len++] = 'e';
			int exp_len = std::snprintf(s2 + len, sizeof(s2) - len, "%d", d_exp);
			len += exp_len;
		}
	} else if (d_exp < 0 && d_exp >= -3) {
		// Add decimal point for numbers of form 0.000x
		for (int i = 0; i < len; ++i) {
			s2[len - d_exp - 1 - i] = s2[len - i - 1];
		}
		s2[0] = '.';
		for (int i = 1; i < -d_exp; ++i) {
			s2[i] = '0';
		}
		len += -d_exp;
	} else if (d_exp < 0 || d_exp > 2) {
		// Add scientific notation
		s2[len++] = 'e';
		int exp_len = std::snprintf(s2 + len, sizeof(s2) - len, "%d", d_exp);
		len += exp_len;
	} else if (d_exp > 0) {
		// Add zeroes instead of scientific notation
		while (d_exp-- > 0) s2[len++] = '0';
	}

	s2[len] = '\0';
	result += s2;
	return result;
}

} // namespace grisu3

// Test program
int main() {
	using namespace grisu3;

	std::cout << "Grisu3 Algorithm Test Suite (MathGeoLib Port)\n\n";

	double tests[] = {
		1.0,
		0.125,
		3.14159,
		-3.14159,
		1.0e20,
		1.0e-20,
		123.456,
		2.0,
		0.1,
		0.5,
		0.0,
		-0.0,
		1.0/3.0,
		2.718281828459045
	};

	for (double value : tests) {
		std::string result = dtoa_grisu3(value);
		std::cout << std::setw(20) << std::setprecision(17) << value << " => " << result << "\n";
	}

	return 0;
}
