#pragma once
// positFraction.hpp: definition of a posit positFractions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <algorithm>
#include <universal/number/posit/exceptions.hpp>

namespace sw { namespace universal {

using namespace sw::universal::internal;

// positFraction is spec'ed with the size of the posit it belongs to.
// However, the size of the positFraction segment is nbits-3, but we maintain an extra guard bit, so the size of the actual positFraction we manage is nbits-2
template<unsigned fbits, typename bt>
class positFraction {
	using UnsignedFraction = blockbinary<fbits, bt, BinaryNumberType::Unsigned>;
	using UnsignedSignificant = blockbinary<fbits+1, bt, BinaryNumberType::Unsigned>;
public:
	positFraction() : _block{}, _nrBits{} {}

	positFraction(const positFraction& f) = default;
	positFraction(positFraction&& f) = default;
	
	positFraction& operator=(const positFraction& f) = default;
	positFraction& operator=(positFraction&& f) = default;
	
	// selectors
	bool none() const {	return _block.none(); }
	UnsignedFraction bits() const noexcept { return _block; }
	unsigned nrBits() const noexcept { return _nrBits;	}
	// positFractions are assumed to have a hidden bit, the case where they do not must be managed by the container of the positFraction
	// calculate the value of the positFraction ignoring the hidden bit. So a positFraction of 1010 has the value 0.5+0.125=5/8
	long double value() const { 
		long double v = 0.0l;
		if (_block.none()) return v;
		if constexpr (fbits > 0) {
			long double scale = 0.5l;
			for (int i = int(fbits) - 1; i >= 0; i--) {
				if (_block.test(unsigned(i))) v += scale;
				scale *= 0.5l;
				if (scale == 0.0l) break;
			}
		}
		return v;
	}

	// modifiers
	void reset() {
		_nrBits = 0;
		_block.clear();
	}
	void setzero() { reset(); }

	void set(const UnsignedFraction& raw, unsigned nrOfFractionBits = fbits) {
		_block = raw;
		_nrBits = (fbits < nrOfFractionBits ? fbits : nrOfFractionBits);
	}
	// get a fixed point number by making the hidden bit explicit: useful for multiply units
	UnsignedSignificant get_fixed_point() const {
		UnsignedSignificant fixed_point_number;
		fixed_point_number.set(fbits, true); // make hidden bit explicit
		for (unsigned int i = 0; i < fbits; i++) {
			fixed_point_number.set(i, _block.test(i));
		}
		return fixed_point_number;
	}
/*
	// Copy the bits into the positFraction. Rounds away from zero.	
	template <unsigned FBits>
	bool assign(unsigned int remaining_bits, blockbinary<FBits, bt>& _positFraction, std::unsigned hpos = FBits)
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
		for (unsigned i = 0, fpos = fbits - 1; i < remaining_bits && ipos >= 0; ++i, --fpos, --ipos, ++_nrBits) 
                    _block[fpos] = _positFraction[ipos];
		
		// If we one or more bit in the input -> use it for round_up decision
		return ipos >= 0 && _positFraction[ipos];
	}

	template <unsigned FBits>
	bool assign2(unsigned int remaining_bits, blockbinary<FBits, bt>& _positFraction)
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
		for (unsigned i = 0, fpos = fbits - 1; i < remaining_bits && ipos >= 0; ++i, --fpos, --ipos, ++_nrBits)
			_block[fpos] = _positFraction[ipos];

		// If we one or more bits left in the input -> use it for round_up decision
		return ipos >= 0 && sticky<FBits, bt>(_positFraction, ipos);
	}

	template<unsigned FBits>
	bool sticky(const blockbinary<FBits, bt>& bits, unsigned msb) {
		bool running = false;
		for (int i = msb; i >= 0; i--) {
			running |= bits.test(i);
		}
		return running;
	}

	/// Normalized shift (e.g., for addition).
	template <unsigned Size>
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
            
        // Copy positFraction bits into certain part
        for (long npos = hpos - 1, fpos = long(fbits) - 1; npos > 0 && fpos >= 0; --npos, --fpos)
            number[npos] = _block[fpos];
                
        // Set uncertainty bit
        bool uncertainty = false;
        for (long fpos = std::min(long(fbits)-1, -shift); fpos >= 0 && !uncertainty; --fpos)
            uncertainty |= _block[fpos];
        number[0] = uncertainty;
        return number;
    }
	
	
	// normalize the positFraction and return its positFraction in the argument: add a sticky bit and two guard bits to the result
	void normalize(blockbinary<fbits+3, bt>& number) const {
		number.set(fbits, true); // set hidden bit
		for (int i = 0; i < fbits; i++) {
			number.set(i, _block[i]);
		}
	}

	void increment() {
		++_block;
	}
*/

	/*   h is hidden bit
	 *   h.bbbb_bbbb_bbbb_b...      positFraction
	 *   0.000h_bbbb_bbbb_bbbb_b... number
	 *  >-.----<                    shift of 4
	 */
	void denormalize(int shift, blockbinary<fbits+3, bt, BinaryNumberType::Unsigned>& number) const {
		number.reset();
		if (fbits == 0) return;
		if (shift < 0) shift = -shift;
		if (shift <= static_cast<int>(fbits)) {
			number.set(static_cast<int>(fbits) - shift); // set hidden bit
			for (int i = static_cast<int>(fbits) - 1 - shift; i >= 0; i--) {
				number.set(i, _block.test(unsigned(i + shift)));
			}
		}
	}


private:
	// maximum size positFraction is <nbits - one sign bit - minimum two regime bits>
	// but we maintain 1 guard bit for rounding decisions
	UnsignedFraction   _block;
	unsigned           _nrBits;

	// template parameters need names different from class template parameters (for gcc and clang)
	// Without the template (i.e. only own operators are friends) we get linker errors
	template<unsigned nfbits, typename bbt>
	friend std::ostream& operator<< (std::ostream& ostr, const positFraction<nfbits, bbt>& f);
	template<unsigned nfbits, typename bbt>
	friend std::istream& operator>> (std::istream& istr, positFraction<nfbits, bbt>& f);

	template<unsigned nfbits, typename bbt>
	friend bool operator==(const positFraction<nfbits, bbt>& lhs, const positFraction<nfbits, bbt>& rhs);
	template<unsigned nfbits, typename bbt>
	friend bool operator!=(const positFraction<nfbits, bbt>& lhs, const positFraction<nfbits, bbt>& rhs);
	template<unsigned nfbits, typename bbt>
	friend bool operator< (const positFraction<nfbits, bbt>& lhs, const positFraction<nfbits, bbt>& rhs);
	template<unsigned nfbits, typename bbt>
	friend bool operator> (const positFraction<nfbits, bbt>& lhs, const positFraction<nfbits, bbt>& rhs);
	template<unsigned nfbits, typename bbt>
	friend bool operator<=(const positFraction<nfbits, bbt>& lhs, const positFraction<nfbits, bbt>& rhs);
	template<unsigned nfbits, typename bbt>
	friend bool operator>=(const positFraction<nfbits, bbt>& lhs, const positFraction<nfbits, bbt>& rhs);
};

////////////////////// FRACTION operators
template<unsigned nfbits, typename bbt>
inline std::ostream& operator<<(std::ostream& ostr, const positFraction<nfbits, bbt>& f) {
	unsigned nrOfFractionBitsProcessed = 0;
	if constexpr (nfbits > 0) {
		int upperbound = int(nfbits) - 1;
		for (int i = upperbound; i >= 0; --i) {
			if (f._nrBits > ++nrOfFractionBitsProcessed) {
				ostr << (f._block.test(unsigned(i)) ? "1" : "0");
			}
			else {
				ostr << "-";
			}
		}
	}
	if (nrOfFractionBitsProcessed == 0) ostr << "~"; // for proper alignment in tables
	return ostr;
}

template<unsigned nfbits, typename bbt>
inline std::istream& operator>> (std::istream& istr, const positFraction<nfbits, bbt>& f) {
	istr >> f._block;
	return istr;
}

template<unsigned nfbits, typename bbt>
inline std::string to_string(const positFraction<nfbits, bbt>& f, bool dashExtent = true, bool nibbleMarker = false) {
	unsigned int nrOfFractionBitsProcessed = 0;
	std::stringstream s;
	if constexpr (nfbits > 0) {
		blockbinary<nfbits, bbt, BinaryNumberType::Unsigned> bb = f.bits();
		for (unsigned i = 0; i < nfbits; ++i) {
			unsigned bitIndex = nfbits - 1ull - i;
			if (f.nrBits() > nrOfFractionBitsProcessed++) {
				s << (bb.test(bitIndex) ? '1' : '0');
			}
			else {
				s << (dashExtent ? "-" : "");
			}
			if (nibbleMarker && ((bitIndex % 4) == 0) && bitIndex != 0) s << '\'';
		}
	}
	if (nrOfFractionBitsProcessed == 0) s << '~'; // for proper alignment in tables
	return s.str();
}

template<unsigned nfbits, typename bbt>
inline bool operator==(const positFraction<nfbits, bbt>& lhs, const positFraction<nfbits, bbt>& rhs) { return lhs._nrBits == rhs._nrBits && lhs._block == rhs._block; }
template<unsigned nfbits, typename bbt>
inline bool operator!=(const positFraction<nfbits, bbt>& lhs, const positFraction<nfbits, bbt>& rhs) { return !operator==(lhs, rhs); }
template<unsigned nfbits, typename bbt>
inline bool operator< (const positFraction<nfbits, bbt>& lhs, const positFraction<nfbits, bbt>& rhs) { return lhs._nrBits <= rhs._nrBits && lhs._block < rhs._block; }
template<unsigned nfbits, typename bbt>
inline bool operator> (const positFraction<nfbits, bbt>& lhs, const positFraction<nfbits, bbt>& rhs) { return  operator< (rhs, lhs); }
template<unsigned nfbits, typename bbt>
inline bool operator<=(const positFraction<nfbits, bbt>& lhs, const positFraction<nfbits, bbt>& rhs) { return !operator> (lhs, rhs); }
template<unsigned nfbits, typename bbt>
inline bool operator>=(const positFraction<nfbits, bbt>& lhs, const positFraction<nfbits, bbt>& rhs) { return !operator< (lhs, rhs); }

}} // namespace sw::universal

