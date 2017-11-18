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
	static constexpr size_t qbits = range + capacity;         // size of the quire minus the sign bit: we are managing the sign explicitly
	quire() : _sign(false), _accu(0) {}
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
	quire(const value& rhs) {
		*this = rhs;
	}
	quire& operator=(const value& rhs) {
		_sign = rhs._sign;
		_accu = rhs._accu;
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
		return *this;
	}
	quire& operator=(uint64_t rhs) {
		reset();
		return *this;
	}
	quire& operator=(float rhs) {
		reset();
		return *this;
	}
	quire& operator=(double rhs) {
		reset();
		return *this;
	}
	void reset() {
		_sign  = false;
		_accu.reset();
	}
	bool isNegative() {	return _sign; }

	double sign_value() const {	return (_sign ? -1.0 : 1.0); }
	double to_double() const {
		return 0.0; // TODO
	}

private:
	bool				_sign;
	std::bitset<qbits>	_accu;

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

