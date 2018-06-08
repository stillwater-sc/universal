#pragma once
// value.hpp: definition of a (sign, scale, significant) representation of an approximation to a real value
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <limits>

#include "bit_functions.hpp"
#include "trace_constants.hpp"

namespace sw {
	namespace unum {
		
		// Forward definitions
		template<size_t fbits> class value;
		template<size_t fbits> value<fbits> abs(const value<fbits>& v);

		// template class representing a value in scientific notation, using a template size for the number of fraction bits
		template<size_t fbits>
		class value {
		public:
			static constexpr size_t fhbits = fbits + 1;    // number of fraction bits including the hidden bit
			value() : _sign(false), _scale(0), _nrOfBits(fbits), _zero(true), _inf(false), _nan(false) {}
			value(bool sign, int scale, const bitblock<fbits>& fraction_without_hidden_bit, bool zero = true, bool inf = false) : _sign(sign), _scale(scale), _nrOfBits(fbits), _fraction(fraction_without_hidden_bit), _inf(inf), _zero(zero), _nan(false) {}

			value(const signed char initial_value)        { *this = initial_value; }
			value(const short initial_value)              { *this = initial_value; }
			value(const int initial_value)                { *this = initial_value; }
			value(const long initial_value)               { *this = initial_value; }
			value(const long long initial_value)          { *this = initial_value; }
			value(const char initial_value)               { *this = initial_value; }
			value(const unsigned short initial_value)     { *this = initial_value; }
			value(const unsigned int initial_value)       { *this = initial_value; }
			value(const unsigned long initial_value)      { *this = initial_value; }
			value(const unsigned long long initial_value) { *this = initial_value; }
			value(const float initial_value)              { *this = initial_value; }
			value(const double initial_value)             { *this = initial_value; }
			value(const long double initial_value)        { *this = initial_value; }
			value(const value& rhs)                       { *this = rhs; }

			value& operator=(const value& rhs) {
				_sign	  = rhs._sign;
				_scale	  = rhs._scale;
				_fraction = rhs._fraction;
				_nrOfBits = rhs._nrOfBits;
				_inf      = rhs._inf;
				_zero     = rhs._zero;
				_nan      = rhs._nan;
				return *this;
			}
			value<fbits>& operator=(const signed char rhs) {
				*this = (long long)(rhs);
				return *this;
			}
			value<fbits>& operator=(const short rhs) {
				*this = (long long)(rhs);
				return *this;
			}
			value<fbits>& operator=(const int rhs) {
				*this = (long long)(rhs);
				return *this;
			}
			value<fbits>& operator=(const long rhs) {
				*this = (long long)(rhs);
				return *this;
			}
			value<fbits>& operator=(const long long rhs) {
				if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;
				if (rhs == 0) {
					setToZero();
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
			value<fbits>& operator=(const char rhs) {
				*this = (unsigned long long)(rhs);
				return *this;
			}
			value<fbits>& operator=(const unsigned short rhs) {
				*this = (long long)(rhs);
				return *this;
			}
			value<fbits>& operator=(const unsigned int rhs) {
				*this = (long long)(rhs);
				return *this;
			}
			value<fbits>& operator=(const unsigned long rhs) {
				*this = (long long)(rhs);
				return *this;
			}
			value<fbits>& operator=(const unsigned long long rhs) {
				if (_trace_conversion) std::cout << "---------------------- CONVERT -------------------" << std::endl;
				if (rhs == 0) {
					setToZero();
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
			value<fbits>& operator=(const float rhs) {
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
			value<fbits>& operator=(const double rhs) {
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
			value<fbits>& operator=(const long double rhs) {
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
			value<fbits> operator-() const {				
				return value<fbits>(!_sign, _scale, _fraction, _zero, _inf);
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
			}
			void set(bool sign, int scale, bitblock<fbits> fraction_without_hidden_bit, bool zero, bool inf, bool nan = false) {
				_sign     = sign;
				_scale    = scale;
				_fraction = fraction_without_hidden_bit;
				_zero     = zero;
				_inf      = inf;
				_nan      = nan;
			}
			void setToZero() {
				_zero     = true;
				_sign     = false;
				_inf      = false;
				_nan      = false;
				_scale    = 0;
				_nrOfBits = fbits;
				_fraction.reset();
			}
			void setToInfinite() {
				_inf      = true;
				_sign     = false;
				_zero     = false;
				_nan      = false;
				_scale    = 0;
				_nrOfBits = fbits;
				_fraction.reset();
			}
			void setToNan() {
				_nan      = true;
				_sign     = false;
				_zero     = false;
				_inf      = false;
				_scale    = 0;
				_nrOfBits = fbits;	
				_fraction.reset();
			}
			inline void setExponent(int e) { _scale = e; }
			inline bool isNegative() const { return _sign; }
			inline bool isZero() const { return _zero; }
			inline bool isInfinite() const { return _inf; }
			inline bool isNaN() const { return _nan; }
			inline bool sign() const { return _sign; }
			inline int scale() const { return _scale; }
			bitblock<fbits> fraction() const { return _fraction; }
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

			template<size_t srcbits, size_t tgtbits>
			void right_extend(const value<srcbits>& src) {
				_sign = src.sign();
				_scale = src.scale();
				_nrOfBits = tgtbits;
				_inf = src.isInfinite();
				_zero = src.isZero();
				_nan = src.isNaN();
				bitblock<srcbits> src_fraction = src.fraction();
				if (!_inf && !_zero && !_nan) {
					for (int s = srcbits - 1, t = tgtbits - 1; s >= 0 && t >= 0; --s, --t)
						_fraction[t] = src_fraction[s];
				}
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
			bool				_sign;
			int					_scale;
			bitblock<fbits>	    _fraction;
			int					_nrOfBits;  // in case the fraction is smaller than the full fbits
			bool                _inf;
			bool                _zero;
			bool                _nan;

			// template parameters need names different from class template parameters (for gcc and clang)
			template<size_t nfbits>
			friend std::ostream& operator<< (std::ostream& ostr, const value<nfbits>& r);
			template<size_t nfbits>
			friend std::istream& operator>> (std::istream& istr, value<nfbits>& r);

			template<size_t nfbits>
			friend bool operator==(const value<nfbits>& lhs, const value<nfbits>& rhs);
			template<size_t nfbits>
			friend bool operator!=(const value<nfbits>& lhs, const value<nfbits>& rhs);
			template<size_t nfbits>
			friend bool operator< (const value<nfbits>& lhs, const value<nfbits>& rhs);
			template<size_t nfbits>
			friend bool operator> (const value<nfbits>& lhs, const value<nfbits>& rhs);
			template<size_t nfbits>
			friend bool operator<=(const value<nfbits>& lhs, const value<nfbits>& rhs);
			template<size_t nfbits>
			friend bool operator>=(const value<nfbits>& lhs, const value<nfbits>& rhs);
		};

		////////////////////// VALUE operators
		template<size_t nfbits>
		inline std::ostream& operator<<(std::ostream& ostr, const value<nfbits>& v) {
			if (v._inf) {
				ostr << FP_INFINITE;
			}
			else {
				ostr << (long double)v;
			}
			return ostr;
		}

		template<size_t nfbits>
		inline std::istream& operator>> (std::istream& istr, const value<nfbits>& v) {
			istr >> v._fraction;
			return istr;
		}

		template<size_t nfbits>
		inline bool operator==(const value<nfbits>& lhs, const value<nfbits>& rhs) { return lhs._sign == rhs._sign && lhs._scale == rhs._scale && lhs._fraction == rhs._fraction && lhs._nrOfBits == rhs._nrOfBits && lhs._zero == rhs._zero && lhs._inf == rhs._inf; }
		template<size_t nfbits>
		inline bool operator!=(const value<nfbits>& lhs, const value<nfbits>& rhs) { return !operator==(lhs, rhs); }
		template<size_t nfbits>
		inline bool operator< (const value<nfbits>& lhs, const value<nfbits>& rhs) { return lhs.to_long_double() < rhs.to_long_double(); }
		template<size_t nfbits>
		inline bool operator> (const value<nfbits>& lhs, const value<nfbits>& rhs) { return  operator< (rhs, lhs); }
		template<size_t nfbits>
		inline bool operator<=(const value<nfbits>& lhs, const value<nfbits>& rhs) { return !operator> (lhs, rhs); }
		template<size_t nfbits>
		inline bool operator>=(const value<nfbits>& lhs, const value<nfbits>& rhs) { return !operator< (lhs, rhs); }

		template<size_t fbits>
		inline std::string components(const value<fbits>& v) {
			std::stringstream s;
			if (v.isZero()) {
				s << " zero b" << std::setw(fbits) << v.fraction();
				return s.str();
			}
			else if (v.isInfinite()) {
				s << " infinite b" << std::setw(fbits) << v.fraction();
				return s.str();
			}
			s << "(" << (v.sign() ? "-" : "+") << "," << v.scale() << "," << v.fraction() << ")";
			return s.str();
		}

		/// Magnitude of a scientific notation value (equivalent to turning the sign bit off).
		template<size_t nfbits>
		value<nfbits> abs(const value<nfbits>& v) {
			return value<nfbits>(false, v.scale(), v.fraction(), v.isZero());
		}

		// add module
		template<size_t fbits, size_t abits>
		void module_add(const value<fbits>& lhs, const value<fbits>& rhs, value<abits + 1>& result) {
			// with sign/magnitude adders it is customary to organize the computation 
			// along the four quadrants of sign combinations
			//  + + = +
			//  + - =   lhs > rhs ? + : -
			//  - + =   lhs > rhs ? - : +
			//  - - = 
			// to simplify the result processing assign the biggest 
			// absolute value to R1, then the sign of the result will be sign of the value in R1.

			if (lhs.isInfinite() || rhs.isInfinite()) {
				result.setToInfinite();
				return;
			}
			int lhs_scale = lhs.scale(), rhs_scale = rhs.scale(), scale_of_result = std::max(lhs_scale, rhs_scale);

			// align the fractions
			bitblock<abits> r1 = lhs.template nshift<abits>(lhs_scale - scale_of_result + 3);
			bitblock<abits> r2 = rhs.template nshift<abits>(rhs_scale - scale_of_result + 3);
			bool r1_sign = lhs.sign(), r2_sign = rhs.sign();
			bool signs_are_different = r1_sign != r2_sign;

			if (signs_are_different && sw::unum::abs(lhs) < sw::unum::abs(rhs)) {
				std::swap(r1, r2);
				std::swap(r1_sign, r2_sign);
			}

			if (signs_are_different) r2 = twos_complement(r2);

			if (_trace_add) {
				std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r1       " << r1 << std::endl;
				std::cout << (r2_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r2       " << r2 << std::endl;
			}

			bitblock<abits + 1> sum;
			const bool carry = add_unsigned(r1, r2, sum);

			if (_trace_add) std::cout << (r1_sign ? "sign -1" : "sign  1") << " carry " << std::setw(3) << (carry ? 1 : 0) << " sum     " << sum << std::endl;

			long shift = 0;
			if (carry) {
				if (r1_sign == r2_sign) {  // the carry && signs== implies that we have a number bigger than r1
					shift = -1;
				} 
				else {
					// the carry && signs!= implies r2 is complement, result < r1, must find hidden bit (in the complement)
					for (int i = abits - 1; i >= 0 && !sum[i]; i--) {
						shift++;
					}
				}
			}
			assert(shift >= -1);

			if (shift >= long(abits)) {            // we have actual 0                            
				sum.reset();
				result.set(false, 0, sum, true, false, false);
				return;
			}

			scale_of_result -= shift;
			const int hpos = abits - 1 - shift;         // position of the hidden bit 
			sum <<= abits - hpos + 1;
			if (_trace_add) std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " sum     " << sum << std::endl;
			result.set(r1_sign, scale_of_result, sum, false, false, false);
		}

		// subtract module: use ADDER
		template<size_t fbits, size_t abits>
		void module_subtract(const value<fbits>& lhs, const value<fbits>& rhs, value<abits + 1>& result) {
			if (lhs.isInfinite() || rhs.isInfinite()) {
				result.setToInfinite();
				return;
			}
			int lhs_scale = lhs.scale(), rhs_scale = rhs.scale(), scale_of_result = std::max(lhs_scale, rhs_scale);

			// align the fractions
			bitblock<abits> r1 = lhs.template nshift<abits>(lhs_scale - scale_of_result + 3);
			bitblock<abits> r2 = rhs.template nshift<abits>(rhs_scale - scale_of_result + 3);
			bool r1_sign = lhs.sign(), r2_sign = !rhs.sign();
			bool signs_are_different = r1_sign != r2_sign;

			if (sw::unum::abs(lhs) < sw::unum::abs(rhs)) {
				std::swap(r1, r2);
				std::swap(r1_sign, r2_sign);
			}

			if (signs_are_different) r2 = twos_complement(r2);

			if (_trace_sub) {
				std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r1       " << r1 << std::endl;
				std::cout << (r2_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r2       " << r2 << std::endl;
			}

			bitblock<abits + 1> sum;
			const bool carry = add_unsigned(r1, r2, sum);

			if (_trace_sub) std::cout << (r1_sign ? "sign -1" : "sign  1") << " carry " << std::setw(3) << (carry ? 1 : 0) << " sum     " << sum << std::endl;

			long shift = 0;
			if (carry) {
				if (r1_sign == r2_sign) {  // the carry && signs== implies that we have a number bigger than r1
					shift = -1;
				}
				else {
					// the carry && signs!= implies r2 is complement, result < r1, must find hidden bit (in the complement)
					for (int i = abits - 1; i >= 0 && !sum[i]; i--) {
						shift++;
					}
				}
			}
			assert(shift >= -1);

			if (shift >= long(abits)) {            // we have actual 0                            
				sum.reset();
				result.set(false, 0, sum, true, false, false);
				return;
			}

			scale_of_result -= shift;
			const int hpos = abits - 1 - shift;         // position of the hidden bit 
			sum <<= abits - hpos + 1;
			if (_trace_sub) std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " sum     " << sum << std::endl;
			result.set(r1_sign, scale_of_result, sum, false, false, false);
		}

		// subtract module using SUBTRACTOR: CURRENTLY BROKEN FOR UNKNOWN REASON
		template<size_t fbits, size_t abits>
		void module_subtract_BROKEN(const value<fbits>& lhs, const value<fbits>& rhs, value<abits + 1>& result) {

			if (lhs.isInfinite() || rhs.isInfinite()) {
				result.setToInfinite();
				return;
			}
			int lhs_scale = lhs.scale(), rhs_scale = rhs.scale(), scale_of_result = std::max(lhs_scale, rhs_scale);

			// align the fractions
			bitblock<abits> r1 = lhs.template nshift<abits>(lhs_scale - scale_of_result + 3);
			bitblock<abits> r2 = rhs.template nshift<abits>(rhs_scale - scale_of_result + 3);
			bool r1_sign = lhs.sign(), r2_sign = rhs.sign();
			bool signs_are_equal = r1_sign == r2_sign;

			if (r1_sign) r1 = twos_complement(r1);
			if (r1_sign) r2 = twos_complement(r2);

			if (_trace_sub) {
				std::cout << (r1_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r1       " << r1 << std::endl;
				std::cout << (r2_sign ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " r2       " << r2 << std::endl;
			}

			bitblock<abits + 1> difference;
			const bool borrow = subtract_unsigned(r1, r2, difference);

			if (_trace_sub) std::cout << (r1_sign ? "sign -1" : "sign  1") << " borrow" << std::setw(3) << (borrow ? 1 : 0) << " diff    " << difference << std::endl;

			long shift = 0;
			if (borrow) {   // we have a negative value result
				difference = twos_complement(difference);
			}
			// find hidden bit
			for (int i = abits - 1; i >= 0 && difference[i]; i--) {
				shift++;
			}
			assert(shift >= -1);

			if (shift >= long(abits)) {            // we have actual 0                            
				difference.reset();
				result.set(false, 0, difference, true, false, false);
				return;
			}

			scale_of_result -= shift;
			const int hpos = abits - 1 - shift;         // position of the hidden bit 
			difference <<= abits - hpos + 1;
			if (_trace_sub) std::cout << (borrow ? "sign -1" : "sign  1") << " scale " << std::setw(3) << scale_of_result << " result  " << difference << std::endl;
			result.set(borrow, scale_of_result, difference, false, false, false);
		}

		// multiply module
		template<size_t fbits, size_t mbits>
		void module_multiply(const value<fbits>& lhs, const value<fbits>& rhs, value<mbits>& result) {
			static constexpr size_t fhbits = fbits + 1;  // fraction + hidden bit
			if (_trace_mul) std::cout << "lhs  " << components(lhs) << std::endl << "rhs  " << components(rhs) << std::endl;

			if (lhs.isInfinite() || rhs.isInfinite()) {
				result.setToInfinite();
				return;
			}
			if (lhs.isZero() || rhs.isZero()) {
				result.setToZero();
				return;
			}

			bool new_sign = lhs.sign() ^ rhs.sign();
			int new_scale = lhs.scale() + rhs.scale();
			bitblock<mbits> result_fraction;

			if (fbits > 0) {
				// fractions are without hidden bit, get_fixed_point adds the hidden bit back in
				bitblock<fhbits> r1 = lhs.get_fixed_point();
				bitblock<fhbits> r2 = rhs.get_fixed_point();
				multiply_unsigned(r1, r2, result_fraction);

				if (_trace_mul) std::cout << "r1  " << r1 << std::endl << "r2  " << r2 << std::endl << "res " << result_fraction << std::endl;
				// check if the radix point needs to shift
				int shift = 2;
				if (result_fraction.test(mbits - 1)) {
					shift = 1;
					if (_trace_mul) std::cout << " shift " << shift << std::endl;
					new_scale += 1;
				}
				result_fraction <<= shift;    // shift hidden bit out	
			}
			else {   // posit<3,0>, <4,1>, <5,2>, <6,3>, <7,4> etc are pure sign and scale
				// multiply the hidden bits together, i.e. 1*1: we know the answer a priori
			}
			if (_trace_mul) std::cout << "sign " << (new_sign ? "-1 " : " 1 ") << "scale " << new_scale << " fraction " << result_fraction << std::endl;

			result.set(new_sign, new_scale, result_fraction, false, false, false);
		}

		// divide module
		template<size_t fbits, size_t divbits>
		void module_divide(const value<fbits>& lhs, const value<fbits>& rhs, value<divbits>& result) {
			static constexpr size_t fhbits = fbits + 1;  // fraction + hidden bit
			if (_trace_div) std::cout << "lhs  " << components(lhs) << std::endl << "rhs  " << components(rhs) << std::endl;

			if (lhs.isInfinite() || rhs.isInfinite()) {
				result.setToInfinite();
				return;
			}
			if (lhs.isZero() || rhs.isInfinite()) {
				result.setToZero();
				return;
			}

			bool new_sign = lhs.sign() ^ rhs.sign();
			int new_scale = lhs.scale() - rhs.scale();
			bitblock<divbits> result_fraction;

			if (fbits > 0) {
				// fractions are without hidden bit, get_fixed_point adds the hidden bit back in
				bitblock<fhbits> r1 = lhs.get_fixed_point();
				bitblock<fhbits> r2 = rhs.get_fixed_point();
				divide_with_fraction(r1, r2, result_fraction);
				if (_trace_div) std::cout << "r1     " << r1 << std::endl << "r2     " << r2 << std::endl << "result " << result_fraction << std::endl << "scale  " << new_scale << std::endl;
				// check if the radix point needs to shift
				// radix point is at divbits - fhbits
				int msb = divbits - fhbits;
				int shift = fhbits;
				if (!result_fraction.test(msb)) {
					msb--; shift++;
					while (!result_fraction.test(msb)) { // search for the first 1
						msb--; shift++;
					}
				}
				result_fraction <<= shift;    // shift hidden bit out
				new_scale -= (shift - fhbits);
				if (_trace_div) std::cout << "shift  " << shift << std::endl << "result " << result_fraction << std::endl << "scale  " << new_scale << std::endl;;
			}
			else {   // posit<3,0>, <4,1>, <5,2>, <6,3>, <7,4> etc are pure sign and scale
					 // no need to multiply the hidden bits together, i.e. 1*1: we know the answer a priori
			}
			if (_trace_div) std::cout << "sign " << (new_sign ? "-1 " : " 1 ") << "scale " << new_scale << " fraction " << result_fraction << std::endl;

			result.set(new_sign, new_scale, result_fraction, false, false, false);
		}

	}  // namespace unum

}  // namespace sw
