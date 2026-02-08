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
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstring>
#include <limits>
#include <algorithm>

#include <universal/number/e8m0/e8m0.hpp>
#include <universal/number/microfloat/microfloat.hpp>
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

	// quantize a float array into this MX block
	// Per OCP MX v1.0 spec:
	//   amax = max(|x_i|) over the block
	//   shared_exp = clamp(floor(log2(amax)), -127, 127)
	//   scale_exp = shared_exp - max_elem_exp(ElementType)
	//   scale = e8m0(scale_exp + 127)
	//   q_i = RNE(x_i / 2^scale_exp) quantized to ElementType
	void quantize(const float* src, size_t n = BlockSize) noexcept {
		if (n > BlockSize) n = BlockSize;

		// Step 1: find absolute maximum across input
		float amax = 0.0f;
		for (size_t i = 0; i < n; ++i) {
			float a = std::fabs(src[i]);
			if (a > amax) amax = a;
		}

		// Step 2: compute shared exponent and e8m0 scale
		if (amax == 0.0f) {
			// all-zeros input: set scale to represent 2^(-elemMaxExp)
			int biased = (-elemMaxExp) + e8m0::bias;
			if (biased < 0) biased = 0;
			_scale.setbits(static_cast<unsigned>(biased));
			// set all elements to zero
			for (size_t i = 0; i < n; ++i) {
				set_element_zero(i);
			}
			for (size_t i = n; i < BlockSize; ++i) {
				set_element_zero(i);
			}
			return;
		}

		int shared_exp = static_cast<int>(std::floor(std::log2(amax)));
		// clamp to e8m0 representable range
		if (shared_exp < -127) shared_exp = -127;
		if (shared_exp > 127) shared_exp = 127;

		int scale_exp = shared_exp - elemMaxExp;
		int biased_scale = scale_exp + e8m0::bias;
		if (biased_scale < 0) biased_scale = 0;
		if (biased_scale > 254) biased_scale = 254;
		_scale.setbits(static_cast<unsigned>(biased_scale));

		// Step 3: compute the actual power-of-2 scale factor for quantization
		float inv_scale = 1.0f / std::ldexp(1.0f, scale_exp);

		// Step 4: quantize each element
		for (size_t i = 0; i < n; ++i) {
			float scaled = src[i] * inv_scale;
			set_element_from_float(i, scaled);
		}
		// zero-fill remaining elements
		for (size_t i = n; i < BlockSize; ++i) {
			set_element_zero(i);
		}
	}

	// dequantize this MX block into a float array
	// If scale is NaN (encoding 0xFF), all output values are NaN
	void dequantize(float* dst, size_t n = BlockSize) const noexcept {
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
	float operator[](size_t i) const noexcept {
		if (i >= BlockSize) return 0.0f;
		if (_scale.isnan()) return std::numeric_limits<float>::quiet_NaN();
		return _scale.to_float() * get_element_float(i);
	}

	// block dot product: FP32-accumulated
	// result = float(a.scale) * float(b.scale) * sum_i(float(a[i]) * float(b[i]))
	float dot(const mxblock& rhs) const noexcept {
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

	void setbits(unsigned scaleBits) noexcept {
		_scale.setbits(scaleBits);
	}

private:
	// helper: set element i to zero
	constexpr void set_element_zero(size_t i) noexcept {
		if constexpr (std::is_integral_v<ElementType>) {
			_elements[i] = 0;
		} else {
			_elements[i].clear();
		}
	}

	// helper: convert element i to float
	float get_element_float(size_t i) const noexcept {
		if constexpr (std::is_integral_v<ElementType>) {
			return static_cast<float>(_elements[i]);
		} else {
			return _elements[i].to_float();
		}
	}

	// helper: set element from float (quantized space, no further scaling)
	void set_element_from_float(size_t i, float v) noexcept {
		if constexpr (std::is_integral_v<ElementType>) {
			// Round to nearest integer, clamp to int8_t range
			float rounded = std::round(v);
			if (rounded > 127.0f) rounded = 127.0f;
			if (rounded < -128.0f) rounded = -128.0f;
			_elements[i] = static_cast<int8_t>(rounded);
		} else {
			_elements[i].from_float(v);
		}
	}

	e8m0        _scale;
	ElementType _elements[BlockSize];
};

////////////////////////    functions   /////////////////////////////////

/// stream operators

template<typename ElementType, size_t BlockSize>
inline std::ostream& operator<<(std::ostream& ostr, const mxblock<ElementType, BlockSize>& blk) {
	ostr << "mxblock(scale=" << blk.scale();
	ostr << ", elements=[";
	for (size_t i = 0; i < BlockSize; ++i) {
		if (i > 0) ostr << ", ";
		ostr << blk[i];
		if (i >= 7 && BlockSize > 10) {
			ostr << ", ... (" << (BlockSize - i - 1) << " more)";
			break;
		}
	}
	ostr << "])";
	return ostr;
}

}} // namespace sw::universal
