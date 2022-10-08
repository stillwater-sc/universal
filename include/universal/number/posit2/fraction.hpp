#pragma once
// fraction.hpp: definition of a posit fractions
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <algorithm>
#include <universal/number/posit/exceptions.hpp>

namespace sw { namespace universal {

using namespace sw::universal::internal;

// fraction is spec'ed with the size of the posit it belongs to.
// However, the size of the fraction segment is nbits-3, but we maintain an extra guard bit, so the size of the actual fraction we manage is nbits-2
template<size_t fbits, typename bt>
class fraction {
public:
	fraction() : _Bits(), _NrOfBits(0) {}

	fraction(const fraction& f) = default;
	fraction(fraction&& f) = default;
	
	fraction& operator=(const fraction& f) = default;
	fraction& operator=(fraction&& f) = default;
	
	// selectors
	bool none() const {	return _Bits.none(); }
	blockbinary<fbits, bt> bits() const noexcept { return _Bits; }
	size_t nrBits() const noexcept { return _NrOfBits;	}
	// fractions are assumed to have a hidden bit, the case where they do not must be managed by the container of the fraction
	// calculate the value of the fraction ignoring the hidden bit. So a fraction of 1010 has the value 0.5+0.125=5/8
	long double value() const { 
		long double v = 0.0l;
		if (_Bits.none()) return v;
		if constexpr (fbits > 0) {
			long double scale = 0.5l;
			for (int i = int(fbits) - 1; i >= 0; i--) {
				if (_Bits.test(size_t(i))) v += scale;
				scale *= 0.5l;
				if (scale == 0.0l) break;
			}
		}
		return v;
	}

	// modifiers
	void reset() {
		_NrOfBits = 0;
		_Bits.clear();
	}
	void setzero() { reset(); }



	void set(const blockbinary<fbits, bt>& raw, std::size_t nrOfFractionBits = fbits) {
		_Bits = raw;
		_NrOfBits = (fbits < nrOfFractionBits ? fbits : nrOfFractionBits);
	}
	// get a fixed point number by making the hidden bit explicit: useful for multiply units
	blockbinary<fbits + 1, bt> get_fixed_point() const {
		blockbinary<fbits + 1, bt> fixed_point_number;
		fixed_point_number.set(fbits, true); // make hidden bit explicit
		for (unsigned int i = 0; i < fbits; i++) {
			fixed_point_number[i] = _Bits[i];
		}
		return fixed_point_number;
	}
	// Copy the bits into the fraction. Rounds away from zero.	
	template <size_t FBits>
	bool assign(unsigned int remaining_bits, blockbinary<FBits, bt>& _fraction, std::size_t hpos = FBits)
	{
        if (hpos > FBits)
            throw posit_hpos_too_large{};
                    
        if (remaining_bits > fbits)
            throw posit_rbits_too_large{};
                    
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
	bool assign2(unsigned int remaining_bits, blockbinary<FBits, bt>& _fraction)
	{
		if (remaining_bits > fbits)
			throw posit_rbits_too_large{};

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
		return ipos >= 0 && sticky<FBits, bt>(_fraction, ipos);
	}

	template<size_t FBits>
	bool sticky(const blockbinary<FBits, bt>& bits, unsigned msb) {
		bool running = false;
		for (int i = msb; i >= 0; i--) {
			running |= bits.test(i);
		}
		return running;
	}

	/// Normalized shift (e.g., for addition).
	template <size_t Size>
	blockbinary<Size, bt> nshift(long shift) const
	{
		blockbinary<Size, bt> number;
            
        // Check range
		if (long(fbits) + shift >= long(Size))
			throw std::runtime_error("shift_too_large"); // value_shift_too_large{};
                
        const long hpos = fbits + shift;       // position of hidden bit
            
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
	void normalize(blockbinary<fbits+3, bt>& number) const {
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
	void denormalize(int shift, blockbinary<fbits+3, bt>& number) const {
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
	blockbinary<fbits, bt>    _Bits;
	size_t             _NrOfBits;

	// template parameters need names different from class template parameters (for gcc and clang)
	// Without the template (i.e. only own operators are friends) we get linker errors
	template<size_t nfbits, typename bbt>
	friend std::ostream& operator<< (std::ostream& ostr, const fraction<nfbits, bbt>& f);
	template<size_t nfbits, typename bbt>
	friend std::istream& operator>> (std::istream& istr, fraction<nfbits, bbt>& f);

	template<size_t nfbits, typename bbt>
	friend bool operator==(const fraction<nfbits, bbt>& lhs, const fraction<nfbits, bbt>& rhs);
	template<size_t nfbits, typename bbt>
	friend bool operator!=(const fraction<nfbits, bbt>& lhs, const fraction<nfbits, bbt>& rhs);
	template<size_t nfbits, typename bbt>
	friend bool operator< (const fraction<nfbits, bbt>& lhs, const fraction<nfbits, bbt>& rhs);
	template<size_t nfbits, typename bbt>
	friend bool operator> (const fraction<nfbits, bbt>& lhs, const fraction<nfbits, bbt>& rhs);
	template<size_t nfbits, typename bbt>
	friend bool operator<=(const fraction<nfbits, bbt>& lhs, const fraction<nfbits, bbt>& rhs);
	template<size_t nfbits, typename bbt>
	friend bool operator>=(const fraction<nfbits, bbt>& lhs, const fraction<nfbits, bbt>& rhs);
};

////////////////////// FRACTION operators
template<size_t nfbits, typename bbt>
inline std::ostream& operator<<(std::ostream& ostr, const fraction<nfbits, bbt>& f) {
	size_t nrOfFractionBitsProcessed = 0;
	if (nfbits > 0) {
		int upperbound = int(nfbits) - 1;
		for (int i = upperbound; i >= 0; --i) {
			if (f._NrOfBits > ++nrOfFractionBitsProcessed) {
				ostr << (f._Bits[size_t(i)] ? "1" : "0");
			}
			else {
				ostr << "-";
			}
		}
	}
	if (nrOfFractionBitsProcessed == 0) ostr << "~"; // for proper alignment in tables
	return ostr;
}

template<size_t nfbits, typename bbt>
inline std::istream& operator>> (std::istream& istr, const fraction<nfbits, bbt>& f) {
	istr >> f._Bits;
	return istr;
}

template<size_t nfbits, typename bbt>
inline std::string to_string(const fraction<nfbits, bbt>& f, bool dashExtent = true, bool nibbleMarker = false) {
	unsigned int nrOfFractionBitsProcessed = 0;
	std::stringstream s;
	if (nfbits > 0) {
		blockbinary<nfbits, bbt> bb = f.bits();
		int upperbound = nfbits;
		upperbound--;
		for (int i = upperbound; i >= 0; --i) {
			if (f.nrBits() > nrOfFractionBitsProcessed++) {
				s << (bb[static_cast<size_t>(i)] ? '1' : '0');

			}
			else {
				s << (dashExtent ? "-" : "");
			}
			if (nibbleMarker && ((i % 4) == 0) && i != 0) s << '\'';
		}
	}
	if (nrOfFractionBitsProcessed == 0) s << '~'; // for proper alignment in tables
	return s.str();
}

template<size_t nfbits, typename bbt>
inline bool operator==(const fraction<nfbits, bbt>& lhs, const fraction<nfbits, bbt>& rhs) { return lhs._NrOfBits == rhs._NrOfBits && lhs._Bits == rhs._Bits; }
template<size_t nfbits, typename bbt>
inline bool operator!=(const fraction<nfbits, bbt>& lhs, const fraction<nfbits, bbt>& rhs) { return !operator==(lhs, rhs); }
template<size_t nfbits, typename bbt>
inline bool operator< (const fraction<nfbits, bbt>& lhs, const fraction<nfbits, bbt>& rhs) { return lhs._NrOfBits <= rhs._NrOfBits && lhs._Bits < rhs._Bits; }
template<size_t nfbits, typename bbt>
inline bool operator> (const fraction<nfbits, bbt>& lhs, const fraction<nfbits, bbt>& rhs) { return  operator< (rhs, lhs); }
template<size_t nfbits, typename bbt>
inline bool operator<=(const fraction<nfbits, bbt>& lhs, const fraction<nfbits, bbt>& rhs) { return !operator> (lhs, rhs); }
template<size_t nfbits, typename bbt>
inline bool operator>=(const fraction<nfbits, bbt>& lhs, const fraction<nfbits, bbt>& rhs) { return !operator< (lhs, rhs); }

}} // namespace sw::universal

