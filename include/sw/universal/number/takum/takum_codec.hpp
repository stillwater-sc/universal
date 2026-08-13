#pragma once
// takum_codec.hpp: shared field codec for the linear and logarithmic takum formats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The takum specification defines two variants that share an IDENTICAL bit layout
// and differ only in how the decoded (c, m) pair maps to a real number:
//
//   logarithmic takum (arXiv:2404.18603 Def. 2 / arXiv:2408.10594 Def. 1)
//       l = (-1)^S (c + m),   value = (-1)^S sqrt(e)^l
//   linear takum      (arXiv:2404.18603 Def. 8 / arXiv:2408.10594 Def. 2)
//       e = (-1)^S (c + S),   value = [(1 - 3S) + f] 2^e
//
// This header owns everything the two variants have in common:
//
//   [S:1][D:1][R:rbits][C:r][M:p]        r  = dr_to_r(DR)
//                                        p  = maxCharBits - min(r, maxCharBits)
//                                        c  = dr_to_c_bias(DR) + uint(C)
//                                        m  = 2^-p uint(M)   in [0,1)
//
// Each variant supplies only the value map, i.e. real <-> (sign, c, m).  This
// mirrors the libtakum reference implementation, whose single codec.c decodes a
// linear takum by "treat[ing] a linear takum as a logarithmic one: We obtain c
// and m from the logarithmic value l and compute f and e".
//
// The specification fixes rbits = 3 (r in {0..7}, c in [-255,254]); rbits is a
// template parameter here so that Universal can instantiate narrower and wider
// regime fields.  That generalization is purely a codec-level property and is
// therefore shared by both variants.
//
// Representation invariants (see docs/takum-design.md):
//   I1  c_stored_bits + p == maxCharBits, c_stored_bits == min(r, maxCharBits)
//   I2  decode(mag).c in [min_characteristic(), max_characteristic()]
//   I3  decode(mag).M_bits < 2^p            (equivalently m in [0,1))
//   I4  encode_*() never sets the sign bit: magnitude < 2^(nbits-1)
//   I5  encode_exact(decode(mag)) == mag    for mag in [1, 2^(nbits-1))
//   I6  mag1 < mag2  =>  (c1,m1) < (c2,m2)  lexicographically (Prop. 4)
//
// All entry points are constexpr and free of stdlib intrinsics, so they are
// usable in constant-evaluated contexts.
//
// A note on encode_rounded() and wide trailing fields: its scale step
// m * 2^p is exact for every supported p (scaling by a power of two only
// adjusts the exponent), so the function introduces no error of its own even at
// p = 59, which takum_codec<64,3> reaches.  The limit is the input channel, not
// the arithmetic: a double carries at most 53 significant bits, so for p > 53
// it simply cannot express a trailing field whose low p-53 bits are non-zero.
// Callers that need the full width -- native arithmetic, exact round-trips --
// use encode_exact(), which carries M_bits as an integer.

#include <cassert>
#include <cstdint>

namespace sw { namespace universal {

// Status of an encode attempt.  The codec has no storage and cannot set special
// values itself, so it reports saturation to the caller, which owns the bits.
enum class takum_encode_status : uint8_t {
	ok,         // magnitude is a valid encoding of the requested (c, m)
	overflow,   // |value| exceeded the format's largest magnitude -> caller sets maxpos
	underflow   // |value| fell below the format's smallest magnitude -> caller sets zero
};

// Stateless codec shared by takum<> (linear) and takum_log<> (logarithmic).
// Never instantiated; every member is static constexpr.
template<unsigned nbits, unsigned rbits>
struct takum_codec {

	static constexpr unsigned overhead = 2 + rbits;  // S:1 + D:1 + R:rbits

	static_assert(nbits > overhead, "takum requires more bits than the fixed overhead (S + D + R)");
	static_assert(rbits > 0, "takum requires at least 1 regime bit");
	// rbits <= 5 keeps `1ull << (max_r + 1)` (with max_r = 2^rbits - 1) within the
	// [0, 63] shift range required by C++ for uint64_t.  Larger rbits values exist
	// only in degenerate test configurations; both takum specifications use rbits = 3.
	static_assert(rbits <= 5, "takum regime field limited to 5 bits (avoids 1ull << 64+ UB)");
	static_assert(nbits <= 64, "takum codec is limited to 64 bits");

	// DR field geometry
	static constexpr unsigned dr_bits       = 1 + rbits;           // D:1 + R:rbits
	static constexpr unsigned nr_dr_values  = 1u << dr_bits;       // number of DR combinations
	static constexpr unsigned max_r         = (1u << rbits) - 1;   // maximum characteristic bit count
	static constexpr unsigned r_mask        = max_r;               // mask for the R field
	static constexpr unsigned dr_field_mask = (1u << dr_bits) - 1; // mask for the DR field

	// Characteristic bits available for this nbits, i.e. everything after S, D, R.
	// The C and M fields together always occupy exactly this many bits (I1).
	static constexpr unsigned maxCharBits = (nbits > overhead) ? (nbits - overhead) : 0;

	// Mask of the nbits-wide storage word.
	static constexpr uint64_t nbits_mask() noexcept {
		if constexpr (nbits < 64) return (1ull << nbits) - 1;
		else return ~0ull;
	}
	// Mask of the magnitude, i.e. everything below the sign bit.
	static constexpr uint64_t magnitude_mask() noexcept {
		return (1ull << (nbits - 1)) - 1;
	}

	// ------------------------------------------------------------------
	// DR field decode
	// ------------------------------------------------------------------

	// Number of characteristic bits r encoded by the DR field.
	// Pre: dr < nr_dr_values.  Post: result in [0, max_r].
	static constexpr unsigned dr_to_r(unsigned dr) noexcept {
		bool D = (dr >> rbits) & 1;
		unsigned R = dr & r_mask;
		return D ? R : (max_r - R);
	}

	// Characteristic bias for a DR field.
	// Returns int64_t: for rbits up to 5 the positive branch reaches 2^31 - 1 and
	// the negative branch reaches 1 - 2^32, which both exceed int range.
	// Pre: dr < nr_dr_values.
	static constexpr int64_t dr_to_c_bias(unsigned dr) noexcept {
		bool D = (dr >> rbits) & 1;
		unsigned r = dr_to_r(dr);
		// D=1: c_bias = 2^r - 1        (positive range start)
		// D=0: c_bias = -(2^(r+1)) + 1 (negative range start)
		return D ? static_cast<int64_t>((1ull << r) - 1ull)
		         : (1 - static_cast<int64_t>(1ull << (r + 1)));
	}

	// Largest representable characteristic: DR = nr_dr_values - 1 (D=1, R=max_r).
	static constexpr int64_t max_characteristic() noexcept {
		return dr_to_c_bias(nr_dr_values - 1) + static_cast<int64_t>((1ull << max_r) - 1ull);
	}
	// Smallest representable characteristic: DR = 0 (D=0, R=0).
	static constexpr int64_t min_characteristic() noexcept {
		return dr_to_c_bias(0);
	}

	// DR index whose characteristic range contains c.
	// Pre: c in [min_characteristic(), max_characteristic()] -- callers must range
	//      check first; encode_exact() / encode_rounded() do so and report
	//      underflow/overflow rather than relying on a fallback here.
	// Post: result < nr_dr_values and dr_to_c_bias(result) <= c.
	static constexpr unsigned find_dr(int64_t c) noexcept {
		// Scan down to dr == 1; dr == 0 carries the lowest bias
		// (dr_to_c_bias(0) == min_characteristic()), so it is the answer for any c
		// that matches no higher DR -- a result, not an unreachable fallback.
		for (unsigned dr = nr_dr_values - 1; dr > 0; --dr) {
			if (c >= dr_to_c_bias(dr)) return dr;
		}
		return 0;
	}

	// ------------------------------------------------------------------
	// Field geometry
	// ------------------------------------------------------------------

	// How the maxCharBits trailing bits split between C and M for a given DR.
	struct field_layout {
		unsigned r;              // characteristic bits the format calls for
		unsigned c_stored_bits;  // characteristic bits that actually fit
		unsigned p;              // trailing (mantissa / fraction) bits
	};

	// Pre: dr < nr_dr_values.
	// Post: I1 -- c_stored_bits + p == maxCharBits, c_stored_bits == min(r, maxCharBits).
	static constexpr field_layout layout_of(unsigned dr) noexcept {
		unsigned r = dr_to_r(dr);
		unsigned c_stored_bits = (r < maxCharBits) ? r : maxCharBits;
		unsigned p = maxCharBits - c_stored_bits;
		return field_layout{ r, c_stored_bits, p };
	}

	// ------------------------------------------------------------------
	// Decode
	// ------------------------------------------------------------------

	// The DR field is deliberately not carried here: no caller needs it, and it is
	// recoverable as find_dr(c).  Add it back when a variant actually wants it.
	struct decoded {
		int64_t  c;        // characteristic
		uint64_t M_bits;   // trailing field, unnormalized
		unsigned p;        // width of M_bits; m == M_bits / 2^p

		// The trailing field as a real in [0,1) (I3): the mantissa m of a
		// logarithmic takum, the fraction f of a linear one.  Exact for p <= 53.
		template<typename Real = double>
		constexpr Real fraction() const noexcept {
			if (p == 0) return Real(0);
			return static_cast<Real>(M_bits) / static_cast<Real>(1ull << p);
		}
	};

	// Split a magnitude into its takum fields.
	// Pre: 0 < magnitude <= magnitude_mask() -- the caller has already stripped the
	//      sign and handled the zero / NaR encodings.
	// Post: I2 (c in range), I3 (M_bits < 2^p).
	//
	// Note: when r > maxCharBits -- only reachable in degenerate configurations where
	// nbits - overhead < max_r -- only the high c_stored_bits of C are stored, so the
	// recovered c is quantized to a multiple of 2^(r - c_stored_bits).  The magnitude
	// still round-trips exactly through encode_exact() (I5); it is c that is coarse.
	static constexpr decoded decode(uint64_t magnitude) noexcept {
		unsigned dr = static_cast<unsigned>((magnitude >> (nbits - overhead)) & dr_field_mask);
		field_layout g = layout_of(dr);

		uint64_t C_stored = 0;
		if (g.c_stored_bits > 0) {
			C_stored = (magnitude >> g.p) & ((1ull << g.c_stored_bits) - 1);
		}
		uint64_t C_bits = (g.r > maxCharBits) ? (C_stored << (g.r - g.c_stored_bits)) : C_stored;
		int64_t c = dr_to_c_bias(dr) + static_cast<int64_t>(C_bits);

		uint64_t M_bits = (g.p > 0) ? (magnitude & ((1ull << g.p) - 1)) : 0ull;

		return decoded{ c, M_bits, g.p };
	}

	// Convenience: characteristic only, for callers that do not need the trailing field.
	// Pre: as decode().
	static constexpr int64_t characteristic_of(uint64_t magnitude) noexcept {
		return decode(magnitude).c;
	}

	// ------------------------------------------------------------------
	// Encode
	// ------------------------------------------------------------------

	struct encoded {
		uint64_t magnitude;
		takum_encode_status status;

		constexpr bool ok()          const noexcept { return status == takum_encode_status::ok; }
		constexpr bool overflowed()  const noexcept { return status == takum_encode_status::overflow; }
		constexpr bool underflowed() const noexcept { return status == takum_encode_status::underflow; }

		// I4: the codec owns the magnitude only; the sign bit belongs to the caller.
		constexpr bool sign_bit_clear() const noexcept { return magnitude <= magnitude_mask(); }
	};

	// Assemble a magnitude from already-resolved fields.  No rounding is performed;
	// this is the exact inverse of decode() and the entry point for future native
	// arithmetic, which carries M_bits as an integer rather than a double.
	//
	// Pre: c in [min_characteristic(), max_characteristic()]; M_bits < 2^p for the
	//      p implied by c's DR field (I3).
	// Post: I4 (sign bit clear), I5 (round-trips with decode()).
	static constexpr encoded encode_exact(int64_t c, uint64_t M_bits) noexcept {
		if (c > max_characteristic()) return encoded{ magnitude_mask(), takum_encode_status::overflow };
		if (c < min_characteristic()) return encoded{ 0ull, takum_encode_status::underflow };

		unsigned dr = find_dr(c);
		field_layout g = layout_of(dr);

		// I3 is a hard precondition here: pack() ORs M_bits into the magnitude
		// without masking, so an oversized M_bits would silently corrupt the C and
		// DR fields rather than fail.  Checked in debug builds only; the assert is
		// not reached during constant evaluation when the precondition holds.
		assert((g.p == 0) ? (M_bits == 0) : (M_bits < (1ull << g.p)));

		uint64_t C_full = static_cast<uint64_t>(c - dr_to_c_bias(dr));
		uint64_t C_stored = (g.r > maxCharBits) ? (C_full >> (g.r - g.c_stored_bits)) : C_full;

		return encoded{ pack(dr, C_stored, M_bits, g), takum_encode_status::ok };
	}

	// Assemble a magnitude from a characteristic and a real fraction m in [0,1),
	// rounding the trailing field to nearest-even and propagating any carry into
	// the characteristic (and, if that overflows the DR field, into the next DR).
	//
	// This is the shared half of both variants' float conversion: the linear takum
	// arrives here with (c, m) taken straight from the IEEE 754 exponent and
	// fraction; the logarithmic takum arrives with c = floor(l), m = l - c.
	//
	// Pre: m_real in [0.0, 1.0).
	// Post: I4; status == ok implies the result is the round-to-nearest-even
	//       encoding of (c, m_real), status otherwise reports saturation.
	//
	// The rounding itself is exact at every p; it is the double argument that
	// carries only 53 significant bits.  See the note at the top of this file.
	static constexpr encoded encode_rounded(int64_t c, double m_real) noexcept {
		if (c > max_characteristic()) return encoded{ magnitude_mask(), takum_encode_status::overflow };
		if (c < min_characteristic()) return encoded{ 0ull, takum_encode_status::underflow };

		unsigned dr = find_dr(c);
		field_layout g = layout_of(dr);

		// Characteristic bits.  When the format calls for more characteristic bits than
		// this nbits can hold, the low ones are dropped; the value they carry is not
		// rounded away here but folded into the residual below.  Note g.r > maxCharBits
		// implies c_stored_bits == maxCharBits and therefore g.p == 0 (I1), so the two
		// quantities below are consumed by exactly one rounding decision.
		uint64_t C_full = static_cast<uint64_t>(c - dr_to_c_bias(dr));
		uint64_t C_stored = C_full;
		// The part of the value sitting above C_stored, and half of one stored step,
		// both in characteristic units.  Without dropped bits a stored step spans one
		// characteristic unit; with them it spans 2^shift.
		double dropped_value = 0.0;
		double step_half     = 0.5;
		if (g.r > maxCharBits) {
			unsigned shift = g.r - g.c_stored_bits;
			C_stored      = C_full >> shift;
			dropped_value = static_cast<double>(C_full & ((1ull << shift) - 1ull));
			step_half     = static_cast<double>(1ull << (shift - 1));
		}

		// When the layout leaves no trailing field, the residual still carries value
		// information and must round the CHARACTERISTIC -- discarding it would make
		// conversion truncate rather than round to nearest, contradicting the format's
		// rounding function.  Ties go to an even stored characteristic.
		//
		// The dropped characteristic bits and m are weighed together, in one decision.
		// Rounding them separately is wrong twice over: it can carry into C_stored
		// twice, and it measures a bare m against half a CHARACTERISTIC unit when a
		// stored step is 2^shift of them.
		if (g.p == 0) {
			double residual = dropped_value + m_real;
			if (residual > step_half || (residual == step_half && (C_stored & 1ull))) {
				++C_stored;
				if (C_stored >= (1ull << g.c_stored_bits)) {
					if (dr + 1 >= nr_dr_values) return encoded{ magnitude_mask(), takum_encode_status::overflow };
					++dr;
					g = layout_of(dr);
					C_stored = 0;
				}
			}
			return encoded{ pack(dr, C_stored, 0ull, g), takum_encode_status::ok };
		}

		// Trailing field, round-to-nearest-even.  Reached only when g.p > 0, hence
		// g.r <= maxCharBits, C_stored == C_full and no characteristic bits were dropped.
		double scaled = m_real * static_cast<double>(1ull << g.p);
		uint64_t M_bits = static_cast<uint64_t>(scaled);
		double remainder = scaled - static_cast<double>(M_bits);
		if (remainder > 0.5 || (remainder == 0.5 && (M_bits & 1ull))) ++M_bits;
		if (M_bits >= (1ull << g.p)) {
			// Carry out of the trailing field into the characteristic.
			M_bits = 0;
			++C_stored;
			if (C_stored >= (1ull << g.c_stored_bits)) {
				if (dr + 1 >= nr_dr_values) return encoded{ magnitude_mask(), takum_encode_status::overflow };
				++dr;
				g = layout_of(dr);
				C_stored = 0;
			}
		}

		return encoded{ pack(dr, C_stored, M_bits, g), takum_encode_status::ok };
	}

	// Assemble a magnitude from a characteristic and an EXACT binary fraction N/2^q,
	// rounding to nearest-even.  The third encode entry point, and the one the
	// logarithmic takum's math library runs on.
	//
	// encode_rounded() takes the fraction as a double, which caps it at 53
	// significant bits.  That is enough when the fraction comes from an IEEE
	// significand, but not for arithmetic in the logarithmic domain: halving a
	// logarithmic value to take a square root produces a fraction one bit WIDER than
	// the source layout carries, and takum_log<64,3> already reaches p = 59.  Routing
	// that through a double would discard the low bits before rounding ever happened.
	// Carrying (N, q) as integers keeps the operation exact at every width, so the
	// result is off by at most the single rounding the target layout forces.
	//
	// Pre: q < 64 and N < 2^q, i.e. a proper binary fraction.
	// Post: I4; status == ok implies the result is the round-to-nearest-even encoding
	//       of c + N/2^q.
	static constexpr encoded encode_fraction(int64_t c, uint64_t N, unsigned q) noexcept {
		if (c > max_characteristic()) return encoded{ magnitude_mask(), takum_encode_status::overflow };
		if (c < min_characteristic()) return encoded{ 0ull, takum_encode_status::underflow };

		unsigned dr = find_dr(c);
		field_layout g = layout_of(dr);

		uint64_t C_full   = static_cast<uint64_t>(c - dr_to_c_bias(dr));
		uint64_t C_stored = C_full;
		uint64_t dropped  = 0;      // characteristic bits the layout cannot store
		uint64_t step_half = 0;     // half a stored step, in characteristic units
		if (g.r > maxCharBits) {
			unsigned shift = g.r - g.c_stored_bits;
			C_stored  = C_full >> shift;
			dropped   = C_full & ((1ull << shift) - 1ull);
			step_half = 1ull << (shift - 1);
		}

		if (g.p == 0) {
			// Round the characteristic, weighing the dropped bits and the fraction
			// together exactly as encode_rounded() does.  Every comparison stays in
			// integers: the residual is dropped + N/2^q against a threshold of either
			// step_half or 1/2, and neither product is ever formed.
			bool round_up;
			if (g.r > maxCharBits) {
				round_up = (dropped > step_half)
				        || (dropped == step_half && (N > 0 || (C_stored & 1ull)));
			}
			else {
				uint64_t half = (q > 0) ? (1ull << (q - 1)) : 0ull;
				round_up = (q > 0) && (N > half || (N == half && (C_stored & 1ull)));
			}
			if (round_up) {
				++C_stored;
				if (C_stored >= (1ull << g.c_stored_bits)) {
					if (dr + 1 >= nr_dr_values) return encoded{ magnitude_mask(), takum_encode_status::overflow };
					++dr;
					g = layout_of(dr);
					C_stored = 0;
				}
			}
			return encoded{ pack(dr, C_stored, 0ull, g), takum_encode_status::ok };
		}

		// Rescale N/2^q onto the layout's p bits.  Widening is exact; narrowing is the
		// one place a value can be lost, and it rounds to nearest-even.
		uint64_t M_bits = 0;
		if (g.p >= q) {
			M_bits = N << (g.p - q);
		}
		else {
			unsigned s   = q - g.p;
			M_bits       = N >> s;
			uint64_t rem = N & ((1ull << s) - 1ull);
			uint64_t half = 1ull << (s - 1);
			if (rem > half || (rem == half && (M_bits & 1ull))) ++M_bits;
			if (M_bits >= (1ull << g.p)) {
				// Carry out of the trailing field into the characteristic.
				M_bits = 0;
				++C_stored;
				if (C_stored >= (1ull << g.c_stored_bits)) {
					if (dr + 1 >= nr_dr_values) return encoded{ magnitude_mask(), takum_encode_status::overflow };
					++dr;
					g = layout_of(dr);
					C_stored = 0;
					return encoded{ pack(dr, 0ull, 0ull, g), takum_encode_status::ok };
				}
			}
		}

		return encoded{ pack(dr, C_stored, M_bits, g), takum_encode_status::ok };
	}

private:
	// Stateless: the codec is a namespace of static constexpr members, never an
	// object.  Deleting the default constructor makes that intent a compile error
	// rather than a comment.
	takum_codec() = delete;

	// Lay the three fields out in a magnitude word.  Post: I4.
	static constexpr uint64_t pack(unsigned dr, uint64_t C_stored, uint64_t M_bits,
	                               const field_layout& g) noexcept {
		uint64_t magnitude = static_cast<uint64_t>(dr) << (nbits - overhead);
		if (g.c_stored_bits > 0) magnitude |= (C_stored << g.p);
		if (g.p > 0)             magnitude |= M_bits;
		return magnitude;
	}
};

}} // namespace sw::universal
