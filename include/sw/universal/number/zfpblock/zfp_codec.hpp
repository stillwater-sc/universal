#pragma once
// zfp_codec.hpp: ZFP block codec — transform, encoding, and decoding pipeline
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Implements LLNL ZFP's single-block codec:
//   float[4^d] → block-float → lifting → reorder → negabinary → bit-plane → bits

#include <cstdint>
#include <cstddef>
#include <cmath>
#include <cstring>
#include <climits>
#include <algorithm>
#include <array>

#ifdef _MSC_VER
#include <intrin.h>
#endif

#include <universal/number/zfpblock/zfp_codec_traits.hpp>

namespace sw { namespace universal {

// Portable count-trailing-zeros for uint64_t
// Returns the index of the lowest set bit (0-63). Undefined if x == 0.
inline unsigned zfp_ctzll(uint64_t x) {
#ifdef _MSC_VER
	unsigned long index;
	_BitScanForward64(&index, x);
	return static_cast<unsigned>(index);
#else
	return static_cast<unsigned>(__builtin_ctzll(x));
#endif
}

// ============================================================
// Permutation tables (reorder by total sequency)
// ============================================================

// 1D: identity
constexpr std::array<unsigned, 4> zfp_perm_1d = { 0, 1, 2, 3 };

// 2D: 4x4 block ordered by (i+j, i*i+j*j)
constexpr std::array<unsigned, 16> zfp_perm_2d = {
	 0,  1,  4,  5,
	 2,  8,  6,  9,
	 3, 12, 10,  7,
	13, 14, 11, 15
};

// 3D: 4x4x4 block ordered by total sequency
constexpr std::array<unsigned, 64> zfp_perm_3d = {
	 0,  1,  4,  5, 16, 17, 20, 21,
	 2,  8,  6,  9, 18, 24, 22, 25,
	 3, 12, 10,  7, 19, 28, 26, 23,
	13, 14, 11, 15, 29, 30, 27, 31,
	32, 33, 36, 37, 48, 49, 52, 53,
	34, 40, 38, 41, 50, 56, 54, 57,
	35, 44, 42, 39, 51, 60, 58, 55,
	45, 46, 43, 47, 61, 62, 59, 63
};

// Return permutation table for given dimension
template<unsigned Dim>
inline const auto& zfp_perm() {
	if constexpr (Dim == 1) return zfp_perm_1d;
	else if constexpr (Dim == 2) return zfp_perm_2d;
	else return zfp_perm_3d;
}

// ============================================================
// Bitstream: in-memory bit-level reader/writer (LSB-first)
// ============================================================

class zfp_bitstream {
public:
	zfp_bitstream(uint8_t* buffer, size_t max_bytes)
		: _buffer(buffer), _max_bytes(max_bytes), _bits(0), _buffer_bits(0), _byte_pos(0) {}

	// Write 'n' bits from value (LSB first), n <= 64
	void write_bits(uint64_t value, unsigned n) {
		_bits += n;
		while (n > 0) {
			unsigned space = 8 - _buffer_bits;
			unsigned chunk = (n < space) ? n : space;
			uint8_t mask = static_cast<uint8_t>((1u << chunk) - 1);
			if (_byte_pos < _max_bytes) {
				if (_buffer_bits == 0) _buffer[_byte_pos] = 0;
				_buffer[_byte_pos] |= static_cast<uint8_t>((value & mask) << _buffer_bits);
			}
			value >>= chunk;
			n -= chunk;
			_buffer_bits += chunk;
			if (_buffer_bits == 8) {
				_buffer_bits = 0;
				++_byte_pos;
			}
		}
	}

	// Read 'n' bits (LSB first), n <= 64
	uint64_t read_bits(unsigned n) {
		_bits += n;
		uint64_t result = 0;
		unsigned shift = 0;
		while (n > 0) {
			unsigned avail = 8 - _buffer_bits;
			unsigned chunk = (n < avail) ? n : avail;
			uint8_t mask = static_cast<uint8_t>((1u << chunk) - 1);
			uint8_t byte_val = (_byte_pos < _max_bytes) ? _buffer[_byte_pos] : 0;
			result |= static_cast<uint64_t>((byte_val >> _buffer_bits) & mask) << shift;
			shift += chunk;
			n -= chunk;
			_buffer_bits += chunk;
			if (_buffer_bits == 8) {
				_buffer_bits = 0;
				++_byte_pos;
			}
		}
		return result;
	}

	// Write a single bit
	void write_bit(unsigned bit) { write_bits(bit, 1); }

	// Read a single bit
	unsigned read_bit() { return static_cast<unsigned>(read_bits(1)); }

	// Total bits written/read
	size_t total_bits() const { return _bits; }

	// Reset to beginning for reading
	void rewind() { _bits = 0; _buffer_bits = 0; _byte_pos = 0; }

	// Flush any partial byte (for writing)
	void flush() {
		if (_buffer_bits > 0) {
			_buffer_bits = 0;
			++_byte_pos;
		}
	}

	// Bytes used (rounded up)
	size_t bytes_used() const { return (_bits + 7) / 8; }

private:
	uint8_t* _buffer;
	size_t   _max_bytes;
	size_t   _bits;         // total bits read/written
	unsigned _buffer_bits;  // bits consumed in current byte
	size_t   _byte_pos;     // current byte position
};

// ============================================================
// Negabinary conversion
// ============================================================

template<typename Int, typename UInt>
inline UInt int2uint(Int x) {
	if constexpr (sizeof(Int) == 4) {
		return (static_cast<UInt>(x) + static_cast<UInt>(0xAAAAAAAAu)) ^ static_cast<UInt>(0xAAAAAAAAu);
	} else {
		return (static_cast<UInt>(x) + static_cast<UInt>(0xAAAAAAAAAAAAAAAAull)) ^ static_cast<UInt>(0xAAAAAAAAAAAAAAAAull);
	}
}

template<typename Int, typename UInt>
inline Int uint2int(UInt x) {
	if constexpr (sizeof(Int) == 4) {
		return static_cast<Int>((x ^ static_cast<UInt>(0xAAAAAAAAu)) - static_cast<UInt>(0xAAAAAAAAu));
	} else {
		return static_cast<Int>((x ^ static_cast<UInt>(0xAAAAAAAAAAAAAAAAull)) - static_cast<UInt>(0xAAAAAAAAAAAAAAAAull));
	}
}

// ============================================================
// Block-float conversion (fwd_cast / inv_cast)
// ============================================================

// Forward: float block → integer block, returns shared exponent
template<typename Real, size_t N>
inline int fwd_cast(const Real* fblock, typename zfp_type_traits<Real>::Int* iblock) {
	using Traits = zfp_type_traits<Real>;
	using Int = typename Traits::Int;
	constexpr int prec = Traits::frac_bits;

	// find maximum exponent
	int emax = -INT_MAX;
	for (size_t i = 0; i < N; ++i) {
		if (fblock[i] != Real(0)) {
			int e;
			std::frexp(fblock[i], &e);
			if (e > emax) emax = e;
		}
	}

	if (emax == -INT_MAX) {
		// all zeros
		std::memset(iblock, 0, N * sizeof(Int));
		return 0;
	}

	// quantize: iblock[i] = (Int)(fblock[i] * 2^(prec - emax))
	for (size_t i = 0; i < N; ++i) {
		iblock[i] = static_cast<Int>(std::ldexp(fblock[i], prec - emax));
	}
	return emax;
}

// Inverse: integer block → float block using shared exponent
template<typename Real, size_t N>
inline void inv_cast(const typename zfp_type_traits<Real>::Int* iblock, Real* fblock, int emax) {
	using Traits = zfp_type_traits<Real>;
	constexpr int prec = Traits::frac_bits;

	for (size_t i = 0; i < N; ++i) {
		fblock[i] = std::ldexp(static_cast<Real>(iblock[i]), emax - prec);
	}
}

// ============================================================
// Lifting transforms
// ============================================================

// Forward lifting: 4-point decorrelating transform (in-place, strided)
template<typename Int>
inline void fwd_lift(Int* p, unsigned s) {
	Int x = p[0*s], y = p[1*s], z = p[2*s], w = p[3*s];

	// TN (a]er for sub-band decomposition
	x += w; x >>= 1; w -= x;
	z += y; z >>= 1; y -= z;
	x += z; x >>= 1; z -= x;
	w += y; w >>= 1; y -= w;
	w += y >> 1; y -= w >> 1;

	p[0*s] = x; p[1*s] = y; p[2*s] = z; p[3*s] = w;
}

// Inverse lifting: undo 4-point transform (in-place, strided)
// Uses `a -= b - a` pattern to avoid signed left-shift UB
template<typename Int>
inline void inv_lift(Int* p, unsigned s) {
	Int x = p[0*s], y = p[1*s], z = p[2*s], w = p[3*s];

	y += w >> 1; w -= y >> 1;
	y += w; w -= y - w;     // undo w += y; w >>= 1; y -= w;
	z += x; x -= z - x;     // undo x += z; x >>= 1; z -= x;
	y += z; z -= y - z;     // undo z += y; z >>= 1; y -= z;
	w += x; x -= w - x;     // undo x += w; x >>= 1; w -= x;

	p[0*s] = x; p[1*s] = y; p[2*s] = z; p[3*s] = w;
}

// Forward multi-dimensional transform (separable)
template<typename Int, unsigned Dim>
inline void fwd_xform(Int* iblock) {
	if constexpr (Dim == 1) {
		fwd_lift(iblock, 1);
	} else if constexpr (Dim == 2) {
		// rows (stride 1)
		for (unsigned y = 0; y < 4; ++y)
			fwd_lift(iblock + 4 * y, 1);
		// columns (stride 4)
		for (unsigned x = 0; x < 4; ++x)
			fwd_lift(iblock + x, 4);
	} else if constexpr (Dim == 3) {
		// x-direction (stride 1)
		for (unsigned z = 0; z < 4; ++z)
			for (unsigned y = 0; y < 4; ++y)
				fwd_lift(iblock + 4 * y + 16 * z, 1);
		// y-direction (stride 4)
		for (unsigned z = 0; z < 4; ++z)
			for (unsigned x = 0; x < 4; ++x)
				fwd_lift(iblock + x + 16 * z, 4);
		// z-direction (stride 16)
		for (unsigned y = 0; y < 4; ++y)
			for (unsigned x = 0; x < 4; ++x)
				fwd_lift(iblock + x + 4 * y, 16);
	}
}

// Inverse multi-dimensional transform (separable, reverse order)
template<typename Int, unsigned Dim>
inline void inv_xform(Int* iblock) {
	if constexpr (Dim == 1) {
		inv_lift(iblock, 1);
	} else if constexpr (Dim == 2) {
		// columns first (stride 4)
		for (unsigned x = 0; x < 4; ++x)
			inv_lift(iblock + x, 4);
		// rows (stride 1)
		for (unsigned y = 0; y < 4; ++y)
			inv_lift(iblock + 4 * y, 1);
	} else if constexpr (Dim == 3) {
		// z-direction (stride 16)
		for (unsigned y = 0; y < 4; ++y)
			for (unsigned x = 0; x < 4; ++x)
				inv_lift(iblock + x + 4 * y, 16);
		// y-direction (stride 4)
		for (unsigned z = 0; z < 4; ++z)
			for (unsigned x = 0; x < 4; ++x)
				inv_lift(iblock + x + 16 * z, 4);
		// x-direction (stride 1)
		for (unsigned z = 0; z < 4; ++z)
			for (unsigned y = 0; y < 4; ++y)
				inv_lift(iblock + 4 * y + 16 * z, 1);
	}
}

// ============================================================
// Bit-plane encoding / decoding
// ============================================================

// Encode bit-planes from unsigned integer coefficients
// Returns number of bits written
template<typename UInt, size_t N>
inline size_t encode_bitplanes(zfp_bitstream& stream, const UInt* ublock,
                               unsigned maxprec, size_t maxbits) {
	size_t start_bits = stream.total_bits();
	constexpr unsigned intprec = CHAR_BIT * sizeof(UInt);

	// Process bit planes from MSB to LSB
	uint64_t sig = 0;  // significance bitmask: bit i = 1 if coefficient i is significant
	for (unsigned k = intprec; k-- > 0 && stream.total_bits() - start_bits < maxbits; ) {
		if (k >= maxprec) continue;

		// extract bit k from all N coefficients
		uint64_t plane = 0;
		for (size_t i = 0; i < N; ++i) {
			plane |= static_cast<uint64_t>((ublock[i] >> k) & 1u) << i;
		}

		// encode bits for previously-significant coefficients (known to have data)
		uint64_t known = sig;
		while (known != 0) {
			if (stream.total_bits() - start_bits >= maxbits) return stream.total_bits() - start_bits;
			unsigned i = zfp_ctzll(known);  // lowest set bit
			stream.write_bit(static_cast<unsigned>((plane >> i) & 1u));
			known &= known - 1;  // clear lowest set bit
		}

		// encode bits for not-yet-significant coefficients using group testing
		uint64_t unsig = plane & ~sig;
		uint64_t remaining = ~sig & ((N < 64) ? ((uint64_t(1) << N) - 1) : ~uint64_t(0));
		while (remaining != 0) {
			if (stream.total_bits() - start_bits >= maxbits) return stream.total_bits() - start_bits;
			if (unsig != 0) {
				stream.write_bit(1);  // at least one newly significant coefficient
				// find it by scanning
				unsigned i = zfp_ctzll(remaining);
				if (stream.total_bits() - start_bits >= maxbits) return stream.total_bits() - start_bits;
				if ((unsig >> i) & 1u) {
					// this coefficient is newly significant
					stream.write_bit(1);
					sig |= uint64_t(1) << i;
					// encode sign bit
					// For negabinary, the "sign" is embedded in the coding;
					// we don't need a separate sign bit here since negabinary already handles it.
					unsig &= ~(uint64_t(1) << i);
				} else {
					stream.write_bit(0);
				}
				remaining &= ~(uint64_t(1) << i);
			} else {
				stream.write_bit(0);  // no more newly significant coefficients on this plane
				break;
			}
		}

		// update significance mask
		sig |= plane;  // all coefficients with bit k set are now significant
	}
	return stream.total_bits() - start_bits;
}

// Decode bit-planes to unsigned integer coefficients
// Returns number of bits read
template<typename UInt, size_t N>
inline size_t decode_bitplanes(zfp_bitstream& stream, UInt* ublock,
                               unsigned maxprec, size_t maxbits) {
	size_t start_bits = stream.total_bits();
	constexpr unsigned intprec = CHAR_BIT * sizeof(UInt);

	std::memset(ublock, 0, N * sizeof(UInt));

	uint64_t sig = 0;
	for (unsigned k = intprec; k-- > 0 && stream.total_bits() - start_bits < maxbits; ) {
		if (k >= maxprec) continue;

		// decode bits for previously-significant coefficients
		uint64_t known = sig;
		while (known != 0) {
			if (stream.total_bits() - start_bits >= maxbits) return stream.total_bits() - start_bits;
			unsigned i = zfp_ctzll(known);
			unsigned bit = stream.read_bit();
			ublock[i] |= static_cast<UInt>(bit) << k;
			known &= known - 1;
		}

		// decode group-tested coefficients
		uint64_t remaining = ~sig & ((N < 64) ? ((uint64_t(1) << N) - 1) : ~uint64_t(0));
		while (remaining != 0) {
			if (stream.total_bits() - start_bits >= maxbits) return stream.total_bits() - start_bits;
			unsigned has_new = stream.read_bit();
			if (has_new) {
				unsigned i = zfp_ctzll(remaining);
				if (stream.total_bits() - start_bits >= maxbits) return stream.total_bits() - start_bits;
				unsigned is_sig = stream.read_bit();
				if (is_sig) {
					sig |= uint64_t(1) << i;
					ublock[i] |= static_cast<UInt>(1) << k;
				}
				remaining &= ~(uint64_t(1) << i);
			} else {
				break;
			}
		}

		sig |= sig;  // already updated above
	}
	return stream.total_bits() - start_bits;
}

// ============================================================
// Full encode/decode pipeline
// ============================================================

// Encode a block of 4^Dim floating-point values
// Returns total number of bits written
template<typename Real, unsigned Dim>
inline size_t encode_block(const Real* fblock, uint8_t* buffer, size_t max_bytes,
                           unsigned maxprec, size_t maxbits) {
	using Traits = zfp_type_traits<Real>;
	using Int = typename Traits::Int;
	using UInt = typename Traits::UInt;
	constexpr size_t N = zfp_block_size<Dim>::value;

	zfp_bitstream stream(buffer, max_bytes);

	// Step 1: block-float conversion
	Int iblock[N];
	int emax = fwd_cast<Real, N>(fblock, iblock);

	// Check for all-zero block
	bool all_zero = true;
	for (size_t i = 0; i < N; ++i) {
		if (iblock[i] != 0) { all_zero = false; break; }
	}

	// Minimum header size: 1 (zero flag) + ebits (exponent) for nonzero blocks
	constexpr size_t min_header_bits = 1 + Traits::ebits;

	if (all_zero || maxbits < min_header_bits) {
		// All-zero block or insufficient bits for header: encode as zero block
		stream.write_bit(0);
		// pad to maxbits if fixed-rate
		while (stream.total_bits() < maxbits) {
			stream.write_bit(0);
		}
		return stream.total_bits();
	}

	// Write nonzero indicator
	stream.write_bit(1);

	// Write biased exponent
	unsigned biased_emax = static_cast<unsigned>(emax + Traits::ebias);
	stream.write_bits(biased_emax, Traits::ebits);

	// Step 2: forward lifting transform
	fwd_xform<Int, Dim>(iblock);

	// Step 3: reorder by total sequency
	Int ordered[N];
	const auto& perm = zfp_perm<Dim>();
	for (size_t i = 0; i < N; ++i) {
		ordered[i] = iblock[perm[i]];
	}

	// Step 4: convert to negabinary (unsigned)
	UInt ublock[N];
	for (size_t i = 0; i < N; ++i) {
		ublock[i] = int2uint<Int, UInt>(ordered[i]);
	}

	// Step 5: bit-plane encode
	size_t header_bits = stream.total_bits();
	size_t data_maxbits = (maxbits > header_bits) ? (maxbits - header_bits) : 0;
	encode_bitplanes<UInt, N>(stream, ublock, maxprec, data_maxbits);

	// pad to maxbits if fixed-rate
	while (stream.total_bits() < maxbits) {
		stream.write_bit(0);
	}

	return stream.total_bits();
}

// Decode a block of 4^Dim floating-point values
// Returns total number of bits read
template<typename Real, unsigned Dim>
inline size_t decode_block(const uint8_t* buffer, size_t max_bytes,
                           Real* fblock,
                           unsigned maxprec, size_t maxbits) {
	using Traits = zfp_type_traits<Real>;
	using Int = typename Traits::Int;
	using UInt = typename Traits::UInt;
	constexpr size_t N = zfp_block_size<Dim>::value;

	zfp_bitstream stream(const_cast<uint8_t*>(buffer), max_bytes);

	// Read zero indicator
	unsigned nonzero = stream.read_bit();
	if (!nonzero) {
		// all-zero block
		for (size_t i = 0; i < N; ++i) fblock[i] = Real(0);
		return maxbits > 0 ? maxbits : 1;
	}

	// Read biased exponent
	unsigned biased_emax = static_cast<unsigned>(stream.read_bits(Traits::ebits));
	int emax = static_cast<int>(biased_emax) - Traits::ebias;

	// Step 5 (inverse): bit-plane decode
	UInt ublock[N];
	size_t header_bits = stream.total_bits();
	size_t data_maxbits = (maxbits > header_bits) ? (maxbits - header_bits) : 0;
	decode_bitplanes<UInt, N>(stream, ublock, maxprec, data_maxbits);

	// Step 4 (inverse): negabinary to signed
	Int ordered[N];
	for (size_t i = 0; i < N; ++i) {
		ordered[i] = uint2int<Int, UInt>(ublock[i]);
	}

	// Step 3 (inverse): inverse reorder
	Int iblock[N];
	const auto& perm = zfp_perm<Dim>();
	for (size_t i = 0; i < N; ++i) {
		iblock[perm[i]] = ordered[i];
	}

	// Step 2 (inverse): inverse lifting transform
	inv_xform<Int, Dim>(iblock);

	// Step 1 (inverse): integer to float
	inv_cast<Real, N>(iblock, fblock, emax);

	return maxbits > 0 ? maxbits : stream.total_bits();
}

}} // namespace sw::universal
