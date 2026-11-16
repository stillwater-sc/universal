#pragma once
// zfp_codec.hpp: ZFP block codec -- transform, encoding, and decoding pipeline
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Implements LLNL ZFP's single-block codec:
//   float[4^d] -> block-float -> lifting -> reorder -> negabinary -> bit-plane -> bits
//
// All public entry points are usable in constant-evaluated contexts: the
// arithmetic helpers, the bit-stream packer, and the orchestrators
// (encode_block / decode_block) are constexpr.  Stdlib intrinsics that are
// not constexpr (std::frexp / std::ldexp / std::memset / __builtin_ctzll)
// are guarded by std::is_constant_evaluated() and replaced with
// constexpr-safe equivalents (cx_ldexp / cx_frexp_exp / bit-cast / loops /
// std::countr_zero) on the constant-evaluation branch.  See PR #815.

#include <cstdint>
#include <cstddef>
#include <cmath>
#include <cstring>
#include <climits>
#include <algorithm>
#include <array>
#include <bit>
#include <limits>
#include <type_traits>

#include <universal/utility/bit_cast.hpp>
#include <universal/number/zfpblock/zfp_codec_traits.hpp>

namespace sw { namespace universal {

// Compute a bitmask of the lowest N bits of a uint64_t without
// undefined behavior when N == 64 (shifting by the bit-width is UB).
template<size_t N>
constexpr uint64_t zfp_lowbits_mask() {
	if constexpr (N >= 64) return ~uint64_t(0);
	else return (uint64_t(1) << N) - 1;
}

// Portable constexpr count-trailing-zeros for uint64_t.
// Returns the index of the lowest set bit (0..63).  Undefined if x == 0.
// std::countr_zero is constexpr in C++20.
constexpr unsigned zfp_ctzll(uint64_t x) {
	return static_cast<unsigned>(std::countr_zero(x));
}

// ============================================================
// Constexpr-safe replacements for std::frexp / std::ldexp.
//
// These are used only on the constant-evaluation branch of
// fwd_cast / inv_cast; the runtime branch keeps the stdlib calls
// for speed.  Both helpers are templated on Real in { float, double }.
// ============================================================

// cx_ldexp: x * 2^exp via a power-of-2 multiplication loop.
// The codec's emax is bounded by Real's exponent range, so the loop
// count is at most ~150 even for double -- well within constexpr
// step limits.  Power-of-two multiplications are exact in IEEE 754
// (until under/overflow), so the result is bit-identical to std::ldexp
// across the domain the codec actually exercises.
template<typename Real>
constexpr Real cx_ldexp(Real x, int exp) noexcept {
	Real two  = static_cast<Real>(2);
	Real half = static_cast<Real>(0.5);
	if (exp >= 0) {
		for (int i = 0; i < exp; ++i) x *= two;
	} else {
		for (int i = 0; i < -exp; ++i) x *= half;
	}
	return x;
}

// cx_frexp_exp: returns the exponent that std::frexp would write to its
// out-parameter for value v -- that is, v = frac * 2^exp with frac in
// [0.5, 1.0) for finite nonzero v.  Caller must check v != 0 before
// calling.  Inf/NaN are out of contract (only finite values matter for
// fwd_cast's emax search).
//
// Implementation: extract the IEEE 754 biased-exponent field via
// sw::bit_cast.  For a normal float, frexp_exp = raw_exp - 126
// (because frexp(1.0) returns e=1, and 1.0 has raw_exp=127).  For
// subnormals the leading bit of the fraction is scanned to determine
// the effective exponent.
template<typename Real>
constexpr int cx_frexp_exp(Real v) noexcept {
	static_assert(std::is_same_v<Real, float> || std::is_same_v<Real, double>,
		"cx_frexp_exp supports float and double only");
	if constexpr (std::is_same_v<Real, float>) {
		static_assert(sizeof(float) == sizeof(uint32_t) && std::numeric_limits<float>::is_iec559,
			"cx_frexp_exp<float> requires IEEE 754 binary32");
		uint32_t bits = sw::bit_cast<uint32_t>(v);
		int raw_exp = static_cast<int>((bits >> 23) & 0xFFu);
		uint32_t frac = bits & 0x7FFFFFu;
		if (raw_exp == 0) {
			// subnormal: leading-bit scan to find the highest set bit
			// of the 23-bit fraction, mapping to frexp's exponent via
			//   exp = k - 148   where k is the bit position (0..22),
			//   shift = 22 - k  so   exp = -126 - shift.
			int shift = 0;
			if (frac == 0u) return 0;  // value == 0 (out of contract; defensive)
			while ((frac & 0x400000u) == 0u && shift < 22) {
				frac <<= 1;
				++shift;
			}
			return -126 - shift;
		}
		// normal (and inf/nan -- treated as normal; result unused by callers)
		return raw_exp - 126;
	} else {
		static_assert(sizeof(double) == sizeof(uint64_t) && std::numeric_limits<double>::is_iec559,
			"cx_frexp_exp<double> requires IEEE 754 binary64");
		uint64_t bits = sw::bit_cast<uint64_t>(v);
		int raw_exp = static_cast<int>((bits >> 52) & 0x7FFull);
		uint64_t frac = bits & 0xFFFFFFFFFFFFFull;
		if (raw_exp == 0) {
			// subnormal: same scheme as the float branch but with
			// 52-bit fraction and double's bias.
			//   exp = k - 1073   where k is bit position (0..51),
			//   shift = 51 - k   so   exp = -1022 - shift.
			int shift = 0;
			if (frac == 0u) return 0;
			while ((frac & (uint64_t(1) << 51)) == 0u && shift < 51) {
				frac <<= 1;
				++shift;
			}
			return -1022 - shift;
		}
		return raw_exp - 1022;
	}
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
constexpr const auto& zfp_perm() {
	if constexpr (Dim == 1) return zfp_perm_1d;
	else if constexpr (Dim == 2) return zfp_perm_2d;
	else return zfp_perm_3d;
}

// ============================================================
// Bitstream: in-memory bit-level reader/writer (LSB-first)
//
// Two construction modes:
//   * writer ctor takes uint8_t* (mutable buffer); supports write_*
//     and read_* operations
//   * reader ctor takes const uint8_t* (read-only buffer); supports
//     only read_* operations.  This eliminates the const_cast that
//     would otherwise be required in decode_block, which is forbidden
//     in constant-evaluated contexts when the storage is const-qualified.
//
// Read operations always go through the const _read_buffer pointer
// (the writer ctor mirrors its buffer into both _write_buffer and
// _read_buffer so reads work in either mode).  Write operations go
// through _write_buffer; calling write_* on a stream constructed via
// the reader ctor is a contract violation (and harmlessly no-ops, to
// keep the constexpr path clean of throws).
// ============================================================

class zfp_bitstream {
public:
	// Writer constructor: buffer is read/write
	constexpr zfp_bitstream(uint8_t* buffer, size_t max_bytes)
		: _write_buffer(buffer), _read_buffer(buffer),
		  _max_bytes(max_bytes), _bits(0), _buffer_bits(0), _byte_pos(0) {}

	// Reader constructor: buffer is read-only
	constexpr zfp_bitstream(const uint8_t* buffer, size_t max_bytes)
		: _write_buffer(nullptr), _read_buffer(buffer),
		  _max_bytes(max_bytes), _bits(0), _buffer_bits(0), _byte_pos(0) {}

	// Write 'n' bits from value (LSB first), n <= 64.
	// True no-op in reader mode -- early-returns before any state is modified
	// (including _bits), so an accidental write_* on a reader stream cannot
	// desync subsequent reads.
	constexpr void write_bits(uint64_t value, unsigned n) {
		if (_write_buffer == nullptr) return;  // reader mode: true no-op
		_bits += n;
		while (n > 0) {
			unsigned space = 8 - _buffer_bits;
			unsigned chunk = (n < space) ? n : space;
			uint8_t mask = static_cast<uint8_t>((1u << chunk) - 1);
			if (_byte_pos < _max_bytes) {
				if (_buffer_bits == 0) _write_buffer[_byte_pos] = 0;
				_write_buffer[_byte_pos] |= static_cast<uint8_t>((value & mask) << _buffer_bits);
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
	constexpr uint64_t read_bits(unsigned n) {
		_bits += n;
		uint64_t result = 0;
		unsigned shift = 0;
		while (n > 0) {
			unsigned avail = 8 - _buffer_bits;
			unsigned chunk = (n < avail) ? n : avail;
			uint8_t mask = static_cast<uint8_t>((1u << chunk) - 1);
			uint8_t byte_val = (_byte_pos < _max_bytes) ? _read_buffer[_byte_pos] : uint8_t(0);
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
	constexpr void write_bit(unsigned bit) { write_bits(bit, 1); }

	// Read a single bit
	constexpr unsigned read_bit() { return static_cast<unsigned>(read_bits(1)); }

	// Total bits written/read
	constexpr size_t total_bits() const { return _bits; }

	// Reset to beginning for reading
	constexpr void rewind() { _bits = 0; _buffer_bits = 0; _byte_pos = 0; }

	// Flush any partial byte (for writing)
	constexpr void flush() {
		if (_buffer_bits > 0) {
			_buffer_bits = 0;
			++_byte_pos;
		}
	}

	// Bytes used (rounded up)
	constexpr size_t bytes_used() const { return (_bits + 7) / 8; }

private:
	uint8_t*       _write_buffer;  // nullptr in reader mode
	const uint8_t* _read_buffer;   // always non-null
	size_t   _max_bytes;
	size_t   _bits;         // total bits read/written
	unsigned _buffer_bits;  // bits consumed in current byte
	size_t   _byte_pos;     // current byte position
};

// ============================================================
// Negabinary conversion
// ============================================================

template<typename Int, typename UInt>
constexpr UInt int2uint(Int x) {
	if constexpr (sizeof(Int) == 4) {
		return (static_cast<UInt>(x) + static_cast<UInt>(0xAAAAAAAAu)) ^ static_cast<UInt>(0xAAAAAAAAu);
	} else {
		return (static_cast<UInt>(x) + static_cast<UInt>(0xAAAAAAAAAAAAAAAAull)) ^ static_cast<UInt>(0xAAAAAAAAAAAAAAAAull);
	}
}

template<typename Int, typename UInt>
constexpr Int uint2int(UInt x) {
	if constexpr (sizeof(Int) == 4) {
		return static_cast<Int>((x ^ static_cast<UInt>(0xAAAAAAAAu)) - static_cast<UInt>(0xAAAAAAAAu));
	} else {
		return static_cast<Int>((x ^ static_cast<UInt>(0xAAAAAAAAAAAAAAAAull)) - static_cast<UInt>(0xAAAAAAAAAAAAAAAAull));
	}
}

// ============================================================
// Block-float conversion (fwd_cast / inv_cast)
//
// The only places in the codec where Real-typed arithmetic happens.
// At constant evaluation we substitute cx_frexp_exp / cx_ldexp for
// std::frexp / std::ldexp, and replace the all-zero std::memset with
// an explicit loop.
// ============================================================

// Forward: float block -> integer block, returns shared exponent
template<typename Real, size_t N>
constexpr int fwd_cast(const Real* fblock, typename zfp_type_traits<Real>::Int* iblock) {
	using Traits = zfp_type_traits<Real>;
	using Int = typename Traits::Int;
	constexpr int prec = Traits::frac_bits;

	// find maximum exponent
	int emax = -INT_MAX;
	for (size_t i = 0; i < N; ++i) {
		if (fblock[i] != Real(0)) {
			int e;
			if (std::is_constant_evaluated()) {
				e = cx_frexp_exp<Real>(fblock[i]);
			} else {
				std::frexp(fblock[i], &e);
			}
			if (e > emax) emax = e;
		}
	}

	if (emax != -INT_MAX && emax < 1 - Traits::ebias) {
		// subnormal magnitudes: the block exponent is clamped at the format's smallest, as the ZFP
		// reference does, so that the biased exponent stays positive. Unclamped it wrapped in ebits,
		// and a float block of 1e-40 decoded as 1.16e37 (#2097).
		emax = 1 - Traits::ebias;
	}

	if (emax == -INT_MAX) {
		// all zeros
		if (std::is_constant_evaluated()) {
			for (size_t i = 0; i < N; ++i) iblock[i] = Int(0);
		} else {
			std::memset(iblock, 0, N * sizeof(Int));
		}
		return 0;
	}

	// quantize: iblock[i] = (Int)(fblock[i] * 2^(prec - emax))
	for (size_t i = 0; i < N; ++i) {
		if (std::is_constant_evaluated()) {
			iblock[i] = static_cast<Int>(cx_ldexp<Real>(fblock[i], prec - emax));
		} else {
			iblock[i] = static_cast<Int>(std::ldexp(fblock[i], prec - emax));
		}
	}
	return emax;
}

// Inverse: integer block -> float block using shared exponent
template<typename Real, size_t N>
constexpr void inv_cast(const typename zfp_type_traits<Real>::Int* iblock, Real* fblock, int emax) {
	using Traits = zfp_type_traits<Real>;
	constexpr int prec = Traits::frac_bits;

	for (size_t i = 0; i < N; ++i) {
		if (std::is_constant_evaluated()) {
			fblock[i] = cx_ldexp<Real>(static_cast<Real>(iblock[i]), emax - prec);
		} else {
			fblock[i] = std::ldexp(static_cast<Real>(iblock[i]), emax - prec);
		}
	}
}

// ============================================================
// Lifting transforms
// ============================================================

// Forward lifting: 4-point decorrelating transform (in-place, strided)
template<typename Int>
constexpr void fwd_lift(Int* p, unsigned s) {
	Int x = p[0*s], y = p[1*s], z = p[2*s], w = p[3*s];

	// TN (filter) for sub-band decomposition
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
constexpr void inv_lift(Int* p, unsigned s) {
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
constexpr void fwd_xform(Int* iblock) {
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
constexpr void inv_xform(Int* iblock) {
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
constexpr size_t encode_bitplanes(zfp_bitstream& stream, const UInt* ublock,
                                  unsigned maxprec, size_t maxbits) {
	size_t start_bits = stream.total_bits();
	constexpr unsigned intprec = CHAR_BIT * sizeof(UInt);

	// Process bit planes from MSB to LSB: the top maxprec planes, k >= kmin, as ZFP codes them.
	// The test was k >= maxprec, which skipped the top planes and coded the bottom ones, so a fixed
	// precision dropped every significant bit (#2097).
	const unsigned kmin = (intprec > maxprec) ? intprec - maxprec : 0u;
	uint64_t sig = 0;  // significance bitmask: bit i = 1 if coefficient i is significant
	for (unsigned k = intprec; k-- > kmin && stream.total_bits() - start_bits < maxbits; ) {

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
		uint64_t remaining = ~sig & zfp_lowbits_mask<N>();
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
constexpr size_t decode_bitplanes(zfp_bitstream& stream, UInt* ublock,
                                  unsigned maxprec, size_t maxbits) {
	size_t start_bits = stream.total_bits();
	constexpr unsigned intprec = CHAR_BIT * sizeof(UInt);

	if (std::is_constant_evaluated()) {
		for (size_t i = 0; i < N; ++i) ublock[i] = UInt(0);
	} else {
		std::memset(ublock, 0, N * sizeof(UInt));
	}

	const unsigned kmin = (intprec > maxprec) ? intprec - maxprec : 0u;   // the top maxprec planes (#2097)
	uint64_t sig = 0;
	for (unsigned k = intprec; k-- > kmin && stream.total_bits() - start_bits < maxbits; ) {

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
		uint64_t remaining = ~sig & zfp_lowbits_mask<N>();
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

	}
	return stream.total_bits() - start_bits;
}

// ============================================================
// Full encode/decode pipeline
// ============================================================

// ------------------------------------------------------------
// Block header (#2097)
//
//   bit 0         0: an all-zero block, nothing follows
//                 1: a coded or a raw block, the biased exponent follows
//   ebits bits    the block exponent + ebias, in [1, 2^ebits - 1] for a coded block (fwd_cast clamps
//                 emax at 1 - ebias); 0 marks a raw block
//   raw block     the N IEEE bit patterns, verbatim: a block with an inf or NaN, which the lossy
//                 pipeline cannot represent, and a reversible block the pipeline does not reproduce
// ------------------------------------------------------------

// the size of a raw block: the header and every value's bit pattern
template<typename Real, unsigned Dim>
constexpr size_t zfp_raw_block_bits() {
	return 1u + zfp_type_traits<Real>::ebits + zfp_block_size<Dim>::value * (CHAR_BIT * sizeof(Real));
}

// the bits a fixed rate gives a block.  A rate below the header + 4 bits (13 bits for float, 16 for
// double), a negative rate and a NaN all clamp up to it: below the header every block was silently
// encoded as zero (#2097). Above the floor a rate keeps its exact size; a block can still decode to
// zero at a very low rate, as in the ZFP reference: a bit plane costs up to 2N bits, so a nonzero
// result needs up to header + 2N + 5 (142 bits for a 3D float block), and clamping to that would
// enlarge the common 3D rates of 1-2 bits per value (decided on #2097). A rate past the raw size is
// capped there, the most any block takes.
template<typename Real, unsigned Dim>
constexpr size_t zfp_fixed_rate_bits(double rate) {
	constexpr size_t N = zfp_block_size<Dim>::value;
	constexpr size_t min_bits = 1u + zfp_type_traits<Real>::ebits + 4u;
	constexpr size_t max_bits = zfp_raw_block_bits<Real, Dim>();
	const double bits = rate * static_cast<double>(N);
	if (!(bits >= static_cast<double>(min_bits))) return min_bits;   // NaN fails the comparison too
	if (bits >= static_cast<double>(max_bits)) return max_bits;
	return static_cast<size_t>(bits);
}

// the bit planes a block codes: maxprec, or for a fixed accuracy, as many as the tolerance needs.
// ZFP's rule: emax - minexp + 2 (Dim + 1) planes keep the error below 2^minexp (#2097).
template<unsigned Dim>
constexpr unsigned zfp_block_precision(int emax, unsigned maxprec, int minexp) {
	if (minexp == INT_MIN) return maxprec;
	const long long p = static_cast<long long>(emax) - static_cast<long long>(minexp) + 2ll * (Dim + 1);
	if (p <= 0) return 0;
	return (p < static_cast<long long>(maxprec)) ? static_cast<unsigned>(p) : maxprec;
}

// a block with an inf or a NaN
template<typename Real, size_t N>
constexpr bool zfp_has_nonfinite(const Real* fblock) {
	for (size_t i = 0; i < N; ++i) {
		const Real v = fblock[i];
		if (v != v || v == std::numeric_limits<Real>::infinity() || v == -std::numeric_limits<Real>::infinity()) return true;
	}
	return false;
}

// write zero bits until the stream holds `bits`, a word at a time
constexpr void zfp_pad_to(zfp_bitstream& stream, size_t bits) {
	while (stream.total_bits() < bits) {
		const size_t left = bits - stream.total_bits();
		stream.write_bits(0, static_cast<unsigned>(left < 64 ? left : 64));
	}
}

// Encode a block raw: the header with exponent field 0, then each value's bit pattern. When
// maxbits cannot hold it (a fixed rate too small), only the header is written, and the block
// decodes as all NaN. Returns the bits written.
template<typename Real, unsigned Dim>
constexpr size_t encode_raw_block(const Real* fblock, uint8_t* buffer, size_t max_bytes, size_t maxbits, bool pad) {
	using Traits = zfp_type_traits<Real>;
	using UInt = typename Traits::UInt;
	constexpr size_t N = zfp_block_size<Dim>::value;
	zfp_bitstream stream(buffer, max_bytes);
	stream.write_bit(1);
	stream.write_bits(0, Traits::ebits);
	if (maxbits >= zfp_raw_block_bits<Real, Dim>()) {
		for (size_t i = 0; i < N; ++i) stream.write_bits(static_cast<uint64_t>(std::bit_cast<UInt>(fblock[i])), CHAR_BIT * sizeof(Real));
	}
	if (pad) zfp_pad_to(stream, maxbits);
	return stream.total_bits();
}

// Encode a block of 4^Dim floating-point values
// Returns total number of bits written.  minexp (INT_MIN: none) is the fixed-accuracy floor; pad
// fills the block to maxbits, which only a fixed rate needs (#2097: the other modes were padded to
// their worst case, 137 bits for 4 floats).
template<typename Real, unsigned Dim>
constexpr size_t encode_block(const Real* fblock, uint8_t* buffer, size_t max_bytes,
                              unsigned maxprec, size_t maxbits, int minexp = INT_MIN, bool pad = true) {
	using Traits = zfp_type_traits<Real>;
	using Int = typename Traits::Int;
	using UInt = typename Traits::UInt;
	constexpr size_t N = zfp_block_size<Dim>::value;

	// inf and NaN have no block-floating-point form: they were cast to integers, which is undefined,
	// and {inf, 1, 2, 3} decoded as {-8, 1, 2, 3} (#2097). Such a block is stored raw.
	if (zfp_has_nonfinite<Real, N>(fblock)) return encode_raw_block<Real, Dim>(fblock, buffer, max_bytes, maxbits, pad);

	zfp_bitstream stream(buffer, max_bytes);

	// Step 1: block-float conversion
	Int iblock[N]{};
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
		if (pad) zfp_pad_to(stream, maxbits);
		return stream.total_bits();
	}

	// Write nonzero indicator
	stream.write_bit(1);

	// Write biased exponent: at least 1, since fwd_cast clamps emax; 0 marks a raw block
	unsigned biased_emax = static_cast<unsigned>(emax + Traits::ebias);
	stream.write_bits(biased_emax, Traits::ebits);

	// Step 2: forward lifting transform
	fwd_xform<Int, Dim>(iblock);

	// Step 3: reorder by total sequency
	Int ordered[N]{};
	const auto& perm = zfp_perm<Dim>();
	for (size_t i = 0; i < N; ++i) {
		ordered[i] = iblock[perm[i]];
	}

	// Step 4: convert to negabinary (unsigned)
	UInt ublock[N]{};
	for (size_t i = 0; i < N; ++i) {
		ublock[i] = int2uint<Int, UInt>(ordered[i]);
	}

	// Step 5: bit-plane encode
	size_t header_bits = stream.total_bits();
	size_t data_maxbits = (maxbits > header_bits) ? (maxbits - header_bits) : 0;
	encode_bitplanes<UInt, N>(stream, ublock, zfp_block_precision<Dim>(emax, maxprec, minexp), data_maxbits);

	if (pad) zfp_pad_to(stream, maxbits);
	return stream.total_bits();
}

// Decode a block of 4^Dim floating-point values
// Returns total number of bits read
template<typename Real, unsigned Dim>
constexpr size_t decode_block(const uint8_t* buffer, size_t max_bytes,
                              Real* fblock,
                              unsigned maxprec, size_t maxbits, int minexp = INT_MIN) {
	using Traits = zfp_type_traits<Real>;
	using Int = typename Traits::Int;
	using UInt = typename Traits::UInt;
	constexpr size_t N = zfp_block_size<Dim>::value;

	// Reader-mode bitstream avoids const_cast (forbidden in constexpr
	// for storage that originated as const-qualified).
	zfp_bitstream stream(buffer, max_bytes);

	// Read zero indicator
	unsigned nonzero = stream.read_bit();
	if (!nonzero) {
		// all-zero block
		for (size_t i = 0; i < N; ++i) fblock[i] = Real(0);
		return maxbits > 0 ? maxbits : 1;
	}

	// Read biased exponent
	unsigned biased_emax = static_cast<unsigned>(stream.read_bits(Traits::ebits));
	if (biased_emax == 0) {
		// a raw block: the bit patterns, or all NaN when the rate could not hold them
		if (maxbits >= zfp_raw_block_bits<Real, Dim>()) {
			for (size_t i = 0; i < N; ++i) fblock[i] = std::bit_cast<Real>(static_cast<UInt>(stream.read_bits(CHAR_BIT * sizeof(Real))));
		}
		else {
			for (size_t i = 0; i < N; ++i) fblock[i] = std::numeric_limits<Real>::quiet_NaN();
		}
		return stream.total_bits();
	}
	int emax = static_cast<int>(biased_emax) - Traits::ebias;

	// Step 5 (inverse): bit-plane decode
	UInt ublock[N]{};
	size_t header_bits = stream.total_bits();
	size_t data_maxbits = (maxbits > header_bits) ? (maxbits - header_bits) : 0;
	decode_bitplanes<UInt, N>(stream, ublock, zfp_block_precision<Dim>(emax, maxprec, minexp), data_maxbits);

	// Step 4 (inverse): negabinary to signed
	Int ordered[N]{};
	for (size_t i = 0; i < N; ++i) {
		ordered[i] = uint2int<Int, UInt>(ublock[i]);
	}

	// Step 3 (inverse): inverse reorder
	Int iblock[N]{};
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
