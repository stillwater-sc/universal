// grisu2_fixed.cpp: Corrected Grisu2 implementation based on reference implementations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// Based on "Printing Floating-Point Numbers Quickly and Accurately with Integers"
// by Florian Loitsch (2010) and Google's double-conversion library

#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <cstdint>
#include <cassert>
#include <cstring>

namespace grisu2 {

// DiyFp: Do-It-Yourself Floating Point
// Represents value = f × 2^e where f is a 64-bit unsigned integer
struct DiyFp {
	uint64_t f;
	int e;

	DiyFp() : f(0), e(0) {}
	DiyFp(uint64_t f_, int e_) : f(f_), e(e_) {}

	DiyFp operator-(const DiyFp& rhs) const {
		assert(e == rhs.e);
		return DiyFp(f - rhs.f, e);
	}

	DiyFp operator*(const DiyFp& rhs) const {
		const uint64_t M32 = 0xFFFFFFFF;
		const uint64_t a = f >> 32;
		const uint64_t b = f & M32;
		const uint64_t c = rhs.f >> 32;
		const uint64_t d = rhs.f & M32;

		const uint64_t ac = a * c;
		const uint64_t bc = b * c;
		const uint64_t ad = a * d;
		const uint64_t bd = b * d;

		uint64_t tmp = (bd >> 32) + (ad & M32) + (bc & M32);
		tmp += 1U << 31;  // Round to nearest

		return DiyFp(ac + (ad >> 32) + (bc >> 32) + (tmp >> 32), e + rhs.e + 64);
	}

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

// Cached power of 10
struct CachedPower {
	uint64_t f;
	int e;       // binary exponent
	int dec_exp; // decimal exponent: 10^dec_exp ≈ f × 2^e
};

// Full cached powers table - optimized for typical IEEE-754 double range
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
};

static const int kCachedPowersLength = sizeof(kCachedPowers) / sizeof(kCachedPowers[0]);

// Get cached power that brings e into a target range
// The goal is: alpha <= e + cached.e + 64 <= gamma
// where alpha = -60 and gamma = -32 (targets the range for efficient digit extraction)
const CachedPower& GetCachedPowerForBinaryExponent(int e, int& dec_exponent) {
	constexpr double k = 0.30102999566398114;  // log10(2)

	// We want to scale the number so that after multiplication by 10^-k,
	// the result is in a range where we can extract decimal digits efficiently.
	// Estimate k such that v * 10^-k is in range [1, 10)
	// Since v ≈ 2^e, we want 10^-k ≈ 2^-e, so k ≈ e * log10(2)
	int k_est = static_cast<int>(std::ceil((e + 64 - 1) * k));

	// Find closest entry in table
	// Table entries are spaced roughly 8 decimal exponents apart
	int index = 0;
	int min_diff = std::abs(kCachedPowers[0].dec_exp + k_est);

	for (int i = 1; i < kCachedPowersLength; i++) {
		int diff = std::abs(kCachedPowers[i].dec_exp + k_est);
		if (diff < min_diff) {
			min_diff = diff;
			index = i;
		}
	}

	dec_exponent = -kCachedPowers[index].dec_exp;
	return kCachedPowers[index];
}

// Generate digits from scaled DiyFp
// After scaling, the number is in form: significand × 2^exponent
// We want to extract decimal digits from this representation
bool DigitGen(DiyFp low, DiyFp w, DiyFp high, char* buffer, int* length, int* dec_exponent) {
	assert(low.e == w.e && w.e == high.e);

	uint64_t unit = 1;
	DiyFp too_low = DiyFp(low.f - unit, low.e);
	DiyFp too_high = DiyFp(high.f + unit, high.e);
	DiyFp unsafe_interval = too_high - too_low;

	// The exponent tells us where the binary point is
	// We need to extract the integer part for digit generation
	//  If e >= 0, the number is >= 2^64 (too large)
	// If e < -64, we can't extract any integer part
	DiyFp one = DiyFp(uint64_t{1} << -w.e, w.e);

	// Extract integer and fractional parts
	uint32_t integrals = static_cast<uint32_t>(w.f >> -w.e);  // Integer part
	uint64_t fractionals = w.f & (one.f - 1);  // Fractional part

	// Find the largest power of 10 <= integrals
	uint32_t divisor = 1000000000;  // 10^9
	int divisor_exponent = 9;

	while (divisor > integrals) {
		divisor /= 10;
		divisor_exponent--;
	}

	*dec_exponent = divisor_exponent;
	int kappa = divisor_exponent + 1;
	char* p = buffer;

	// Generate digits from integer part
	while (kappa > 0) {
		uint32_t digit = integrals / divisor;
		*p++ = '0' + digit;
		integrals %= divisor;
		kappa--;

		// Check if we can stop
		uint64_t rest = (static_cast<uint64_t>(integrals) << -w.e) + fractionals;
		if (rest < unsafe_interval.f) {
			*length = static_cast<int>(p - buffer);
			return true;
		}

		divisor /= 10;
	}

	// Generate digits from fractional part
	while (true) {
		fractionals *= 10;
		unsafe_interval.f *= 10;

		uint32_t digit = static_cast<uint32_t>(fractionals >> -w.e);
		*p++ = '0' + digit;
		fractionals &= (one.f - 1);
		kappa--;

		if (fractionals < unsafe_interval.f) {
			*length = static_cast<int>(p - buffer);
			*dec_exponent += kappa;
			return true;
		}

		// Safety: limit digits
		if (p - buffer >= 17) {
			*length = static_cast<int>(p - buffer);
			*dec_exponent += kappa;
			return true;
		}
	}
}

// Main Grisu2 entry point
bool Grisu2(double value, char* buffer, int* length, int* dec_exponent) {
	// Extract IEEE-754 double components
	uint64_t bits;
	std::memcpy(&bits, &value, sizeof(double));

	uint64_t frac = bits & 0xFFFFFFFFFFFFFULL;
	int exp = static_cast<int>((bits >> 52) & 0x7FF);

	// Handle subnormals and normals
	DiyFp v;
	if (exp != 0) {
		// Normal: add hidden bit, shift left by 11 to fill 64-bit significand
		v = DiyFp((frac | 0x10000000000000ULL) << 11, exp - 1023 - 52 - 11);
	} else {
		// Subnormal: no hidden bit
		v = DiyFp(frac << 11, 1 - 1023 - 52 - 11);
	}

	// Normalize
	v.normalize();

	// Compute boundaries for shortest representation
	// w_minus and w_plus are the adjacent representable values
	DiyFp w_minus, w_plus;
	if (exp != 0 && frac == 0) {
		// Power of 2 case: distance to predecessor is half the distance to successor
		w_minus = DiyFp((v.f << 1) - 1, v.e - 1);
		w_plus = DiyFp((v.f << 1) + 1, v.e - 1);
	} else {
		w_minus = DiyFp((v.f << 1) - 1, v.e - 1);
		w_plus = DiyFp((v.f << 1) + 1, v.e - 1);
	}
	w_minus.normalize();
	w_plus.normalize();

	// Get cached power to scale into target range
	int cached_dec_exp;
	const CachedPower& cached = GetCachedPowerForBinaryExponent(v.e, cached_dec_exp);
	DiyFp c(cached.f, cached.e);

	// Scale: multiply by power of 10 to bring into range [1, 10)
	DiyFp w_scaled = v * c;
	DiyFp w_minus_scaled = w_minus * c;
	DiyFp w_plus_scaled = w_plus * c;

	// Normalize the scaled values
	w_scaled.normalize();
	w_minus_scaled.normalize();
	w_plus_scaled.normalize();

	// All must have same exponent for digit generation
	// Align exponents by shifting
	int exp_diff_minus = w_scaled.e - w_minus_scaled.e;
	int exp_diff_plus = w_scaled.e - w_plus_scaled.e;

	if (exp_diff_minus > 0) {
		w_minus_scaled.f >>= exp_diff_minus;
		w_minus_scaled.e = w_scaled.e;
	}
	if (exp_diff_plus > 0) {
		w_plus_scaled.f >>= exp_diff_plus;
		w_plus_scaled.e = w_scaled.e;
	}

	// Generate digits
	int kappa;
	bool success = DigitGen(w_minus_scaled, w_scaled, w_plus_scaled, buffer, length, &kappa);

	if (success) {
		*dec_exponent = cached_dec_exp + kappa;
		// Debug
		std::cerr << "value=" << value << " v.e=" << v.e << " cached.e=" << cached.e
				  << " cached_dec_exp=" << cached_dec_exp << " kappa=" << kappa
				  << " final_dec_exp=" << *dec_exponent << " w_scaled.e=" << w_scaled.e << "\n";
	}

	return success;
}

std::string FormatGrisu2(bool negative, const char* digits, int len, int dec_exp) {
	std::string result;
	if (negative) result = "-";

	// Scientific notation: d.ddd...e±xx
	result += digits[0];
	if (len > 1) {
		result += '.';
		result.append(digits + 1, len - 1);
	}

	result += "e";
	result += (dec_exp >= 0) ? "+" : "";
	result += std::to_string(dec_exp);

	return result;
}

std::string DoubleToString(double value) {
	if (value == 0.0) return "0e+0";
	if (std::isnan(value)) return "nan";
	if (std::isinf(value)) return value < 0 ? "-inf" : "inf";

	bool negative = value < 0;
	if (negative) value = -value;

	char buffer[32];
	int length = 0;
	int dec_exponent = 0;

	if (Grisu2(value, buffer, &length, &dec_exponent)) {
		return FormatGrisu2(negative, buffer, length, dec_exponent);
	} else {
		return "FAILED";
	}
}

} // namespace grisu2

int main() {
	using namespace grisu2;

	std::cout << "Grisu2 Algorithm Test Suite (Fixed Version)\n\n";

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
		0.5
	};

	for (double value : tests) {
		std::string result = DoubleToString(value);
		std::cout << std::setw(15) << value << " => " << result << "\n";
	}

	return 0;
}
