#pragma once
// posit.hpp: definition of arbitrary posit number configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


namespace sw {
	namespace unum {
		
		// template class representing a value in scientific notation, using a template size for the fraction bits
		template<size_t fbits>
		class value {
		public:
			static constexpr size_t fhbits = fbits + 1;    // size of the fixed point number with hidden bit made explicity
			value() : _sign(false), _scale(0), _nrOfBits(fbits), _zero(true), _inf(false), _nan(false) {}
			value(bool sign, int scale, std::bitset<fbits> fraction_without_hidden_bit, bool zero = true, bool inf = false) : _sign(sign), _scale(scale), _nrOfBits(fbits), _fraction(fraction_without_hidden_bit), _inf(inf), _zero(zero), _nan(false) {}
			value(int8_t initial_value) {
				*this = initial_value;
			}
			value(int16_t initial_value) {
				*this = initial_value;
			}
			value(int32_t initial_value) {
				*this = initial_value;
			}
			value(int64_t initial_value) {
				*this = initial_value;
			}
			value(uint64_t initial_value) {
				*this = initial_value;
			}
			value(float initial_value) {
				*this = initial_value;
			}
			value(double initial_value) {
				*this = initial_value;
			}
			value(const value& rhs) {
				*this = rhs;
			}
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
			value<fbits>& operator=(int8_t rhs) {
				*this = int64_t(rhs);
				return *this;
			}
			value<fbits>& operator=(int16_t rhs) {
				*this = int64_t(rhs);
				return *this;
			}
			value<fbits>& operator=(int32_t rhs) {
				*this = int64_t(rhs);
				return *this;
			}
			value<fbits>& operator=(int64_t rhs) {
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
			value<fbits>& operator=(uint64_t rhs) {
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
			value<fbits>& operator=(float rhs) {
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
					_sign = extract_sign(rhs);
					_scale = extract_exponent(rhs) - 1;
					uint32_t _23b_fraction_without_hidden_bit = extract_fraction(rhs);
					_fraction = extract_float_fraction<fbits>(_23b_fraction_without_hidden_bit);
					_nrOfBits = fbits;
					if (_trace_conversion) std::cout << "float " << rhs << " sign " << _sign << " scale " << _scale << " 23b fraction 0x" << std::hex << _23b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;
				}
				break;
				}
				return *this;
			}
			value<fbits>& operator=(double rhs) {
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
					_sign = extract_sign(rhs);
					_scale = extract_exponent(rhs) - 1;
					uint64_t _52b_fraction_without_hidden_bit = extract_fraction(rhs);
					_fraction = extract_double_fraction<fbits>(_52b_fraction_without_hidden_bit);
					_nrOfBits = fbits;
					if (_trace_conversion) std::cout << "double " << rhs << " sign " << _sign << " scale " << _scale << " 52b fraction 0x" << std::hex << _52b_fraction_without_hidden_bit << " _fraction b" << _fraction << std::dec << std::endl;
				}
				break;
				}
				return *this;
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
			void set(bool sign, int scale, std::bitset<fbits> fraction_without_hidden_bit, bool zero, bool inf, bool nan = false) {
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
			inline bool isNegative() const { return _sign; }
			inline bool isZero() const { return _zero; }
			inline bool isInfinite() const { return _inf; }
			inline bool isNaN() const { return _nan; }
			inline bool sign() const { return _sign; }
			inline int scale() const { return _scale; }
			std::bitset<fbits> fraction() const { return _fraction; }
			/// Normalized shift (e.g., for addition).
			template <size_t Size>
			std::bitset<Size> nshift(long shift) const {
				std::bitset<Size> number;

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
			std::bitset<fhbits> get_fixed_point() const {
				std::bitset<fbits + 1> fixed_point_number;
				fixed_point_number.set(fbits, true); // make hidden bit explicit
				for (unsigned int i = 0; i < fbits; i++) {
					fixed_point_number[i] = _fraction[i];
				}
				return fixed_point_number;
			}
			double sign_value() const { return (_sign ? -1.0 : 1.0); }
			double scale_value() const {
				double v = 0.0;
				if (_zero) return v;
				// TODO: breaks when _scale is larger than 64
				if (_scale >= 0) {
					v = double(uint64_t(1) << _scale);
				}
				else {
					v = double(1.0) / double(uint64_t(1) << -_scale);
				}
				return v;
			}
			double fraction_value() const {
				// TODO: this fails when fbits > 64 and we cannot represent the fraction by a 64bit unsigned integer
				return double(_fraction.to_ullong()) / double(uint64_t(1) << (fbits));
			}
			double to_double() const {
				return sign_value() * scale_value() * (1.0 + fraction_value());
			}
			template<size_t tgt_size>
			value<tgt_size> round_to() {
				value<tgt_size> result;
				std::bitset<tgt_size> rounded_fraction;
				result.set(sign(), scale(), rounded_fraction, isZero());
				// round the fraction
				return result;
			}
		private:
			bool				_sign;
			int					_scale;
			std::bitset<fbits>	_fraction;
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
				ostr << v.to_double();
			}
			return ostr;
		}

		template<size_t nfbits>
		inline std::istream& operator>> (std::istream& istr, const value<nfbits>& v) {
			istr >> v._fraction;
			return istr;
		}

		template<size_t nfbits>
		inline bool operator==(const value<nfbits>& lhs, const value<nfbits>& rhs) { return lhs._sign == rhs._sign && lhs._scale == rhs._scale && lhs._fraction == rhs._fraction && lhs._nrOfBits == rhs._nrOfBits; }
		template<size_t nfbits>
		inline bool operator!=(const value<nfbits>& lhs, const value<nfbits>& rhs) { return !operator==(lhs, rhs); }
		template<size_t nfbits>
		inline bool operator< (const value<nfbits>& lhs, const value<nfbits>& rhs) { return lhs.to_double() < rhs.to_double(); }
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

}  // namespace unum

}  // namespace sw