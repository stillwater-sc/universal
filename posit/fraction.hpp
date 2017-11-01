#pragma once
// fraction.hpp: definition of a posit fractions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// fraction is spec'ed with the size of the posit it belongs to.
// However, the size of the fraction segment is nbits-3, but we maintain an extra guard bit, so the size of the actual fraction we manage is nbits-2
template<size_t fbits>
class fraction {
public:
	fraction() {
		_Bits.reset();
	}
	fraction(const fraction& f) {
		_Bits = f._Bits;
		_NrOfBits = f._NrOfBits;
	}
	fraction& operator=(const fraction& f) {
		_Bits = f._Bits;
		_NrOfBits = f._NrOfBits;
		return *this;
	}
	void reset() {
		_NrOfBits = 0;
		_Bits.reset();
	}
	unsigned int nrBits() const {
		return _NrOfBits;
	}
	double value() const {
		// TODO: this fails when fbits > 64 and we cannot represent the fraction by a 64bit unsigned integer
		return double(_Bits.to_ullong()) / double(uint64_t(1) << (fbits));
	}
	std::bitset<fbits> get() const {
		return _Bits;
	}
	void set(const std::bitset<fbits>& raw, std::size_t nrOfFractionBits) {
		_Bits = raw;
		_NrOfBits = (fbits < nrOfFractionBits ? fbits : nrOfFractionBits);
	}
	// copy the remaining bits into the fraction
	bool assign_fraction(unsigned int remaining_bits, std::bitset<fbits>& _fraction) {
		bool round_up = false;
		if (fbits == 0) return false;
		if (remaining_bits > 0 && fbits > 0) {
			_NrOfBits = 0;
			for (unsigned int i = 0; i < remaining_bits; i++) {
				_Bits[fbits - 1 - i] = _fraction[fbits - 1 - i];
				_NrOfBits++;
			}
			if (fbits > remaining_bits) {
				round_up = _fraction[fbits - 1 - remaining_bits];
			}		
		}
		else {
			round_up = _fraction[fbits - 1];
			_NrOfBits = 0;
		}
		return round_up;
	}
	// normalize the fraction and return its fraction in the argument: add a sticky bit and two guard bits to the result
	void normalize(std::bitset<fbits+3>& number) const {
		number.set(fbits, true); // set hidden bit
		for (int i = 0; i < fbits; i++) {
			number.set(i, _Bits[i]);
		}
	}
	/*   h is hidden bit
	*   h.bbbb_bbbb_bbbb_b...      fraction
	*   0.000h_bbbb_bbbb_bbbb_b... number
	*  >-.----<                    shift of 4
	*/
	void denormalize(int shift, std::bitset<fbits+3>& number) const {
		number.reset();
		if (fbits == 0) return;
		if (shift < 0) shift = -shift;
		if (shift <= static_cast<int>(fbits)) {
			number.set(static_cast<int>(fbits) - shift); // set hidden bit
			for (int i = static_cast<int>(fbits) - 1 - shift; i >= 0; i--) {
				number.set(i, _Bits[i + shift]);
			}
		}
	}
	bool increment() {
		return increment_unsigned(_Bits, _NrOfBits);
	}
private:
	// maximum size fraction is <nbits - one sign bit - minimum two regime bits>
	// but we maintain 1 guard bit for rounding decisions
	std::bitset<fbits> _Bits;
	unsigned int       _NrOfBits;

	// template parameters need names different from class template parameters (for gcc and clang)
	// Without the template (i.e. only own operators are friends) we get linker errors
	template<size_t nfbits>
	friend std::ostream& operator<< (std::ostream& ostr, const fraction<nfbits>& f);
	template<size_t nfbits>
	friend std::istream& operator>> (std::istream& istr, fraction<nfbits>& f);

	template<size_t nfbits>
	friend bool operator==(const fraction<nfbits>& lhs, const fraction<nfbits>& rhs);
	template<size_t nfbits>
	friend bool operator!=(const fraction<nfbits>& lhs, const fraction<nfbits>& rhs);
	template<size_t nfbits>
	friend bool operator< (const fraction<nfbits>& lhs, const fraction<nfbits>& rhs);
	template<size_t nfbits>
	friend bool operator> (const fraction<nfbits>& lhs, const fraction<nfbits>& rhs);
	template<size_t nfbits>
	friend bool operator<=(const fraction<nfbits>& lhs, const fraction<nfbits>& rhs);
	template<size_t nfbits>
	friend bool operator>=(const fraction<nfbits>& lhs, const fraction<nfbits>& rhs);
};
