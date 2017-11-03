#pragma once
// fraction.hpp: definition of a posit fractions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <algorithm>

#include "exceptions.hpp"

// fraction is spec'ed with the size of the posit it belongs to.
// However, the size of the fraction segment is nbits-3, but we maintain an extra guard bit, so the size of the actual fraction we manage is nbits-2
template<size_t fbits>
class fraction {
public:
	fraction() : _Bits(), _NrOfBits(0) {}

	fraction(const fraction& f) = default;
	fraction(fraction&& f) = default;
	
	fraction& operator=(const fraction& f) = default;
	fraction& operator=(fraction&& f) = default;
	
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
	
	/// Normalized shift (e.g., for addition).
	template <size_t Size>
	std::bitset<Size> nshift(long shift) const 
	{
            std::bitset<Size> number;
            
            // Check range
            if (long(fbits) + shift >= long(Size))
                throw shift_too_large{};
                
            const long hpos = fbits + shift;              // position of hidden bit
            
            // If hidden bit is LSB or beyond just set uncertainty bit and call it a day
            if (hpos <= 0) {
                number[0] = true;
                return number;
            }
                
            number[hpos] = true;                   // hidden bit now safely set
            
            // Copy fraction bits into certain part
            for (long npos = hpos - 1, fpos = long(fbits) - 1; npos > 0 && fpos >= 0; --npos, --fpos)
                number[npos] = _Bits[fpos];
                
            // Set uncertainty bit
            bool uncertainty = false;
            for (long fpos = std::min(long(fbits)-1, -shift); fpos >= 0 && !uncertainty; --fpos)
                uncertainty |= _Bits[fpos];
            number[0] = uncertainty;
            return number;
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
