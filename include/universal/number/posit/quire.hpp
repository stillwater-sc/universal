#pragma once
// quire.hpp: definition of a parameterized quire configurations for posits
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/native/boolean_logic_operators.hpp>
#include <universal/number/quire/exceptions.hpp>

namespace sw { namespace universal {

	using namespace sw::universal::internal;

// Forward definitions
template<size_t nbits, size_t es, size_t capacity> class quire;
template<size_t nbits, size_t es, size_t capacity> quire<nbits, es, capacity> abs(const quire<nbits, es, capacity>& q);
//template<size_t nbits, size_t es, size_t capacity> value<(size_t(1) << es)*(4*nbits-8)+capacity> abs(const quire<nbits, es, capacity>& q);

template<size_t nbits, size_t es, size_t capacity> 
std::string quire_properties() {
	constexpr size_t escale = size_t(1) << es;         // 2^es
	constexpr size_t range = escale * (4 * nbits - 8); // dynamic range of the posit configuration
	constexpr size_t half_range = range >> 1;          // position of the fixed point
	constexpr size_t radix_point = half_range;
	// the upper is 1 bit bigger than the lower because maxpos^2 has that scale
	constexpr size_t upper_range = half_range + 1;     // size of the upper accumulator
	constexpr size_t qbits = range + capacity;     // size of the quire minus the sign bit: we are managing the sign explicitly

	std::stringstream ss;
	ss << "Properties of a quire<" << nbits << ", " << es << ", " << capacity << ">\n";
	ss << "  dynamic range of product   : " << range << std::endl;
	ss << "  radix point of accumulator : " << radix_point << std::endl;
	ss << "  full  quire size in bits   : " << qbits << std::endl;
	ss << "  lower quire size in bits   : " << half_range << std::endl;
	ss << "  upper quire size in bits   : " << upper_range << std::endl;
	ss << "  capacity bits              : " << capacity << std::endl;
	return ss.str();
}

template<size_t nbits, size_t es, size_t capacity>
inline int quire_size() {
	constexpr size_t escale = size_t(1) << es;         // 2^es
	constexpr size_t range = escale * (4 * nbits - 8); // dynamic range of the posit configuration
	//constexpr size_t half_range = range >> 1;          // position of the fixed point
	//constexpr size_t radix_point = half_range;
	// the upper is 1 bit bigger than the lower because maxpos^2 has that scale
	//constexpr size_t upper_range = half_range + 1;     // size of the upper accumulator
	constexpr size_t qbits = range + capacity;     // size of the quire minus the sign bit: we are managing the sign explicitly

	return int(qbits);  // why int? so that we can do arithmetic on it 
}


// return the dynamic range of a posit product
template<size_t nbits, size_t es, size_t capacity>
inline int dynamic_range_product() {
	constexpr size_t escale = size_t(1) << es;         // 2^es
	constexpr size_t range = escale * (4 * nbits - 8); // dynamic range of the posit configuration
	return range;
}

// return the dynamic range of the full quire
template<size_t nbits, size_t es, size_t capacity>
inline int dynamic_range() { 
	constexpr size_t escale = size_t(1) << es;         // 2^es
	constexpr size_t range = escale * (4 * nbits - 8); // dynamic range of the posit configuration
	constexpr size_t qbits = range + capacity;     // size of the quire minus the sign bit: we are managing the sign explicitly
	return qbits;
}

// Return the dynamic range of the upper quire
template<size_t nbits, size_t es, size_t capacity>
inline int max_scale() {
	constexpr size_t escale = size_t(1) << es;         // 2^es
	constexpr size_t range = escale * (4 * nbits - 8); // dynamic range of the posit configuration
	constexpr size_t half_range = range >> 1;          // position of the fixed point
	//constexpr size_t radix_point = half_range;
	// the upper is 1 bit bigger than the lower because maxpos^2 has that scale
	constexpr size_t upper_range = half_range + 1;     // size of the upper accumulator
	return int(upper_range); 
}

// Return the dynamic range of the lower quire
template<size_t nbits, size_t es, size_t capacity>
inline int min_scale() {
	constexpr size_t escale = size_t(1) << es;         // 2^es
	constexpr size_t range = escale * (4 * nbits - 8); // dynamic range of the posit configuration
	constexpr size_t half_range = range >> 1; 
	return -int(half_range); 
}

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
	// the upper is 1 bit bigger than the lower because maxpos^2 has that scale
	static constexpr size_t upper_range = half_range + 1;     // size of the upper accumulator
	static constexpr size_t qbits = range + capacity;		  // size of the quire minus the sign bit: we are managing the sign explicitly
	
	// Constructors
	quire() : _sign(false) { _capacity.reset(); _upper.reset(); _lower.reset(); }

	quire(int8_t initial_value)   { *this = initial_value; }
	quire(int16_t initial_value)  { *this = initial_value; }
	quire(int32_t initial_value)  { *this = initial_value; }
	quire(int64_t initial_value)  { *this = initial_value; }
	quire(uint64_t initial_value) { *this = initial_value; }
	quire(float initial_value)    { *this = initial_value; }
	quire(double initial_value)   { *this = initial_value; }
	quire(const posit<nbits, es>& rhs) { *this = rhs; }
	template<size_t fbits> quire(const internal::value<fbits>& rhs) { *this = rhs; }

	// Assignment operators: the class only supports native type values
	// assigning a posit requires the convertion to a normalized value, i.e. q = posit<nbits,es>().to_value()

	// operator=() takes a normalized (sign, scale, fraction) triplet
	template<size_t fbits>
	quire& operator=(const internal::value<fbits>& rhs) {
		reset();
		if (rhs.iszero()) return *this;
		if (rhs.isinf() || rhs.isnan()) throw posit_operand_is_nar{};
		_sign = rhs.sign();

		int scale = rhs.scale();
		// TODO: we are clamping the values of the RHS to be within the dynamic range of the posit
		// TODO: however, on the upper side we also have the capacity bits, which gives us the opportunity 
		// TODO: to accept larger scale values than the dynamic range of the posit.
		// TODO: When you are assigning the sum of quires you could hit this condition.
		if (scale >  int(half_range)) 	throw operand_too_large_for_quire{};
		if (scale < -int(half_range)) 	throw operand_too_small_for_quire{};

		int i, f; // running bit pointers, i for the quire, f for the incoming fraction
		sw::universal::bitblock<fbits+1> fraction = rhs.get_fixed_point();
		// divide bits between upper and lower accumulator
		if (scale - int(fbits) >= 0) {
			// all upper accumulator
			for (i = scale, f = int(fbits); i >= 0 && f >= 0; i--, f--) {
				_upper[static_cast<size_t>(i)] = fraction[static_cast<size_t>(f)];
			}
		}
		else if (scale < 0) {
			// all lower accumulator
			for (i = int(half_range) + scale, f = int(fbits); i >= 0 && f >= 0; i--, f--) {
				_lower[static_cast<size_t>(i)] = fraction[static_cast<size_t>(f)];
			}
		}
		else {
			// part upper, and part lower accumulator
			// first assign the bits in the upper accumulator
			for (i = scale, f = int(fbits); i >= 0 && f >= 0; i--, f--) {
				_upper[static_cast<size_t>(i)] = fraction[static_cast<size_t>(f)];
			}
			// next assign the bits in the lower accumulator
			for (i = int(half_range) - 1; i >= 0 && f >= 0; i--, f--) {
				_lower[static_cast<size_t>(i)] = fraction[static_cast<size_t>(f)];
			}
		}
		return *this;
	}
	quire& operator=(const posit<nbits, es>& rhs) {
		*this = rhs.to_value();
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
		magnitude = static_cast<unsigned long long>(_sign ? -rhs : rhs);
		unsigned msb = findMostSignificantBit(magnitude);
		if (msb > half_range + capacity) {
			throw operand_too_large_for_quire{};
		}
		else {
			// copy the value into the quire
			uint32_t i;
			uint64_t mask = uint64_t(1);
			for (i = 0; i < msb && i < half_range; i++) {
				_upper[i] = magnitude & mask;
				mask <<= 1;
			}
			if (msb >= half_range) {
				uint32_t c;
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
			throw operand_too_large_for_quire{};
		}
		else {
			// copy the value into the quire
			uint64_t mask = uint64_t(1);
			for (unsigned i = 0; i < msb && i < half_range; i++) {
				_upper[i] = rhs & mask;
				mask <<= 1;
			}
			if (msb >= half_range) {
				for (unsigned i = half_range, c = 0; i < msb && i < half_range + capacity; i++, c++) {
					_capacity[c] = rhs & mask;
					mask <<= 1;
				}
			}
		}
		return *this;
	}
	quire& operator=(float rhs) {
		constexpr int bits = std::numeric_limits<float>::digits - 1;
		*this = internal::value<bits>(rhs);
		return *this;
	}
	quire& operator=(double rhs) {
		constexpr int bits = std::numeric_limits<double>::digits - 1;
		*this = internal::value<bits>(rhs);
		return *this;
	}
	quire& operator=(long double rhs) {
		constexpr int bits = std::numeric_limits<long double>::digits - 1;
		*this = internal::value<bits>(rhs);
		return *this;
	}

	// All values in (and out) of the quire are normalized (sign, scale, fraction) triplets.

	// Add a normalized value to the quire value. 
	template<size_t fbits>
	quire& operator+=(const internal::value<fbits>& rhs) {
		if (rhs.iszero()) return *this;

		if (rhs.scale() > int(half_range)) {
			throw operand_too_large_for_quire{};
		}
		if (rhs.scale() < -int(half_range)) {
			throw operand_too_small_for_quire{};
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
				internal::value<qbits> subtractend = this->to_value();
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
	quire& operator-=(const internal::value<fbits>& rhs) {
		return *this += -rhs;
	}
	
	// add a posit directly (syntactic sugar)
	quire& operator+=(const posit<nbits, es>& rhs) {
		return operator+=(rhs.to_value());
	}
	// subtract a posit directly (syntactic sugar)
	quire& operator-=(const posit<nbits, es>& rhs) {
		return operator-=(rhs.to_value());
	}

	// add two quires
	quire& operator+=(const quire& q) {
		return operator+=(q.to_value());
	}
	// subtract two quires
	quire& operator-=(const quire& q) {
		return operator-=(q.to_value());
	}
	
	// bit addressing operator
	bool operator[](int index) const {
		if (index < int(radix_point)) return _lower[static_cast<size_t>(index)];
		if (index < int(radix_point + upper_range)) return _upper[static_cast<size_t>(index) - radix_point];
		if (index < int(radix_point + upper_range + capacity)) return _capacity[static_cast<size_t>(index) - radix_point - upper_range];
		throw "index out of range";
	}

// Modifiers

	// state management operators
	// reset the state of a quire to zero
	void reset() {
		_sign = false;
		_lower.reset();
		_upper.reset();
		_capacity.reset();
	}
	// semantic sugar: clear the state of a quire to zero
	void clear() { reset(); }
	void set_sign(bool v) { _sign = v; }
	bool load_bits(const std::string& string_of_bits) {
		reset();
		// format is "+:0000_000000000.000000000"
		std::string::const_iterator it = string_of_bits.begin();
		if (*it == '-') {
			_sign = true;
		}
		else if (string_of_bits[0] == '+') {
			_sign = false;
		}
		else {
			return false; // fail
		}
		++it;
		if (*it == ':') {
			++it;
		}
		else {
			return false; // fail, wrong format
		}
		int segment = 0; // capacity segment = 0, upper segment = 1, lower segment = 2
		int msb_c = capacity - 1;
		int msb_u = upper_range - 1;
		int msb_l = half_range - 1;
		for (; it != string_of_bits.end(); ++it) {
			if (*it == '_') {
				if (msb_c != -1) return false; // fail: incorrect format
				segment = 1;
			}
			else if (*it == '.') {
				if (msb_u != -1) return false; // fail, incorrect format
				segment = 2;
			}
			else if (*it == '1') {
				switch (segment) {
				case 0:
					_capacity.set(msb_c--);
					break;
				case 1:
					_upper.set(msb_u--);
					break;
				case 2:
					if (msb_l < 0) return false; // fail, incorrect format
					_lower.set(msb_l--);
					break;
				default:
					return false; // fail, incorrect state
				}
			}
			else {
				switch (segment) {
				case 0:
					_capacity.reset(msb_c--);
					break;
				case 1:
					_upper.reset(msb_u--);
					break;
				case 2:
					if (msb_l < 0) return false; // fail, incorrect format
					_lower.reset(msb_l--);
					break;
				default:
					return false; // fail, incorrect state
				}
			}
		}
		return true;
	}

// Selectors
	
	// Compare magnitudes between quire and value: returns -1 if q < v, 0 if q == v, and 1 if q > v
	template<size_t fbits>
	int CompareMagnitude(const internal::value<fbits>& v) {
		// inefficient as we are copying a whole quire just to reset the sign bit, but we are leveraging the comparison logic
		quire<nbits, es, capacity> absq = abs(*this);
		//value<qbits> absq = abs(*this);
		internal::value<fbits> absv = abs(v);
		if (absq < absv) {
			return -1;
		}
		else if (absq > absv) {
			return 1;
		}
		return 0;
	}
	// query functions for quire attributes
	inline int dynamic_range() const { return int(range); }
	inline int max_scale() const { return int(upper_range); }
	inline int min_scale() const { return -int(half_range); }
	inline int capacity_range() const { return int(capacity); }
	inline size_t total_bits() const { return qbits + 1; }
	inline bool isneg() const { return _sign; }
	inline bool ispos() const { return _sign; }
	inline bool iszero() const { return _capacity.none() && _upper.none() && _lower.none(); }
	int scale() const {
		int msb = int(capacity)-1; // indicative of no bits set
		for (; msb >= 0; msb--) {
			if (_capacity.test(static_cast<size_t>(msb))) break;
		}
		if (msb >= 0) return msb + int(upper_range);
		for (msb = int(upper_range) - 1; msb >= 0; msb--) {
			if (_upper.test(static_cast<size_t>(msb))) break;
		}
		if (msb >= 0) return msb;
		for (int i = int(half_range) - 1; i >= 0; i--, msb--) {
			if (_lower.test(static_cast<size_t>(i))) break;
		}
		return msb;
	}

	// Return value of the sign bit: true indicates a negative number, false a positive number or zero
	inline bool sign() const { return _sign; }
	inline float sign_value() const {	return (_sign ? -1.0 : 1.0); }
	internal::bitblock<qbits+1> get() const {
		internal::bitblock<qbits+1> q;
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
	internal::value<qbits> to_value() const {
		// find the MSB and build the fraction
		internal::bitblock<qbits> fraction;
		bool isZero = false;
		bool isNaR = false;   // TODO
		int i;
		int qbit = int(qbits);
		int fbit = qbit - 1;
		int scale = qbits;
		for (i = int(capacity) - 1; i >= 0; i--, qbit--) {
			if (scale == qbits) {
				if (_capacity.test(static_cast<size_t>(i))) scale = qbit - int(half_range);
			}
			else {
				fraction[static_cast<size_t>(fbit)] = _capacity[static_cast<size_t>(i)];
				--fbit;
			}
		}
		for (i = int(upper_range) - 1; i >= 0; i--, qbit--) {
			if (scale == qbits) {
				if (_upper.test(static_cast<size_t>(i))) scale = qbit - int(half_range);
			}
			else {
				fraction[static_cast<size_t>(fbit)] = _upper[static_cast<size_t>(i)];
				--fbit;
			}
		}
		for (i = int(half_range) - 1; i >= 0; i--, qbit--) {
			if (scale == qbits) {
				if (_lower.test(static_cast<size_t>(i))) scale = qbit - int(half_range);
			}
			else {
				fraction[static_cast<size_t>(fbit)] = _lower[static_cast<size_t>(i)];
				--fbit;
			}
		}
		if (scale == qbits) {
			isZero = true;
			scale = 0;
		}
		return internal::value<qbits>(_sign, scale, fraction, isZero, isNaR);
	}
	template <typename ToValue>
	ToValue convert_to() const {
            ToValue v;
            convert(to_value(), v);
            return v;
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
	void add_value(const internal::value<fbits>& v) {
		if (v.iszero()) return;
		// scale is the location of the msb in the fixed point representation
		// so scale  =  0 is the hidden bit at location 0, scale 1 = bit 1, etc.
		// and scale = -1 is the first bit of the fraction
		// we manage scale >= 0 in the _upper accumulator, and scale < 0 in the _lower accumulator
		int lsb = v.scale() - static_cast<int>(fbits);
		bool carry = false;
		bitblock<fbits + 1> fraction = v.get_fixed_point();
		int i, f;  // bit pointers, i pointing to the quire bits, f pointing to the fraction bits of rhs
		// divide bits between upper and lower accumulator
		if (v.scale() < 0) {		// all lower accumulator
			lsb = int(half_range) + v.scale() - int(fbits); /// TODO: double check this fix works
			int qlsb = lsb > 0 ? lsb : 0;
			int flsb = lsb >= 0 ? 0 : -lsb;
			for (i = qlsb, f = flsb; i < int(half_range) && f <= static_cast<int>(fbits); i++, f++) {
				bool _a = _lower[static_cast<size_t>(i)];
				bool _b = fraction[static_cast<size_t>(f)];
				_lower[static_cast<size_t>(i)] = _a ^ _b ^ carry;
				carry = (_a && _b) || (carry && (_a ^ _b));
			}
			// propagate any carries to the end of the lower accumulator
			while (carry && i < int(half_range)) {
				bool _a = _lower[static_cast<size_t>(i)];
				_lower[static_cast<size_t>(i)] = _a ^ carry;
				carry = carry && _a;
				i++;
			}
			if (carry) {  // carry propagate to the _upper accumulator
						  // need to increment the _upper
				i = 0;
				while (carry && i < int(upper_range)) {
					bool _a = _upper[static_cast<size_t>(i)];
					_upper[static_cast<size_t>(i)] = _a ^ carry;
					carry = carry && _a;
					i++;
				}
				if (carry) {
					// next add the bits to the capacity segment
					i = 0;
					while (carry && i < int(capacity)) {
						bool _a = _capacity[static_cast<size_t>(i)];
						_capacity[static_cast<size_t>(i)] = _a ^ carry;
						carry = carry && _a;
						i++;
					}
				}
			}
		}
		else if (lsb >= 0) {	// all upper accumulator
			int upper_bound = v.scale();
			for (i = lsb, f = 0; i <= upper_bound && f <= static_cast<int>(fbits); i++, f++) {
				bool _a = _upper[static_cast<size_t>(i)];
				bool _b = fraction[static_cast<size_t>(f)];
				_upper[static_cast<size_t>(i)] = _a ^ _b ^ carry;
				carry = (_a && _b) || (carry && (_a ^ _b));
			}
			while (carry && i < int(upper_range)) {
				bool _a = _upper[static_cast<size_t>(i)];
				_upper[static_cast<size_t>(i)] = _a ^ carry;
				carry = carry && _a;
				i++;
			}
			if (carry) {
				// next add the bits to the capacity segment
				i = 0;
				while (carry && i < int(capacity)) {
					bool _a = _capacity[static_cast<size_t>(i)];
					_capacity[static_cast<size_t>(i)] = _a ^ carry;
					carry = carry && _a;
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
			for (i = qlsb, f = flsb; i < int(half_range) && f <= static_cast<int>(fbits); i++, f++) {
				bool _a = _lower[static_cast<size_t>(i)];
				bool _b = fraction[static_cast<size_t>(f)];
				_lower[static_cast<size_t>(i)] = _a ^ _b ^ carry;
				carry = (_a && _b) || (carry && (_a ^ _b));
			}
			// next add the bits in the upper accumulator
			for (i = 0; i <= v.scale() && f <= static_cast<int>(fbits); i++, f++) {
				bool _a = _upper[static_cast<size_t>(i)];
				bool _b = fraction[static_cast<size_t>(f)];
				_upper[static_cast<size_t>(i)] = _a ^ _b ^ carry;
				carry = (_a && _b) || (carry && (_a ^ _b));
			}
			// propagate any carries to the end of the upper accumulator
			while (carry && i < int(upper_range)) {
				bool _a = _upper[static_cast<size_t>(i)];
				_upper[static_cast<size_t>(i)] = _a ^ carry;
				carry = carry && _a;
				i++;
			}
			// next add the bits to the capacity segment
			if (carry) {
				i = 0;
				while (carry && i < int(capacity)) {
					bool _a = _capacity[static_cast<size_t>(i)];
					_capacity[static_cast<size_t>(i)] = _a ^ carry;
					carry = carry && _a;
					i++;
				}
			}
		}
	}

	// subtract a value from the quire
	template<size_t fbits>
	void subtract_value(const internal::value<fbits>& v) {
		if (v.iszero()) return;
		// lsb in the quire of the lowest bit of the explicit fixed point value including the hidden bit of the fraction
		int lsb = v.scale() - static_cast<int>(fbits);
		bool borrow = false;
		bitblock<fbits + 1> fraction = v.get_fixed_point();
		int i, f;  // bit pointers, i pointing to the quire bits, f pointing to the fraction bits of rhs
		// divide bits between upper and lower accumulator
		if (v.scale() < 0) {		// all lower accumulator
			lsb = int(half_range) + v.scale() - static_cast<int>(fbits);
			int qlsb = lsb > 0 ? lsb : 0;
			int flsb = lsb >= 0 ? 0 : -lsb;
			for (i = qlsb, f = flsb; i < int(half_range) && f <= static_cast<int>(fbits); i++, f++) {
				bool _a = _lower[size_t(i)];
				bool _b = fraction[size_t(f)];
				_lower[size_t(i)] = _a ^ _b ^ borrow;
				borrow = (!_a && _b) || (bxnor(!_a, !_b) && borrow);
			}
			// propagate any borrows to the end of the lower accumulator
			while (borrow && i < int(half_range)) {
				bool _a = _lower[size_t(i)];
				_lower[size_t(i)] = _a ^ borrow;
				borrow = borrow && !_a;
				i++;
			}
			if (borrow) { // borrow propagate to the _upper accumulator
						  // need to decrement the _upper
				i = 0;
				while (borrow && i < int(upper_range)) {
					bool _a = _upper[size_t(i)];
					_upper[size_t(i)] = _a ^ borrow;
					borrow = borrow && !_a;
					i++;
				}
				if (borrow) {
					// propagate the borrow into the capacity segment
					i = 0;
					while (borrow && i < int(capacity)) {
						bool _a = _capacity[size_t(i)];
						_capacity[size_t(i)] = _a ^ borrow;
						borrow = borrow && !_a;
						i++;
					}
				}
			}
		}
		else if (lsb >= 0) {	// all upper accumulator
			int upper_bound = v.scale();
			for (i = lsb, f = 0; i <= upper_bound && f <= static_cast<int>(fbits); i++, f++) {
				bool _a = _upper[size_t(i)];
				bool _b = fraction[size_t(f)];
				_upper[size_t(i)] = _a ^ _b ^ borrow;
				borrow = (!_a && _b) || (bxnor(!_a, !_b) && borrow);
			}
			// propagate any borrows to the end of the upper accumulator
			while (borrow && i < int(upper_range)) {
				bool _a = _upper[size_t(i)];
				_upper[size_t(i)] = _a ^ borrow;
				borrow = borrow && !_a;
				i++;
			}
			if (borrow) {
				// propagate the borrow into the capacity segment
				i = 0;
				while (borrow && i < int(capacity)) {
					bool _a = _capacity[size_t(i)];
					_capacity[size_t(i)] = _a ^ borrow;
					borrow = borrow && !_a;
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
			for (i = qlsb, f = flsb; i < int(half_range) && f <= static_cast<int>(fbits); i++, f++) {
				bool _a = _lower[size_t(i)];
				bool _b = fraction[size_t(f)];
				_lower[size_t(i)] = _a ^ _b ^ borrow;
				borrow = (!_a && _b) || (bxnor(!_a, !_b) && borrow);
			}
			// next add the bits in the upper accumulator
			for (i = 0; i <= v.scale() && f <= static_cast<int>(fbits); i++, f++) {
				bool _a = _upper[size_t(i)];
				bool _b = fraction[size_t(f)];
				_upper[size_t(i)] = _a ^ _b ^ borrow;
				borrow = (!_a && _b) || (bxnor(!_a, !_b) && borrow);
			}
			// propagate any borrows to the end of the upper accumulator
			while (borrow && i < int(upper_range)) {
				bool _a = _upper[size_t(i)];
				_upper[size_t(i)] = _a ^ borrow;
				borrow = borrow && !_a;
				i++;
			}
			if (borrow) {
				// propagate the borrow into the capacity segment
				i = 0;
				while (borrow && i < int(capacity)) {
					bool _a = _capacity[size_t(i)];
					_capacity[size_t(i)] = _a ^ borrow;
					borrow = borrow && !_a;
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
	friend bool operator==(const quire<nnbits, nes, ncapacity>& q, const internal::value<nfbits>& v);
	template<size_t nnbits, size_t nes, size_t ncapacity, size_t nfbits >
	friend bool operator< (const quire<nnbits, nes, ncapacity>& q, const internal::value<nfbits>& v);
	template<size_t nnbits, size_t nes, size_t ncapacity, size_t nfbits >
	friend bool operator> (const quire<nnbits, nes, ncapacity>& q, const internal::value<nfbits>& v);

};

// Magnitude of a quire
#if 1
template<size_t nbits, size_t es, size_t capacity>
quire<nbits, es, capacity> abs(const quire<nbits, es, capacity>& q) {
	quire<nbits, es, capacity> magnitude(q);
	magnitude.set_sign(false);
	return magnitude;
}
#else
template<size_t nbits, size_t es, size_t capacity>
value<(size_t(1) << es)*(4 * nbits - 8) + capacity> abs(const quire<nbits, es, capacity>& q) {
	quire<nbits, es, capacity> magnitude(q);
	magnitude.set_sign(false);
	return magnitude;
}
#endif

// QUIRE BINARY ARITHMETIC OPERATORS
template<size_t nbits, size_t es, size_t capacity>
inline quire<nbits, es, capacity> operator+(const quire<nbits, es, capacity>& lhs, const quire<nbits, es, capacity>& rhs) {
	quire<nbits, es, capacity> sum = lhs;
	sum += rhs;
	return sum;
}


////////////////// QUIRE stream operators
template<size_t nbits, size_t es, size_t capacity>
inline std::ostream& operator<<(std::ostream& ostr, const quire<nbits, es, capacity>& q) {
	ostr << (q._sign ? "-:" : "+:") << q._capacity << "_" << q._upper << "." << q._lower;
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
inline bool operator== (const quire<nbits, es, capacity>& q, const internal::value<fbits>& v) {
	// not efficient, but leverages < and >
	return !(q < v) && !(q > v);
}
template<size_t nbits, size_t es, size_t capacity, size_t fbits>
inline bool operator< (const quire<nbits, es, capacity>& q, const internal::value<fbits>& v) {
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
			bitblock<fbits + 1> fixed = v.get_fixed_point();
			int i, f;  // bit pointers, i for the quire, f for the fraction in v
			bool undecided = true;
			for (i = int(quire<nbits, es, capacity>::radix_point) + qscale, f = int(fbits); i >= 0 && f >= 0; --i, --f) {
				if (!q[i] && fixed[static_cast<size_t>(f)]) {
					bSmaller = true;
					undecided = false;
					break;
				}
				else if (q[i] && !fixed[static_cast<size_t>(f)]) {
					bSmaller = false;
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
inline bool operator> (const quire<nbits, es, capacity>& q, const internal::value<fbits>& v) {
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
			bitblock<fbits + 1> fixed_point = v.get_fixed_point();
			int i, f;  // bit pointers, i for the quire, f for the fraction in v
			bool undecided = true;
			for (i = int(quire<nbits, es, capacity>::radix_point) + qscale, f = int(fbits); i >= 0 && f >= 0; --i, --f) {
				if (q[i] && !fixed_point[static_cast<size_t>(f)]) {
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



// QUIRE OPERATORS

// unrounded posit addition to be added to the quire
template<size_t nbits, size_t es>
internal::value<nbits - es + 2> quire_add(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) {
	static constexpr size_t fbits     = nbits - 3 - es;
	static constexpr size_t fhbits    = fbits + 1;            // size of fraction + hidden bit
	static constexpr size_t guardbits = 3;
	static constexpr size_t abits     = fhbits + guardbits;   // size of the addend
	internal::value<abits + 1> sum;
	internal::value<fbits> a, b;

	// special case handling
	if (lhs.isnar() || rhs.isnar()) { sum.setinf(); return sum; }
	if (lhs.iszero() && rhs.iszero()) return sum;
	if (lhs.iszero()) { rhs.template normalize_to<abits+1>(sum); return sum; }
	if (rhs.iszero()) { lhs.template normalize_to<abits+1>(sum); return sum; }

	// transform the inputs into (sign,scale,fraction) triples
	a.set(sign(lhs), scale(lhs), extract_fraction<nbits, es, fbits>(lhs), lhs.iszero(), lhs.isnar());
	b.set(sign(rhs), scale(rhs), extract_fraction<nbits, es, fbits>(rhs), rhs.iszero(), rhs.isnar());

	module_add<fbits, abits>(a, b, sum);		// add the two inputs

	return sum;
}

// unrounded posit multiplication to be added to the quire
template<size_t nbits, size_t es>
internal::value<2 * (nbits - 2 - es)> quire_mul(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) {
	static constexpr size_t fbits = nbits - 3 - es;
	static constexpr size_t fhbits = fbits + 1;      // size of fraction + hidden bit
	static constexpr size_t mbits = 2 * fhbits;      // size of the multiplier output

	internal::value<mbits> product;  // constructs to zero value
	internal::value<fbits> a, b;

	// special case handling
	if (lhs.isnar() || rhs.isnar()) { product.setinf(); return product; }
	if (lhs.iszero() || rhs.iszero()) return product;

	// transform the inputs into (sign,scale,fraction) triples
	a.set(sign(lhs), scale(lhs), extract_fraction<nbits, es, fbits>(lhs), lhs.iszero(), lhs.isnar());
	b.set(sign(rhs), scale(rhs), extract_fraction<nbits, es, fbits>(rhs), rhs.iszero(), rhs.isnar());

	module_multiply(a, b, product);    // multiply the two inputs

	return product;
}

}} // namespace sw::universal
