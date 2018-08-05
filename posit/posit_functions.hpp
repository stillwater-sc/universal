#pragma once

// posit_functions.hpp: simple math functions
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
	namespace unum {

		// universal information functions to provide details regarding the properties of a posit configuration

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

		// this comparison is for a two's complement number only, for example, the raw bits of a posit
		template<size_t nbits>
		bool lessThan(const bitblock<nbits>& lhs, const bitblock<nbits>& rhs) {
			// comparison of the sign bit
			if (lhs[nbits - 1] == 0 && rhs[nbits - 1] == 1)	return false;
			if (lhs[nbits - 1] == 1 && rhs[nbits - 1] == 0) return true;
			// sign is equal, compare the remaining bits
			for (int i = nbits - 2; i >= 0; --i) {
				if (lhs[i] == 0 && rhs[i] == 1)	return true;
				if (lhs[i] == 1 && rhs[i] == 0) return false;
			}
			// numbers are equal
			return false;
		}

		// special case check for projecting values between (0, minpos] to minpos and [maxpos, inf) to maxpos
		// Returns true if the scale is too small or too large for this posit config
		// DO NOT USE the k value for this, as the k value encodes the useed regions
		// and thus is too coarse to make this decision.
		// Using the scale directly is the simplest expression of the inward projection test.
		template<size_t nbits, size_t es>
		bool check_inward_projection_range(int scale) {
			// calculate the min/max k factor for this posit config
			int posit_size = nbits;
			int k = scale < 0 ? -(posit_size - 2) : (posit_size - 2);
			return scale < 0 ? scale < k*(1 << es) : scale > k*(1 << es);
		}

		// decode_regime measures the run-length of the regime and returns the k value associated with that run-length
		template<size_t nbits>
		int decode_regime(bitblock<nbits>& raw_bits) {
			// let m be the number of identical bits in the regime
			int m = 0;   // regime runlength counter
			int k = 0;   // converted regime scale
			if (raw_bits[nbits - 2] == 1) {   // run length of 1's
				m = 1;   // if a run of 1's k = m - 1
				int start = (nbits == 2 ? nbits - 2 : nbits - 3);
				for (int i = start; i >= 0; --i) {
					if (raw_bits[i] == 1) {
						m++;
					}
					else {
						break;
					}
				}
				k = m - 1;
			}
			else {
				m = 1;  // if a run of 0's k = -m
				int start = (nbits == 2 ? nbits - 2 : nbits - 3);
				for (int i = start; i >= 0; --i) {
					if (raw_bits[i] == 0) {
						m++;
					}
					else {
						break;
					}
				}
				k = -m;
			}
			return k;
		}
	
		// extract_fields takes a raw posit encoding and extracts the sign, regime, exponent, and fraction components
		template<size_t nbits, size_t es, size_t fbits>
		void extract_fields(const bitblock<nbits>& raw_bits, bool& _sign, regime<nbits,es>& _regime, exponent<nbits,es>& _exponent, fraction<fbits>& _fraction) {
			bitblock<nbits> tmp(raw_bits);
			if (_sign) tmp = twos_complement(tmp);
			size_t nrRegimeBits = _regime.assign_regime_pattern(decode_regime(tmp));

			// get the exponent bits
			// start of exponent is nbits - (sign_bit + regime_bits)
			int msb = int(static_cast<int>(nbits) - 1 - (1 + nrRegimeBits));
			size_t nrExponentBits = 0;
			if (es > 0) {
				bitblock<es> _exp;
				if (msb >= 0 && es > 0) {
					nrExponentBits = (msb >= static_cast<int>(es) - 1 ? es : msb + 1);
					for (size_t i = 0; i < nrExponentBits; i++) {
						_exp[es - 1 - i] = tmp[msb - i];
					}
				}
				_exponent.set(_exp, nrExponentBits);
			}

			// finally, set the fraction bits
			// we do this so that the fraction is right extended with 0;
			// The max fraction is <nbits - 3 - es>, but we are setting it to <nbits - 3> and right-extent
			// The msb bit of the fraction represents 2^-1, the next 2^-2, etc.
			// If the fraction is empty, we have a fraction of nbits-3 0 bits
			// If the fraction is one bit, we have still have fraction of nbits-3, with the msb representing 2^-1, and the rest are right extended 0's
			bitblock<fbits> _frac;
			msb = msb - int(nrExponentBits);
			size_t nrFractionBits = (msb < 0 ? 0 : msb + 1);
			if (msb >= 0) {
				for (int i = msb; i >= 0; --i) {
					_frac[fbits - 1 - (msb - i)] = tmp[i];
				}
			}
			_fraction.set(_frac, nrFractionBits);
		}

		// decode takes the raw bits representing a posit coming from memory
		// and decodes the sign, regime, the exponent, and the fraction.
		// This function has the functionality of the posit register-file load.
		template<size_t nbits, size_t es, size_t fbits>
		void decode(const bitblock<nbits>& raw_bits, bool& _sign, regime<nbits, es>& _regime, exponent<nbits, es>& _exponent, fraction<fbits>& _fraction) {
			//_raw_bits = raw_bits;	// store the raw bits for reference
			// check special cases
			_sign = raw_bits.test(nbits - 1);
			if (_sign) {
				std::bitset<nbits> tmp(raw_bits);
				tmp.reset(nbits - 1);
				if (tmp.none()) {
					// setToNaR();   special case = NaR (Not a Real)
					_sign = true;
					_regime.setToInfinite();
					_exponent.reset();
				}
				else {
					extract_fields(raw_bits, _sign, _regime, _exponent, _fraction);
				}
			}
			else {
				if (raw_bits.none()) {  
					// setToZero();  special case = 0
					_sign = false;
					_regime.setToZero();
					_exponent.reset();
					_fraction.reset();
				}
				else {
					extract_fields(raw_bits, _sign, _regime, _exponent, _fraction);
				}
			}
			//if (_trace_decode) std::cout << "raw bits: " << raw_bits << " posit bits: " << (_sign ? "1|" : "0|") << _regime << "|" << _exponent << "|" << _fraction << " posit value: " << *this << std::endl;
			if (_trace_decode) std::cout << "raw bits: " << raw_bits << " posit bits: " << (_sign ? "1|" : "0|") << _regime << "|" << _exponent << "|" << _fraction << std::endl;

			// we are storing both the raw bit representation and the decoded form
			// so no need to transform back via 2's complement of regime/exponent/fraction
		}

		// forward reference
		template<size_t nbits, size_t es> class posit;

		template<size_t nbits, size_t es>
		inline int sign_value(const posit<nbits, es>& p) {
			bitblock<nbits> _bits = p.get();
			return (_bits[nbits - 1] ? -1 : 1);
		}

		template<size_t nbits, size_t es>
		inline double regime_value(const posit<nbits, es>& p) {
			constexpr size_t fbits = nbits - 3 - es;
			bool		     	 _sign;
			regime<nbits, es>    _regime;
			exponent<nbits, es>  _exponent;
			fraction<fbits>      _fraction;
			decode(p.get(), _sign, _regime, _exponent, _fraction);
			return _regime.value();
		}

		template<size_t nbits, size_t es>
		inline double exponent_value(const posit<nbits, es>& p) {
			constexpr size_t fbits = nbits - 3 - es;
			bool		     	 _sign;
			regime<nbits, es>    _regime;
			exponent<nbits, es>  _exponent;
			fraction<fbits>      _fraction;
			decode(p.get(), _sign, _regime, _exponent, _fraction);
			return _exponent.value();
		}

		template<size_t nbits, size_t es>
		inline double fraction_value(const posit<nbits, es>& p) {
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
			return p.isNegative();
		}

		// calculate the scale of a posit
		template<size_t nbits, size_t es>
		inline int scale(const posit<nbits, es>& p) {
			constexpr size_t fbits = nbits - 3 - es;
			bool		     	 _sign;
			regime<nbits, es>    _regime;
			exponent<nbits, es>  _exponent;
			fraction<fbits>      _fraction;
			decode(p.get(), _sign, _regime, _exponent, _fraction);
			return _regime.scale() + _exponent.scale();
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
			constexpr size_t fbits = nbits - 3 - es;
			bool		     	 _sign;
			regime<nbits, es>    _regime;
			exponent<nbits, es>  _exponent;
			fraction<fbits>      _fraction;
			decode(p.get(), _sign, _regime, _exponent, _fraction);
			return _regime.scale();
		}

		// calculate the scale of the exponent component of the posit
		template<size_t nbits, size_t es>
		inline int exponent_scale(const posit<nbits, es>& p) {
			constexpr size_t fbits = nbits - 3 - es;
			bool		     	 _sign;
			regime<nbits, es>    _regime;
			exponent<nbits, es>  _exponent;
			fraction<fbits>      _fraction;
			decode(p.get(), _sign, _regime, _exponent, _fraction);
			return _exponent.scale();
		}

	}
}
