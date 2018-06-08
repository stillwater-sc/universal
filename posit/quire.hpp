#pragma once
// quire.hpp: definition of a parameterized quire configurations
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
	namespace unum {

// Forward definitions
template<size_t nbits, size_t es, size_t capacity> class quire;
template<size_t nbits, size_t es, size_t capacity> quire<nbits, es, capacity> abs(const quire<nbits, es, capacity>& v);

/* 
 quire: template class representing a quire associated with a posit configuration
 nbits and es are the same as the posit configuration, 
 capacity indicates the power of 2 number of accumulations of maxpos^2 the quire can support

 All values in and out of the quire are normalized (sign, scale, fraction) triplets.
 Even though a quire is very strongly coupled to a posit configuration via the dynamic range
 a particular posit configuration exhibits, the class is designed to NOT depend on the posit<nbits,es> class definition.
 */
template<size_t nbits, size_t es, size_t capacity = 30>
class quire {
public:
	static constexpr size_t escale = size_t(1) << es;         // 2^es
	static constexpr size_t range = escale * (4 * nbits - 8); // dynamic range of the posit configuration
	static constexpr size_t half_range = range >> 1;          // position of the fixed point
	static constexpr size_t radix_point = half_range;
	static constexpr size_t upper_range = half_range + 1;     // size of the upper accumulator
	static constexpr size_t qbits = range + capacity;     // size of the quire minus the sign bit: we are managing the sign explicitly
	
	// Constructors
	quire() : _sign(false) { _capacity.reset(); _upper.reset(); _lower.reset(); }
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

	// Assignment operators: the class only supports native type values
	// assigning a posit requires the convertion to a normalized value, i.e. q = posit<nbits,es>().to_value()

	// operator=() takes a normalized (sign, scale, fraction) triplet
	template<size_t fbits>
	quire& operator=(const value<fbits>& rhs) {
		reset();
		if (rhs.isZero()) return *this;
		if (rhs.isInfinite() || rhs.isNaN())           throw "NaR not implemented";
		_sign = rhs.sign();

		int scale = rhs.scale();
		// TODO: we are clamping the values of the RHS to be within the dynamic range of the posit
		// TODO: however, on the upper side we also have the capacity bits, which gives us the opportunity 
		// TODO: to accept larger scale values than the dynamic range of the posit.
		// TODO: When you are assigning the sum of quires you could hit this condition.
		if (scale >  int(half_range)) 	throw "RHS value too large for quire";
		if (scale < -int(half_range)) 	throw "RHS value too small for quire";

		int i, f; // running bit pointers, i for the quire, f for the incoming fraction
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
		clear();
		// transform to sign-magnitude
		_sign = rhs & 0x8000000000000000;
		unsigned long long magnitude;
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
	quire& operator=(unsigned long long rhs) {
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
		constexpr int bits = std::numeric_limits<float>::digits - 1;
		*this = value<bits>(rhs);
		return *this;
	}
	quire& operator=(double rhs) {
		constexpr int bits = std::numeric_limits<double>::digits - 1;
		*this = value<bits>(rhs);
		return *this;
	}
	quire& operator=(long double rhs) {
		constexpr int bits = std::numeric_limits<long double>::digits - 1;	
		*this = value<bits>(rhs);
		return *this;
	}

	// Add a normalized value to the quire value. 
	// All values in (and out) of the quire are normalized (sign, scale, fraction) triplets.
	template<size_t fbits>
	quire& operator+=(const value<fbits>& rhs) {
		if (rhs.isZero()) return *this;

		if (rhs.scale() >  int(half_range)) {
			throw "RHS value too large for quire";
		}
		if (rhs.scale() < -int(half_range)) {
			throw "RHS value too small for quire";
		}
		// sign/magnitude classification
		// operation      add magnitudes           subtract magnitudes
		//                                     a < b       a = b      a > b
		// (+a) + (+b)      +(a + b)
		// (+a) + (-b)                       -(b - a)    +(a - b)   +(a - b)
		// (-a) + (+b)                       +(b - a)    +(a - b)   -(a - b)
		// (-a) + (-b)      -(a + b)
		if (_sign == rhs.sign()) {
			add_value(rhs);
			// _sign stays the same, so nothing new to assign
		}
		else {
			// subtract magnitudes
			int cmp = CompareMagnitude(rhs);
			if (cmp < 0) {
				// TODO: is there a way to NOT have to swap the whole quire?
				value<qbits> subtractend = this->to_value();
				*this = rhs;
				subtract_value(subtractend);
				_sign = rhs.sign();
			}
			else if (cmp > 0) {
				subtract_value(rhs);
				// _sign stays the same
			}
			else {
				subtract_value(rhs);
				_sign = false;
			}
		}
		return *this;
	}
	
	// Subtract a normalized value from the quire value
	template<size_t fbits>
	quire& operator-=(const value<fbits>& rhs) {
		return *this += -rhs;
	}
	// bit addressing operator
	bool operator[](int index) const {
		if (index < int(radix_point)) return _lower[index];
		if (index < int(radix_point) + int(upper_range)) return _upper[index - int(radix_point)];
		if (index < int(radix_point) + int(upper_range) + int(capacity)) return _capacity[index - int(radix_point) - int(upper_range)];
		throw "index out of range";
	}

	// Modifiers

	// state management operators
	// reset the state of a quire to zero
	void reset() {
		_sign  = false;
		_lower.reset();
		_upper.reset();
		_capacity.reset();
	}
	// semantic sugar: clear the state of a quire to zero
	void clear() { reset(); }
	void set_sign(bool v) { _sign = v; }

	// Selectors
	
	// Compare magnitudes between quire and value: returns -1 if q < v, 0 if q == v, and 1 if q > v
	template<size_t fbits>
	int CompareMagnitude(const value<fbits>& v) {
		// inefficient as we are copying a whole quire just to reset the sign bit, but we are leveraging the comparison logic
		quire<nbits, es, capacity> absq = abs(*this);
		value<fbits> absv = abs(v);
		if (absq < absv) {
			return -1;
		}
		else if (absq > absv) {
			return 1;
		}
		return 0;
	}
	// query functions for quire attributes
	int dynamic_range() const { return range; }
	int max_scale() const { return upper_range; }
	int min_scale() const { return -int(half_range); }
	int capacity_range() const { return capacity; }
	bool isNegative() const { return _sign; }
	bool isZero() const { return _capacity.none() && _upper.none() && _lower.none(); }
	int scale() const {
		int msb = int(capacity)-1; // indicative of no bits set
		for (; msb >= 0; msb--) {
			if (_capacity.test(msb)) break;
		}
		if (msb >= 0) return msb + upper_range;
		for (msb = int(upper_range) - 1; msb >= 0; msb--) {
			if (_upper.test(msb)) break;
		}
		if (msb >= 0) return msb;
		for (int i = int(half_range) - 1; i >= 0; i--, msb--) {
			if (_lower.test(i)) break;
		}
		return msb;
	}

	// Return value of the sign bit: true indicates a negative number, false a positive number or zero
	bool sign() const { return _sign; }
	float sign_value() const {	return (_sign ? -1.0 : 1.0); }
	bitblock<qbits+1> get() const {
		bitblock<qbits+1> q;
		int msb = 0;
		for (int i = 0; i < half_range; i++) {
			q[msb] = _lower[i];
			msb++;
		}
		for (int i = 0; i < upper_range; i++) {
			q[msb] = _upper[i];
			msb++;
		}
		for (int i = 0; i < capacity; i++) {
			q[msb] = _capacity[i];
			msb++;
		}
		return q;
	}
	value<qbits> to_value() const {
		// find the MSB and build the fraction
		bitblock<qbits> fraction;
		bool isZero = false;
		bool isNaR = false;   // TODO
		int i;
		int qbit = int(qbits);
		int fbit = qbit - 1;
		int scale = qbits;
		for (i = int(capacity) - 1; i >= 0; i--, qbit--) {
			if (scale == qbits) {
				if (_capacity.test(i)) scale = qbit - int(half_range);
			}
			else {
				fraction[fbit--] = _capacity[i];
			}
		}
		for (i = int(upper_range) - 1; i >= 0; i--, qbit--) {
			if (scale == qbits) {
				if (_upper.test(i)) scale = qbit - int(half_range);
			}
			else {
				fraction[fbit--] = _upper[i];
			}
		}
		for (i = int(half_range) - 1; i >= 0; i--, qbit--) {
			if (scale == qbits) {
				if (_lower.test(i)) scale = qbit - int(half_range);
			}
			else {
				fraction[fbit--] = _lower[i];
			}
		}
		if (scale == qbits) {
			isZero = true;
			scale = 0;
		}
		return value<qbits>(_sign, scale, fraction, isZero, isNaR);
	}
	bool anyAfter(int index) const {
		for (int i = index; i >= 0; i--) {
			if (this->operator[](i)) return true;
		}
		return false;
	}

private:
	bool				   _sign;
	// segmented accumulator to demonstrate potential hw concurrency for high performance quires
	bitblock<half_range>   _lower;
	bitblock<upper_range>  _upper;
	bitblock<capacity>     _capacity;

	// add a value to the quire
	template<size_t fbits>
	void add_value(const value<fbits>& v) {
		if (v.isZero()) return;
		// scale is the location of the msb in the fixed point representation
		// so scale  =  0 is the hidden bit at location 0, scale 1 = bit 1, etc.
		// and scale = -1 is the first bit of the fraction
		// we manage scale >= 0 in the _upper accumulator, and scale < 0 in the _lower accumulator
		int lsb = v.scale() - int(fbits);
		bool carry = false;
		std::bitset<fbits + 1> fraction = v.get_fixed_point();
		int i, f;  // bit pointers, i pointing to the quire bits, f pointing to the fraction bits of rhs
		// divide bits between upper and lower accumulator
		if (v.scale() < 0) {		// all lower accumulator
			int lsb = int(half_range) + v.scale() - int(fbits);
			int qlsb = lsb > 0 ? lsb : 0;
			int flsb = lsb >= 0 ? 0 : -lsb;
			for (i = qlsb, f = flsb; i < int(half_range) && f <= int(fbits); i++, f++) {
				bool _a = _lower[i];
				bool _b = fraction[f];
				_lower[i] = _a ^ _b ^ carry;
				carry = (_a & _b) | (carry & (_a ^ _b));
			}
			// propagate any carries to the end of the lower accumulator
			while (carry && i < half_range) {
				bool _a = _lower[i];
				_lower[i] = _a ^ carry;
				carry = carry & _a;
				i++;
			}
			if (carry) {  // carry propagate to the _upper accumulator
						  // need to increment the _upper
				i = 0;
				while (carry && i < upper_range) {
					bool _a = _upper[i];
					_upper[i] = _a ^ carry;
					carry = carry & _a;
					i++;
				}
				if (carry) {
					// next add the bits to the capacity segment
					i = 0;
					while (carry && i < capacity) {
						bool _a = _capacity[i];
						_capacity[i] = _a ^ carry;
						carry = carry & _a;
						i++;
					}
				}
			}
		}
		else if (lsb >= 0) {	// all upper accumulator
			for (i = lsb, f = 0; i <= v.scale() && f <= int(fbits); i++, f++) {
				bool _a = _upper[i];
				bool _b = fraction[f];
				_upper[i] = _a ^ _b ^ carry;
				carry = (_a & _b) | (carry & (_a ^ _b));
			}
			while (carry && i < int(upper_range)) {
				bool _a = _upper[i];
				_upper[i] = _a ^ carry;
				carry = carry & _a;
				i++;
			}
			if (carry) {
				// next add the bits to the capacity segment
				i = 0;
				while (carry && i < capacity) {
					bool _a = _capacity[i];
					_capacity[i] = _a ^ carry;
					carry = carry & _a;
					i++;
				}
			}
		}
		else {  // lsb < 0 && scale > 0
				// part upper, and part lower accumulator
				// first add the lower accumulator component
			lsb = int(half_range) + lsb; // remember lsb is negative in this block
			int qlsb = lsb > 0 ? lsb : 0;
			int flsb = lsb >= 0 ? 0 : -lsb;
			for (i = qlsb, f = flsb; i < int(half_range) && f <= int(fbits); i++, f++) {
				bool _a = _lower[i];
				bool _b = fraction[f];
				_lower[i] = _a ^ _b ^ carry;
				carry = (_a & _b) | (carry & (_a ^ _b));
			}
			// next add the bits in the upper accumulator
			for (i = 0; i <= v.scale() && f <= int(fbits); i++, f++) {
				bool _a = _upper[i];
				bool _b = fraction[f];
				_upper[i] = _a ^ _b ^ carry;
				carry = (_a & _b) | (carry & (_a ^ _b));
			}
			// propagate any carries to the end of the upper accumulator
			while (carry && i < int(upper_range)) {
				bool _a = _upper[i];
				_upper[i] = _a ^ carry;
				carry = carry & _a;
				i++;
			}
			// next add the bits to the capacity segment
			if (carry) {
				i = 0;
				while (carry && i < capacity) {
					bool _a = _capacity[i];
					_capacity[i] = _a ^ carry;
					carry = carry & _a;
					i++;
				}
			}
		}
	}
	// subtract a value from the quire
	template<size_t fbits>
	void subtract_value(const value<fbits>& v) {
		if (v.isZero()) return;
		// lsb in the quire of the lowest bit of the explicit fixed point value including the hidden bit of the fraction
		int lsb = v.scale() - int(fbits);
		bool borrow = false;
		std::bitset<fbits + 1> fraction = v.get_fixed_point();
		int i, f;  // bit pointers, i pointing to the quire bits, f pointing to the fraction bits of rhs
		// divide bits between upper and lower accumulator
		if (v.scale() < 0) {		// all lower accumulator
			int lsb = int(half_range) + v.scale() - int(fbits);
			int qlsb = lsb > 0 ? lsb : 0;
			int flsb = lsb >= 0 ? 0 : -lsb;
			for (i = qlsb, f = flsb; i < int(half_range) && f <= int(fbits); i++, f++) {
				bool _a = _lower[i];
				bool _b = fraction[f];
				_lower[i] = _a ^ _b ^ borrow;
				borrow = (!_a & _b) | (!(!_a ^ !_b) & borrow);
			}
			// propagate any borrows to the end of the lower accumulator
			while (borrow && i < half_range) {
				bool _a = _lower[i];
				_lower[i] = _a ^ borrow;
				borrow = borrow & !_a;
				i++;
			}
			if (borrow) { // borrow propagate to the _upper accumulator
						  // need to decrement the _upper
				i = 0;
				while (borrow && i < upper_range) {
					bool _a = _upper[i];
					_upper[i] = _a ^ borrow;
					borrow = borrow & !_a;
					i++;
				}
				if (borrow) {
					// propagate the borrow into the capacity segment
					i = 0;
					while (borrow && i < capacity) {
						bool _a = _capacity[i];
						_capacity[i] = _a ^ borrow;
						borrow = borrow & !_a;
						i++;
					}
				}
			}
		}
		else if (lsb >= 0) {	// all upper accumulator
			for (i = lsb, f = 0; i <= v.scale() && f <= int(fbits); i++, f++) {
				bool _a = _upper[i];
				bool _b = fraction[f];
				_upper[i] = _a ^ _b ^ borrow;
				borrow = (!_a & _b) | (!(!_a ^ !_b) & borrow);
			}
			// propagate any borrows to the end of the upper accumulator
			while (borrow && i < upper_range) {
				bool _a = _upper[i];
				_upper[i] = _a ^ borrow;
				borrow = borrow & !_a;
				i++;
			}
			if (borrow) {
				// propagate the borrow into the capacity segment
				i = 0;
				while (borrow && i < capacity) {
					bool _a = _capacity[i];
					_capacity[i] = _a ^ borrow;
					borrow = borrow & !_a;
					i++;
				}
			}
		}
		else {   // lsb < 0 && scale >= 0
				 // part upper, and part lower accumulator
				 // first add the lower accumulator component
			lsb = int(half_range) + lsb; // remember lsb is negative in this block
			int qlsb = lsb > 0 ? lsb : 0;
			int flsb = lsb >= 0 ? 0 : -lsb;
			for (i = qlsb, f = flsb; i < int(half_range) && f <= int(fbits); i++, f++) {
				bool _a = _lower[i];
				bool _b = fraction[f];
				_lower[i] = _a ^ _b ^ borrow;
				borrow = (!_a & _b) | (!(!_a ^ !_b) & borrow);
			}
			// next add the bits in the upper accumulator
			for (i = 0; i <= v.scale() && f <= int(fbits); i++, f++) {
				bool _a = _upper[i];
				bool _b = fraction[f];
				_upper[i] = _a ^ _b ^ borrow;
				borrow = (!_a & _b) | (!(!_a ^ !_b) & borrow);
			}
			// propagate any borrows to the end of the upper accumulator
			while (borrow && i < upper_range) {
				bool _a = _upper[i];
				_upper[i] = _a ^ borrow;
				borrow = borrow & !_a;
				i++;
			}
			if (borrow) {
				// propagate the borrow into the capacity segment
				i = 0;
				while (borrow && i < capacity) {
					bool _a = _capacity[i];
					_capacity[i] = _a ^ borrow;
					borrow = borrow & !_a;
					i++;
				}
			}
		}
	}

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

	// value comparisons
	template<size_t nnbits, size_t nes, size_t ncapacity, size_t nfbits>
	friend bool operator==(const quire<nnbits, nes, ncapacity>& q, const value<nfbits>& v);
	template<size_t nnbits, size_t nes, size_t ncapacity, size_t nfbits >
	friend bool operator< (const quire<nnbits, nes, ncapacity>& q, const value<nfbits>& v);
	template<size_t nnbits, size_t nes, size_t ncapacity, size_t nfbits >
	friend bool operator> (const quire<nnbits, nes, ncapacity>& q, const value<nfbits>& v);

};

// Magnitude of a quire
template<size_t nbits, size_t es, size_t capacity>
quire<nbits, es, capacity> abs(const quire<nbits, es, capacity>& q) {
	quire<nbits, es, capacity> magnitude(q);
	magnitude.set_sign(false);
	return magnitude;
}

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

// magnitude comparison between quire and value
template<size_t nbits, size_t es, size_t capacity, size_t fbits>
inline bool operator== (const quire<nbits, es, capacity>& q, const value<fbits>& v) {
	// not efficient, but leverages < and >
	bool bSmaller = q < v;
	bool bBigger = q > v;
	return !(q < v) && !(q > v);
}
template<size_t nbits, size_t es, size_t capacity, size_t fbits>
inline bool operator< (const quire<nbits, es, capacity>& q, const value<fbits>& v) {
	bool bSmaller = false;  // default fall through is quire q is bigger than value v
	if (!v.sign() && q.sign()) {
		bSmaller = true;
	}
	else if (q.sign() == v.sign()) {
		// compare magnitudes
		int qscale = q.scale();
		int vscale = v.scale();
		if (qscale < vscale) {
			bSmaller = true;
		}
		else if (qscale == vscale) {
			// got to compare the fraction bits
			std::bitset<fbits + 1> fixed = v.get_fixed_point();
			int i, f;  // bit pointers, i for the quire, f for the fraction in v
			bool undecided = true;
			for (i = quire<nbits, es, capacity>::radix_point + qscale, f = int(fbits); i >= 0 && f >= 0; --i, --f) {
				if (!q[i] && fixed[f]) {
					bSmaller = true;
					undecided = false;
					break;
				}
			}
			if (undecided && i >= 0) { // fraction bits have been identical
				// any bit set will make the quire bigger than the value
				if (q.anyAfter(i)) {
					bSmaller = false;
				}
			}
		}
	}
	return bSmaller;
}
template<size_t nbits, size_t es, size_t capacity, size_t fbits>
inline bool operator> (const quire<nbits, es, capacity>& q, const value<fbits>& v) { 
	bool bBigger = false;  // default fall through is quire q is smaller than value v
	if (!q.sign() && v.sign()) {
		bBigger = true;
	}
	else if (q.sign() == v.sign()) {
		// compare magnitudes
		int qscale = q.scale();
		int vscale = v.scale();
		if (qscale > vscale) {
			bBigger = true;
		}
		else if (qscale == vscale) {
			// got to compare the fraction bits
			std::bitset<fbits + 1> fixed_point = v.get_fixed_point();
			int i, f;  // bit pointers, i for the quire, f for the fraction in v
			bool undecided = true;
			for (i = quire<nbits, es, capacity>::radix_point + qscale, f = int(fbits); i >= 0 && f >= 0; --i, --f) {
				if (q[i] && !fixed_point[f]) {
					bBigger = true;
					undecided = false;
					break;
				}
			}
			if (undecided && i >= 0) { // fraction bits have been identical
									   // any bit set will make the quire bigger
				if (q.anyAfter(i)) {
					bBigger = true;
				}
			}
		}
	}
	return bBigger;
}

}  // namespace unum

}  // namespace sw
