#pragma once
// mxblock_impl.hpp: definition of the mxblock type for OCP Microscaling block formats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// An mxblock pairs one shared e8m0 scale factor with BlockSize micro-float elements,
// implementing the OCP Microscaling (MX) v1.0 block floating-point format.
// Each MX block provides 4-8x compression vs FP32 with controlled quantization error.

#include <string>
#include <iosfwd>       // std::ostream in the stream-operator declaration (#1334)
#include <string>       // std::string, returned by the text layer's builders
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <algorithm>
#include <type_traits>

#include <universal/utility/bit_cast.hpp>
#include <universal/number/e8m0/core.hpp>         // the scale type; its text layer is separate (#1334)
#include <universal/number/microfloat/core.hpp>   // the element type; its text layer is separate (#1334)
#include <universal/number/mxfloat/mxfloat_fwd.hpp>
#include <universal/number/mxfloat/exceptions.hpp>

namespace sw { namespace universal {

// mxblock: OCP Microscaling block floating-point format
// Template parameters:
//   ElementType - microfloat alias (e2m1, e2m3, e3m2, e4m3, e5m2) or int8_t
//   BlockSize   - number of elements per block (default 32 per OCP MX v1.0 spec)
template<typename ElementType, size_t BlockSize>
class mxblock {
public:
	static constexpr size_t blockSize = BlockSize;
	static constexpr int    elemMaxExp = max_elem_exponent<ElementType>;

	mxblock() = default;

	constexpr mxblock(const mxblock&) = default;
	constexpr mxblock(mxblock&&) = default;

	constexpr mxblock& operator=(const mxblock&) = default;
	constexpr mxblock& operator=(mxblock&&) = default;

	// quantize a float array into this MX block.
	// Per OCP MX v1.0 spec:
	//   amax = max(|x_i|) over the block
	//   scale_exp = clamp(floor(log2(amax)) - max_elem_exp(ElementType), -127, 127)
	//   scale = e8m0(scale_exp + 127)
	//   q_i = RNE(x_i / 2^scale_exp) quantized to ElementType
	//
	// floor(log2(amax)) is read off the float's exponent field (std::ilogb), not computed
	// with std::log2: a float log2 rounds up to the next integer just below a power of two
	// (log2(16 - 2^-20) gives 4.0f), and of an infinite amax it gives inf, which no int
	// can hold.  The scale exponent, not floor(log2(amax)), is clamped to e8m0's range,
	// and the elements are scaled with ldexp: a reciprocal 1 / 2^scale_exp is inf once
	// scale_exp <= -128, which turned zeros into NaN (#2083).
	//
	// Constexpr-safe: std::fabs / std::ilogb / std::ldexp are not constexpr.  We dispatch
	// via std::is_constant_evaluated():
	//   * runtime path keeps the stdlib calls (fast, exact)
	//   * constexpr path uses cx_fabs / cx_floor_log2 / cx_ldexp helpers
	//     defined privately below.  All helpers operate on IEEE 754
	//     binary32 fields via sw::bit_cast or bounded power-of-2 loops.
	constexpr void quantize(const float* src, size_t n = BlockSize) noexcept {
		if (n > BlockSize) n = BlockSize;

		// Step 1: find absolute maximum across input
		float amax = 0.0f;
		for (size_t i = 0; i < n; ++i) {
			float a = std::is_constant_evaluated() ? cx_fabs(src[i]) : std::fabs(src[i]);
			if (a > amax) amax = a;
		}

		// Step 2: compute shared exponent and e8m0 scale
		if (amax == 0.0f) {
			// all-zeros input: set scale to represent 2^(-elemMaxExp)
			int biased = (-elemMaxExp) + e8m0::bias;
			if (biased < 0) biased = 0;
			_scale.setbits(static_cast<unsigned>(biased));
			// every element is a zero; convert it like any other, so a -0 keeps its sign
			for (size_t i = 0; i < n; ++i) {
				set_element_from_float(i, src[i]);
			}
			for (size_t i = n; i < BlockSize; ++i) {
				set_element_zero(i);
			}
			return;
		}

		int shared_exp = 0;
		if (std::is_constant_evaluated()) {
			shared_exp = cx_floor_log2(amax);
		}
		else {
			shared_exp = std::ilogb(amax);   // exact; INT_MAX for an infinite amax
			if (shared_exp > 127) shared_exp = 127;
		}

		// clamp the scale exponent to e8m0's range, 2^-127 .. 2^127
		int scale_exp = shared_exp - elemMaxExp;
		if (scale_exp < -e8m0::bias) scale_exp = -e8m0::bias;
		if (scale_exp > 127) scale_exp = 127;
		_scale.setbits(static_cast<unsigned>(scale_exp + e8m0::bias));

		// Step 3: quantize each element: x_i * 2^-scale_exp is exact for every
		// element that the element type can tell apart from zero
		for (size_t i = 0; i < n; ++i) {
			float scaled = std::is_constant_evaluated()
				? cx_ldexp(src[i], -scale_exp)
				: std::ldexp(src[i], -scale_exp);
			set_element_from_float(i, scaled);
		}
		// zero-fill remaining elements
		for (size_t i = n; i < BlockSize; ++i) {
			set_element_zero(i);
		}
	}

	// dequantize this MX block into a float array
	// If scale is NaN (encoding 0xFF), all output values are NaN
	constexpr void dequantize(float* dst, size_t n = BlockSize) const noexcept {
		if (n > BlockSize) n = BlockSize;

		if (_scale.isnan()) {
			for (size_t i = 0; i < n; ++i) {
				dst[i] = std::numeric_limits<float>::quiet_NaN();
			}
			return;
		}

		float s = _scale.to_float();
		for (size_t i = 0; i < n; ++i) {
			dst[i] = s * get_element_float(i);
		}
	}

	// return dequantized element i
	constexpr float operator[](size_t i) const noexcept {
		if (i >= BlockSize) return 0.0f;
		if (_scale.isnan()) return std::numeric_limits<float>::quiet_NaN();
		return _scale.to_float() * get_element_float(i);
	}

	// block dot product: FP32-accumulated
	// result = float(a.scale) * float(b.scale) * sum_i(float(a[i]) * float(b[i]))
	constexpr float dot(const mxblock& rhs) const noexcept {
		if (_scale.isnan() || rhs._scale.isnan()) {
			return std::numeric_limits<float>::quiet_NaN();
		}
		float sum = 0.0f;
		for (size_t i = 0; i < BlockSize; ++i) {
			sum += get_element_float(i) * rhs.get_element_float(i);
		}
		return _scale.to_float() * rhs._scale.to_float() * sum;
	}

	// accessors
	constexpr const e8m0& scale() const noexcept { return _scale; }
	constexpr e8m0& scale() noexcept { return _scale; }

	constexpr const ElementType& element(size_t i) const noexcept { return _elements[i]; }
	constexpr ElementType& element(size_t i) noexcept { return _elements[i]; }

	static constexpr size_t size() noexcept { return BlockSize; }

	// compute byte size: 1 byte for scale + BlockSize * element bytes
	static constexpr size_t byte_size() noexcept {
		if constexpr (std::is_integral_v<ElementType>) {
			return 1 + BlockSize; // int8_t = 1 byte each
		} else {
			// microfloat element: nbits bits each
			// For simplicity, each element occupies 1 byte of storage
			return 1 + BlockSize;
		}
	}

	// modifiers
	constexpr void clear() noexcept {
		_scale.clear();
		for (size_t i = 0; i < BlockSize; ++i) {
			set_element_zero(i);
		}
	}

	constexpr void setbits(unsigned scaleBits) noexcept {
		_scale.setbits(scaleBits);
	}

private:
	// Constexpr-safe |x| via direct sign-flip (std::fabs is not constexpr).
	static constexpr float cx_fabs(float v) noexcept {
		return v < 0.0f ? -v : v;
	}

	// Constexpr-safe floor(log2(amax)) via IEEE 754 bit-extraction.
	// Precondition: amax > 0 and finite.  Returns the integer power of 2
	// such that 2^result <= amax < 2^(result+1).
	//
	// For an IEEE 754 binary32 normal float v = (1 + frac/2^23) * 2^E,
	// log2(v) = E + log2(1 + frac/2^23) which is in [E, E+1), so
	// floor(log2(v)) == E == rawExp - 127.  Subnormal floats are handled
	// by counting the leading-zero offset of the mantissa.
	static constexpr int cx_floor_log2(float v) noexcept {
		uint32_t bits_u = sw::bit_cast<uint32_t>(v);
		uint32_t rawExp  = (bits_u >> 23) & 0xFFu;
		uint32_t rawFrac = bits_u & 0x7FFFFFu;
		if (rawExp != 0u) {
			// normal: floor(log2) = rawExp - 127
			return static_cast<int>(rawExp) - 127;
		}
		// subnormal: value = rawFrac * 2^-149.  Find top bit of rawFrac.
		// rawFrac > 0 by precondition.
		int leading = 22;
		while (leading >= 0 && ((rawFrac >> leading) & 1u) == 0u) --leading;
		// value's exponent = -149 + leading.
		return -149 + leading;
	}

	// Constexpr-safe x * 2^exp via power-of-2 multiplication loop.
	// Bounded by |exp| <= 127 in our use (the clamped scale exponent),
	// so the linear loop is well-bounded.
	static constexpr float cx_ldexp(float x, int exp) noexcept {
		if (exp >= 0) {
			for (int i = 0; i < exp; ++i) x *= 2.0f;
		}
		else {
			for (int i = 0; i < -exp; ++i) x *= 0.5f;
		}
		return x;
	}

	// helper: set element i to zero
	constexpr void set_element_zero(size_t i) noexcept {
		if constexpr (std::is_integral_v<ElementType>) {
			_elements[i] = 0;
		} else {
			_elements[i].clear();
		}
	}

	// helper: convert element i to float
	constexpr float get_element_float(size_t i) const noexcept {
		if constexpr (std::is_integral_v<ElementType>) {
			return static_cast<float>(_elements[i]);
		} else {
			return _elements[i].to_float();
		}
	}

	// helper: set element from float (quantized space, no further scaling).
	// int8 codes round to nearest, ties to even, as OCP MX specifies, and saturate to
	// [-128, 127].  std::round rounded ties away from zero (2.5 -> 3, #2083).  The
	// rounding is done by hand so that one path serves runtime and constant evaluation:
	// past the clamp |v| < 2^7, so the truncation and the remainder are exact.
	constexpr void set_element_from_float(size_t i, float v) noexcept {
		if constexpr (std::is_integral_v<ElementType>) {
			if (v >= 127.0f) { _elements[i] = 127; return; }
			if (v <= -128.0f) { _elements[i] = -128; return; }
			int t = static_cast<int>(v);                  // toward zero
			float r = v - static_cast<float>(t);          // exact, in (-1, 1)
			if (r > 0.5f || (r == 0.5f && (t & 1) != 0)) ++t;
			else if (r < -0.5f || (r == -0.5f && (t & 1) != 0)) --t;
			_elements[i] = static_cast<int8_t>(t);
		} else {
			_elements[i].from_float(v);
		}
	}

	e8m0        _scale;
	ElementType _elements[BlockSize];
};

////////////////////////    functions   /////////////////////////////////

/// stream operators


}} // namespace sw::universal
