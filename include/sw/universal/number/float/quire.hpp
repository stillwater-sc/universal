#pragma once
// quire.hpp: definition of a parameterized quire configurations for IEEE float/double/long double
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/boolean_logic_operators.hpp>
#include <universal/internal/value/value.hpp>
#include <universal/number/quire/exceptions.hpp>

namespace sw::ieee {

	// inject sw::universal namespaces into the public interface

using namespace sw::universal;
using namespace sw::universal::internal;

// template class representing a quire associated with an ieee float configuration
// capacity indicates the power of 2 number of accumulations the quire can support
template<unsigned nbits, unsigned es, unsigned capacity = 30>
class quire {
public:
	// fixed-point representation of a float multiply requires 1 + 2*(2^ebits + mbits), where ebits and mbits are the number of bits in exponent and mantissa, respectively
//	type	size	ebits	mbits	exponent range	capacity	quire size	total
//	float	 32		 8		 24		256				30			561			  591
//	double	 64		11		 53		2048			30			4203		 4233
//	lng dbl	128		15		113		32768			30			65763		65793

	static constexpr unsigned ebits = es;
	static constexpr unsigned mbits = nbits - es;
	static constexpr unsigned escale = 2*((unsigned(1) << es) + mbits + 1);
	static constexpr unsigned range = escale; 		  // dynamic range of the float configuration
	static constexpr unsigned half_range = range >> 1;          // position of the fixed point
	static constexpr unsigned upper_range = half_range + 1;     // size of the upper accumulator
	static constexpr unsigned qbits = range + capacity;         // size of the quire minus the sign bit: we are managing the sign explicitly
	
	quire() : _sign(false) { _capacity.reset(); _upper.reset(); _lower.reset(); }
	quire(int8_t initial_value) {		*this = initial_value;	}
	quire(int16_t initial_value) {		*this = initial_value;	}
	quire(int32_t initial_value) {		*this = initial_value;	}
	quire(int64_t initial_value) {		*this = initial_value;	}
	quire(uint64_t initial_value) {		*this = initial_value;	}
	quire(float initial_value) {		*this = initial_value;	}
	quire(double initial_value) {		*this = initial_value;	}
	template<unsigned fbits>
	quire(const sw::universal::internal::value<fbits>& rhs) { *this = rhs; }  // TODO: an internal type in a public interface
	
	// TODO: we are clamping the values of the RHS to be withing the dynamic range of the float
	// TODO: however, on the upper side we also have the capacity bits, which gives us the opportunity to accept larger scale values than the dynamic range of the float.
	// TODO: is that a good idea?
	template<unsigned fbits>
	quire& operator=(const value<fbits>& rhs) {
		reset();
		_sign = rhs.sign();
		int i,f, scale = rhs.scale();
		if (scale > int(half_range)) {
			throw operand_too_large_for_quire{};
		}
		if (scale < -int(half_range)) {
			throw operand_too_small_for_quire{};
		}
		sw::universal::internal::bitblock<fbits+1> fraction = rhs.get_fixed_point();
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
	quire& operator=(signed char rhs) {
		*this = (long long)(rhs);
		return *this;
	}
	quire& operator=(short int rhs) {
		*this = (long long)(rhs);
		return *this;
	}
	quire& operator=(int rhs) {
		*this = (long long)(rhs);
		return *this;
	}
	quire& operator=(long long rhs) {
		clear();
		// transform to sign-magnitude
		_sign = rhs & 0x8000000000000000;
		unsigned long long magnitude;
		magnitude = _sign ? -rhs : rhs;
		unsigned msb = find_msb(magnitude);
		if (msb > half_range + capacity) {
			throw operand_too_large_for_quire{};
		}
		else {
			// copy the value into the quire
			uint64_t mask = uint64_t(1);
			for (unsigned i = 0; i < msb && i < half_range; i++) {
				_upper[i] = magnitude & mask;
				mask <<= 1;
			}
			if (msb >= half_range) {
				unsigned c = 0;
				for (unsigned i = half_range; i < msb && i < half_range + capacity; i++, c++) {
					_capacity[c] = magnitude & mask;
					mask <<= 1;
				}
			}
		}
		return *this;
	}
	quire& operator=(long long unsigned rhs) {
		reset();
		unsigned msb = find_msb(rhs);
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
				unsigned c = 0;
				for (unsigned i = half_range; i < msb && i < half_range + capacity; i++, c++) {
					_capacity[c] = rhs & mask;
					mask <<= 1;
				}
			}
		}
		return *this;
	}
	quire& operator=(float rhs) {
		constexpr int bits = std::numeric_limits<float>::digits - 1;
		*this = sw::universal::internal::value<bits>(rhs);
		return *this;
	}
	quire& operator=(double rhs) {
		constexpr int bits = std::numeric_limits<double>::digits - 1;
		*this = sw::universal::internal::value<bits>(rhs);
		return *this;
	}
	quire& operator=(long double rhs) {
		constexpr int bits = std::numeric_limits<long double>::digits - 1;	
		*this = sw::universal::internal::value<bits>(rhs);
		return *this;
	}
	
	template<unsigned fbits>
	quire& operator+=(const value<fbits>& rhs) {
		if (rhs.iszero()) return *this;
		int i, f, scale = rhs.scale();
		if (scale >  int(half_range)) {
			throw operand_too_large_for_quire{};
		}
		if (scale < -int(half_range)) {
			throw operand_too_small_for_quire{};
		}
		if (rhs.sign()) {			// subtract
			// lsb in the quire of the lowest bit of the explicit fixed point value including the hidden bit of the fraction
			int lsb = scale - int(fbits);  
			bool borrow = false;
			bitblock<fbits + 1> fraction = rhs.get_fixed_point();
			// divide bits between upper and lower accumulator
			if (scale < 0) {		// all lower accumulator
				int lsb = int(half_range) + scale - int(fbits);
				int qlsb = lsb > 0 ? lsb : 0;
				int flsb = lsb >= 0 ? 0 : -lsb;
				for (i = qlsb, f = flsb; i < int(half_range) && f <= int(fbits); i++, f++) {
					bool _a = _lower[i];
					bool _b = fraction[f];
					_lower[i] = bxor(_a, _b, borrow);
					borrow = (!_a && _b) || (bxnor(!_a, !_b) && borrow);
				}
				// propagate any borrows to the end of the lower accumulator
				while (borrow && i < int(half_range)) {
					bool _a = _lower[i];
					_lower[i] = bxor(_a, borrow);
					borrow = borrow && !_a;
					i++;
				}
				if (borrow) { // borrow propagate to the _upper accumulator
							  // need to decrement the _upper
					i = 0;
					while (borrow && i < int(upper_range)) {
						bool _a = _upper[i];
						_upper[i] = bxor(_a, borrow);
						borrow = borrow && !_a;
						i++;
					}
					if (borrow) {
						// propagate the borrow into the capacity segment
						i = 0;
						while (borrow && i < int(capacity)) {
							bool _a = _capacity[i];
							_capacity[i] = bxor(_a, borrow);
							borrow = borrow && !_a;
							i++;
						}
					}
				}
			}
			else if (lsb >= 0) {	// all upper accumulator
				for (i = lsb, f = 0; i <= scale && f <= int(fbits); i++, f++) {
					bool _a = _upper[i];
					bool _b = fraction[f];
					_upper[i] = sw::universal::bxor(_a, _b, borrow);
					borrow = (!_a && _b) || (sw::universal::bxnor(!_a, !_b) && borrow);
				}
				// propagate any borrows to the end of the upper accumulator
				while (borrow && i < int(upper_range)) {
					bool _a = _upper[i];
					_upper[i] = sw::universal::bxor(_a, borrow);
					borrow = borrow && !_a;
					i++;
				}
				if (borrow) {
					// propagate the borrow into the capacity segment
					i = 0;
					while (borrow && i < int(capacity)) {
						bool _a = _capacity[i];
						_capacity[i] = sw::universal::bxor(_a, borrow);
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
				for (i = qlsb, f = flsb; i < int(half_range) && f <= int(fbits); i++, f++) {
					bool _a = _lower[i];
					bool _b = fraction[f];
					_lower[i] = sw::universal::bxor(_a, _b, borrow);
					borrow = (!_a && _b) || (sw::universal::bxnor(!_a, !_b) && borrow);
				}
				// next add the bits in the upper accumulator
				for (i = 0; i <= scale && f <= int(fbits); i++, f++) {
					bool _a = _upper[i];
					bool _b = fraction[f];
					_upper[i] = sw::universal::bxor(_a, _b, borrow);
					borrow = (!_a && _b) || (sw::universal::bxnor(!_a, !_b) && borrow);
				}
				// propagate any borrows to the end of the upper accumulator
				while (borrow && i < int(upper_range)) {
					bool _a = _upper[i];
					_upper[i] = _a ^ borrow;
					borrow = borrow && !_a;
					i++;
				}
				if (borrow) {
					// propagate the borrow into the capacity segment
					i = 0;
					while (borrow && i < int(capacity)) {
						bool _a = _capacity[i];
						_capacity[i] = _a ^ borrow;
						borrow = borrow && !_a;
						i++;
					}
				}			
			}
		}
		else {			// add
			// scale is the location of the msb in the fixed point representation
			// so scale  =  0 is the hidden bit at location 0, scale 1 = bit 1, etc.
			// and scale = -1 is the first bit of the fraction
			// we manage scale >= 0 in the _upper accumulator, and scale < 0 in the _lower accumulator
			int lsb = scale - int(fbits);
			bool carry = false;
			bitblock<fbits + 1> fraction = rhs.get_fixed_point();
			// divide bits between upper and lower accumulator
			if (scale < 0) {		// all lower accumulator
				int lsb = int(half_range) + scale - int(fbits);
				int qlsb = lsb > 0 ? lsb : 0;
				int flsb = lsb >= 0 ? 0 : -lsb;
				for (i = qlsb, f = flsb; i < int(half_range) && f <= int(fbits); i++, f++) {
					bool _a = _lower[i];
					bool _b = fraction[f];
					_lower[i] = sw::universal::bxor(_a, _b, carry);
					carry = (_a && _b) || (carry && sw::universal::bxor(_a, _b));
				}
				// propagate any carries to the end of the lower accumulator
				while (carry && i < int(half_range)) {
					bool _a = _lower[i];
					_lower[i] = _a ^ carry;
					carry = carry && _a;
					i++;
				}
				if (carry) {  // carry propagate to the _upper accumulator
							  // need to increment the _upper
					i = 0;
					while (carry && i < int(upper_range)) {
						bool _a = _upper[i];
						_upper[i] = _a ^ carry;
						carry = carry && _a;
						i++;
					}
					if (carry) {
						// next add the bits to the capacity segment
						i = 0;
						while (carry && i < int(capacity)) {
							bool _a = _capacity[i];
							_capacity[i] = _a ^ carry;
							carry = carry && _a;
							i++;
						}
					}
				}
			}
			else if (lsb >= 0) {	// all upper accumulator
				for (i = lsb, f = 0; i <= scale && f <= int(fbits); i++, f++) {
					bool _a = _upper[i];
					bool _b = fraction[f];
					_upper[i] = sw::universal::bxor(_a, _b, carry);
					carry = (_a && _b) || (carry && sw::universal::bxor(_a, _b));
				}
				while (carry && i < int(upper_range)) {
					bool _a = _upper[i];
					_upper[i] = _a ^ carry;
					carry = carry && _a;
					i++;
				}
				if (carry) {
					// next add the bits to the capacity segment
					i = 0;
					while (carry && i < int(capacity)) {
						bool _a = _capacity[i];
						_capacity[i] = _a ^ carry;
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
				for (i = qlsb, f = flsb; i < int(half_range) && f <= int(fbits); i++, f++) {
					bool _a = _lower[i];
					bool _b = fraction[f];
					_lower[i] = sw::universal::bxor(_a, _b, carry);
					carry = (_a && _b) || (carry && sw::universal::bxor(_a, _b));
				}
				// next add the bits in the upper accumulator
				for (i = 0; i <= scale && f <= int(fbits); i++, f++) {
					bool _a = _upper[i];
					bool _b = fraction[f];
					_upper[i] = sw::universal::bxor(_a, _b, carry);
					carry = (_a && _b) || (carry && sw::universal::bxor(_a, _b));
				}
				// propagate any carries to the end of the upper accumulator
				while (carry && i < int(upper_range)) {
					bool _a = _upper[i];
					_upper[i] = _a ^ carry;
					carry = carry && _a;
					i++;
				}
				// next add the bits to the capacity segment
				if (carry) {
					i = 0;
					while (carry && i < int(capacity)) {
						bool _a = _capacity[i];
						_capacity[i] = _a ^ carry;
						carry = carry && _a;
						i++;
					}
				}
			}
		}
		return *this;
	}
	// reset the state of a quire to zero
	void reset() {
		_sign  = false;
		_lower.reset();
		_upper.reset();
		_capacity.reset();
	}
	// clear the state of a quire to zero
	void clear() { reset(); }
	int dynamic_range() const { return range; }
	int radix_point() const { return half_range; }
	int max_scale() const { return half_range; }
	int min_scale() const { return -int(half_range); }
	int capacity_range() const { return capacity; }
	bool isneg() const { return _sign; }
	bool ispos() const { return !_sign; }
	bool iszero() const { return _capacity.none() && _upper.none() && _lower.none(); }

	// Return value of the sign bit: true indicates a negative number, false a positive number or zero
	bool get_sign() const { return _sign; }
	float sign_value() const {	return (_sign ? -1.0 : 1.0); }
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

private:
	bool				   _sign;
	// segmented accumulator to demonstrate potential hw concurrency for high performance quires
	bitblock<half_range>   _lower;
	bitblock<upper_range>  _upper;
	bitblock<capacity>     _capacity;

#if TEMPLATIZED_TYPE
	//  when we figure out how to templatize the extraction of exponent bits from type

	// template parameters need names different from class template parameters (for gcc and clang)
	template<typename TTy, unsigned ncapacity>
	friend std::ostream& operator<< (std::ostream& ostr, const quire<TTy,ncapacity>& q);
	template<typename TTy, unsigned ncapacity>
	friend std::istream& operator>> (std::istream& istr, quire<TTy, ncapacity>& q);

	template<unsigned nnbits, unsigned nes, unsigned ncapacity>
	friend bool operator==(const quire<TTy, ncapacity>& lhs, const quire<TTy, ncapacity>& rhs);
	template<typename TTy, unsigned ncapacity>
	friend bool operator!=(const quire<TTy, ncapacity>& lhs, const quire<TTy, ncapacity>& rhs);
	template<typename TTy, unsigned ncapacity>
	friend bool operator< (const quire<TTy, ncapacity>& lhs, const quire<TTy, ncapacity>& rhs);
	template<typename TTy, unsigned ncapacity>
	friend bool operator> (const quire<TTy, ncapacity>& lhs, const quire<TTy, ncapacity>& rhs);
	template<typename TTy, unsigned ncapacity>
	friend bool operator<=(const quire<TTy, ncapacity>& lhs, const quire<TTy, ncapacity>& rhs);
	template<typename TTy, unsigned ncapacity>
	friend bool operator>=(const quire<TTy, ncapacity>& lhs, const quire<TTy, ncapacity>& rhs);

#else

	// template parameters need names different from class template parameters (for gcc and clang)
	template<unsigned nnbits, unsigned nes, unsigned ncapacity>
	friend std::ostream& operator<< (std::ostream& ostr, const quire<nnbits, nes, ncapacity>& q);
	template<unsigned nnbits, unsigned nes, unsigned ncapacity>
	friend std::istream& operator>> (std::istream& istr, quire<nnbits, nes, ncapacity>& q);

	template<unsigned nnbits, unsigned nes, unsigned ncapacity>
	friend bool operator==(const quire<nnbits, nes, ncapacity>& lhs, const quire<nnbits, nes, ncapacity>& rhs);
	template<unsigned nnbits, unsigned nes, unsigned ncapacity>
	friend bool operator!=(const quire<nnbits, nes, ncapacity>& lhs, const quire<nnbits, nes, ncapacity>& rhs);
	template<unsigned nnbits, unsigned nes, unsigned ncapacity>
	friend bool operator< (const quire<nnbits, nes, ncapacity>& lhs, const quire<nnbits, nes, ncapacity>& rhs);
	template<unsigned nnbits, unsigned nes, unsigned ncapacity>
	friend bool operator> (const quire<nnbits, nes, ncapacity>& lhs, const quire<nnbits, nes, ncapacity>& rhs);
	template<unsigned nnbits, unsigned nes, unsigned ncapacity>
	friend bool operator<=(const quire<nnbits, nes, ncapacity>& lhs, const quire<nnbits, nes, ncapacity>& rhs);
	template<unsigned nnbits, unsigned nes, unsigned ncapacity>
	friend bool operator>=(const quire<nnbits, nes, ncapacity>& lhs, const quire<nnbits, nes, ncapacity>& rhs);

#endif // TEMPLATIZED_TYPE
};

#if TEMPLATIZED_TYPE

// QUIRE BINARY ARITHMETIC OPERATORS
template<typename Ty, unsigned capacity>
inline quire<Ty, capacity> operator+(const quire<Ty, capacity>& lhs, const quire<Ty, capacity>& rhs) {
	quire<Ty, capacity> sum = lhs;
	sum += rhs;
	return sum;
}


////////////////// QUIRE operators
template<typename Ty, unsigned capacity>
inline std::ostream& operator<<(std::ostream& ostr, const quire<Ty, capacity>& q) {
	ostr << (q._sign ? "-1" : " 1") << ": " << q._capacity << "_" << q._upper << "." << q._lower;
	return ostr;
}

template<typename Ty, unsigned capacity>
inline std::istream& operator>> (std::istream& istr, const quire<Ty, capacity>& q) {
	istr >> q._accu;
	return istr;
}

template<typename Ty, unsigned capacity>
inline bool operator==(const quire<Ty, capacity>& lhs, const quire<Ty, capacity>& rhs) { return lhs._sign == rhs._sign && lhs._capacity == rhs._capacity && lhs._upper == rhs._upper && lhs._lower == rhs._lower; }
template<typename Ty, unsigned capacity>
inline bool operator!=(const quire<Ty, capacity>& lhs, const quire<Ty, capacity>& rhs) { return !operator==(lhs, rhs); }
template<typename Ty, unsigned capacity>
inline bool operator< (const quire<Ty, capacity>& lhs, const quire<Ty, capacity>& rhs) { 
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
template<typename Ty, unsigned capacity>
inline bool operator> (const quire<Ty, capacity>& lhs, const quire<Ty, capacity>& rhs) { return  operator< (rhs, lhs); }
template<typename Ty, unsigned capacity>
inline bool operator<=(const quire<Ty, capacity>& lhs, const quire<Ty, capacity>& rhs) { return !operator> (lhs, rhs) || lhs == rhs; }
template<typename Ty, unsigned capacity>
inline bool operator>=(const quire<Ty, capacity>& lhs, const quire<Ty, capacity>& rhs) { return !operator< (lhs, rhs) || lhs == rhs; }

#else

// QUIRE BINARY ARITHMETIC OPERATORS
template<unsigned nbits, unsigned es, unsigned capacity>
inline quire<nbits, es, capacity> operator+(const quire<nbits, es, capacity>& lhs, const quire<nbits, es, capacity>& rhs) {
	quire<nbits, es, capacity> sum = lhs;
	sum += rhs;
	return sum;
}


////////////////// QUIRE operators
template<unsigned nnbits, unsigned nes, unsigned capacity>
inline std::ostream& operator<<(std::ostream& ostr, const quire<nnbits, nes, capacity>& q) {
	ostr << (q._sign ? "-1" : " 1") << ": " << q._capacity << "_" << q._upper << "." << q._lower;
	return ostr;
}

template<unsigned nnbits, unsigned nes, unsigned capacity>
inline std::istream& operator>> (std::istream& istr, const quire<nnbits, nes, capacity>& q) {
	istr >> q._accu;
	return istr;
}

template<unsigned nbits, unsigned es, unsigned capacity>
inline bool operator==(const quire<nbits, es, capacity>& lhs, const quire<nbits, es, capacity>& rhs) { return lhs._sign == rhs._sign && lhs._capacity == rhs._capacity && lhs._upper == rhs._upper && lhs._lower == rhs._lower; }
template<unsigned nbits, unsigned es, unsigned capacity>
inline bool operator!=(const quire<nbits, es, capacity>& lhs, const quire<nbits, es, capacity>& rhs) { return !operator==(lhs, rhs); }
template<unsigned nbits, unsigned es, unsigned capacity>
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
template<unsigned nbits, unsigned es, unsigned capacity>
inline bool operator> (const quire<nbits, es, capacity>& lhs, const quire<nbits, es, capacity>& rhs) { return  operator< (rhs, lhs); }
template<unsigned nbits, unsigned es, unsigned capacity>
inline bool operator<=(const quire<nbits, es, capacity>& lhs, const quire<nbits, es, capacity>& rhs) { return !operator> (lhs, rhs) || lhs == rhs; }
template<unsigned nbits, unsigned es, unsigned capacity>
inline bool operator>=(const quire<nbits, es, capacity>& lhs, const quire<nbits, es, capacity>& rhs) { return !operator< (lhs, rhs) || lhs == rhs; }
#endif

}  // namespace sw::ieee
