#pragma once
// valid.hpp: definition of arbitrary valid number configurations
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <limits>

namespace sw {

	namespace unum {

		template<size_t nbits, size_t es>
		class valid {

			static_assert(es + 3 <= nbits, "Value for 'es' is too large for this 'nbits' value");
			//static_assert(sizeof(long double) == 16, "Valid library requires compiler support for 128 bit long double.");

			template <typename T>
			valid<nbits, es>& _assign(const T& rhs) {
				constexpr int fbits = std::numeric_limits<T>::digits - 1;
				value<fbits> v((T)rhs);

				return *this;
			}

		public:
			static constexpr size_t somebits = 10;

			valid<nbits, es>() { clear(); }

			valid(const valid&) = default;
			valid(valid&&) = default;

			valid& operator=(const valid&) = default;
			valid& operator=(valid&&) = default;

			valid(long initial_value) { *this = initial_value; }
			valid(unsigned long long initial_value) { *this = initial_value; }
			valid(double initial_value) { *this = initial_value; }
			valid(long double initial_value) { *this = initial_value; }

			valid& operator=(int rhs) { return _assign(rhs); }
			valid& operator=(unsigned long long rhs) { return _assign(rhs); }
			valid& operator=(double rhs) { return _assign(rhs); }
			valid& operator=(long double rhs) { return _assign(rhs); }

			valid& operator+=(const valid& rhs) {
				return *this;
			}
			valid& operator-=(const valid& rhs) {
				return *this;
			}
			valid& operator*=(const valid& rhs) {
				return *this;
			}
			valid& operator/=(const valid& rhs) {
				return *this;
			}

			// conversion operators

			// selectors
			inline bool isOpen() const {
				return !isClosed();
			}
			inline bool isClosed() const {
				return lubit && uubit;
			}
			inline bool isOpenLower() const {
				return lubit;
			}
			inline bool isOpenUpper() const {
				return uubit;
			}
			inline bool getlb(sw::unum::posit<nbits, es>& _lb) const {
				_lb = lb;
				return lubit;
			}
			inline bool getub(sw::unum::posit<nbits, es>& _ub) const {
				_ub = ub;
				return uubit;
			}

	// modifiers

	// TODO: do we clear to exact 0, or [-inf, inf]?
	inline void clear() {
		lb.clear();
		ub.clear();
		lubit = true;
		uubit = true;
	}
	inline void setToInclusive() {
		lb.setToNaR();
		ub.setToNaR();
		lubit = true;
		uubit = true;
	}
	inline void setlb(sw::unum::posit<nbits, es>& _lb, bool ubit) {
		lb = _lb;
		lubit = ubit;
	}
	inline void setub(sw::unum::posit<nbits, es>& _ub, bool ubit) {
		ub = _ub;
		uubit = ubit;
	}

	// relative_order returns -1 if v was rounded up, 0 if it was exact, and 1 if v was rounded down
	template <size_t FBits>
	inline int relative_order(const value<FBits>& v) {
		if (v.isZero()) {
			return 0;
		}
		if (v.isNaN() || v.isInfinite()) {
			return 0;
		}
		return convert(v.sign(), v.scale(), v.fraction());
	}

private:
	// member variables
	sw::unum::posit<nbits, es> lb, ub;  // lower_bound and upper_bound of the tile
	bool lubit, uubit; // lower ubit, upper ubit


	// helper methods	

	// special case check for projecting values between (0, minpos] to minpos and [maxpos, inf) to maxpos
	// Returns true if the scale is too small or too large for this posit config
	bool check_inward_projection_range(int scale) {
		// calculate the max k factor for this posit config
		int posit_size = nbits;
		int k = scale < 0 ? -(posit_size - 2) : (posit_size - 2);
		return scale < 0 ? scale < k*(1 << es) : scale > k*(1 << es);
	}


	// convert assumes that ZERO and NaR cases are handled. Only non-zero and non-NaR values are allowed.
	template<size_t input_fbits>
	int convert(bool sign, int scale, bitblock<input_fbits> input_fraction) {
		// construct the posit
		//int k = calculate_unconstrained_k<nbits, es>(scale);
		// interpolation rule checks
		if (check_inward_projection_range(scale)) {    // regime dominated
			return 1; // rounded down
		}
		else {
			const size_t pt_len = nbits + 3 + es;
			bitblock<pt_len> pt_bits;
			bitblock<pt_len> regime;
			bitblock<pt_len> exponent;
			bitblock<pt_len> fraction;
			bitblock<pt_len> sticky_bit;

			bool s = sign;
			int e = scale;
			bool r = (e >= 0);

			unsigned run = (r ? 1 + (e >> es) : -(e >> es));
			regime.set(0, 1 ^ r);
			for (unsigned i = 1; i <= run; i++) regime.set(i, r);

			unsigned esval = e % (uint32_t(1) << es);
			exponent = convert_to_bitblock<pt_len>(esval);
			unsigned nf = (unsigned)std::max<int>(0, (nbits + 1) - (2 + run + es));
			// TODO: what needs to be done if nf > fbits?
			//assert(nf <= input_fbits);
			// copy the most significant nf fraction bits into fraction
			unsigned lsb = nf <= input_fbits ? 0 : nf - input_fbits;
			for (unsigned i = lsb; i < nf; i++) fraction[i] = input_fraction[input_fbits - nf + i];

			bool sb = anyAfter(input_fraction, input_fbits - 1 - nf);

			// construct the untruncated posit
			// pt    = BitOr[BitShiftLeft[reg, es + nf + 1], BitShiftLeft[esval, nf + 1], BitShiftLeft[fv, 1], sb];
			regime <<= es + nf + 1;
			exponent <<= nf + 1;
			fraction <<= 1;
			sticky_bit.set(0, sb);

			pt_bits |= regime;
			pt_bits |= exponent;
			pt_bits |= fraction;
			pt_bits |= sticky_bit;

			unsigned len = 1 + std::max<unsigned>((nbits + 1), (2 + run + es));
			bool blast = pt_bits.test(len - nbits);
			bool bafter = pt_bits.test(len - nbits - 1);
			bool bsticky = anyAfter(pt_bits, len - nbits - 1 - 1);

			bool rb = (blast & bafter) | (bafter & bsticky);

			pt_bits <<= pt_len - len;
			bitblock<nbits> ptt;
			truncate(pt_bits, ptt);

			if (rb) increment_bitset(ptt);
			if (s) ptt = twos_complement(ptt);

			return -1; // TODO
		}
	}

	// friends
	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nnbits, size_t ees>
	friend std::ostream& operator<< (std::ostream& ostr, const valid<nnbits, ees>& p);
	template<size_t nnbits, size_t ees>
	friend std::istream& operator>> (std::istream& istr, valid<nnbits, ees>& p);

	template<size_t nnbits, size_t ees>
	friend bool operator==(const valid<nnbits, ees>& lhs, const valid<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator!=(const valid<nnbits, ees>& lhs, const valid<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator< (const valid<nnbits, ees>& lhs, const valid<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator> (const valid<nnbits, ees>& lhs, const valid<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator<=(const valid<nnbits, ees>& lhs, const valid<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator>=(const valid<nnbits, ees>& lhs, const valid<nnbits, ees>& rhs);
};


// VALID operators
template<size_t nbits, size_t es>
inline std::ostream& operator<<(std::ostream& ostr, const valid<nbits, es>& v) {
	// determine lower bound
	if (v.lubit == true) {  // exact lower bound
		ostr << '[' << v.lb << ", ";
	}
	else {					// inexact lower bound
		ostr << '(' << v.lb << ", ";
	}
	if (v.uubit == true) { // exact upper bound
		ostr << v.ub << ']';
	}
	else {					// inexact upper bound
		ostr << v.ub << ')';
	}
	return ostr;
}

template<size_t nbits, size_t es>
inline std::istream& operator>> (std::istream& istr, const valid<nbits, es>& v) {
	istr >> v._Bits;
	return istr;
}

	}  // namespace unum

} // namespace sw
