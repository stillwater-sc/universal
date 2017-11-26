#pragma once
// quire.hpp: definition of a parameterized quire configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// template class representing a quire associated with a posit configuration
// nbits and es are the same as the posit configuration, 
// capacity indicates the power of 2 number of accumulations the quire can support
template<size_t nbits, size_t es, size_t capacity = 30>
class quire {
public:
	static constexpr size_t escale = size_t(1) << es;         // 2^es
	static constexpr size_t range = escale * (4 * nbits - 8); // dynamic range of the posit configuration
	static constexpr size_t half_range = range >> 1;          // position of the fixed point
	static constexpr size_t qbits = range + capacity;         // size of the quire minus the sign bit: we are managing the sign explicitly
	quire() : _sign(false), _capacity(0), _upper(0), _lower(0) {}
	quire(int8_t initial_value) {
		*this = initial_value;
	}
	quire(int16_t initial_value) {
		*this = initial_value;
	}
	quire(int32_t initial_value) {
		*this = initial_value;
	}
	quire(int64_t initial_value) {
		*this = initial_value;
	}
	quire(uint64_t initial_value) {
		*this = initial_value;
	}
	quire(float initial_value) {
		*this = initial_value;
	}
	quire(double initial_value) {
		*this = initial_value;
	}
	template<size_t fbits>
	quire(const value<fbits>& rhs) {
		*this = rhs;
	}
	// TODO: we are clamping the values of the RHS to be withing the dynamic range of the posit
	// TODO: however, on the upper side we also have the capacity bits, which gives us the opportunity to accept larger scale values than the dynamic range of the posit.
	// TODO: is that a good idea?
	template<size_t fbits>
	quire& operator=(const value<fbits>& rhs) {
		reset();
		_sign = rhs.sign();
		int i,f, scale = rhs.scale();
		if (scale >= int(half_range)) {
			throw "RHS value too large for quire";
		}
		if (scale < -int(half_range)) {
			throw "RHS value too small for quire";
		}
		std::bitset<fbits+1> fraction = rhs.get_fixed_point();
		// divide bits between upper and lower accumulator
		if (scale - int(fbits) >= 0) {
			// all upper accumulator
			for (i = scale, f = int(fbits); i >= 0 && f >= 0; i--, f--) {
				_upper[i] = fraction[f];
			}
		}
		else if (scale < 0) {
			// all lower accumulator
			for (i = half_range + scale, f = int(fbits); i >= 0 && f >= 0; i--, f--) {
				_lower[i] = fraction[f];
			}
		}
		else {
			// part upper, and part lower accumulator
			// first assign the bits in the upper accumulator
			for (i = scale, f = int(fbits); i >= 0 && f >= 0; i--, f--) {
				_upper[i] = fraction[f];
			}
			// next assign the bits in the lower accumulator
			for (i = half_range - 1; i >= 0 && f >= 0; i--, f--) {
				_lower[i] = fraction[f];
			}
		}
		return *this;
	}
	quire& operator=(int8_t rhs) {
		*this = int64_t(rhs);
		return *this;
	}
	quire& operator=(int16_t rhs) {
		*this = int64_t(rhs);
		return *this;
	}
	quire& operator=(int32_t rhs) {
		*this = int64_t(rhs);
		return *this;
	}
	quire& operator=(int64_t rhs) {
		reset();
		// transform to sign-magnitude
		_sign = rhs & 0x8000000000000000;
		uint64_t magnitude;
		magnitude = _sign ? -rhs : rhs;
		unsigned msb = findMostSignificantBit(magnitude);
		if (msb > half_range + capacity) {
			throw "RHS value too large for quire";
		}
		else {
			// copy the value into the quire
			unsigned i, c;
			uint64_t mask = uint64_t(1);
			for (i = 0; i < msb && i < half_range; i++) {
				_upper[i] = magnitude & mask;
				mask <<= 1;
			}
			if (msb >= half_range) {
				for (i = half_range, c = 0; i < msb && i < half_range + capacity; i++, c++) {
					_capacity[c] = magnitude & mask;
					mask <<= 1;
				}
			}
		}
		return *this;
	}
	quire& operator=(uint64_t rhs) {
		reset();
		unsigned msb = findMostSignificantBit(rhs);
		if (msb > half_range + capacity) {
			throw "Assigned value too large for quire";
		}
		else {
			// copy the value into the quire
			unsigned i, c;
			uint64_t mask = uint64_t(1);
			for (i = 0; i < msb && i < half_range; i++) {
				_upper[i] = rhs & mask;
				mask <<= 1;
			}
			if (msb >= half_range) {
				for (i = half_range, c = 0; i < msb && i < half_range + capacity; i++, c++) {
					_capacity[c] = rhs & mask;
					mask <<= 1;
				}
			}
		}
		return *this;
	}
	quire& operator=(float rhs) {
		constexpr int float_bits = std::numeric_limits<float>::digits - 1;
		value<float_bits> v(rhs);
		*this = v;
		return *this;
	}
	quire& operator=(double rhs) {
		constexpr int double_bits = std::numeric_limits<double>::digits - 1;
		value<double_bits> v(rhs);
		*this = v;
		return *this;
	}
	template<size_t fbits>
	quire& operator+=(const value<fbits>& rhs) {
		int i, f, scale = rhs.scale();
		if (scale >= int(half_range)) {
			throw "RHS value too large for quire";
		}
		if (scale < -int(half_range)) {
			throw "RHS value too small for quire";
		}
		if (rhs.sign()) {
			// subtract
			throw "TBD";
		}
		else {
			// add
			// scale is the location of the msb in the fixed point representation
			// so scale  =  0 is the hidden bit at location 0, scale 1 = bit 1, etc.
			// and scale = -1 is the first bit of the fraction
			// we manage scale >= 0 in the _upper accumulator, and scale < 0 in the _lower accumulator
			int lsb = scale - int(fbits) - 1;
			uint8_t carry = 0;
			std::bitset<fbits + 1> fraction = rhs.get_fixed_point();
			// divide bits between upper and lower accumulator
			if (lsb >= 0) {	// all upper accumulator
				carry = 0;
				for (i = scale - int(fbits) - 1, f = 0; i <= scale && f <= int(fbits); i++, f++) {
					uint8_t _a = _upper[i];
					uint8_t _b = fraction[f];
					uint8_t _slice = _a + _b + carry;
					carry = _slice >> 1;
					_upper[i] = (0x1 & _slice);
				}
				while (carry && i < half_range + capacity) {
					uint8_t _a = _upper[i];
					uint8_t _slice = _a + carry;
					carry = _slice >> 1;
					_upper[i] = (0x1 & _slice);
					i++;
				}
			}
			else if (scale < 0) {		// all lower accumulator
				carry = 0;
				int lsb = int(half_range) + scale - int(fbits);
				int qlsb = lsb > 0 ? lsb : 0;
				int flsb = lsb >= 0 ? 0 : -lsb;
				for (i = qlsb, f = flsb; i < int(half_range) && f <= int(fbits); i++, f++) {
					uint8_t _a = _lower[i];
					uint8_t _b = fraction[f];
					uint8_t _slice = _a + _b + carry;
					carry = _slice >> 1;
					_lower[i] = (0x1 & _slice);
				}
				while (carry && i < half_range) {
					uint8_t _a = _lower[i];
					uint8_t _slice = _a + carry;
					carry = _slice >> 1;
					_lower[i] = (0x1 & _slice);
					i++;
				}
				if (carry) {  // carry propagate to the _upper accumulator
					// need to increment the _upper
					i = 0;
					while (carry && i < half_range + capacity) {
						uint8_t _a = _upper[i];
						uint8_t _slice = _a + carry;
						carry = _slice >> 1;
						_upper[i] = (0x1 & _slice);
						i++;
					}
				}
			}
			else {
				// part upper, and part lower accumulator
				// first add the lower accumulator component
				carry = 0;
				lsb = int(half_range) + lsb + 1; // remember lsb is negative in this block
				int qlsb = lsb > 0 ? lsb : 0;
				int flsb = lsb >= 0 ? 0 : -lsb;
				for (i = qlsb, f = flsb; i < int(half_range) && f <= int(fbits); i++, f++) {
					uint8_t _a = _lower[i];
					uint8_t _b = fraction[f];
					uint8_t _slice = _a + _b + carry;
					carry = _slice >> 1;
					_lower[i] = (0x1 & _slice);
				}
				// next add the bits in the upper accumulator
				for (i = 0; i <= scale && f <= int(fbits); i++, f++) {
					uint8_t _a = _upper[i];
					uint8_t _b = fraction[f];
					uint8_t _slice = _a + _b + carry;
					carry = _slice >> 1;
					_upper[i] = (0x1 & _slice);
				}
				while (carry && i < half_range + capacity) {
					uint8_t _a = _upper[i];
					uint8_t _slice = _a + carry;
					carry = _slice >> 1;
					_upper[i] = (0x1 & _slice);
					i++;
				}
			}
		}
		return *this;
	}
	void reset() {
		_sign  = false;
		_lower.reset();
		_upper.reset();
		_capacity.reset();
	}
	size_t dynamic_range() const { return range; }
	size_t upper_range() const { return half_range; }
	size_t lower_range() const { return half_range; }
	size_t capacity_range() const { return capacity; }
	bool isNegative() const { return _sign; }
	bool isZero() const { return _capacity.none() && _upper.none() && _lower.none(); }

	double sign_value() const {	return (_sign ? -1.0 : 1.0); }
	double to_double() const {
		return 0.0; // TODO
	}

private:
	bool				     _sign;
	std::bitset<half_range>  _lower, _upper;  // to demonstrate potential hw concurrency for high performance quires
	std::bitset<capacity>    _capacity;

	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nnbits, size_t nes, size_t ncapacity>
	friend std::ostream& operator<< (std::ostream& ostr, const quire<nnbits,nes,ncapacity>& q);
	template<size_t nnbits, size_t nes, size_t ncapacity>
	friend std::istream& operator>> (std::istream& istr, quire<nnbits, nes, ncapacity>& q);

	template<size_t nnbits, size_t nes, size_t ncapacity>
	friend bool operator==(const quire<nnbits, nes, ncapacity>& lhs, const quire<nnbits, nes, ncapacity>& rhs);
	template<size_t nnbits, size_t nes, size_t ncapacity>
	friend bool operator!=(const quire<nnbits, nes, ncapacity>& lhs, const quire<nnbits, nes, ncapacity>& rhs);
	template<size_t nnbits, size_t nes, size_t ncapacity>
	friend bool operator< (const quire<nnbits, nes, ncapacity>& lhs, const quire<nnbits, nes, ncapacity>& rhs);
	template<size_t nnbits, size_t nes, size_t ncapacity>
	friend bool operator> (const quire<nnbits, nes, ncapacity>& lhs, const quire<nnbits, nes, ncapacity>& rhs);
	template<size_t nnbits, size_t nes, size_t ncapacity>
	friend bool operator<=(const quire<nnbits, nes, ncapacity>& lhs, const quire<nnbits, nes, ncapacity>& rhs);
	template<size_t nnbits, size_t nes, size_t ncapacity>
	friend bool operator>=(const quire<nnbits, nes, ncapacity>& lhs, const quire<nnbits, nes, ncapacity>& rhs);
};


// QUIRE BINARY ARITHMETIC OPERATORS
template<size_t nbits, size_t es, size_t capacity>
inline quire<nbits, es, capacity> operator+(const quire<nbits, es, capacity>& lhs, const quire<nbits, es, capacity>& rhs) {
	quire<nbits, es, capacity> sum = lhs;
	sum += rhs;
	return sum;
}


////////////////// QUIRE operators
template<size_t nbits, size_t es, size_t capacity>
inline std::ostream& operator<<(std::ostream& ostr, const quire<nbits, es, capacity>& q) {
	ostr << (q._sign ? "-1" : " 1") << ": " << q._capacity << "_" << q._upper << "." << q._lower;
	return ostr;
}

template<size_t nbits, size_t es, size_t capacity>
inline std::istream& operator>> (std::istream& istr, const quire<nbits, es, capacity>& q) {
	istr >> q._accu;
	return istr;
}

template<size_t nbits, size_t es, size_t capacity>
inline bool operator==(const quire<nbits, es, capacity>& lhs, const quire<nbits, es, capacity>& rhs) { return lhs._sign == rhs._sign && lhs._capacity == rhs._capacity && lhs._upper == rhs._upper && lhs._lower == rhs._lower; }
template<size_t nbits, size_t es, size_t capacity>
inline bool operator!=(const quire<nbits, es, capacity>& lhs, const quire<nbits, es, capacity>& rhs) { return !operator==(lhs, rhs); }
template<size_t nbits, size_t es, size_t capacity>
inline bool operator< (const quire<nbits, es, capacity>& lhs, const quire<nbits, es, capacity>& rhs) { 
	bool bSmaller = false;
	if (!lhs._sign && rhs._sign) {
		bSmaller = true;
	}
	else if (lhs._sign == rhs._sign) {
		if (lhs._capacity < rhs._capacity) {
			bSmaller = true;
		}
		else if (lhs._capacity == rhs._capacity && lhs._upper < rhs._upper) {
			bSmaller = true;
		}
		else if (lhs._capacity == rhs._capacity && lhs._upper == rhs._upper && lhs._lower < rhs._lower) {
			bSmaller = true;
		}
	}
	return bSmaller;
}
template<size_t nbits, size_t es, size_t capacity>
inline bool operator> (const quire<nbits, es, capacity>& lhs, const quire<nbits, es, capacity>& rhs) { return  operator< (rhs, lhs); }
template<size_t nbits, size_t es, size_t capacity>
inline bool operator<=(const quire<nbits, es, capacity>& lhs, const quire<nbits, es, capacity>& rhs) { return !operator> (lhs, rhs) || lhs == rhs; }
template<size_t nbits, size_t es, size_t capacity>
inline bool operator>=(const quire<nbits, es, capacity>& lhs, const quire<nbits, es, capacity>& rhs) { return !operator< (lhs, rhs) || lhs == rhs; }

