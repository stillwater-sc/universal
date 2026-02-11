#pragma once
// nvblock_impl.hpp: definition of the nvblock type for NVIDIA NVFP4 block formats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// NVIDIA NVFP4 differs from OCP MX in three key ways:
//   1. Smaller blocks (16 vs 32)
//   2. Non-power-of-two block scale (e4m3 vs e8m0)
//   3. External per-tensor FP32 scale
//
// Dequantize: dst[i] = tensor_scale * block_scale * element[i]
// Quantize:   raw_scale = amax / elem_max, block_scale = round_to_e4m3(raw_scale)

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstring>
#include <limits>
#include <algorithm>

#include <universal/number/microfloat/microfloat.hpp>
#include <universal/number/nvblock/nvblock_fwd.hpp>
#include <universal/number/nvblock/exceptions.hpp>

namespace sw { namespace universal {

// nvblock: NVIDIA two-level block scaling format
// Template parameters:
//   ElementType - microfloat element type (e.g. e2m1 for NVFP4)
//   BlockSize   - number of elements per block (16 for NVFP4)
//   ScaleType   - block-level scale type (e4m3 for NVFP4, fractional precision)
template<typename ElementType, size_t BlockSize, typename ScaleType>
class nvblock {
public:
	static constexpr size_t blockSize = BlockSize;

	nvblock() = default;

	constexpr nvblock(const nvblock&) = default;
	constexpr nvblock(nvblock&&) = default;

	constexpr nvblock& operator=(const nvblock&) = default;
	constexpr nvblock& operator=(nvblock&&) = default;

	// quantize a float array into this NV block
	// NVIDIA NVFP4 algorithm:
	//   1. Pre-divide inputs by tensor_scale
	//   2. amax = max(|pre_scaled[i]|)
	//   3. raw_scale = amax / elem_max
	//   4. block_scale = round_to_ScaleType(raw_scale)
	//   5. element[i] = round_to_ElementType(pre_scaled[i] / block_scale)
	void quantize(const float* src, float tensor_scale = 1.0f, size_t n = BlockSize) noexcept {
		if (n > BlockSize) n = BlockSize;

		// Compute elem_max: the largest representable value in ElementType
		float elem_max = compute_elem_max();

		// Step 1: pre-divide by tensor_scale and find amax
		float amax = 0.0f;
		for (size_t i = 0; i < n; ++i) {
			float pre_scaled = (tensor_scale != 0.0f) ? src[i] / tensor_scale : 0.0f;
			float a = std::fabs(pre_scaled);
			if (a > amax) amax = a;
		}

		// Step 2: compute block scale
		if (amax == 0.0f) {
			_block_scale.clear(); // zero scale
			for (size_t i = 0; i < BlockSize; ++i) {
				set_element_zero(i);
			}
			return;
		}

		// raw_scale = amax / elem_max, rounded to nearest ScaleType
		float raw_scale = amax / elem_max;
		_block_scale.from_float(raw_scale);

		// Ensure block_scale is not zero (would cause division by zero)
		float bs = _block_scale.to_float();
		if (bs == 0.0f) {
			// Scale underflowed to zero; use minimum positive representable
			_block_scale.minpos();
			bs = _block_scale.to_float();
		}

		// Step 3: quantize each element
		float inv_bs = 1.0f / bs;
		for (size_t i = 0; i < n; ++i) {
			float pre_scaled = (tensor_scale != 0.0f) ? src[i] / tensor_scale : 0.0f;
			float scaled = pre_scaled * inv_bs;
			set_element_from_float(i, scaled);
		}
		// zero-fill remaining elements
		for (size_t i = n; i < BlockSize; ++i) {
			set_element_zero(i);
		}
	}

	// dequantize this NV block into a float array
	// dst[i] = tensor_scale * block_scale * element[i]
	// If block_scale is NaN, all outputs are NaN
	void dequantize(float* dst, float tensor_scale = 1.0f, size_t n = BlockSize) const noexcept {
		if (n > BlockSize) n = BlockSize;

		if (_block_scale.isnan()) {
			for (size_t i = 0; i < n; ++i) {
				dst[i] = std::numeric_limits<float>::quiet_NaN();
			}
			return;
		}

		float s = tensor_scale * _block_scale.to_float();
		for (size_t i = 0; i < n; ++i) {
			dst[i] = s * get_element_float(i);
		}
	}

	// return dequantized element i (without tensor_scale; just block_scale * elem)
	float operator[](size_t i) const noexcept {
		if (i >= BlockSize) return 0.0f;
		if (_block_scale.isnan()) return std::numeric_limits<float>::quiet_NaN();
		return _block_scale.to_float() * get_element_float(i);
	}

	// block dot product with dual tensor scales
	// result = scale_a * scale_b * block_scale_a * block_scale_b * sum(elem_a[i] * elem_b[i])
	float dot(const nvblock& rhs, float scale_a = 1.0f, float scale_b = 1.0f) const noexcept {
		if (_block_scale.isnan() || rhs._block_scale.isnan()) {
			return std::numeric_limits<float>::quiet_NaN();
		}
		float sum = 0.0f;
		for (size_t i = 0; i < BlockSize; ++i) {
			sum += get_element_float(i) * rhs.get_element_float(i);
		}
		return scale_a * scale_b * _block_scale.to_float() * rhs._block_scale.to_float() * sum;
	}

	// accessors
	constexpr const ScaleType& block_scale() const noexcept { return _block_scale; }
	constexpr ScaleType& block_scale() noexcept { return _block_scale; }

	constexpr const ElementType& element(size_t i) const noexcept { return _elements[i]; }
	constexpr ElementType& element(size_t i) noexcept { return _elements[i]; }

	static constexpr size_t size() noexcept { return BlockSize; }

	// modifiers
	constexpr void clear() noexcept {
		_block_scale.clear();
		for (size_t i = 0; i < BlockSize; ++i) {
			set_element_zero(i);
		}
	}

	void setscalebits(unsigned bits) noexcept {
		_block_scale.setbits(bits);
	}

private:
	// compute the maximum representable value in the element type
	static float compute_elem_max() noexcept {
		ElementType mp;
		mp.maxpos();
		return mp.to_float();
	}

	// helper: set element i to zero
	constexpr void set_element_zero(size_t i) noexcept {
		_elements[i].clear();
	}

	// helper: convert element i to float
	float get_element_float(size_t i) const noexcept {
		return _elements[i].to_float();
	}

	// helper: set element from float (in quantized space, no further scaling)
	void set_element_from_float(size_t i, float v) noexcept {
		_elements[i].from_float(v);
	}

	ScaleType   _block_scale;
	ElementType _elements[BlockSize];
};

////////////////////////    functions   /////////////////////////////////

/// stream operators

template<typename ElementType, size_t BlockSize, typename ScaleType>
inline std::ostream& operator<<(std::ostream& ostr, const nvblock<ElementType, BlockSize, ScaleType>& blk) {
	ostr << "nvblock(scale=" << blk.block_scale().to_float();
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
