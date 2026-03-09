#pragma once
// quire_impl.hpp: generalized quire (super-accumulator) using limb-based storage
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This implementation uses a single blockbinary<qbits, LimbType, Unsigned> for the
// magnitude of the accumulator, with the sign managed externally. The blockbinary
// provides fast limb-based carry propagation using uint32_t or uint64_t limbs.
//
// Relates to #345, #546

namespace sw { namespace universal {

// Forward declarations
template<typename NumberType, unsigned capacity, typename LimbType> class quire;
template<typename NumberType, unsigned capacity, typename LimbType>
quire<NumberType, capacity, LimbType> abs(const quire<NumberType, capacity, LimbType>& q);

/// quire_properties: return a string describing the quire configuration
template<typename NumberType, unsigned capacity = 30, typename LimbType = uint32_t>
std::string quire_properties() {
	using QT = quire_traits<NumberType>;
	constexpr unsigned qbits = QT::range + capacity;
	std::stringstream ss;
	ss << "Properties of a quire<" << type_tag(NumberType{}) << ", " << capacity << ">\n";
	ss << "  dynamic range of product   : " << QT::range << '\n';
	ss << "  radix point of accumulator : " << QT::radix_point << '\n';
	ss << "  full  quire size in bits   : " << qbits << '\n';
	ss << "  lower range in bits        : " << QT::half_range << '\n';
	ss << "  upper range in bits        : " << QT::upper_range << '\n';
	ss << "  capacity bits              : " << capacity << '\n';
	ss << "  limb type                  : " << typeid(LimbType).name() << '\n';
	ss << "  limb size                  : " << sizeof(LimbType) * 8 << " bits\n";
	return ss.str();
}

/*
 generalized quire: a super-accumulator parameterized on the number type

 The quire is a fixed-point accumulator sized to hold the full dynamic range
 of products for the given number system. It uses quire_traits<NumberType> to
 determine the accumulator width, and stores the magnitude in a single
 blockbinary using fast limb-based carry propagation.

 Template parameters:
   NumberType - the scalar type being accumulated (posit, cfloat, fixpnt, lns, dbns)
   capacity   - power-of-2 overflow guard bits (default 30, allows ~2^30 accumulations)
   LimbType   - the unsigned integer type for limbs (uint32_t or uint64_t)

 All values in and out of the quire are (sign, scale, significand) triplets,
 represented as blocktriple values from arithmetic operations.
*/
template<typename NumberType, unsigned capacity = 30, typename LimbType = uint32_t>
class quire {
public:
	using Traits = quire_traits<NumberType>;

	static constexpr unsigned range       = Traits::range;
	static constexpr unsigned half_range  = Traits::half_range;
	static constexpr unsigned radix_point = Traits::radix_point;
	static constexpr unsigned upper_range = Traits::upper_range;
	static constexpr unsigned qbits       = range + capacity;

	// the accumulator: unsigned magnitude with sign managed externally
	using accumulator_type = blockbinary<qbits, LimbType, BinaryNumberType::Unsigned>;

	// Constructors
	quire() : _sign(false), _accu{} {}
	quire(int8_t   iv) { *this = static_cast<int64_t>(iv); }
	quire(int16_t  iv) { *this = static_cast<int64_t>(iv); }
	quire(int32_t  iv) { *this = static_cast<int64_t>(iv); }
	quire(int64_t  iv) { *this = iv; }
	quire(uint64_t iv) { *this = iv; }
	quire(float    iv) { *this = iv; }
	quire(double   iv) { *this = iv; }

	// construct from a blocktriple (any operator tag)
	template<unsigned fbits, BlockTripleOperator op, typename bt>
	quire(const blocktriple<fbits, op, bt>& rhs) { *this = rhs; }

	// ====================================================================
	// Assignment from blocktriple (the primary input path)
	// ====================================================================
	template<unsigned fbits, BlockTripleOperator op, typename bt>
	quire& operator=(const blocktriple<fbits, op, bt>& rhs) {
		reset();
		if (rhs.iszero()) return *this;
		if (rhs.isinf() || rhs.isnan()) throw operand_is_nar{};
		_sign = rhs.sign();

		int scale = rhs.scale();
		if (scale >  static_cast<int>(half_range)) throw operand_too_large_for_quire{};
		if (scale < -static_cast<int>(half_range)) throw operand_too_small_for_quire{};

		place_blocktriple(rhs);
		return *this;
	}

	// Assignment from native integers
	quire& operator=(int64_t rhs) {
		clear();
		_sign = (rhs < 0);
		uint64_t magnitude = static_cast<uint64_t>(_sign ? -rhs : rhs);
		if (magnitude == 0) return *this;
		unsigned msb = find_msb(magnitude);
		if (msb > half_range + capacity) throw operand_too_large_for_quire{};
		// place integer value at the radix point (integers have scale >= 0)
		for (unsigned i = 0; i < 64 && i < msb; ++i) {
			if (magnitude & (uint64_t(1) << i)) {
				_accu.setbit(radix_point + i);
			}
		}
		return *this;
	}
	quire& operator=(uint64_t rhs) {
		clear();
		_sign = false;
		if (rhs == 0) return *this;
		unsigned msb = find_msb(rhs);
		if (msb > half_range + capacity) throw operand_too_large_for_quire{};
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
		blocktriple<23, BlockTripleOperator::REP, uint32_t> v(rhs);
		return *this = v;
	}
	quire& operator=(double rhs) {
		reset();
		if (rhs == 0.0) return *this;
		blocktriple<52, BlockTripleOperator::REP, uint32_t> v(rhs);
		return *this = v;
	}

	// ====================================================================
	// Accumulation operators (the core quire operations)
	// ====================================================================

	/// Add a blocktriple value to the quire
	template<unsigned fbits, BlockTripleOperator op, typename bt>
	quire& operator+=(const blocktriple<fbits, op, bt>& rhs) {
		if (rhs.iszero()) return *this;
		if (rhs.isinf() || rhs.isnan()) throw operand_is_nar{};

		int scale = rhs.scale();
		if (scale >  static_cast<int>(half_range)) throw operand_too_large_for_quire{};
		if (scale < -static_cast<int>(half_range)) throw operand_too_small_for_quire{};

		if (_sign == rhs.sign()) {
			add_blocktriple(rhs);
		}
		else {
			// subtract magnitudes
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

	/// Subtract a blocktriple value from the quire
	template<unsigned fbits, BlockTripleOperator op, typename bt>
	quire& operator-=(const blocktriple<fbits, op, bt>& rhs) {
		// negate the rhs and add
		blocktriple<fbits, op, bt> neg(rhs);
		neg.setsign(!rhs.sign());
		return *this += neg;
	}

	/// Add two quires
	quire& operator+=(const quire& rhs) {
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
		return *this += neg;
	}

	// ====================================================================
	// Selectors
	// ====================================================================
	bool iszero() const noexcept { return _accu.none(); }
	bool sign() const noexcept { return _sign; }
	bool isneg() const noexcept { return _sign; }
	bool ispos() const noexcept { return !_sign; }
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
		if (index >= qbits) throw std::out_of_range("quire bit index out of range");
		return _accu.test(index);
	}

	/// Check if any bit is set at or below the given index
	bool anyAfter(unsigned index) const noexcept {
		for (int i = static_cast<int>(index); i >= 0; --i) {
			if (_accu.test(static_cast<unsigned>(i))) return true;
		}
		return false;
	}

	// ====================================================================
	// Modifiers
	// ====================================================================
	void reset() noexcept {
		_sign = false;
		_accu.clear();
	}
	void clear() noexcept { reset(); }
	void set_sign(bool v) noexcept { _sign = v; }

	// ====================================================================
	// Conversion: extract the accumulated value as a blocktriple
	// ====================================================================

	/// Convert quire value to a blocktriple<qbits, REP>
	blocktriple<qbits, BlockTripleOperator::REP, LimbType> to_blocktriple() const {
		blocktriple<qbits, BlockTripleOperator::REP, LimbType> result;
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
	/// NOTE: This implementation extracts at most 53 significant bits (double precision).
	/// For target types with more than 53 bits of significand, a direct bit-extraction
	/// path would be needed for full accuracy.
	template<typename TargetType>
	TargetType convert_to() const {
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

	// ====================================================================
	// Direct access to the accumulator (for testing and diagnostics)
	// ====================================================================
	const accumulator_type& accumulator() const noexcept { return _accu; }

	// ====================================================================
	// String representation
	// ====================================================================

	/// Load from a bit string in format "+:cccc_uuuu.llll"
	bool load_bits(const std::string& string_of_bits) {
		reset();
		auto it = string_of_bits.begin();
		if (it == string_of_bits.end()) return false;
		if (*it == '-') _sign = true;
		else if (*it == '+') _sign = false;
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
				seen_radix = true;
				radix_pos = static_cast<unsigned>(bits.size());
				continue;
			}
			bits.push_back(*it == '1');
		}
		if (!seen_radix) return false;
		// bits[0] is MSB, place from top of accumulator
		unsigned total = static_cast<unsigned>(bits.size());
		unsigned lower_bits = total - radix_pos;
		// the radix in our accumulator is at position radix_point
		// lower_bits go below the radix, upper bits go above
		for (unsigned i = 0; i < radix_pos; ++i) {
			unsigned accu_bit = radix_point + radix_pos - 1 - i;
			if (accu_bit < qbits && bits[i]) {
				_accu.setbit(accu_bit);
			}
		}
		for (unsigned i = 0; i < lower_bits; ++i) {
			int accu_bit = static_cast<int>(radix_point) - 1 - static_cast<int>(i);
			if (accu_bit >= 0 && bits[radix_pos + i]) {
				_accu.setbit(static_cast<unsigned>(accu_bit));
			}
		}
		return true;
	}

private:
	bool             _sign;
	accumulator_type _accu;

	// ====================================================================
	// Internal helpers for blocktriple accumulation
	// ====================================================================

	/// Compute the accumulator base offset for a blocktriple value.
	/// The MSB of the significand represents 2^scale, so bit i in the
	/// significand represents 2^(scale + i - msb_pos), which maps to
	/// accumulator position: radix_point + scale + i - msb_pos.
	/// We return (radix_point + scale - msb_pos) so callers just add i.
	template<unsigned fbits, BlockTripleOperator op, typename bt>
	static int accu_base_offset(const blocktriple<fbits, op, bt>& v) {
		constexpr unsigned bfbits = blocktriple<fbits, op, bt>::bfbits;
		// find the MSB of the significand
		int msb_pos = -1;
		for (int i = static_cast<int>(bfbits) - 1; i >= 0; --i) {
			if (v.test(static_cast<unsigned>(i))) {
				msb_pos = i;
				break;
			}
		}
		return static_cast<int>(radix_point) + v.scale() - msb_pos;
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

// ====================================================================
// Free functions
// ====================================================================

template<typename NumberType, unsigned capacity, typename LimbType>
quire<NumberType, capacity, LimbType> abs(const quire<NumberType, capacity, LimbType>& q) {
	quire<NumberType, capacity, LimbType> result(q);
	result._sign = false;
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

// ====================================================================
// Stream operators
// ====================================================================

template<typename NumberType, unsigned capacity, typename LimbType>
std::ostream& operator<<(std::ostream& ostr, const quire<NumberType, capacity, LimbType>& q) {
	constexpr unsigned rp = quire<NumberType, capacity, LimbType>::radix_point;
	constexpr unsigned qb = quire<NumberType, capacity, LimbType>::qbits;
	ostr << (q._sign ? "-:" : "+:");
	// print capacity + upper bits (above radix), then '.', then lower bits
	for (int i = static_cast<int>(qb) - 1; i >= static_cast<int>(rp); --i) {
		ostr << (q._accu.test(static_cast<unsigned>(i)) ? '1' : '0');
	}
	ostr << '.';
	for (int i = static_cast<int>(rp) - 1; i >= 0; --i) {
		ostr << (q._accu.test(static_cast<unsigned>(i)) ? '1' : '0');
	}
	return ostr;
}

// ====================================================================
// Comparison operators
// ====================================================================

template<typename NumberType, unsigned capacity, typename LimbType>
bool operator==(const quire<NumberType, capacity, LimbType>& lhs,
                const quire<NumberType, capacity, LimbType>& rhs) {
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
	return !(rhs < lhs);
}

template<typename NumberType, unsigned capacity, typename LimbType>
bool operator>=(const quire<NumberType, capacity, LimbType>& lhs,
                const quire<NumberType, capacity, LimbType>& rhs) {
	return !(lhs < rhs);
}

}} // namespace sw::universal
