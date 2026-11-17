#pragma once
// zfpblock_impl.hpp: implementation of zfpblock container
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <type_traits>   // std::is_same_v

#include <cstdint>
#include <cstddef>
#include <cmath>
#include <cstring>
#include <climits>
#include <bit>
#include <limits>
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

	// default constructor -- leaves buffer uninitialized for triviality
	zfpblock() = default;

	// Compress a block of 4^Dim values
	// Returns the number of bits in the compressed representation.
	// constexpr-callable: encode_block dispatches std::frexp / std::ldexp /
	// std::memset / __builtin_ctzll via std::is_constant_evaluated() to
	// constexpr-safe replacements at compile time (PR #815).
	constexpr size_t compress(const Real* src, zfp_mode mode, double param) {
		_mode = mode;
		_param = param;
		unsigned maxprec = 0;
		size_t maxbits = 0;
		compute_limits(mode, param, maxprec, maxbits);
		const int minexp = accuracy_floor(mode, param);
		const bool pad = (mode == zfp_mode::fixed_rate);   // only a fixed rate has a fixed size (#2097)
		_nbits = encode_block<Real, Dim>(src, _buffer, MAX_BYTES, maxprec, maxbits, minexp, pad);
		// encode_block stores a block with an inf or NaN raw, which is exact; the checks below would
		// decode it, see inf - inf or a NaN miss the tolerance, and store it raw again (#2113)
		const bool nonfinite = zfp_has_nonfinite<Real, BLOCK_SIZE>(src);
		if (nonfinite) return _nbits;
		if (mode == zfp_mode::fixed_accuracy && minexp != INT_MIN && !within(src, param, maxprec, maxbits, minexp)) {
			// the plane count bounds the transform's error, but a value below one quantum of the block's
			// exponent (2^(emax - frac_bits)) rounds to zero before it: {1, 2^-40, 0, 0} at 2^-45 missed the
			// tolerance. Such a block is stored raw, which is exact (CodeRabbit on #2099).
			_nbits = encode_raw_block<Real, Dim>(src, _buffer, MAX_BYTES, maxbits, false);
		}
		if (mode == zfp_mode::reversible && !reproduces(src, maxprec, maxbits)) {
			// reversible is lossless: the full-precision pipeline does not reproduce every block bit for
			// bit (0.1f came back as 0.099999994, #2097), and then the block is stored raw
			_nbits = encode_raw_block<Real, Dim>(src, _buffer, MAX_BYTES, maxbits, false);
		}
		return _nbits;
	}

	// Decompress to a block of 4^Dim values.  See compress() note above.
	constexpr void decompress(Real* dst) const {
		unsigned maxprec = 0;
		size_t maxbits = 0;
		compute_limits(_mode, _param, maxprec, maxbits);
		decode_block<Real, Dim>(_buffer, MAX_BYTES, dst, maxprec, maxbits, accuracy_floor(_mode, _param));
	}

	// Convenience: compress with fixed rate (bits per value)
	constexpr size_t compress_fixed_rate(const Real* src, double rate) {
		return compress(src, zfp_mode::fixed_rate, rate);
	}

	// Convenience: compress with fixed precision (number of bit planes)
	constexpr size_t compress_fixed_precision(const Real* src, unsigned prec) {
		return compress(src, zfp_mode::fixed_precision, static_cast<double>(prec));
	}

	// Convenience: compress with fixed accuracy (absolute error tolerance)
	constexpr size_t compress_fixed_accuracy(const Real* src, double tolerance) {
		return compress(src, zfp_mode::fixed_accuracy, tolerance);
	}

	// Convenience: lossless reversible compression
	constexpr size_t compress_reversible(const Real* src) {
		return compress(src, zfp_mode::reversible, 0.0);
	}

	// Query compressed size in bits
	constexpr size_t compressed_bits() const noexcept { return _nbits; }

	// Query compressed size in bytes (rounded up)
	constexpr size_t compressed_bytes() const noexcept { return (_nbits + 7) / 8; }

	// Compression ratio: uncompressed / compressed
	constexpr double compression_ratio() const noexcept {
		if (_nbits == 0) return 0.0;
		return static_cast<double>(BLOCK_SIZE * sizeof(Real) * 8) / static_cast<double>(_nbits);
	}

	// Access the raw compressed buffer
	constexpr const uint8_t* data() const noexcept { return _buffer; }

	// Mode and parameter accessors
	constexpr zfp_mode mode() const noexcept { return _mode; }
	constexpr double param() const noexcept { return _param; }

	// Block size (number of elements)
	static constexpr size_t block_size() noexcept { return BLOCK_SIZE; }

	// Dimensionality
	static constexpr unsigned dim() noexcept { return Dim; }

private:
	uint8_t  _buffer[MAX_BYTES];
	size_t   _nbits;
	zfp_mode _mode;
	double   _param;

	// Compute maxprec and maxbits from mode and parameter
	static constexpr void compute_limits(zfp_mode mode, double param,
	                                     unsigned& maxprec, size_t& maxbits) noexcept {
		constexpr unsigned full_prec = traits_type::precision_bits;
		constexpr size_t header_size = 1 + traits_type::ebits;  // zero bit + exponent
		constexpr size_t max_data_bits = BLOCK_SIZE * full_prec + header_size;

		switch (mode) {
		case zfp_mode::fixed_rate: {
			// clamped to header + one bit plane, and capped at the raw size (#2097)
			maxbits = zfp_fixed_rate_bits<Real, Dim>(param);
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
			// the tolerance sets each block's precision, through accuracy_floor (#2097)
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
		if (maxbits < zfp_raw_block_bits<Real, Dim>() && mode != zfp_mode::fixed_rate) maxbits = zfp_raw_block_bits<Real, Dim>();   // room for a raw block
	}

	// the fixed-accuracy floor: 2^minexp <= tolerance, which ZFP's precision rule turns into a plane
	// count; compress() checks the result, since values below the block's quantum round to zero
	// whatever the count. INT_MIN (none) for the other modes, and for a tolerance that is not positive
	// and finite, which codes every plane
	static constexpr int accuracy_floor(zfp_mode mode, double tolerance) noexcept {
		if (mode != zfp_mode::fixed_accuracy) return INT_MIN;
		if (!(tolerance > 0.0) || !(tolerance <= std::numeric_limits<double>::max())) return INT_MIN;
		int e = 0;
		if (std::is_constant_evaluated()) {
			double t = tolerance;
			while (t >= 2.0) { t /= 2.0; ++e; }
			while (t < 1.0) { t *= 2.0; --e; }
		}
		else {
			e = std::ilogb(tolerance);   // floor(log2(tolerance)), exactly
		}
		return e;
	}

	// the coded block decodes to within tolerance of src, every value
	constexpr bool within(const Real* src, double tolerance, unsigned maxprec, size_t maxbits, int minexp) const {
		Real back[BLOCK_SIZE]{};
		decode_block<Real, Dim>(_buffer, MAX_BYTES, back, maxprec, maxbits, minexp);
		for (size_t i = 0; i < BLOCK_SIZE; ++i) {
			const double d = static_cast<double>(back[i]) - static_cast<double>(src[i]);
			if (!(d <= tolerance && -d <= tolerance)) return false;   // a NaN fails too
		}
		return true;
	}

	// the coded block decodes to src bit for bit
	constexpr bool reproduces(const Real* src, unsigned maxprec, size_t maxbits) const {
		Real back[BLOCK_SIZE]{};
		decode_block<Real, Dim>(_buffer, MAX_BYTES, back, maxprec, maxbits);
		using UInt = typename traits_type::UInt;
		for (size_t i = 0; i < BLOCK_SIZE; ++i) {
			if (std::bit_cast<UInt>(back[i]) != std::bit_cast<UInt>(src[i])) return false;
		}
		return true;
	}
};

}} // namespace sw::universal
