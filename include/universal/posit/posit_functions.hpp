#pragma once
// posit_functions.hpp: simple math functions
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
namespace unum {

	// functions to provide details about
	// the properties of a posit configuration

	// calculate exponential scale of useed
	template<size_t nbits, size_t es>
	int useed_scale() {
		return (uint32_t(1) << es);
	}

	// calculate exponential scale of maxpos
	template<size_t nbits, size_t es>
	int maxpos_scale() {
		return (nbits - 2) * (1 << es);
	}

	// calculate exponential scale of minpos
	template<size_t nbits, size_t es>
	int minpos_scale() {
		return static_cast<int>(2 - int(nbits)) * (1 << es);
	}

	// calculate the constrained k value
	template<size_t nbits, size_t es>
	int calculate_k(int scale) {
		// constrain the scale to range [minpos, maxpos]
		if (scale < 0) {
			scale = scale > minpos_scale<nbits, es>() ? scale : minpos_scale<nbits, es>();
		}
		else {
			scale = scale < maxpos_scale<nbits, es>() ? scale : maxpos_scale<nbits, es>();
		}
		// bad int k = scale < 0 ? -(-scale >> es) - 1 : (scale >> es);
		// the scale of a posit is  2 ^ scale = useed ^ k * 2 ^ exp
		// -> (scale >> es) = (k*2^es + exp) >> es
		// -> (scale >> es) = k + (exp >> es) -> k = (scale >> es)
		int k = scale < 0 ? -(-scale >> es) : (scale >> es);
		if (k == 0 && scale < 0) {
			// project back to south-east quadrant
			k = -1;
		}
		return k;
	}

	// calculate the unconstrained k value
	template<size_t nbits, size_t es>
	int calculate_unconstrained_k(int scale) {
		// the scale of a posit is  2 ^ scale = useed ^ k * 2 ^ exp
		// -> (scale >> es) = (k*2^es + exp) >> es
		// -> (scale >> es) = k + (exp >> es) 
		// -> k = (scale >> es)
		int k = scale < 0 ? -(-scale >> es) : (scale >> es);
		if (k == 0 && scale < 0) {
			// project back to south-east quadrant
			k = -1;
		}
		return k;
	}

	// double value representation of the useed value of a posit<nbits, es>
	template<size_t nbits, size_t es>
	double useed() {
		return std::pow(2.0, std::pow(2.0, es));
	}

	// calculate the value of useed
	template<size_t nbits, size_t es>
	double useed_value() {
		return double(uint64_t(1) << useed_scale<nbits, es>());
	}

	// calculate the value of maxpos
	template<size_t nbits, size_t es>
	long double maxpos_value() {
		return std::pow((long double)(useed_value<nbits, es>()), (long double)(nbits - 2));
	}

	// calculate the value of minpos
	template<size_t nbits, size_t es>
	long double minpos_value() {
		return std::pow((long double)(useed_value<nbits, es>()), (long double)(static_cast<int>(2 - int(nbits))));
	}

	// generate the minpos bit pattern for the sign requested (true is negative half, false is positive half)
	template<size_t nbits, size_t es>
	bitblock<nbits> minpos_pattern(bool sign = false) {
		bitblock<nbits> _bits;
		_bits.reset();
		_bits.set(0, true);
		return (sign ? twos_complement(_bits) : _bits);
	}

	// generate the maxpos bit pattern for the sign requested (true is negative half, false is positive half)
	template<size_t nbits, size_t es>
	bitblock<nbits> maxpos_pattern(bool sign = false) {
		bitblock<nbits> _bits;
		_bits.reset();
		_bits.flip();
		_bits.set(nbits - 1, false);
		return (sign ? twos_complement(_bits) : _bits);
	}

	// forward reference
	template<size_t nbits, size_t es> class posit;

	template<size_t nbits, size_t es>
	inline int sign_value(const posit<nbits, es>& p) {
		bitblock<nbits> _bits = p.get();
		return (_bits[nbits - 1] ? -1 : 1);
	}

	template<size_t nbits, size_t es>
	inline long double regime_value(const posit<nbits, es>& p) {
		regime<nbits, es>    _regime;
		bitblock<nbits> tmp(p.get());
		tmp = sign(p) ? twos_complement(tmp) : tmp;
		_regime.assign_regime_pattern(decode_regime(tmp));
		return _regime.value();
	}

	template<size_t nbits, size_t es>
	inline long double exponent_value(const posit<nbits, es>& p) {
		regime<nbits, es>    _regime;
		exponent<nbits, es>  _exponent;
		bitblock<nbits> tmp(p.get());
		tmp = sign(p) ? twos_complement(tmp) : tmp;
		size_t nrRegimeBits = _regime.assign_regime_pattern(decode_regime(tmp));	// get the regime bits
		_exponent.extract_exponent_bits(tmp, nrRegimeBits);							// get the exponent bits
		return _exponent.value();
	}

	template<size_t nbits, size_t es>
	inline long double fraction_value(const posit<nbits, es>& p) {
		constexpr size_t fbits = nbits - 3 - es;
		bool		     	 _sign;
		regime<nbits, es>    _regime;
		exponent<nbits, es>  _exponent;
		fraction<fbits>      _fraction;
		decode(p.get(), _sign, _regime, _exponent, _fraction);
		return _fraction.value();
	}

	// get the sign of the posit
	template<size_t nbits, size_t es>
	inline bool sign(const posit<nbits, es>& p) {
		return p.isneg();
	}

	// calculate the scale of a posit
	template<size_t nbits, size_t es>
	inline int scale(const posit<nbits, es>& p) {
		regime<nbits, es>    _regime;
		exponent<nbits, es>  _exponent;
		bitblock<nbits> tmp(p.get());
		tmp = sign(p) ? twos_complement(tmp) : tmp;
		size_t nrRegimeBits = _regime.assign_regime_pattern(decode_regime(tmp));	// get the regime bits
		_exponent.extract_exponent_bits(tmp, nrRegimeBits);							// get the exponent bits
		// return the scale
		return _regime.scale() + _exponent.scale();
	}

	// calculate the significant of a posit
	template<size_t nbits, size_t es, size_t fbits>
	inline bitblock<fbits+1> significant(const posit<nbits, es>& p) {
		//constexpr size_t fbits = nbits - 3 - es;
		bool		     	 _sign;
		regime<nbits, es>    _regime;
		exponent<nbits, es>  _exponent;
		fraction<fbits>      _fraction;
		decode(p.get(), _sign, _regime, _exponent, _fraction);
		return _fraction.get_fixed_point();
	}

	// get the fraction bits of a posit
	template<size_t nbits, size_t es, size_t fbits>
	inline bitblock<fbits> extract_fraction(const posit<nbits, es>& p) {
		//constexpr size_t fbits = nbits - 3 - es;
		bool		     	 _sign;
		regime<nbits, es>    _regime;
		exponent<nbits, es>  _exponent;
		fraction<fbits>      _fraction;
		decode(p.get(), _sign, _regime, _exponent, _fraction);
		return _fraction.get();
	}

	// calculate the scale of the regime component of the posit
	template<size_t nbits, size_t es>
	inline int regime_scale(const posit<nbits, es>& p) {
		regime<nbits, es>    _regime;
		bitblock<nbits> tmp(p.get());
		tmp = sign(p) ? twos_complement(tmp) : tmp;
		_regime.assign_regime_pattern(decode_regime(tmp));
		return _regime.scale();
	}

	// calculate the scale of the exponent component of the posit
	template<size_t nbits, size_t es>
	inline int exponent_scale(const posit<nbits, es>& p) {
		regime<nbits, es>    _regime;
		exponent<nbits, es>  _exponent;
		bitblock<nbits> tmp(p.get());
		tmp = sign(p) ? twos_complement(tmp) : tmp;
		size_t nrRegimeBits = _regime.assign_regime_pattern(decode_regime(tmp));
		_exponent.extract_exponent_bits(tmp, nrRegimeBits);
		return _exponent.scale();
	}

	// obtain the decoded posit bits
	template<size_t nbits, size_t es>
	inline bitblock<nbits> decoded(const posit<nbits, es>& p) {
		constexpr size_t rbits = nbits - 1;
		constexpr size_t fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
		bool		     	 _sign;
		regime<nbits, es>    _regime;
		exponent<nbits, es>  _exponent;
		fraction<fbits>      _fraction;
		decode(p.get(), _sign, _regime, _exponent, _fraction);

		bitblock<rbits> r				= _regime.get();
		size_t			nrRegimeBits	= _regime.nrBits();
		bitblock<es>	e				= _exponent.get();
		size_t			nrExponentBits	= _exponent.nrBits();
		bitblock<fbits> f				= _fraction.get();
		size_t			nrFractionBits	= _fraction.nrBits();

		bitblock<nbits> _Bits;
		_Bits.set(nbits - 1, _sign);
		int msb = nbits - 2;
		for (size_t i = 0; i < nrRegimeBits; i++) {
			_Bits.set(std::size_t(msb--), r[nbits - 2 - i]);
		}
		if (msb < 0) 
					return _Bits;
		for (size_t i = 0; i < nrExponentBits && msb >= 0; i++) {
			_Bits.set(std::size_t(msb--), e[es - 1 - i]);
		}
		if (msb < 0) return _Bits;
		for (size_t i = 0; i < nrFractionBits && msb >= 0; i++) {
			_Bits.set(std::size_t(msb--), f[fbits - 1 - i]);
		}
		return _Bits;
	}

	//////////////////////////////////////////////////////////////////////////

	// calculate the integer power a ^ b
	// exponentiation by squaring is the standard method for modular exponentiation of large numbers in asymmetric cryptography
	template<size_t nbits, size_t es>
	posit<nbits, es> ipow(const posit<nbits, es>& a, const posit<nbits, es>& b) {
		// precondition
		if (!a.isinteger() || !b.isinteger()) return posit<nbits, es>(0);

		uint64_t result(1);
		uint64_t base = uint64_t(a); 
		uint64_t exp = uint64_t(b);
		for (;;) {
			if (exp & 0x1) result *= base;
			exp >>= 1;
			if (exp == 0) break;
			base *= base;
		}
		return posit<nbits,es>(result);
	}


	static double huge = 1.0e300;

	// TBD, defined to compile, not functionally correct
#define __HI(x) x
#define __LO(x) x
	// algorithm for floor(double)
	template<size_t nbits, size_t es>
	posit<nbits,es> floor(posit<nbits, es> x)	{
		int i0, i1, j0;
		unsigned i, j;
		i0 = __HI(x);
		i1 = __LO(x);
		j0 = ((i0 >> 20) & 0x7ff) - 0x3ff;
		if (j0 < 20) {
			if (j0 < 0) { 	/* raise inexact if x != 0 */
				if (huge + x > 0.0) {/* return 0*sign(x) if |x|<1 */
					if (i0 >= 0) { i0 = i1 = 0; }
					else if (((i0 & 0x7fffffff) | i1) != 0)
					{
						i0 = 0xbff00000; i1 = 0;
					}
				}
			}
			else {
				i = (0x000fffff) >> j0;
				if (((i0&i) | i1) == 0) return x; /* x is integral */
				if (huge + x > 0.0) {	/* raise inexact flag */
					if (i0 < 0) i0 += (0x00100000) >> j0;
					i0 &= (~i); i1 = 0;
				}
			}
		}
		else if (j0 > 51) {
			if (j0 == 0x400) return x + x;	/* inf or NaN */
			else return x;		/* x is integral */
		}
		else {
			i = ((unsigned)(0xffffffff)) >> (j0 - 20);
			if ((i1&i) == 0) return x;	/* x is integral */
			if (huge + x > 0.0) { 		/* raise inexact flag */
				if (i0 < 0) {
					if (j0 == 20) i0 += 1;
					else {
						j = i1 + (1 << (52 - j0));
						if (j < i1) i0 += 1; 	/* got a carry */
						i1 = j;
					}
				}
				i1 &= (~i);
			}
		}
		__HI(x) = i0;
		__LO(x) = i1;
		return x;
	}

} // namespace unum
} // namespace sw
