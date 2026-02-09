#pragma once
// zfpblock_impl.hpp: implementation of zfpblock container
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cstdint>
#include <cstddef>
#include <cmath>
#include <cstring>
#include <climits>
#include <iostream>
#include <iomanip>
#include <universal/number/zfpblock/zfpblock_fwd.hpp>
#include <universal/number/zfpblock/zfp_codec_traits.hpp>
#include <universal/number/zfpblock/zfp_codec.hpp>

namespace sw { namespace universal {

// zfpblock: ZFP compressed floating-point block codec
//
// Template parameters:
//   Real - floating-point element type (float or double)
//   Dim  - dimensionality (1, 2, or 3)
//
// A zfpblock stores a compressed representation of a 4^Dim block
// of floating-point values using the ZFP transform codec.
template<typename Real, unsigned Dim>
class zfpblock {
	static_assert(std::is_same_v<Real, float> || std::is_same_v<Real, double>,
		"zfpblock requires float or double");
	static_assert(Dim >= 1 && Dim <= 3, "zfpblock requires Dim in {1, 2, 3}");

public:
	static constexpr size_t BLOCK_SIZE = zfp_block_size<Dim>::value;
	static constexpr size_t MAX_BYTES  = zfp_max_bytes<Real, Dim>::value;
	using traits_type = zfp_type_traits<Real>;

	// default constructor — leaves buffer uninitialized for triviality
	zfpblock() = default;

	// Compress a block of 4^Dim values
	// Returns the number of bits in the compressed representation
	size_t compress(const Real* src, zfp_mode mode, double param) {
		_mode = mode;
		_param = param;
		unsigned maxprec = 0;
		size_t maxbits = 0;
		compute_limits(mode, param, maxprec, maxbits);
		_nbits = encode_block<Real, Dim>(src, _buffer, MAX_BYTES, maxprec, maxbits);
		return _nbits;
	}

	// Decompress to a block of 4^Dim values
	void decompress(Real* dst) const {
		unsigned maxprec = 0;
		size_t maxbits = 0;
		compute_limits(_mode, _param, maxprec, maxbits);
		decode_block<Real, Dim>(_buffer, MAX_BYTES, dst, maxprec, maxbits);
	}

	// Convenience: compress with fixed rate (bits per value)
	size_t compress_fixed_rate(const Real* src, double rate) {
		return compress(src, zfp_mode::fixed_rate, rate);
	}

	// Convenience: compress with fixed precision (number of bit planes)
	size_t compress_fixed_precision(const Real* src, unsigned prec) {
		return compress(src, zfp_mode::fixed_precision, static_cast<double>(prec));
	}

	// Convenience: compress with fixed accuracy (absolute error tolerance)
	size_t compress_fixed_accuracy(const Real* src, double tolerance) {
		return compress(src, zfp_mode::fixed_accuracy, tolerance);
	}

	// Convenience: lossless reversible compression
	size_t compress_reversible(const Real* src) {
		return compress(src, zfp_mode::reversible, 0.0);
	}

	// Query compressed size in bits
	size_t compressed_bits() const { return _nbits; }

	// Query compressed size in bytes (rounded up)
	size_t compressed_bytes() const { return (_nbits + 7) / 8; }

	// Compression ratio: uncompressed / compressed
	double compression_ratio() const {
		if (_nbits == 0) return 0.0;
		return static_cast<double>(BLOCK_SIZE * sizeof(Real) * 8) / static_cast<double>(_nbits);
	}

	// Access the raw compressed buffer
	const uint8_t* data() const { return _buffer; }

	// Mode and parameter accessors
	zfp_mode mode() const { return _mode; }
	double param() const { return _param; }

	// Block size (number of elements)
	static constexpr size_t block_size() { return BLOCK_SIZE; }

	// Dimensionality
	static constexpr unsigned dim() { return Dim; }

private:
	uint8_t  _buffer[MAX_BYTES];
	size_t   _nbits;
	zfp_mode _mode;
	double   _param;

	// Compute maxprec and maxbits from mode and parameter
	static void compute_limits(zfp_mode mode, double param,
	                           unsigned& maxprec, size_t& maxbits) {
		constexpr unsigned full_prec = traits_type::precision_bits;
		constexpr size_t header_size = 1 + traits_type::ebits;  // zero bit + exponent
		constexpr size_t max_data_bits = BLOCK_SIZE * full_prec + header_size;

		switch (mode) {
		case zfp_mode::fixed_rate: {
			double rate = param;
			maxbits = static_cast<size_t>(rate * BLOCK_SIZE);
			maxprec = full_prec;
			break;
		}
		case zfp_mode::fixed_precision: {
			unsigned prec = static_cast<unsigned>(param);
			maxprec = prec;
			maxbits = max_data_bits;
			break;
		}
		case zfp_mode::fixed_accuracy: {
			(void)param;  // tolerance parameter reserved for future minexp truncation
			maxprec = full_prec;
			maxbits = max_data_bits;
			break;
		}
		case zfp_mode::reversible: {
			maxprec = full_prec;
			maxbits = max_data_bits;
			break;
		}
		}
	}
};

}} // namespace sw::universal
