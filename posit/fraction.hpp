#pragma once
// fraction.hpp: definition of a posit fractions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <algorithm>

#include "exceptions.hpp"

namespace sw {
namespace unum {

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
	
	// selectors
	bool none() const {	return _Bits.none(); }
	size_t nrBits() const { return _NrOfBits;	}
	// fractions are assumed to have a hidden bit, the case where they do not must be managed by the container of the fraction
	// calculate the value of the fraction ignoring the hidden bit. So a fraction of 1010 has the value 0.5+0.125=5/8
	double value() const { 
		double v = 0.0;
		if (_Bits.none()) return v;
		double scale = 0.5;
		for (int i = int(fbits) - 1; i >= 0; i--) {
			if (_Bits.test(i)) v += scale;
			scale *= (double)0.5;
			if (scale == 0.0) break;
		}
		return v;
	}

	// modifiers
	void reset() {
		_NrOfBits = 0;
		_Bits.reset();
	}

	bitblock<fbits> get() const {
		return _Bits;
	}
	void set(const bitblock<fbits>& raw, std::size_t nrOfFractionBits = fbits) {
		_Bits = raw;
		_NrOfBits = (fbits < nrOfFractionBits ? fbits : nrOfFractionBits);
	}
	// get a fixed point number by making the hidden bit explicit: useful for multiply units
	bitblock<fbits + 1> get_fixed_point() const {
		bitblock<fbits + 1> fixed_point_number;
		fixed_point_number.set(fbits, true); // make hidden bit explicit
		for (unsigned int i = 0; i < fbits; i++) {
			fixed_point_number[i] = _Bits[i];
		}
		return fixed_point_number;
	}
	// Copy the bits into the fraction. Rounds away from zero.	
	template <size_t FBits>
	bool assign(unsigned int remaining_bits, bitblock<FBits>& _fraction, std::size_t hpos = FBits)
	{
        if (hpos > FBits)
            throw hpos_too_large{};
                    
        if (remaining_bits > fbits)
            throw rbits_too_large{};
                    
        reset();                                    // In any case
        
		// if input is empty -> reset
		if (FBits == 0 || hpos == 0)
                    return false;
		
		// if my fraction is empty -> check whether to round up (first bit after hidden bit)
		if (fbits == 0 || remaining_bits == 0) 
                    return hpos > 0 && _fraction[hpos-1];                                        
                
		long   ipos = hpos - 1;
		for (size_t i = 0, fpos = fbits - 1; i < remaining_bits && ipos >= 0; ++i, --fpos, --ipos, ++_NrOfBits) 
                    _Bits[fpos] = _fraction[ipos];
		
		// If we one or more bit in the input -> use it for round_up decision
		return ipos >= 0 && _fraction[ipos];
	}

	template <size_t FBits>
	bool assign2(unsigned int remaining_bits, bitblock<FBits>& _fraction)
	{
		if (remaining_bits > fbits)
			throw rbits_too_large{};

		reset();                                    // In any case
												// if input is empty -> reset
		if (FBits == 0)
			return false;

		unsigned hpos = fbits - remaining_bits;

		// if my fraction is empty -> check whether to round up (first bit after hidden bit)
		if (fbits == 0 || remaining_bits == 0)
			return hpos > 0 && _fraction[hpos - 1];

		long   ipos = hpos - 1;
		for (size_t i = 0, fpos = fbits - 1; i < remaining_bits && ipos >= 0; ++i, --fpos, --ipos, ++_NrOfBits)
			_Bits[fpos] = _fraction[ipos];

		// If we one or more bits left in the input -> use it for round_up decision
		return ipos >= 0 && sticky<FBits>(_fraction, ipos);
	}

	template<size_t FBits>
	bool sticky(const bitblock<FBits>& bits, unsigned msb) {
		bool running = false;
		for (int i = msb; i >= 0; i--) {
			running |= bits.test(i);
		}
		return running;
	}

#if 0
	/// Copy the bits into the fraction. Rounds away from zero. TODO: probably superseded by assign	
	bool assign_fraction_(unsigned int remaining_bits, std::bitset<fbits>& _fraction) {
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
#endif 

	/// Normalized shift (e.g., for addition).
	template <size_t Size>
	bitblock<Size> nshift(long shift) const
	{
		bitblock<Size> number;
            
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
	void normalize(bitblock<fbits+3>& number) const {
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
	void denormalize(int shift, bitblock<fbits+3>& number) const {
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
	bitblock<fbits>    _Bits;
	size_t             _NrOfBits;

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

////////////////////// FRACTION operators
template<size_t nfbits>
inline std::ostream& operator<<(std::ostream& ostr, const fraction<nfbits>& f) {
	unsigned int nrOfFractionBitsProcessed = 0;
	if (nfbits > 0) {
		int upperbound = nfbits;
		upperbound--;
		for (int i = upperbound; i >= 0; --i) {
			if (f._NrOfBits > nrOfFractionBitsProcessed++) {
				ostr << (f._Bits[i] ? "1" : "0");
			}
			else {
				ostr << "-";
			}
		}
	}
	if (nrOfFractionBitsProcessed == 0) ostr << "~"; // for proper alignment in tables
	return ostr;
}

template<size_t nfbits>
inline std::istream& operator>> (std::istream& istr, const fraction<nfbits>& f) {
	istr >> f._Bits;
	return istr;
}

template<size_t nfbits>
inline bool operator==(const fraction<nfbits>& lhs, const fraction<nfbits>& rhs) { return lhs._NrOfBits == rhs._NrOfBits && lhs._Bits == rhs._Bits; }
template<size_t nfbits>
inline bool operator!=(const fraction<nfbits>& lhs, const fraction<nfbits>& rhs) { return !operator==(lhs, rhs); }
template<size_t nfbits>
inline bool operator< (const fraction<nfbits>& lhs, const fraction<nfbits>& rhs) { return lhs._NrOfBits <= rhs._NrOfBits && lhs._Bits < rhs._Bits; }
template<size_t nfbits>
inline bool operator> (const fraction<nfbits>& lhs, const fraction<nfbits>& rhs) { return  operator< (rhs, lhs); }
template<size_t nfbits>
inline bool operator<=(const fraction<nfbits>& lhs, const fraction<nfbits>& rhs) { return !operator> (lhs, rhs); }
template<size_t nfbits>
inline bool operator>=(const fraction<nfbits>& lhs, const fraction<nfbits>& rhs) { return !operator< (lhs, rhs); }

}  // namespace unum

}  // namespace sw

