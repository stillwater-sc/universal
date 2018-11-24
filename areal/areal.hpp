#pragma once
// areal.hpp: definition of a variable float representation that mimics the posit configuration
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <limits>
#include <bitset>

#include "exponent.hpp"
#include "fraction.hpp"
#include "value.hpp"

namespace sw {
	namespace unum {
		
		// Forward definitions
		template<size_t nbits, size_t es> class areal;
		template<size_t nbits, size_t es> areal<nbits,es> abs(const areal<nbits,es>& v);

		template<size_t nbits, size_t es>
		void extract_fields(const bitblock<nbits>& raw_bits, bool& _sign, exponent<nbits, es>& _exponent, fraction<nbits - es -1>& _fraction) {
			constexpr size_t fbits = nbits - es - 1;
			_sign = raw_bits[nbits - 1];
			bitblock<es> bits;
			for (size_t i = nbits - 2; i > nbits - es - 1; --i) bits[i - nbits - es] = raw_bits[i];
			_exponent.set(bits, es);
			bitblock<fbits> fraction_bits;
			int f = fbits - 1;
			for (size_t i = nbits - es - 2; f >= 0; --i, --f) fraction_bits[f] = raw_bits[i];
			_fraction.set(fraction_bits, fbits);
		}

		// decode takes the raw bits representing an arbitrary real coming from memory
		// and decodes the sign, the exponent, and the fraction.
		// This function has the functionality of the real(float) register-file load.
		template<size_t nbits, size_t es>
		void decode(const bitblock<nbits>& raw_bits, bool& _sign, exponent<nbits, es>& _exponent, fraction<nbits - es - 1>& _fraction) {
			// check special cases
			_sign = raw_bits.test(nbits - 1);
			if (_sign) {
				std::bitset<nbits> tmp(raw_bits);
				tmp.reset(nbits - 1);
				if (tmp.none()) {
					// setnan();   special case = NaR (Not a Real)
					_sign = true;
					_exponent.reset();
				}
				else {
					extract_fields(raw_bits, _sign, _exponent, _fraction);
				}
			}
			else {
				if (raw_bits.none()) {
					// setzero();  special case = 0
					_sign = false;
					_exponent.reset();
					_fraction.reset();
				}
				else {
					extract_fields(raw_bits, _sign, _exponent, _fraction);
				}
			}
			if (_trace_decode) std::cout << "raw bits: " << raw_bits << " posit bits: " << (_sign ? "1|" : "0|") << "|" << _exponent << "|" << _fraction << std::endl;

			// we are storing both the raw bit representation and the decoded form
			// so no need to transform back via 2's complement of regime/exponent/fraction
		}

		// needed to avoid double rounding situations: TODO: does that mean the condensed version above should be removed?
		template<size_t nbits, size_t es, size_t fbits>
		inline areal<nbits, es>& convert_(bool _sign, int _scale, const bitblock<fbits>& fraction_in, areal<nbits, es>& r) {
			if (_trace_conversion) std::cout << "------------------- CONVERT ------------------" << std::endl;
			if (_trace_conversion) std::cout << "sign " << (_sign ? "-1 " : " 1 ") << "scale " << std::setw(3) << _scale << " fraction " << fraction_in << std::endl;

			r.reset();

			return r;
		}

		// convert a floating point value to a specific areal configuration. Semantically, p = v, return reference to p
		template<size_t nbits, size_t es>
		inline areal<nbits, es>& convert(const value<nbits - es - 1>& v, areal<nbits, es>& p) {
			constexpr size_t fbits = nbits - es - 1;
			if (_trace_conversion) std::cout << "------------------- CONVERT ------------------" << std::endl;
			if (_trace_conversion) std::cout << "sign " << (v.sign() ? "-1 " : " 1 ") << "scale " << std::setw(3) << v.scale() << " fraction " << v.fraction() << std::endl;

			if (v.iszero()) {
				p.setzero();
				return p;
			}
			if (v.isnan() || v.isinf()) {
				p.setnan();
				return p;
			}
			return convert_<nbits, es, fbits>(v.sign(), v.scale(), v.fraction(), p);
		}

		// template class representing a value in scientific notation, using a template size for the number of fraction bits
		template<size_t nbits, size_t es>
		class areal {
		public:
			static constexpr size_t fbits  = nbits - 1 - es;    // number of fraction bits excluding the hidden bit
			static constexpr size_t fhbits = fbits + 1;         // number of fraction bits including the hidden bit
			static constexpr size_t abits = fhbits + 3;         // size of the addend
			static constexpr size_t mbits = 2 * fhbits;         // size of the multiplier output
			static constexpr size_t divbits = 3 * fhbits + 4;   // size of the divider output

			areal() : _sign(false), _scale(0), _nrOfBits(fbits), _inf(false), _zero(true), _nan(false) {}
			areal(bool sign, int scale, const bitblock<fbits>& fraction_without_hidden_bit, bool zero = true, bool inf = false) : _sign(sign), _scale(scale), _nrOfBits(fbits), _fraction(fraction_without_hidden_bit), _inf(inf), _zero(zero), _nan(false) {}
			areal(signed char initial_value) {
				*this = initial_value;
			}
			areal(short initial_value) {
				*this = initial_value;
			}
			areal(int initial_value) {
				*this = initial_value;
			}
			areal(long long initial_value) {
				*this = initial_value;
			}
			areal(unsigned long long initial_value) {
				*this = initial_value;
			}
			areal(float initial_value) {
				*this = initial_value;
			}
			areal(double initial_value) {
				*this = initial_value;
			}
			areal(long double initial_value) {
				*this = initial_value;
			}
			areal(const areal& rhs) {
				*this = rhs;
			}
			areal& operator=(const areal& rhs) {
				_sign	  = rhs._sign;
				_scale	  = rhs._scale;
				_fraction = rhs._fraction;
				_raw_bits = rhs._raw_bits ;
				_nrOfBits = rhs._nrOfBits;
				_inf      = rhs._inf;
				_zero     = rhs._zero;
				_nan      = rhs._nan;
				return *this;
			}
			areal& operator=(signed char rhs) {
				*this = (long long)(rhs);
				return *this;
			}
			areal& operator=(short rhs) {
				*this = (long long)(rhs);
				return *this;
			}
			areal& operator=(int rhs) {
				*this = (long long)(rhs);
				return *this;
			}
			areal& operator=(long long rhs) {
				if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;
				if (rhs == 0) {
					setzero();
					return *this;
				}
				reset();
				_sign = (0x8000000000000000 & rhs);  // 1 is negative, 0 is positive
				if (_sign) {
					// process negative number: process 2's complement of the input
					_scale = findMostSignificantBit(-rhs) - 1;
					uint64_t _fraction_without_hidden_bit = (-rhs << (64 - _scale));
					_fraction = copy_integer_fraction<fbits>(_fraction_without_hidden_bit);
					//take_2s_complement();
					_nrOfBits = fbits;
					if (_trace_conversion) std::cout << "int64 " << rhs << " sign " << _sign << " scale " << _scale << " fraction b" << _fraction << std::dec << std::endl;
				}
				else {
					// process positive number
					if (rhs != 0) {
						_scale = findMostSignificantBit(rhs) - 1;
						uint64_t _fraction_without_hidden_bit = (rhs << (64 - _scale));
						_fraction = copy_integer_fraction<fbits>(_fraction_without_hidden_bit);
						_nrOfBits = fbits;
						if (_trace_conversion) std::cout << "int64 " << rhs << " sign " << _sign << " scale " << _scale << " fraction b" << _fraction << std::dec << std::endl;
					}
				}
				return *this;
			}
			areal& operator=(unsigned long long rhs) {
				if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;
				if (rhs == 0) {
					setzero();
				}
				else {
					reset();
					_scale = findMostSignificantBit(rhs) - 1;
					uint64_t _fraction_without_hidden_bit = (rhs << (64 - _scale));
					_fraction = copy_integer_fraction<fbits>(_fraction_without_hidden_bit);
					_nrOfBits = fbits;
				}
				if (_trace_conversion) std::cout << "uint64 " << rhs << " sign " << _sign << " scale " << _scale << " fraction b" << _fraction << std::dec << std::endl;
				return *this;
			}
			areal& operator=(float rhs) {
				reset();
				if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;

				switch (std::fpclassify(rhs)) {
				case FP_ZERO:
					_nrOfBits = fbits;
					_zero = true;
					break;
				case FP_INFINITE:
					_inf  = true;
					break;
				case FP_NAN:
					_nan = true;
					break;
				case FP_SUBNORMAL:
					std::cerr << "TODO: subnormal number: returning 0" << std::endl;
					break;
				case FP_NORMAL:
				{
					float _fr;
					unsigned int _23b_fraction_without_hidden_bit;
					int _exponent;
					extract_fp_components(rhs, _sign, _exponent, _fr, _23b_fraction_without_hidden_bit);
					_scale = _exponent - 1;
					_fraction = extract_23b_fraction<fbits>(_23b_fraction_without_hidden_bit);
					_nrOfBits = fbits;
					if (_trace_conversion) std::cout << "float " << rhs << " sign " << _sign << " scale " << _scale << " 23b fraction 0x" << std::hex << _23b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;
				}
				break;
				}
				return *this;
			}
			areal& operator=(double rhs) {
				reset();
				if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;

				switch (std::fpclassify(rhs)) {
				case FP_ZERO:
					_nrOfBits = fbits;
					_zero = true;
					break;
				case FP_INFINITE:
					_inf = true;
					break;
				case FP_NAN:
					_nan = true;
					break;
				case FP_SUBNORMAL:
					std::cerr << "TODO: subnormal number: returning 0" << std::endl;
					break;
				case FP_NORMAL:
				{
					double _fr;
					unsigned long long _52b_fraction_without_hidden_bit;
					int _exponent;
					extract_fp_components(rhs, _sign, _exponent, _fr, _52b_fraction_without_hidden_bit);
					_scale = _exponent - 1;
					_fraction = extract_52b_fraction<fbits>(_52b_fraction_without_hidden_bit);
					_nrOfBits = fbits;
					if (_trace_conversion) std::cout << "double " << rhs << " sign " << _sign << " scale " << _scale << " 52b fraction 0x" << std::hex << _52b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;
				}
				break;
				}
				return *this;
			}
			areal& operator=(long double rhs) {
				reset();
				if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;

				switch (std::fpclassify(rhs)) {
				case FP_ZERO:
					_nrOfBits = fbits;
					_zero = true;
					break;
				case FP_INFINITE:
					_inf = true;
					break;
				case FP_NAN:
					_nan = true;
					break;
				case FP_SUBNORMAL:
					std::cerr << "TODO: subnormal number: returning 0" << std::endl;
					break;
				case FP_NORMAL:
				{
					long double _fr;
					unsigned long long _63b_fraction_without_hidden_bit;
					int _exponent;
					extract_fp_components(rhs, _sign, _exponent, _fr, _63b_fraction_without_hidden_bit);
					_scale = _exponent - 1;
					// how to interpret the fraction bits: TODO: this should be a static compile-time code block
					if (sizeof(long double) == 8) {
						// we are just a double and thus only have 52bits of fraction
						_fraction = extract_52b_fraction<fbits>(_63b_fraction_without_hidden_bit);
						if (_trace_conversion) std::cout << "long double " << rhs << " sign " << _sign << " scale " << _scale << " 52b fraction 0x" << std::hex << _63b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;

					}
					else if (sizeof(long double) == 16) {
						// how to differentiate between 80bit and 128bit formats?
						_fraction = extract_63b_fraction<fbits>(_63b_fraction_without_hidden_bit);
						if (_trace_conversion) std::cout << "long double " << rhs << " sign " << _sign << " scale " << _scale << " 63b fraction 0x" << std::hex << _63b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;

					}
					_nrOfBits = fbits;
				}
				break;
				}
				return *this;
			}

			// operators
			// prefix operator
			areal operator-() const {				
				return areal<nbits,es>(!_sign, _scale, _fraction, _zero, _inf);
			}

			// we model a hw pipeline with register assignments, functional block, and conversion
			areal& operator+=(const areal& rhs) {
				if (_trace_add) std::cout << "---------------------- ADD -------------------" << std::endl;
				// special case handling of the inputs
				if (isnan() || rhs.isnan()) {
					setnan();
					return *this;
				}
				if (iszero()) {
					*this = rhs;
					return *this;
				}
				if (rhs.iszero()) return *this;

				// arithmetic operation
				value<abits + 1> sum;
				value<fbits> a, b;
				// transform the inputs into (sign,scale,fraction) triples
				normalize(a);
				rhs.normalize(b);
				module_add<fbits, abits>(a, b, sum);		// add the two inputs

															// special case handling of the result
				if (sum.iszero()) {
					setzero();
				}
				else if (sum.isinf()) {
					setnan();
				}
				else {
					//convert(sum, *this);
				}
				return *this;
			}
			areal& operator+=(double rhs) {
				return *this += areal<nbits, es>(rhs);
			}
			areal& operator-=(const areal& rhs) {
				if (_trace_sub) std::cout << "---------------------- SUB -------------------" << std::endl;
				// special case handling of the inputs
				if (isnan() || rhs.isnan()) {
					setnan();
					return *this;
				}
				if (iszero()) {
					*this = -rhs;
					return *this;
				}
				if (rhs.iszero()) return *this;

				// arithmetic operation
				value<abits + 1> difference;
				value<fbits> a, b;
				// transform the inputs into (sign,scale,fraction) triples
				normalize(a);
				rhs.normalize(b);
				module_subtract<fbits, abits>(a, b, difference);	// add the two inputs

																	// special case handling of the result
				if (difference.iszero()) {
					setzero();
				}
				else if (difference.isinf()) {
					setnan();
				}
				else {
					//convert(difference, *this);
				}
				return *this;
			}
			areal& operator-=(double rhs) {
				return *this -= areal<nbits, es>(rhs);
			}
			areal& operator*=(const areal& rhs) {
				static_assert(fhbits > 0, "posit configuration does not support multiplication");
				if (_trace_mul) std::cout << "---------------------- MUL -------------------" << std::endl;
				// special case handling of the inputs
				if (isnan() || rhs.isnan()) {
					setnan();
					return *this;
				}
				if (iszero() || rhs.iszero()) {
					setzero();
					return *this;
				}

				// arithmetic operation
				value<mbits> product;
				value<fbits> a, b;
				// transform the inputs into (sign,scale,fraction) triples
				normalize(a);
				rhs.normalize(b);

				module_multiply(a, b, product);    // multiply the two inputs

												   // special case handling on the output
				if (product.iszero()) {
					setzero();
				}
				else if (product.isinf()) {
					setnan();
				}
				else {
					//convert(product, *this);
				}
				return *this;
			}
			areal& operator*=(double rhs) {
				return *this *= areal<nbits, es>(rhs);
			}
			areal& operator/=(const areal& rhs) {
				if (_trace_div) std::cout << "---------------------- DIV -------------------" << std::endl;
				// since we are encoding error conditions as NaR (Not a Real), we need to process that condition first
				// quiet signalling NaR
				if (rhs.iszero()) {
					setnan();
					return *this;
				}
				if (rhs.isnan()) {
					setnan();
					return *this;
				}
				if (iszero() || isnan()) {
					return *this;
				}

				value<divbits> ratio;
				value<fbits> a, b;
				// transform the inputs into (sign,scale,fraction) triples
				normalize(a);
				rhs.normalize(b);

				module_divide(a, b, ratio);

				// special case handling on the output
				if (ratio.iszero()) {
					setzero();  // this shouldn't happen as we should project back onto minpos
				}
				else if (ratio.isinf()) {
					setnan();  // this shouldn't happen as we should project back onto maxpos
				}
				else {
					//convert<nbits, es, divbits>(ratio, *this);
				}
				return *this;
			}
			areal& operator/=(double rhs) {
				return *this /= areal<nbits, es>(rhs);
			}
			areal& operator++() {
				// increment_posit();
				return *this;
			}
			areal operator++(int) {
				areal tmp(*this);
				operator++();
				return tmp;
			}
			areal& operator--() {
				//decrement_posit();
				return *this;
			}
			areal operator--(int) {
				areal tmp(*this);
				operator--();
				return tmp;
			}

			// modifiers
			void reset() {
				_sign  = false;
				_scale = 0;
				_nrOfBits = 0;
				_inf = false;
				_zero = false;
				_nan = false;
				_fraction.reset();
				_raw_bits.reset();
			}
			void set(bool sign, int scale, bitblock<fbits> fraction_without_hidden_bit, bool zero, bool inf, bool nan = false) {
				_sign     = sign;
				_scale    = scale;
				_fraction = fraction_without_hidden_bit;
				_zero     = zero;
				_inf      = inf;
				_nan      = nan;
			}
			// Set the raw bits of the posit given an unsigned value starting from the lsb. Handy for enumerating a posit state space
			areal& set_raw_bits(uint64_t value) {
				reset();
				bitblock<nbits> raw_bits;
				uint64_t mask = 1;
				for (size_t i = 0; i < nbits; i++) {
					raw_bits.set(i, (value & mask));
					mask <<= 1;
				}
				_raw_bits = raw_bits;
				return *this;
			}
			void setzero() {
				_zero     = true;
				_sign     = false;
				_inf      = false;
				_nan      = false;
				_scale    = 0;
				_nrOfBits = fbits;
				_fraction.reset();
			}
			void setinf() {
				_inf      = true;
				_sign     = false;
				_zero     = false;
				_nan      = false;
				_scale    = 0;
				_nrOfBits = fbits;
				_fraction.reset();
			}
			void setnan() {
				_nan      = true;
				_sign     = false;
				_zero     = false;
				_inf      = false;
				_scale    = 0;
				_nrOfBits = fbits;	
				_fraction.reset();
			}
			inline void setscale(int e) { _scale = e; }

			// selectors
			inline bool isneg() const { return _sign; }
			inline bool iszero() const { return _zero; }
			inline bool isinf() const { return _inf; }
			inline bool isnan() const { return _nan; }
			inline bool sign() const { return _sign; }
			inline int scale() const { return _scale; }
			bitblock<fbits> get_fraction() const { return _fraction; }
			/// Normalized shift (e.g., for addition).
			template <size_t Size>
			bitblock<Size> nshift(long shift) const {
				bitblock<Size> number;

				// Check range
				if (long(fbits) + shift >= long(Size))
					throw shift_too_large{};

				const long hpos = fbits + shift;       // position of hidden bit
														  
				if (hpos <= 0) {   // If hidden bit is LSB or beyond just set uncertainty bit and call it a day
					number[0] = true;
					return number;
				}
				number[hpos] = true;                   // hidden bit now safely set

													   // Copy fraction bits into certain part
				for (long npos = hpos - 1, fpos = long(fbits) - 1; npos > 0 && fpos >= 0; --npos, --fpos)
					number[npos] = _fraction[fpos];

				// Set uncertainty bit
				bool uncertainty = false;
				for (long fpos = std::min(long(fbits) - 1, -shift); fpos >= 0 && !uncertainty; --fpos)
					uncertainty |= _fraction[fpos];
				number[0] = uncertainty;
				return number;
			}

			bitblock<nbits> get() const { return _raw_bits; }
			// get a fixed point number by making the hidden bit explicit: useful for multiply units
			bitblock<fhbits> get_fixed_point() const {
				bitblock<fbits + 1> fixed_point_number;
				fixed_point_number.set(fbits, true); // make hidden bit explicit
				for (unsigned int i = 0; i < fbits; i++) {
					fixed_point_number[i] = _fraction[i];
				}
				return fixed_point_number;
			}
			// get the fraction value including the implicit hidden bit (this is at an exponent level 1 smaller)
			template<typename Ty = double>
			Ty get_implicit_fraction_value() const {
				if (_zero) return (long double)0.0;
				Ty v = 1.0;
				Ty scale = 0.5;
				for (int i = int(fbits) - 1; i >= 0; i--) {
					if (_fraction.test(i)) v += scale;
					scale *= 0.5;
					if (scale == 0.0) break;
				}
				return v;
			}
			int sign_value() const { return (_sign ? -1 : 1); }
			double scale_value() const {
				if (_zero) return (long double)(0.0);
				return std::pow((long double)2.0, (long double)_scale);
			}
			template<typename Ty = double>
			Ty fraction_value() const {
				if (_zero) return (long double)0.0;
				Ty v = 1.0;
				Ty scale = 0.5;
				for (int i = int(fbits) - 1; i >= 0; i--) {
					if (_fraction.test(i)) v += scale;
					scale *= 0.5;
					if (scale == 0.0) break;
				}
				return v;
			}
			long double to_long_double() const {
				return sign_value() * scale_value() * fraction_value<long double>();
			}
			double to_double() const {
				return sign_value() * scale_value() * fraction_value<double>();
			}
			float to_float() const {
				return float(sign_value() * scale_value() * fraction_value<float>());
			}
			// Maybe remove explicit
			explicit operator long double() const { return to_long_double(); }
			explicit operator double() const { return to_double(); }
			explicit operator float() const { return to_float(); }

			// currently, size is tied to fbits size of areal config. Is there a need for a case that captures a user-defined sized fraction?
			value<fbits> to_value() const {
				bool		     	 _sign;
				exponent<nbits, es>  _exponent;
				fraction<fbits>      _fraction;
				decode(_raw_bits, _sign, _exponent, _fraction);
				return value<fbits>(_sign, _exponent.scale(), _fraction.get(), iszero(), isnan());
			}
			void normalize(value<fbits>& v) const {
				bool		     	 _sign;
				exponent<nbits, es>  _exponent;
				fraction<fbits>      _fraction;
				decode(_raw_bits, _sign, _exponent, _fraction);
				v.set(_sign, _exponent.scale(), _fraction.get(), iszero(), isnan());
			}
			template<size_t tgt_size>
			value<tgt_size> round_to() {
				bitblock<tgt_size> rounded_fraction;
				if (tgt_size == 0) {
					bool round_up = false;
					if (fbits >= 2) {
						bool blast = _fraction[int(fbits) - 1];
						bool sb = anyAfter(_fraction, int(fbits) - 2);
						if (blast && sb) round_up = true;
					}
					else if (fbits == 1) {
						round_up = _fraction[0];
					}
					return value<tgt_size>(_sign, (round_up ? _scale + 1 : _scale), rounded_fraction, _zero, _inf);
				}
				else {
					if (!_zero || !_inf) {
						if (tgt_size < fbits) {
							int rb = int(tgt_size) - 1;
							int lb = int(fbits) - int(tgt_size) - 1;
							for (int i = int(fbits) - 1; i > lb; i--, rb--) {
								rounded_fraction[rb] = _fraction[i];
							}
							bool blast = _fraction[lb];
							bool sb = false;
							if (lb > 0) sb = anyAfter(_fraction, lb-1);
							if (blast || sb) rounded_fraction[0] = true;
						}
						else {
							int rb = int(tgt_size) - 1;
							for (int i = int(fbits) - 1; i >= 0; i--, rb--) {
								rounded_fraction[rb] = _fraction[i];
							}
						}
					}
				}
				return value<tgt_size>(_sign, _scale, rounded_fraction, _zero, _inf);
			}
		private:
			bitblock<nbits> _raw_bits;
			bool            _sign;
			int             _scale;
			bitblock<fbits>	_fraction;
			int             _nrOfBits;  // in case the fraction is smaller than the full fbits
			bool            _inf;
			bool            _zero;
			bool            _nan;

			// template parameters need names different from class template parameters (for gcc and clang)
			template<size_t nnbits, size_t nes>
			friend std::ostream& operator<< (std::ostream& ostr, const areal<nnbits,nes>& r);
			template<size_t nnbits, size_t nes>
			friend std::istream& operator>> (std::istream& istr, areal<nnbits,nes>& r);

			template<size_t nnbits, size_t nes>
			friend bool operator==(const areal<nnbits,nes>& lhs, const areal<nnbits,nes>& rhs);
			template<size_t nnbits, size_t nes>
			friend bool operator!=(const areal<nnbits,nes>& lhs, const areal<nnbits,nes>& rhs);
			template<size_t nnbits, size_t nes>
			friend bool operator< (const areal<nnbits,nes>& lhs, const areal<nnbits,nes>& rhs);
			template<size_t nnbits, size_t nes>
			friend bool operator> (const areal<nnbits,nes>& lhs, const areal<nnbits,nes>& rhs);
			template<size_t nnbits, size_t nes>
			friend bool operator<=(const areal<nnbits,nes>& lhs, const areal<nnbits,nes>& rhs);
			template<size_t nnbits, size_t nes>
			friend bool operator>=(const areal<nnbits,nes>& lhs, const areal<nnbits,nes>& rhs);
		};

		////////////////////// VALUE operators
		template<size_t nnbits, size_t nes>
		inline std::ostream& operator<<(std::ostream& ostr, const areal<nnbits,nes>& v) {
			if (v._inf) {
				ostr << FP_INFINITE;
			}
			else {
				ostr << (long double)v;
			}
			return ostr;
		}

		template<size_t nnbits, size_t nes>
		inline std::istream& operator>> (std::istream& istr, const areal<nnbits,nes>& v) {
			istr >> v._fraction;
			return istr;
		}

		template<size_t nnbits, size_t nes>
		inline bool operator==(const areal<nnbits,nes>& lhs, const areal<nnbits,nes>& rhs) { return lhs._sign == rhs._sign && lhs._scale == rhs._scale && lhs._fraction == rhs._fraction && lhs._nrOfBits == rhs._nrOfBits && lhs._zero == rhs._zero && lhs._inf == rhs._inf; }
		template<size_t nnbits, size_t nes>
		inline bool operator!=(const areal<nnbits,nes>& lhs, const areal<nnbits,nes>& rhs) { return !operator==(lhs, rhs); }
		template<size_t nnbits, size_t nes>
		inline bool operator< (const areal<nnbits,nes>& lhs, const areal<nnbits,nes>& rhs) { return lhs.to_long_double() < rhs.to_long_double(); }
		template<size_t nnbits, size_t nes>
		inline bool operator> (const areal<nnbits,nes>& lhs, const areal<nnbits,nes>& rhs) { return  operator< (rhs, lhs); }
		template<size_t nnbits, size_t nes>
		inline bool operator<=(const areal<nnbits,nes>& lhs, const areal<nnbits,nes>& rhs) { return !operator> (lhs, rhs); }
		template<size_t nnbits, size_t nes>
		inline bool operator>=(const areal<nnbits,nes>& lhs, const areal<nnbits,nes>& rhs) { return !operator< (lhs, rhs); }

		// posit - posit binary arithmetic operators
		// BINARY ADDITION
		template<size_t nbits, size_t es>
		inline areal<nbits, es> operator+(const areal<nbits, es>& lhs, const areal<nbits, es>& rhs) {
			areal<nbits, es> sum = lhs;
			sum += rhs;
			return sum;
		}
		// BINARY SUBTRACTION
		template<size_t nbits, size_t es>
		inline areal<nbits, es> operator-(const areal<nbits, es>& lhs, const areal<nbits, es>& rhs) {
			areal<nbits, es> diff = lhs;
			diff -= rhs;
			return diff;
		}
		// BINARY MULTIPLICATION
		template<size_t nbits, size_t es>
		inline areal<nbits, es> operator*(const areal<nbits, es>& lhs, const areal<nbits, es>& rhs) {
			areal<nbits, es> mul = lhs;
			mul *= rhs;
			return mul;
		}
		// BINARY DIVISION
		template<size_t nbits, size_t es>
		inline areal<nbits, es> operator/(const areal<nbits, es>& lhs, const areal<nbits, es>& rhs) {
			areal<nbits, es> ratio = lhs;
			ratio /= rhs;
			return ratio;
		}


		template<size_t nbits, size_t es>
		inline std::string components(const areal<nbits,es>& v) {
			std::stringstream s;
			if (v.iszero()) {
				s << " zero b" << std::setw(nbits) << v.fraction();
				return s.str();
			}
			else if (v.isinf()) {
				s << " infinite b" << std::setw(nbits) << v.fraction();
				return s.str();
			}
			s << "(" << (v.sign() ? "-" : "+") << "," << v.scale() << "," << v.fraction() << ")";
			return s.str();
		}

		/// Magnitude of a scientific notation value (equivalent to turning the sign bit off).
		template<size_t nbits, size_t es>
		areal<nbits,es> abs(const areal<nbits,es>& v) {
			return areal<nbits,es>(false, v.scale(), v.fraction(), v.isZero());
		}


	}  // namespace unum

}  // namespace sw
