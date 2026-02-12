#pragma once
// positFraction.hpp: definition of a posit positFractions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <algorithm>
#include <universal/number/posit1/exceptions.hpp>

namespace sw { namespace universal {

using namespace sw::universal::internal;

// positFraction is spec'ed with the size of the posit it belongs to.
// However, the size of the positFraction segment is nbits-3, but we maintain an extra guard bit, so the size of the actual positFraction we manage is nbits-2
template<unsigned fbits>
class positFraction {
public:
	positFraction() : _Bits(), _NrOfBits(0) {}

	positFraction(const positFraction& f) = default;
	positFraction(positFraction&& f) = default;
	
	positFraction& operator=(const positFraction& f) = default;
	positFraction& operator=(positFraction&& f) = default;
	
	// selectors
	bool none() const {	return _Bits.none(); }
	unsigned nrBits() const { return _NrOfBits;	}
	// positFractions are assumed to have a hidden bit, the case where they do not must be managed by the container of the positFraction
	// calculate the value of the positFraction ignoring the hidden bit. So a positFraction of 1010 has the value 0.5+0.125=5/8
	long double value() const { 
		long double v = 0.0l;
		if (_Bits.none()) return v;
		if constexpr (fbits > 0) {
			long double scale = 0.5l;
			for (int i = int(fbits) - 1; i >= 0; i--) {
				if (_Bits.test(unsigned(i))) v += scale;
				scale *= 0.5l;
				if (scale == 0.0l) break;
			}
		}
		return v;
	}

	// modifiers
	void reset() {
		_NrOfBits = 0;
		_Bits.reset();
	}
	void setzero() { reset(); }
	internal::bitblock<fbits> get() const {
		return _Bits;
	}
	void set(const internal::bitblock<fbits>& raw, unsigned nrOfFractionBits = fbits) {
		_Bits = raw;
		_NrOfBits = (fbits < nrOfFractionBits ? fbits : nrOfFractionBits);
	}
	// get a fixed point number by making the hidden bit explicit: useful for multiply units
	internal::bitblock<fbits + 1> get_fixed_point() const {
		bitblock<fbits + 1> fixed_point_number;
		fixed_point_number.set(fbits, true); // make hidden bit explicit
		for (unsigned int i = 0; i < fbits; i++) {
			fixed_point_number[i] = _Bits[i];
		}
		return fixed_point_number;
	}
	// Copy the bits into the positFraction. Rounds away from zero.	
	template <unsigned FBits>
	bool assign(unsigned int remaining_bits, internal::bitblock<FBits>& _positFraction, unsigned hpos = FBits)
	{
        if (hpos > FBits)
            throw posit_hpos_too_large{};
                    
        if (remaining_bits > fbits)
            throw posit_rbits_too_large{};
                    
        reset();                                    // In any case
        
		// if input is empty -> reset
		if (FBits == 0 || hpos == 0)
                    return false;
		
		// if my positFraction is empty -> check whether to round up (first bit after hidden bit)
		if (fbits == 0 || remaining_bits == 0) 
                    return hpos > 0 && _positFraction[hpos-1];                                        
                
		long   ipos = hpos - 1;
		for (unsigned i = 0, fpos = fbits - 1; i < remaining_bits && ipos >= 0; ++i, --fpos, --ipos, ++_NrOfBits) 
                    _Bits[fpos] = _positFraction[ipos];
		
		// If we one or more bit in the input -> use it for round_up decision
		return ipos >= 0 && _positFraction[ipos];
	}

	template <unsigned FBits>
	bool assign2(unsigned int remaining_bits, internal::bitblock<FBits>& _positFraction)
	{
		if (remaining_bits > fbits)
			throw posit_rbits_too_large{};

		reset();                                    // In any case
												// if input is empty -> reset
		if (FBits == 0)
			return false;

		unsigned hpos = fbits - remaining_bits;

		// if my positFraction is empty -> check whether to round up (first bit after hidden bit)
		if (fbits == 0 || remaining_bits == 0)
			return hpos > 0 && _positFraction[hpos - 1];

		long   ipos = hpos - 1;
		for (unsigned i = 0, fpos = fbits - 1; i < remaining_bits && ipos >= 0; ++i, --fpos, --ipos, ++_NrOfBits)
			_Bits[fpos] = _positFraction[ipos];

		// If we one or more bits left in the input -> use it for round_up decision
		return ipos >= 0 && sticky<FBits>(_positFraction, ipos);
	}

	template<unsigned FBits>
	bool sticky(const internal::bitblock<FBits>& bits, unsigned msb) {
		bool running = false;
		for (int i = msb; i >= 0; i--) {
			running |= bits.test(i);
		}
		return running;
	}

#if 0
	/// Copy the bits into the positFraction. Rounds away from zero. TODO: probably superseded by assign	
	bool assign_positFraction_(unsigned int remaining_bits, std::bitset<fbits>& _positFraction) {
		bool round_up = false;
		if (fbits == 0) return false;
		if (remaining_bits > 0 && fbits > 0) {
			_NrOfBits = 0;
			for (unsigned int i = 0; i < remaining_bits; i++) {
				_Bits[fbits - 1 - i] = _positFraction[fbits - 1 - i];
				_NrOfBits++;
			}
			if (fbits > remaining_bits) {
				round_up = _positFraction[fbits - 1 - remaining_bits];
			}		
		}
		else {
			round_up = _positFraction[fbits - 1];
			_NrOfBits = 0;
		}
		return round_up;
	}
#endif 

	/// Normalized shift (e.g., for addition).
	template <unsigned Size>
	internal::bitblock<Size> nshift(long shift) const
	{
		bitblock<Size> number;
            
        // Check range
        if (long(fbits) + shift >= long(Size))
            throw value_shift_too_large{};
                
        const long hpos = fbits + shift;              // position of hidden bit
            
        // If hidden bit is LSB or beyond just set uncertainty bit and call it a day
        if (hpos <= 0) {
            number[0] = true;
            return number;
        }
                
        number[hpos] = true;                   // hidden bit now safely set
            
        // Copy positFraction bits into certain part
        for (long npos = hpos - 1, fpos = long(fbits) - 1; npos > 0 && fpos >= 0; --npos, --fpos)
            number[npos] = _Bits[fpos];
                
        // Set uncertainty bit
        bool uncertainty = false;
        for (long fpos = std::min(long(fbits)-1, -shift); fpos >= 0 && !uncertainty; --fpos)
            uncertainty |= _Bits[fpos];
        number[0] = uncertainty;
        return number;
    }
	
	
	// normalize the positFraction and return its positFraction in the argument: add a sticky bit and two guard bits to the result
	void normalize(internal::bitblock<fbits+3>& number) const {
		number.set(fbits, true); // set hidden bit
		for (int i = 0; i < fbits; i++) {
			number.set(i, _Bits[i]);
		}
	}
	/*   h is hidden bit
	*   h.bbbb_bbbb_bbbb_b...      positFraction
	*   0.000h_bbbb_bbbb_bbbb_b... number
	*  >-.----<                    shift of 4
	*/
	void denormalize(int shift, internal::bitblock<fbits+3>& number) const {
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
	// maximum size positFraction is <nbits - one sign bit - minimum two regime bits>
	// but we maintain 1 guard bit for rounding decisions
	internal::bitblock<fbits>    _Bits;
	unsigned             _NrOfBits;

	// template parameters need names different from class template parameters (for gcc and clang)
	// Without the template (i.e. only own operators are friends) we get linker errors
	template<unsigned nfbits>
	friend std::ostream& operator<< (std::ostream& ostr, const positFraction<nfbits>& f);
	template<unsigned nfbits>
	friend std::istream& operator>> (std::istream& istr, positFraction<nfbits>& f);

	template<unsigned nfbits>
	friend bool operator==(const positFraction<nfbits>& lhs, const positFraction<nfbits>& rhs);
	template<unsigned nfbits>
	friend bool operator!=(const positFraction<nfbits>& lhs, const positFraction<nfbits>& rhs);
	template<unsigned nfbits>
	friend bool operator< (const positFraction<nfbits>& lhs, const positFraction<nfbits>& rhs);
	template<unsigned nfbits>
	friend bool operator> (const positFraction<nfbits>& lhs, const positFraction<nfbits>& rhs);
	template<unsigned nfbits>
	friend bool operator<=(const positFraction<nfbits>& lhs, const positFraction<nfbits>& rhs);
	template<unsigned nfbits>
	friend bool operator>=(const positFraction<nfbits>& lhs, const positFraction<nfbits>& rhs);
};

////////////////////// FRACTION operators
template<unsigned nfbits>
inline std::ostream& operator<<(std::ostream& ostr, const positFraction<nfbits>& f) {
	unsigned nrOfFractionBitsProcessed = 0;
	if constexpr (nfbits > 0) {
		int upperbound = int(nfbits) - 1;
		for (int i = upperbound; i >= 0; --i) {
			if (f._NrOfBits > ++nrOfFractionBitsProcessed) {
				ostr << (f._Bits[unsigned(i)] ? "1" : "0");
			}
			else {
				ostr << "-";
			}
		}
	}
	if (nrOfFractionBitsProcessed == 0) ostr << "~"; // for proper alignment in tables
	return ostr;
}

template<unsigned nfbits>
inline std::istream& operator>> (std::istream& istr, const positFraction<nfbits>& f) {
	istr >> f._Bits;
	return istr;
}

template<unsigned nfbits>
inline std::string to_string(const positFraction<nfbits>& f, bool dashExtent = true, bool nibbleMarker = false) {
	unsigned int nrOfFractionBitsProcessed = 0;
	std::stringstream sstr;
	unsigned fbits = f.nrBits();
	if constexpr (nfbits != 0) {
		bitblock<nfbits> bb = f.get();
		int upperbound = nfbits;
		upperbound--;
		for (int i = upperbound; i >= 0; --i) {
			if (f.nrBits() > nrOfFractionBitsProcessed++) {
				sstr << (bb[static_cast<unsigned>(i)] ? '1' : '0');

			}
			else {
				sstr << (dashExtent ? "-" : "");
			}
			--fbits;
			if (nibbleMarker && fbits != 0 && (fbits % 4) == 0) sstr << '\'';
		}
	}
	if (nrOfFractionBitsProcessed == 0) sstr << '~'; // for proper alignment in tables
	return sstr.str();
}

template<unsigned nfbits>
inline bool operator==(const positFraction<nfbits>& lhs, const positFraction<nfbits>& rhs) { return lhs._NrOfBits == rhs._NrOfBits && lhs._Bits == rhs._Bits; }
template<unsigned nfbits>
inline bool operator!=(const positFraction<nfbits>& lhs, const positFraction<nfbits>& rhs) { return !operator==(lhs, rhs); }
template<unsigned nfbits>
inline bool operator< (const positFraction<nfbits>& lhs, const positFraction<nfbits>& rhs) { return lhs._NrOfBits <= rhs._NrOfBits && lhs._Bits < rhs._Bits; }
template<unsigned nfbits>
inline bool operator> (const positFraction<nfbits>& lhs, const positFraction<nfbits>& rhs) { return  operator< (rhs, lhs); }
template<unsigned nfbits>
inline bool operator<=(const positFraction<nfbits>& lhs, const positFraction<nfbits>& rhs) { return !operator> (lhs, rhs); }
template<unsigned nfbits>
inline bool operator>=(const positFraction<nfbits>& lhs, const positFraction<nfbits>& rhs) { return !operator< (lhs, rhs); }

}} // namespace sw::universal

