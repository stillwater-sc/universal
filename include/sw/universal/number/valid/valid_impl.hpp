#pragma once
// valid_impl.hpp: implementation of arbitrary valid number configurations
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <limits>
#include <universal/number/posit1/posit_impl.hpp>

namespace sw { namespace universal {

template<unsigned nbits, unsigned es>
class valid {

	static_assert(es + 3 <= nbits, "Value for 'es' is too large for this 'nbits' value");
	//static_assert(sizeof(long double) == 16, "Valid library requires compiler support for 128 bit long double.");

	// Constexpr-promotable assignment helper.  internal::value's constexpr
	// constructor (#717) makes the value-domain construction usable at
	// constant evaluation; the actual posit endpoint encoding is left to a
	// follow-up that wires the convert() result into lb / ub.
	template <typename T>
	CONSTEXPRESSION valid<nbits, es>& _assign(const T& rhs) {
		constexpr int fbits = std::numeric_limits<T>::digits - 1;
		internal::value<fbits> v((T)rhs);

		return *this;
	}

public:
	static constexpr unsigned somebits = 10;

	// Default ctor: zero-initialize both posit endpoints (posit1 has a
	// constexpr default after #718) and clear the ubit flags.
	constexpr valid() noexcept : lb{}, ub{}, lubit{ false }, uubit{ false } { }

	constexpr valid(const valid&) = default;
	constexpr valid(valid&&) = default;

	constexpr valid& operator=(const valid&) = default;
	constexpr valid& operator=(valid&&) = default;

	CONSTEXPRESSION explicit valid(int initial_value)                : lb{}, ub{}, lubit{ false }, uubit{ false } { *this = initial_value; }
	CONSTEXPRESSION explicit valid(long initial_value)               : lb{}, ub{}, lubit{ false }, uubit{ false } { *this = initial_value; }
	CONSTEXPRESSION explicit valid(unsigned long long initial_value) : lb{}, ub{}, lubit{ false }, uubit{ false } { *this = initial_value; }
	CONSTEXPRESSION          valid(double initial_value)             : lb{}, ub{}, lubit{ false }, uubit{ false } { *this = initial_value; }
	CONSTEXPRESSION explicit valid(long double initial_value)        : lb{}, ub{}, lubit{ false }, uubit{ false } { *this = initial_value; }

	CONSTEXPRESSION valid& operator=(int rhs)                { return _assign(rhs); }
	CONSTEXPRESSION valid& operator=(unsigned long long rhs) { return _assign(rhs); }
	CONSTEXPRESSION valid& operator=(double rhs)             { return _assign(rhs); }
	CONSTEXPRESSION valid& operator=(long double rhs)        { return _assign(rhs); }

	// Compound arithmetic stubs: kept as no-ops until #744 follow-up
	// implements interval arithmetic (intersection, hull, midpoint).
	// Mark constexpr so the surface is constant-evaluable today.
	constexpr valid& operator+=(const valid& rhs) noexcept {
		(void)rhs;
		return *this;
	}
	constexpr valid& operator-=(const valid& rhs) noexcept {
		(void)rhs;
		return *this;
	}
	constexpr valid& operator*=(const valid& rhs) noexcept {
		(void)rhs;
		return *this;
	}
	constexpr valid& operator/=(const valid& rhs) noexcept {
		(void)rhs;
		return *this;
	}

	// conversion operators

	// selectors
	constexpr bool isopen() const noexcept {
		return !isclosed();
	}
	constexpr bool isclosed() const noexcept {
		return lubit && uubit;
	}
	constexpr bool isopenlower() const noexcept {
		return lubit;
	}
	constexpr bool isopenupper() const noexcept {
		return uubit;
	}
	constexpr bool getlb(sw::universal::posit<nbits, es>& _lb) const noexcept {
		_lb = lb;
		return lubit;
	}
	constexpr bool getub(sw::universal::posit<nbits, es>& _ub) const noexcept {
		_ub = ub;
		return uubit;
	}

	// modifiers

	// TODO: do we clear to exact 0, or [-inf, inf]?
	constexpr void clear() noexcept {
		lb.clear();
		ub.clear();
		lubit = true;
		uubit = true;
	}
	constexpr void setinclusive() noexcept {
		lb.setnar();
		ub.setnar();
		lubit = true;
		uubit = true;
	}
	constexpr void setlb(sw::universal::posit<nbits, es>& _lb, bool ubit) noexcept {
		lb = _lb;
		lubit = ubit;
	}
	constexpr void setub(sw::universal::posit<nbits, es>& _ub, bool ubit) noexcept {
		ub = _ub;
		uubit = ubit;
	}
	constexpr void setbits(uint64_t v) noexcept { // API to be consistent with the other number systems
		lb.setbits(v & 0xFFFFFFFFul);
		ub.setbits((v >> 32) & 0xFFFFFFFFul);
		lubit = false;
		uubit = false;
	}

	// relative_order returns -1 if v was rounded up, 0 if it was exact, and 1 if v was rounded down
	template <unsigned NrFractionBits>
	inline int relative_order(const internal::value<NrFractionBits>& v) {
		if (v.iszero()) {
			return 0;
		}
		if (v.isnan() || v.isinf()) {
			return 0;
		}
		return convert(v.sign(), v.scale(), v.fraction());
	}

private:
	// member variables
	sw::universal::posit<nbits, es> lb, ub;  // lower_bound and upper_bound of the tile
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
	template<unsigned input_fbits>
	int convert(bool sign, int scale, bitblock<input_fbits> input_fraction) {
		// construct the posit
		//int k = calculate_unconstrained_k<nbits, es>(scale);
		// interpolation rule checks
		if (check_inward_projection_range(scale)) {    // regime dominated
			return 1; // rounded down
		}
		else {
			const unsigned pt_len = nbits + 3 + es;
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
			unsigned nf = (unsigned)std::max<int>(0, (nbits + 1) - (2ull + run + es));
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

			unsigned len = 1 + std::max<unsigned>((nbits + 1), (2ull + run + es));
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
	template<unsigned nnbits, unsigned ees>
	friend std::ostream& operator<< (std::ostream& ostr, const valid<nnbits, ees>& p);
	template<unsigned nnbits, unsigned ees>
	friend std::istream& operator>> (std::istream& istr, valid<nnbits, ees>& p);

	template<unsigned nnbits, unsigned ees>
	friend constexpr bool operator==(const valid<nnbits, ees>& lhs, const valid<nnbits, ees>& rhs) noexcept;
	template<unsigned nnbits, unsigned ees>
	friend constexpr bool operator!=(const valid<nnbits, ees>& lhs, const valid<nnbits, ees>& rhs) noexcept;
	template<unsigned nnbits, unsigned ees>
	friend constexpr bool operator< (const valid<nnbits, ees>& lhs, const valid<nnbits, ees>& rhs) noexcept;
	template<unsigned nnbits, unsigned ees>
	friend constexpr bool operator> (const valid<nnbits, ees>& lhs, const valid<nnbits, ees>& rhs) noexcept;
	template<unsigned nnbits, unsigned ees>
	friend constexpr bool operator<=(const valid<nnbits, ees>& lhs, const valid<nnbits, ees>& rhs) noexcept;
	template<unsigned nnbits, unsigned ees>
	friend constexpr bool operator>=(const valid<nnbits, ees>& lhs, const valid<nnbits, ees>& rhs) noexcept;
};


// VALID operators
template<unsigned nbits, unsigned es>
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

template<unsigned nbits, unsigned es>
inline std::istream& operator>> (std::istream& istr, const valid<nbits, es>& v) {
	istr >> v._Bits;
	return istr;
}

// valid - logic operators
template<unsigned nnbits, unsigned ees>
inline constexpr bool operator==(const valid<nnbits, ees>& lhs, const valid<nnbits, ees>& rhs) noexcept { (void)lhs; (void)rhs; return false; }
template<unsigned nnbits, unsigned ees>
inline constexpr bool operator!=(const valid<nnbits, ees>& lhs, const valid<nnbits, ees>& rhs) noexcept { return !operator==(lhs, rhs); }
template<unsigned nnbits, unsigned ees>
inline constexpr bool operator< (const valid<nnbits, ees>& lhs, const valid<nnbits, ees>& rhs) noexcept { (void)lhs; (void)rhs; return false; }
template<unsigned nnbits, unsigned ees>
inline constexpr bool operator> (const valid<nnbits, ees>& lhs, const valid<nnbits, ees>& rhs) noexcept { return  operator< (rhs, lhs); }
template<unsigned nnbits, unsigned ees>
inline constexpr bool operator<=(const valid<nnbits, ees>& lhs, const valid<nnbits, ees>& rhs) noexcept { return !operator> (lhs, rhs); }
template<unsigned nnbits, unsigned ees>
inline constexpr bool operator>=(const valid<nnbits, ees>& lhs, const valid<nnbits, ees>& rhs) noexcept { return !operator< (lhs, rhs); }

// valid - literal logic operators
template<unsigned nnbits, unsigned ees>
inline CONSTEXPRESSION bool operator==(const valid<nnbits, ees>& lhs, double rhs) { return lhs == valid<nnbits, ees>(rhs); }
template<unsigned nnbits, unsigned ees>
inline CONSTEXPRESSION bool operator!=(const valid<nnbits, ees>& lhs, double rhs) { return !operator==(lhs, rhs); }
template<unsigned nnbits, unsigned ees>
inline CONSTEXPRESSION bool operator< (const valid<nnbits, ees>& lhs, double rhs) { return lhs < valid<nnbits, ees>(rhs); }
template<unsigned nnbits, unsigned ees>
inline CONSTEXPRESSION bool operator> (const valid<nnbits, ees>& lhs, double rhs) { return  operator< (rhs, lhs); }
template<unsigned nnbits, unsigned ees>
inline CONSTEXPRESSION bool operator<=(const valid<nnbits, ees>& lhs, double rhs) { return !operator> (lhs, rhs); }
template<unsigned nnbits, unsigned ees>
inline CONSTEXPRESSION bool operator>=(const valid<nnbits, ees>& lhs, double rhs) { return !operator< (lhs, rhs); }

// valid - valid binary arithmetic operators
// BINARY ADDITION
template<unsigned nbits, unsigned es>
inline constexpr valid<nbits, es> operator+(const valid<nbits, es>& lhs, const valid<nbits, es>& rhs) noexcept {
	valid<nbits, es> sum(lhs);
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<unsigned nbits, unsigned es>
inline constexpr valid<nbits, es> operator-(const valid<nbits, es>& lhs, const valid<nbits, es>& rhs) noexcept {
	valid<nbits, es> diff(lhs);
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<unsigned nbits, unsigned es>
inline constexpr valid<nbits, es> operator*(const valid<nbits, es>& lhs, const valid<nbits, es>& rhs) noexcept {
	valid<nbits, es> mul(lhs);
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<unsigned nbits, unsigned es>
inline constexpr valid<nbits, es> operator/(const valid<nbits, es>& lhs, const valid<nbits, es>& rhs) noexcept {
	valid<nbits, es> ratio(lhs);
	ratio /= rhs;
	return ratio;
}

}} // namespace sw::universal
