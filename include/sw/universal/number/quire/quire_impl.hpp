#pragma once
// quire_impl.hpp: generalized quire (super-accumulator) using limb-based storage
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>      // the fixed-width integer types
#include <universal/utility/exceptions.hpp>  // throw_or_abort (#1925)
#include <cmath>        // std::ldexp
#include <string>       // std::string
#include <vector>       // std::vector
#include <stdexcept>    // std::out_of_range
#include <iosfwd>       // std::ostream in the operator<< friend DECLARATION -- not <ostream>,
                        // which would put a stream header back into the core (#1334)
//
// This implementation uses a single blockbinary<qbits, LimbType, Unsigned> for the
// magnitude of the accumulator, with the sign managed externally. The blockbinary
// provides fast limb-based carry propagation using uint32_t or uint64_t limbs.
//
#include <universal/number/shared/specific_value_encoding.hpp>
#include <limits>
#include <type_traits>

namespace sw { namespace universal {


/*
 generalized quire: a super-accumulator parameterized on the number type

 The quire is a fixed-point accumulator sized to hold the full dynamic range
 of products for the given number system. It uses quire_traits<NumberType> to
 determine the accumulator width, and stores the magnitude in a single
 blockbinary using fast limb-based carry propagation.

 Template parameters:
   NumberType - the scalar type being accumulated (posit, cfloat, fixpnt, lns, dbns)
   capacity   - overflow guard bits (default from quire_traits, typically 30)
   LimbType   - the unsigned integer type for limbs (uint32_t or uint64_t)

 All values in and out of the quire are (sign, scale, significand) triplets,
 represented as blocktriple values from arithmetic operations.
*/
template<typename NumberType,
         unsigned capacity = quire_traits<NumberType>::capacity,
         typename LimbType = uint32_t>
class quire {
public:
	using Traits = quire_traits<NumberType>;

	static constexpr unsigned range       = Traits::range;
	static constexpr unsigned half_range  = Traits::half_range;
	static constexpr unsigned radix_point = Traits::radix_point;
	static constexpr unsigned upper_range = Traits::upper_range;
	static constexpr unsigned qbits       = range + capacity;
	static constexpr unsigned msb_index   = qbits - 1;

	// the accumulator: unsigned magnitude with sign managed externally
	using accumulator_type = blockbinary<qbits, LimbType, BinaryNumberType::Unsigned>;

	// Constructors
	quire() : _sign(false), _is_pinf(false), _is_ninf(false), _is_nan(false), _accu{} {}
	quire(int8_t   iv) { *this = static_cast<int64_t>(iv); }
	quire(int16_t  iv) { *this = static_cast<int64_t>(iv); }
	quire(int32_t  iv) { *this = static_cast<int64_t>(iv); }
	quire(int64_t  iv) { *this = iv; }
	quire(uint8_t  iv) { *this = static_cast<uint64_t>(iv); }
	quire(uint16_t iv) { *this = static_cast<uint64_t>(iv); }
	quire(uint32_t iv) { *this = static_cast<uint64_t>(iv); }
	quire(uint64_t iv) { *this = iv; }
	quire(float    iv) { *this = iv; }
	quire(double   iv) { *this = iv; }

	// construct from a blocktriple (any operator tag)
	template<unsigned fbits, BlockTripleOperator op, typename bt>
	quire(const blocktriple<fbits, op, bt>& rhs) { *this = rhs; }

	// specific value constructor
	constexpr quire(const SpecificValue code) noexcept : _sign{false}, _is_pinf{false}, _is_ninf{false}, _is_nan{false}, _accu{}
	{
		switch (code) {
		case SpecificValue::maxpos:
			maxpos();
			break;
		case SpecificValue::minpos:
			minpos();
			break;
		case SpecificValue::zero:
		default:
			zero();
			break;
		case SpecificValue::minneg:
			minneg();
			break;
		case SpecificValue::maxneg:
			maxneg();
			break;
		case SpecificValue::infpos:
			_is_pinf = true;   // the accumulator has no encoding for an infinity: record the state (#2019)
			break;
		case SpecificValue::infneg:
			_is_ninf = true;
			break;
		case SpecificValue::nar:
		case SpecificValue::qnan:
		case SpecificValue::snan:
			// The quire has no numeric encoding for a NaN; record the non-finite
			// state instead. (This constructor is noexcept, so the old
			// unconditional throw here actually called std::terminate.)
			_is_nan = true;
			break;
		}
	}

	// ////////////////////////////////////////////////////////////////
	// Assignment from blocktriple (the primary input path)

	template<unsigned fbits, BlockTripleOperator op, typename bt>
	quire& operator=(const blocktriple<fbits, op, bt>& rhs) {
		reset();  // clears the non-finite state as well: a fresh assignment starts finite
		if (rhs.iszero()) return *this;
		// NaN or infinite operand: record the non-finite state (propagates on resolve).
		if (rhs.isinf() || rhs.isnan()) { record_nonfinite(rhs); return *this; }
		_sign = rhs.sign();

		int scale = rhs.scale();
#if QUIRE_THROW_ARITHMETIC_EXCEPTION
		if (scale >  static_cast<int>(half_range)) throw operand_too_large_for_quire{};
		if (scale < -static_cast<int>(half_range)) throw operand_too_small_for_quire{};
#else
		// Opt-out build: an operand outside the quire's dynamic range cannot be
		// represented; leave the quire zero rather than throwing.
		if (scale >  static_cast<int>(half_range) || scale < -static_cast<int>(half_range)) {
			reset();
			return *this;
		}
#endif
		place_blocktriple(rhs);
		return *this;
	}
	// ////////////////////////////////////////////////////////////////
	// Assignment from the native Scalar type
	// Disabled for arithmetic types (float/double/int) which have dedicated overloads above.
	template<typename T = NumberType,
	         std::enable_if_t<!std::is_arithmetic_v<T>, int> = 0>
	quire& operator=(const T& rhs) {
		blocktriple<Traits::fbits, BlockTripleOperator::REP, LimbType> v;
		rhs.normalize(v);
		return operator=(v);
	}

	// Assignment from native integers
	quire& operator=(int8_t rhs) { return operator=(static_cast<int64_t>(rhs)); }
	quire& operator=(int16_t rhs) { return operator=(static_cast<int64_t>(rhs)); }
	quire& operator=(int32_t rhs) { return operator=(static_cast<int64_t>(rhs)); }
	quire& operator=(int64_t rhs) {
		clear();
		_sign = (rhs < 0);
		// Avoid UB when rhs == INT64_MIN: -(INT64_MIN) overflows signed int64_t.
		// Instead, compute magnitude in the unsigned domain.
		uint64_t magnitude = _sign
			? (static_cast<uint64_t>(-(rhs + 1)) + 1ull)
			: static_cast<uint64_t>(rhs);
		if (magnitude == 0) return *this;
		unsigned msb = find_msb(magnitude);
#if QUIRE_THROW_ARITHMETIC_EXCEPTION
		if (msb > half_range + capacity) throw operand_too_large_for_quire{};
#else
		if (msb > half_range + capacity) { reset(); return *this; }
#endif
		// place integer value at the radix point (integers have scale >= 0)
		for (unsigned i = 0; i < 64 && i < msb; ++i) {
			if (magnitude & (uint64_t(1) << i)) {
				_accu.setbit(radix_point + i);
			}
		}
		return *this;
	}
	quire& operator=(uint8_t rhs) { return operator=(static_cast<uint64_t>(rhs)); }
	quire& operator=(uint16_t rhs) { return operator=(static_cast<uint64_t>(rhs)); }
	quire& operator=(uint32_t rhs) { return operator=(static_cast<uint64_t>(rhs)); }
	quire& operator=(uint64_t rhs) {
		clear();
		_sign = false;
		if (rhs == 0) return *this;
		unsigned msb = find_msb(rhs);
#if QUIRE_THROW_ARITHMETIC_EXCEPTION
		if (msb > half_range + capacity) throw operand_too_large_for_quire{};
#else
		if (msb > half_range + capacity) { reset(); return *this; }
#endif
		for (unsigned i = 0; i < 64 && i < msb; ++i) {
			if (rhs & (uint64_t(1) << i)) {
				_accu.setbit(radix_point + i);
			}
		}
		return *this;
	}
	quire& operator=(float rhs) {
		reset();
		if (rhs == 0.0f) return *this;
		// all 24 significand bits: the 22 fraction bits used here dodged a blocktriple::round<>
		// defect (fbits == srcbits - 1 dropped a bit, fixed in #2052) by rounding away the
		// last bit of every float, so quire<float> = 1 + FLT_EPSILON resolved to 1 (#2054)
		blocktriple<23, BlockTripleOperator::REP, uint32_t> v(rhs);
		return *this = v;
	}
	quire& operator=(double rhs) {
		reset();
		if (rhs == 0.0) return *this;
		// all 53 significand bits, as for float above
		blocktriple<52, BlockTripleOperator::REP, uint32_t> v(rhs);
		return *this = v;
	}

	// ////////////////////////////////////////////////////////////////=
	// Accumulation operators (the core quire operations)
	// ////////////////////////////////////////////////////////////////=

	/// Add a blocktriple value to the quire
	template<unsigned fbits, BlockTripleOperator op, typename bt>
	quire& operator+=(const blocktriple<fbits, op, bt>& rhs) {
		if (_is_nan) return *this;             // NaN is sticky: once set, accumulation is a no-op
		if (rhs.iszero()) return *this;
		// NaN or infinite operand: the sum becomes that infinity, or NaN, and stays so.
		if (rhs.isinf() || rhs.isnan()) { record_nonfinite(rhs); return *this; }
		// An infinite sum absorbs every finite term; only an opposite infinity or a
		// NaN, recorded above, can still change it.
		if (_is_pinf || _is_ninf) return *this;

		int scale = rhs.scale();
#if QUIRE_THROW_ARITHMETIC_EXCEPTION
		if (scale >  static_cast<int>(half_range)) throw operand_too_large_for_quire{};
		if (scale < -static_cast<int>(half_range)) throw operand_too_small_for_quire{};
#else
		// Opt-out build: an operand outside the quire's dynamic range cannot be
		// represented; skip it rather than throwing (leaves the running sum intact).
		if (scale >  static_cast<int>(half_range) || scale < -static_cast<int>(half_range)) {
			return *this;
		}
#endif
		if (_sign == rhs.sign()) {
			add_blocktriple(rhs);
		}
		else {
			// The quire stores sign out-of-band and keeps _accu as an unsigned magnitude.
			// Opposite-sign accumulation therefore becomes a magnitude comparison followed by
			// unsigned subtraction; only the larger operand contributes the result sign.
			int cmp = compare_magnitude(rhs);
			if (cmp < 0) {
				// |rhs| > |*this|: swap, then subtract
				accumulator_type old_accu = _accu;
				reset();
				place_blocktriple(rhs);
				subtract_accumulator(old_accu);
				_sign = rhs.sign();
			}
			else if (cmp > 0) {
				subtract_blocktriple(rhs);
			}
			else {
				// equal magnitudes: result is zero
				reset();
			}
		}
		return *this;
	}
	// Disabled for arithmetic types (float/double/int) which use the blocktriple path.
	template<typename T = NumberType,
	         std::enable_if_t<!std::is_arithmetic_v<T>, int> = 0>
	quire& operator+=(const T& rhs) {
		blocktriple<Traits::fbits, BlockTripleOperator::REP, LimbType> v;
		rhs.normalize(v);
		return operator+=(v);
	}

	/// Subtract a blocktriple value from the quire
	template<unsigned fbits, BlockTripleOperator op, typename bt>
	quire& operator-=(const blocktriple<fbits, op, bt>& rhs) {
		// negate the rhs and add
		blocktriple<fbits, op, bt> neg(rhs);
		neg.setsign(!rhs.sign());
		return *this += neg;
	}
	// Disabled for arithmetic types (float/double/int) which use the blocktriple path.
	template<typename T = NumberType,
	         std::enable_if_t<!std::is_arithmetic_v<T>, int> = 0>
	quire& operator-=(const T& rhs) {
		blocktriple<Traits::fbits, BlockTripleOperator::REP, LimbType> v;
		rhs.normalize(v);
		v.setsign(!v.sign());
		return operator+=(v);
	}

	/// Add two quires
	quire& operator+=(const quire& rhs) {
		if (_is_nan) return *this;                 // NaN is sticky
		if (rhs._is_nan) { setnan(); return *this; }
		if (rhs._is_pinf || rhs._is_ninf || _is_pinf || _is_ninf) {
			// at least one side is infinite: the sum is that infinity, or NaN for +inf + -inf
			bool pinf = _is_pinf || rhs._is_pinf;
			bool ninf = _is_ninf || rhs._is_ninf;
			if (pinf && ninf) setnan(); else setinf(ninf);
			return *this;
		}
		if (rhs.iszero()) return *this;
		if (_sign == rhs._sign) {
			_accu += rhs._accu;
		}
		else {
			// compare magnitudes by treating accumulators as unsigned
			if (_accu == rhs._accu) {
				reset();
			}
			else {
				// determine which is larger
				bool rhs_larger = false;
				for (int i = static_cast<int>(qbits) - 1; i >= 0; --i) {
					bool a = _accu.test(static_cast<unsigned>(i));
					bool b = rhs._accu.test(static_cast<unsigned>(i));
					if (a != b) {
						rhs_larger = b;
						break;
					}
				}
				if (rhs_larger) {
					// |rhs| > |this|: compute rhs - this (unsigned subtraction is
					// correct via two's complement when minuend >= subtrahend)
					accumulator_type result = rhs._accu;
					result -= _accu;
					_accu = result;
					_sign = rhs._sign;
				}
				else {
					_accu -= rhs._accu;  // |this| > |rhs|: safe unsigned subtraction
				}
			}
		}
		return *this;
	}

	/// Subtract two quires
	quire& operator-=(const quire& rhs) {
		quire neg(rhs);
		neg._sign = !rhs._sign;
		neg._is_pinf = rhs._is_ninf;  // -(+inf) is -inf, and -(-inf) is +inf
		neg._is_ninf = rhs._is_pinf;
		return *this += neg;
	}

	// ////////////////////////////////////////////////////////////////=
	// Selectors
	// ////////////////////////////////////////////////////////////////=
	// The non-finite states (#2019): NaN, +inf and -inf. isnar() is "any of them",
	// which is what a posit, with no infinities, resolves to NaR.
	bool iszero() const noexcept { return !isnar() && _accu.none(); }
	bool isnan()  const noexcept { return _is_nan; }
	bool isinf()  const noexcept { return !_is_nan && (_is_pinf || _is_ninf); }
	bool isnar()  const noexcept { return _is_nan || _is_pinf || _is_ninf; }
	bool sign()   const noexcept { return isinf() ? _is_ninf : _sign; }
	bool isneg()  const noexcept { return sign(); }
	bool ispos()  const noexcept { return !sign(); }
	int  dynamic_range() const noexcept { return static_cast<int>(range); }
	int  max_scale() const noexcept { return static_cast<int>(upper_range); }
	int  min_scale() const noexcept { return -static_cast<int>(half_range); }
	int  capacity_range() const noexcept { return static_cast<int>(capacity); }
	unsigned total_bits() const noexcept { return qbits + 1; }  // +1 for the sign bit

	/// Find the scale (position of MSB relative to radix point)
	int scale() const noexcept {
		for (int i = static_cast<int>(qbits) - 1; i >= 0; --i) {
			if (_accu.test(static_cast<unsigned>(i))) {
				return i - static_cast<int>(radix_point);
			}
		}
		return 0;  // zero
	}

	/// Bit addressing: index 0 is at the bottom of the lower range
	bool operator[](unsigned index) const {
		if (index >= qbits) throw_or_abort(std::out_of_range("quire bit index out of range"));
		return _accu.test(index);
	}
	bool testbit(unsigned index) const {
		if (index >= qbits) throw_or_abort(std::out_of_range("quire bit index out of range"));
		return _accu.test(index);
	}

	/// Check if any bit is set at or below the given index
	bool anyAfter(unsigned index) const noexcept {
		for (int i = static_cast<int>(index); i >= 0; --i) {
			if (_accu.test(static_cast<unsigned>(i))) return true;
		}
		return false;
	}

	// ////////////////////////////////////////////////////////////////=
	// Modifiers
	// ////////////////////////////////////////////////////////////////=
	void reset() noexcept {
		_sign = false;
		_is_pinf = false;
		_is_ninf = false;
		_is_nan  = false;
		_accu.clear();
	}
	void clear() noexcept { reset(); }
	void set_sign(bool v) noexcept { _sign = v; }
	void setnan() noexcept { reset(); _is_nan = true; }  // force the NaN (posit: NaR) state
	void setinf(bool negative) noexcept {            // force the +inf or -inf state
		reset();
		if (negative) _is_ninf = true; else _is_pinf = true;
	}
	void setbit(unsigned index) {
		if (index >= qbits) throw_or_abort(std::out_of_range("quire bit index out of range"));
		_accu.setbit(index);
	}
	void zero() noexcept {
		reset();
	}
	void maxpos() noexcept {
		reset();
		_accu.flip();  // largest value all bits set
	}
	void minpos() noexcept {
		reset();
		_accu.setbit(0);  // smallest value has MSB at 0
	}
	void minneg() noexcept {
		reset();
		_sign = true;
		_accu.setbit(0);  // smallest negative value has MSB at 0
	}
	void maxneg() noexcept {
		reset();
		_sign = true;
		_accu.flip();  // largest negative value has all bits set
	}

	// ////////////////////////////////////////////////////////////////=
	// Conversion: extract the accumulated value as a blocktriple
	// ////////////////////////////////////////////////////////////////=

	/// Convert quire value to a blocktriple<qbits, REP>
	blocktriple<qbits, BlockTripleOperator::REP, LimbType> to_blocktriple() const {
		blocktriple<qbits, BlockTripleOperator::REP, LimbType> result;
		if (_is_nan) { result.setnan(); return result; }
		if (isinf()) { result.setinf(_is_ninf); return result; }
		if (iszero()) { result.setzero(_sign); return result; }

		// find MSB to determine scale
		int msb = -1;
		for (int i = static_cast<int>(qbits) - 1; i >= 0; --i) {
			if (_accu.test(static_cast<unsigned>(i))) {
				msb = i;
				break;
			}
		}
		if (msb < 0) { result.setzero(_sign); return result; }

		int s = msb - static_cast<int>(radix_point);
		result.setnormal();
		result.setsign(_sign);
		result.setscale(s);
		// set the significand bits: MSB is the hidden bit
		for (int i = msb; i >= 0; --i) {
			unsigned bit_in_result = static_cast<unsigned>(msb - i);
			if (bit_in_result < qbits) {
				result.setbit(qbits - 1 - bit_in_result, _accu.test(static_cast<unsigned>(i)));
			}
		}
		return result;
	}

	/// Convert quire to a target number type.
	/// This implementation extracts at most 53 significant bits (double precision).
	/// For target types wider than 64 bits total, a direct bit-extraction path
	/// would be needed; we guard against silent precision loss with a static_assert.
	template<typename TargetType>
	TargetType convert_to() const {
		static_assert(sizeof(TargetType) <= 8,
			"convert_to<T>() uses double as intermediate and is limited to 53 bits "
			"of significand precision. Use to_blocktriple() for wider target types.");
		if (_is_nan) {
			// Only floating-point targets have a NaN encoding; converting a NaN
			// double to an integral type is undefined behavior, so return 0 there.
			if constexpr (std::is_floating_point_v<TargetType>) {
				return TargetType(std::numeric_limits<double>::quiet_NaN());
			}
			else {
				return TargetType(0);
			}
		}
		if (isinf()) {
			// converting an infinite double to an integral type is undefined
			// behavior too: saturate there instead
			if constexpr (std::is_integral_v<TargetType>) {
				return _is_ninf ? std::numeric_limits<TargetType>::lowest() : std::numeric_limits<TargetType>::max();
			}
			else {
				return TargetType(_is_ninf ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity());
			}
		}
		if (iszero()) return TargetType(0);
		// find the scale
		int s = scale();
		// extract the most significant bits as a double for conversion
		double value = 0.0;
		double weight = std::ldexp(1.0, s);
		int msb_pos = s + static_cast<int>(radix_point);
		for (int i = msb_pos; i >= 0 && (msb_pos - i) < 53; --i) {
			if (_accu.test(static_cast<unsigned>(i))) {
				value += weight;
			}
			weight *= 0.5;
		}
		if (_sign) value = -value;
		return TargetType(value);
	}

	// ////////////////////////////////////////////////////////////////=
	// Direct access to the accumulator (for testing and diagnostics)
	// ////////////////////////////////////////////////////////////////=
	const accumulator_type& accumulator() const noexcept { return _accu; }

	// ////////////////////////////////////////////////////////////////=
	// String representation
	// ////////////////////////////////////////////////////////////////=

	/// Load from a bit string in format "+:cccc_uuuu.llll"
	/// Returns false (without modifying the quire) on malformed or oversized input.
	bool load_bits(const std::string& string_of_bits) {
		bool parsed_sign = false;
		auto it = string_of_bits.begin();
		if (it == string_of_bits.end()) return false;
		if (*it == '-') parsed_sign = true;
		else if (*it == '+') parsed_sign = false;
		else return false;
		++it;
		if (it == string_of_bits.end() || *it != ':') return false;
		++it;
		// parse bits from MSB to LSB
		// format: capacity_upper.lower
		// we parse all bits into a linear array, then place them
		std::vector<bool> bits;
		bool seen_radix = false;
		unsigned radix_pos = 0;
		for (; it != string_of_bits.end(); ++it) {
			if (*it == '_' || *it == '\'') continue;  // skip separators
			if (*it == '.') {
				if (seen_radix) return false;  // reject duplicate radix point
				seen_radix = true;
				radix_pos = static_cast<unsigned>(bits.size());
				continue;
			}
			if (*it != '0' && *it != '1') return false;  // reject invalid characters
			bits.push_back(*it == '1');
		}
		if (!seen_radix) return false;
		// reject oversized bit strings that would be silently clipped
		unsigned total = static_cast<unsigned>(bits.size());
		unsigned lower_bits = total - radix_pos;
		unsigned upper_capacity = qbits - radix_point;  // bits above radix in accumulator
		if (radix_pos > upper_capacity || lower_bits > radix_point) return false;
		// parse into temporary accumulator, only commit on success
		accumulator_type parsed_accu{};
		for (unsigned i = 0; i < radix_pos; ++i) {
			unsigned accu_bit = radix_point + radix_pos - 1 - i;
			if (bits[i]) {
				parsed_accu.setbit(accu_bit);
			}
		}
		for (unsigned i = 0; i < lower_bits; ++i) {
			unsigned accu_bit = radix_point - 1 - i;
			if (bits[radix_pos + i]) {
				parsed_accu.setbit(accu_bit);
			}
		}
		reset();        // successfully loaded finite bits: leave the non-finite state behind
		_sign = parsed_sign;
		_accu = parsed_accu;
		return true;
	}

private:
	bool             _sign;
	// Non-finite state. The accumulator has no numeric encoding for infinities
	// or NaNs, so a non-finite operand sets one of these sticky flags; the state
	// propagates through accumulation and surfaces on resolve (#1226, #2019).
	//   _is_nan          a NaN operand, or both infinities met (+inf + -inf)
	//   _is_pinf, _is_ninf  the sum is +inf or -inf; at most one is set without _is_nan
	// While any flag is set, _sign and _accu are cleared and carry no meaning.
	bool             _is_pinf;
	bool             _is_ninf;
	bool             _is_nan;
	accumulator_type _accu;

	/// Record a NaN or infinite operand. Opposite infinities make NaN.
	template<unsigned fbits, BlockTripleOperator op, typename bt>
	void record_nonfinite(const blocktriple<fbits, op, bt>& v) noexcept {
		if (v.isnan() || _is_nan) { setnan(); return; }
		bool negative = v.sign();
		if ((negative && _is_pinf) || (!negative && _is_ninf)) { setnan(); return; }
		setinf(negative);
	}

	// ////////////////////////////////////////////////////////////////=
	// Internal helpers for blocktriple accumulation
	// ////////////////////////////////////////////////////////////////=

	/// Compute the accumulator base offset for a blocktriple value.
	/// The blocktriple's radix defines the binary point position in the
	/// significand. Bit i in the significand represents 2^(i - bt_radix)
	/// in the significand, so the absolute value contribution of bit i is:
	///   2^(scale + i - bt_radix)
	/// which maps to accumulator position: radix_point + scale + i - bt_radix.
	/// We return (radix_point + scale - bt_radix) so callers just add i.
	///
	/// NOTE: We use the static radix (not MSB position) because MUL products
	/// can have overflow form (MSB above radix), and using MSB would misplace
	/// all bits by the overflow amount.
	template<unsigned fbits, BlockTripleOperator op, typename bt>
	static int accu_base_offset(const blocktriple<fbits, op, bt>& v) {
		constexpr int bt_radix = blocktriple<fbits, op, bt>::radix;
		return static_cast<int>(radix_point) + v.scale() - bt_radix;
	}

	/// Scatter a blocktriple's significand bits into a temporary accumulator
	template<unsigned fbits, BlockTripleOperator op, typename bt>
	static void scatter_to_accumulator(const blocktriple<fbits, op, bt>& v, int base, accumulator_type& dest) {
		constexpr unsigned bfbits = blocktriple<fbits, op, bt>::bfbits;
		for (unsigned i = 0; i < bfbits; ++i) {
			if (v.test(i)) {
				int accu_pos = base + static_cast<int>(i);
				if (accu_pos >= 0 && accu_pos < static_cast<int>(qbits)) {
					dest.setbit(static_cast<unsigned>(accu_pos));
				}
			}
		}
	}

	/// Place a blocktriple's significand bits into the accumulator at the correct position
	template<unsigned fbits, BlockTripleOperator op, typename bt>
	void place_blocktriple(const blocktriple<fbits, op, bt>& v) {
		int base = accu_base_offset(v);
		scatter_to_accumulator(v, base, _accu);
	}

	/// Add a blocktriple's magnitude to the accumulator (same sign case)
	template<unsigned fbits, BlockTripleOperator op, typename bt>
	void add_blocktriple(const blocktriple<fbits, op, bt>& v) {
		accumulator_type temp{};
		int base = accu_base_offset(v);
		scatter_to_accumulator(v, base, temp);
		_accu += temp;
	}

	/// Subtract a blocktriple's magnitude from the accumulator (|*this| > |v| assumed)
	/// NOTE: blockbinary::operator-= uses two's complement internally: a -= b computes
	/// a + (~b + 1). For Unsigned blocks with a >= b, this yields (a - b + 2^n) mod 2^n = a - b,
	/// which is mathematically correct. The caller must ensure |*this| >= |v|.
	template<unsigned fbits, BlockTripleOperator op, typename bt>
	void subtract_blocktriple(const blocktriple<fbits, op, bt>& v) {
		accumulator_type temp{};
		int base = accu_base_offset(v);
		scatter_to_accumulator(v, base, temp);
		_accu -= temp;
	}

	/// Subtract another accumulator from this one (|*this| >= |other| required)
	/// See subtract_blocktriple for explanation of unsigned two's complement correctness.
	void subtract_accumulator(const accumulator_type& other) {
		_accu -= other;
	}

	/// Compare magnitude of *this against a blocktriple
	/// Returns -1 if |*this| < |v|, 0 if equal, +1 if |*this| > |v|
	template<unsigned fbits, BlockTripleOperator op, typename bt>
	int compare_magnitude(const blocktriple<fbits, op, bt>& v) const {
		accumulator_type temp{};
		int base = accu_base_offset(v);
		scatter_to_accumulator(v, base, temp);
		// compare from MSB down
		for (int i = static_cast<int>(qbits) - 1; i >= 0; --i) {
			bool a = _accu.test(static_cast<unsigned>(i));
			bool b = temp.test(static_cast<unsigned>(i));
			if (a && !b) return 1;
			if (!a && b) return -1;
		}
		return 0;
	}

	// friends for stream operators and comparisons
	template<typename NT, unsigned c, typename LT>
	friend std::ostream& operator<<(std::ostream& ostr, const quire<NT, c, LT>& q);
	template<typename NT, unsigned c, typename LT>
	friend bool operator==(const quire<NT, c, LT>& lhs, const quire<NT, c, LT>& rhs);
	template<typename NT, unsigned c, typename LT>
	friend bool operator!=(const quire<NT, c, LT>& lhs, const quire<NT, c, LT>& rhs);
	template<typename NT, unsigned c, typename LT>
	friend bool operator<(const quire<NT, c, LT>& lhs, const quire<NT, c, LT>& rhs);
	template<typename NT, unsigned c, typename LT>
	friend bool operator>(const quire<NT, c, LT>& lhs, const quire<NT, c, LT>& rhs);
	template<typename NT, unsigned c, typename LT>
	friend bool operator<=(const quire<NT, c, LT>& lhs, const quire<NT, c, LT>& rhs);
	template<typename NT, unsigned c, typename LT>
	friend bool operator>=(const quire<NT, c, LT>& lhs, const quire<NT, c, LT>& rhs);
	template<typename NT, unsigned c, typename LT>
	friend quire<NT, c, LT> abs(const quire<NT, c, LT>& q);
};

// ////////////////////////////////////////////////////////////////=
// Free functions
// ////////////////////////////////////////////////////////////////=

template<typename NumberType, unsigned capacity, typename LimbType>
quire<NumberType, capacity, LimbType> abs(const quire<NumberType, capacity, LimbType>& q) {
	quire<NumberType, capacity, LimbType> result(q);
	result._sign = false;
	if (result._is_ninf) { result._is_ninf = false; result._is_pinf = true; }  // |-inf| is +inf
	return result;
}

// Binary addition of two quires
template<typename NumberType, unsigned capacity, typename LimbType>
quire<NumberType, capacity, LimbType> operator+(
	const quire<NumberType, capacity, LimbType>& lhs,
	const quire<NumberType, capacity, LimbType>& rhs) {
	quire<NumberType, capacity, LimbType> sum(lhs);
	sum += rhs;
	return sum;
}



// ////////////////////////////////////////////////////////////////=
// Comparison operators
// ////////////////////////////////////////////////////////////////=

template<typename NumberType, unsigned capacity, typename LimbType>
bool operator==(const quire<NumberType, capacity, LimbType>& lhs,
                const quire<NumberType, capacity, LimbType>& rhs) {
	if (lhs._is_nan || rhs._is_nan) return false;         // NaN compares unordered (IEEE-style)
	if (lhs.isinf() || rhs.isinf()) return lhs.isinf() && rhs.isinf() && lhs._is_ninf == rhs._is_ninf;
	if (lhs.iszero() && rhs.iszero()) return true;  // +0 == -0
	return lhs._sign == rhs._sign && lhs._accu == rhs._accu;
}

template<typename NumberType, unsigned capacity, typename LimbType>
bool operator!=(const quire<NumberType, capacity, LimbType>& lhs,
                const quire<NumberType, capacity, LimbType>& rhs) {
	return !(lhs == rhs);
}

template<typename NumberType, unsigned capacity, typename LimbType>
bool operator<(const quire<NumberType, capacity, LimbType>& lhs,
               const quire<NumberType, capacity, LimbType>& rhs) {
	if (lhs._is_nan || rhs._is_nan) return false;          // NaN is unordered
	// an infinite quire compares as that infinity
	if (lhs._is_ninf) return !rhs._is_ninf;                // -inf is below everything but -inf
	if (rhs._is_pinf) return !lhs._is_pinf;                // +inf is above everything but +inf
	if (lhs._is_pinf || rhs._is_ninf) return false;
	if (lhs.iszero() && rhs.iszero()) return false;  // +0 == -0
	if (lhs._sign != rhs._sign) return lhs._sign;  // negative < positive
	// Both same sign: compare magnitudes using MSB-first unsigned comparison.
	// We do NOT delegate to blockbinary::operator< because it uses sign-aware
	// logic (checks MSB as sign bit), which is incorrect for Unsigned blocks.
	constexpr unsigned qb = quire<NumberType, capacity, LimbType>::qbits;
	bool mag_less = false;
	bool mag_equal = true;
	for (int i = static_cast<int>(qb) - 1; i >= 0; --i) {
		bool a = lhs._accu.test(static_cast<unsigned>(i));
		bool b = rhs._accu.test(static_cast<unsigned>(i));
		if (a != b) {
			mag_less = b;  // lhs bit is 0, rhs bit is 1 => lhs magnitude < rhs magnitude
			mag_equal = false;
			break;
		}
	}
	if (mag_equal) return false;
	// both negative: larger magnitude means smaller value
	return lhs._sign ? !mag_less : mag_less;
}

template<typename NumberType, unsigned capacity, typename LimbType>
bool operator>(const quire<NumberType, capacity, LimbType>& lhs,
               const quire<NumberType, capacity, LimbType>& rhs) {
	return rhs < lhs;
}

template<typename NumberType, unsigned capacity, typename LimbType>
bool operator<=(const quire<NumberType, capacity, LimbType>& lhs,
                const quire<NumberType, capacity, LimbType>& rhs) {
	if (lhs._is_nan || rhs._is_nan) return false;  // NaN is unordered (can't reuse !(rhs<lhs))
	return !(rhs < lhs);
}

template<typename NumberType, unsigned capacity, typename LimbType>
bool operator>=(const quire<NumberType, capacity, LimbType>& lhs,
                const quire<NumberType, capacity, LimbType>& rhs) {
	if (lhs._is_nan || rhs._is_nan) return false;  // NaN is unordered (can't reuse !(lhs<rhs))
	return !(lhs < rhs);
}

}} // namespace sw::universal
