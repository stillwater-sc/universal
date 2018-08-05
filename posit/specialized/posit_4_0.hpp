// posit_4_0.cpp: specialized 4-bit posit using lookup table arithmetic
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
	namespace unum {

		constexpr uint8_t posit_4_0_addition_lookup[256] = {
			0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
			1,2,3,4,4,6,6,7,8,9,10,12,13,14,15,0,
			2,3,4,4,5,6,6,7,8,9,11,12,14,15,0,1,
			3,4,4,5,6,6,6,7,8,9,12,13,15,0,1,2,
			4,4,5,6,6,6,6,7,8,10,12,14,0,1,2,3,
			5,6,6,6,6,6,7,7,8,10,14,0,2,3,4,4,
			6,6,6,6,6,7,7,7,8,10,0,2,4,4,5,6,
			7,7,7,7,7,7,7,7,8,0,6,6,6,7,7,7,
			8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
			9,9,9,9,10,10,10,0,8,9,9,9,9,9,9,9,
			10,10,11,12,12,14,0,6,8,9,9,9,10,10,10,10,
			11,12,12,13,14,0,2,6,8,9,9,10,10,10,10,10,
			12,13,14,15,0,2,4,6,8,9,10,10,10,10,11,12,
			13,14,15,0,1,3,4,7,8,9,10,10,10,11,12,12,
			14,15,0,1,2,4,5,7,8,9,10,10,11,12,12,13,
			15,0,1,2,3,4,6,7,8,9,10,10,12,12,13,14,
		};

		constexpr uint8_t posit_4_0_subtraction_lookup[256] = {
			0,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,
			1,0,15,14,13,12,10,9,8,7,6,6,4,4,3,2,
			2,1,0,15,14,12,11,9,8,7,6,6,5,4,4,3,
			3,2,1,0,15,13,12,9,8,7,6,6,6,5,4,4,
			4,3,2,1,0,14,12,10,8,7,6,6,6,6,5,4,
			5,4,4,3,2,0,14,10,8,7,7,6,6,6,6,6,
			6,6,5,4,4,2,0,10,8,7,7,7,6,6,6,6,
			7,7,7,7,6,6,6,0,8,7,7,7,7,7,7,7,
			8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
			9,9,9,9,9,9,9,9,8,0,10,10,10,9,9,9,
			10,10,10,10,10,9,9,9,8,6,0,14,12,12,11,10,
			11,10,10,10,10,10,9,9,8,6,2,0,14,13,12,12,
			12,12,11,10,10,10,10,9,8,6,4,2,0,15,14,13,
			13,12,12,11,10,10,10,9,8,7,4,3,1,0,15,14,
			14,13,12,12,11,10,10,9,8,7,5,4,2,1,0,15,
			15,14,13,12,12,10,10,9,8,7,6,4,3,2,1,0,
		};

		constexpr uint8_t posit_4_0_multiplication_lookup[256] = {
			0,0,0,0,0,0,0,0,8,0,0,0,0,0,0,0,
			0,1,1,1,1,2,2,4,8,12,14,14,15,15,15,15,
			0,1,1,2,2,3,4,6,8,10,12,13,14,14,15,15,
			0,1,2,2,3,4,5,6,8,10,11,12,13,14,14,15,
			0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
			0,2,3,4,5,6,6,7,8,9,10,10,11,12,13,14,
			0,2,4,5,6,6,7,7,8,9,9,10,10,11,12,14,
			0,4,6,6,7,7,7,7,8,9,9,9,9,10,10,12,
			8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
			0,12,10,10,9,9,9,9,8,7,7,7,7,6,6,4,
			0,14,12,11,10,10,9,9,8,7,7,6,6,5,4,2,
			0,14,13,12,11,10,10,9,8,7,6,6,5,4,3,2,
			0,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,
			0,15,14,14,13,12,11,10,8,6,5,4,3,2,2,1,
			0,15,15,14,14,13,12,10,8,6,4,3,2,2,1,1,
			0,15,15,15,15,14,14,12,8,4,2,2,1,1,1,1,
		};

		constexpr uint8_t posit_4_0_division_lookup[256] = {
			8,0,0,0,0,0,0,0,8,0,0,0,0,0,0,0,
			8,4,2,1,1,1,1,1,8,15,15,15,15,15,14,12,
			8,6,4,3,2,1,1,1,8,15,15,15,14,13,12,10,
			8,6,5,4,3,2,2,1,8,15,14,14,13,12,11,10,
			8,7,6,5,4,3,2,1,8,15,14,13,12,11,10,9,
			8,7,6,6,5,4,3,2,8,14,13,12,11,10,10,9,
			8,7,7,6,6,5,4,2,8,14,12,11,10,10,9,9,
			8,7,7,7,7,6,6,4,8,12,10,10,9,9,9,9,
			8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
			8,9,9,9,9,10,10,12,8,4,6,6,7,7,7,7,
			8,9,9,10,10,11,12,14,8,2,4,5,6,6,7,7,
			8,9,10,10,11,12,13,14,8,2,3,4,5,6,6,7,
			8,9,10,11,12,13,14,15,8,1,2,3,4,5,6,7,
			8,10,11,12,13,14,14,15,8,1,2,2,3,4,5,6,
			8,10,12,13,14,15,15,15,8,1,1,1,2,3,4,6,
			8,12,14,15,15,15,15,15,8,1,1,1,1,1,2,4,
		};

		template<>
		class posit<NBITS_IS_4, ES_IS_0> {
		public:
			static constexpr size_t nbits	= NBITS_IS_4;
			static constexpr size_t es		= ES_IS_0;
			static constexpr size_t sbits	= 1;
			static constexpr size_t rbits	= nbits - sbits;
			static constexpr size_t ebits	= es;
			static constexpr size_t fbits   = nbits - 3;
			static constexpr size_t fhbits  = fbits + 1;

			posit() { _bits = 0; }
			posit(const posit&) = default;
			posit(posit&&) = default;
			posit& operator=(const posit&) = default;
			posit& operator=(posit&&) = default;

			posit(int initial_value) { _bits = uint8_t(initial_value & 0x0f); }
			// assignment operators for native types
			posit& operator=(const float rhs) {
				return float_assign(rhs);
			}
			posit& operator=(const double rhs) {
				return float_assign(rhs);
			}
			posit& operator=(const long double rhs) {
				return float_assign(rhs);
			}

			explicit operator long double() const { return to_long_double(); }
			explicit operator double() const { return to_double(); }
			explicit operator float() const { return to_float(); }
			explicit operator long long() const { return to_long_long(); }
			explicit operator long() const { return to_long(); }
			explicit operator int() const { return to_int(); }
			explicit operator unsigned long long() const { return to_long_long(); }
			explicit operator unsigned long() const { return to_long(); }
			explicit operator unsigned int() const { return to_int(); }

			posit& set(sw::unum::bitblock<NBITS_IS_4>& raw) {
				_bits = uint8_t(raw.to_ulong());
				return *this;
			}
			posit& set_raw_bits(uint64_t value) { 
				_bits = uint8_t(value & 0x0f); 
				return *this; 
			}
			posit operator-() const {
				if (isZero()) {
					return *this;
				}
				if (isNaR()) {
					return *this;
				}
				posit p;
				return p.set_raw_bits((~_bits) + 1);
			}
			posit& operator+=(const posit& b) {
				uint16_t index = (_bits << 4) | b._bits;
				_bits = posit_4_0_addition_lookup[index];
				return *this;
			}
			posit& operator-=(const posit& b) {
				uint16_t index = (_bits << 4) | b._bits;
				_bits = posit_4_0_subtraction_lookup[index];
				return *this;
			}
			posit& operator*=(const posit& b) {
				uint16_t index = (_bits << 4) | b._bits;
				_bits = posit_4_0_multiplication_lookup[index];
				return *this;
			}
			posit& operator/=(const posit& b) {
				uint16_t index = (_bits << 4) | b._bits;
				_bits = posit_4_0_division_lookup[index];
				return *this;
			}
			posit& operator++() {
				++_bits;
				return *this;
			}
			posit operator++(int) {
				posit tmp(*this);
				operator++();
				return tmp;
			}
			posit& operator--() {
				--_bits;
				return *this;
			}
			posit operator--(int) {
				posit tmp(*this);
				operator--();
				return tmp;
			}
			
			// SELECTORS
			inline bool isNaR() const {
				return (_bits & 0x8);
			}
			inline bool isZero() const {
				return (_bits == 0);
			}
			inline bool isOne() const { // pattern 010000....
				return (_bits & 0x4);
			}
			inline bool isMinusOne() const { // pattern 110000...
				return (_bits & 0xC);
			}
			inline bool isNegative() const {
				return (_bits & 0x8);
			}
			inline bool isPositive() const {
				return !isNegative();
			}
			inline bool isPowerOf2() const {
				return !(_bits & 0x1);
			}

			inline int sign_value() const { return (_bits & 0x8 ? -1 : 1); }

			bitblock<NBITS_IS_4> get() const { bitblock<NBITS_IS_4> bb; bb = int(_bits); return bb; }
			unsigned long long encoding() const { return (unsigned long long)(_bits); }
			bitblock<NBITS_IS_4> get_decoded() const {
				bitblock<NBITS_IS_4> _Bits;
				uint8_t mask = 1;
				for (size_t i = 0; i < NBITS_IS_4; ++i) {
					_Bits.set(i, _bits & mask);
					mask <<= 1;
				}
				return _Bits;
			}
			long long get_encoding_as_integer() const { return _bits; }
			regime<NBITS_IS_4, ES_IS_0> get_regime() const { return regime<NBITS_IS_4, ES_IS_0>(); }
			exponent<NBITS_IS_4, ES_IS_0> get_exponent() const { return exponent<NBITS_IS_4, ES_IS_0>(); }
			fraction<fbits> get_fraction() const { return fraction<fbits>(); }

			inline void clear() { _bits = 0; }
			inline void setToZero() { clear(); }
			inline void setToNaR() { _bits = 0x8; }
		private:
			uint8_t _bits;

			// Conversion functions
			int         to_int() const {
				if (isZero()) return 0;
				if (isNaR()) throw "NaR (Not a Real)";
				return int(to_float());
			}
			long        to_long() const {
				if (isZero()) return 0;
				if (isNaR()) throw "NaR (Not a Real)";
				return long(to_double());
			}
			long long   to_long_long() const {
				if (isZero()) return 0;
				if (isNaR()) throw "NaR (Not a Real)";
				return long(to_long_double());
			}
			float       to_float() const {
				return (float)to_double();
			}
			double      to_double() const {
				if (isZero())	return 0.0;
				if (isNaR())	return NAN;
				bool		     	 _sign;
				regime<nbits, es>    _regime;
				exponent<nbits, es>  _exponent;
				fraction<fbits>      _fraction;
				bitblock<nbits>		 _raw_bits;
				_raw_bits.reset();
				uint64_t mask = 1;
				for (size_t i = 0; i < nbits; i++) {
					_raw_bits.set(i, (_bits & mask));
					mask <<= 1;
				}
				decode(_raw_bits, _sign, _regime, _exponent, _fraction);
				double s = (_sign ? -1.0 : 1.0);
				double r = _regime.value();
				double e = _exponent.value();
				double f = (1.0 + _fraction.value());
				return s * r * e * f;
			}
			long double to_long_double() const {
				if (isZero())  return 0.0;
				if (isNaR())   return NAN;
				bool		     	 _sign;
				regime<nbits, es>    _regime;
				exponent<nbits, es>  _exponent;
				fraction<fbits>      _fraction;
				bitblock<nbits>		 _raw_bits;
				_raw_bits.reset();
				uint64_t mask = 1;
				for (size_t i = 0; i < nbits; i++) {
					_raw_bits.set(i, (_bits & mask));
					mask <<= 1;
				}
				decode(_raw_bits, _sign, _regime, _exponent, _fraction);
				long double s = (_sign ? -1.0 : 1.0);
				long double r = _regime.value();
				long double e = _exponent.value();
				long double f = (1.0 + _fraction.value());
				return s * r * e * f;
			}

			template <typename T>
			posit& float_assign(const T& rhs) {
				constexpr int dfbits = std::numeric_limits<T>::digits - 1;
				value<dfbits> v((T)rhs);

				// special case processing
				if (v.isZero()) {
					setToZero();
					return *this;
				}
				if (v.isInfinite() || v.isNaN()) {  // posit encode for FP_INFINITE and NaN as NaR (Not a Real)
					setToNaR();
					return *this;
				}

				//convert(v);
				_bits = uint8_t(rhs); // TODO: not correct
				return *this;
			}

			// I/O operators
			friend std::ostream& operator<< (std::ostream& ostr, const posit<NBITS_IS_4, 0>& p);
			friend std::istream& operator>> (std::istream& istr, posit<NBITS_IS_4, 0>& p);

			// posit - posit logic functions
			friend bool operator==(const posit<NBITS_IS_4, 0>& lhs, const posit<NBITS_IS_4, 0>& rhs);
			friend bool operator!=(const posit<NBITS_IS_4, 0>& lhs, const posit<NBITS_IS_4, 0>& rhs);
			friend bool operator< (const posit<NBITS_IS_4, 0>& lhs, const posit<NBITS_IS_4, 0>& rhs);
			friend bool operator> (const posit<NBITS_IS_4, 0>& lhs, const posit<NBITS_IS_4, 0>& rhs);
			friend bool operator<=(const posit<NBITS_IS_4, 0>& lhs, const posit<NBITS_IS_4, 0>& rhs);
			friend bool operator>=(const posit<NBITS_IS_4, 0>& lhs, const posit<NBITS_IS_4, 0>& rhs);

		};

		// posit I/O operators
		inline std::ostream& operator<<(std::ostream& ostr, const posit<NBITS_IS_4, ES_IS_0>& p) {
			return ostr << p.get();
		}
		// posit - posit binary logic operators
		inline bool operator==(const posit<NBITS_IS_4, ES_IS_0>& lhs, const posit<NBITS_IS_4, ES_IS_0>& rhs) {
			return lhs._bits == rhs._bits;
		}
		inline bool operator!=(const posit<NBITS_IS_4, ES_IS_0>& lhs, const posit<NBITS_IS_4, ES_IS_0>& rhs) {
			return !operator==(lhs, rhs);
		}
		inline bool operator< (const posit<NBITS_IS_4, ES_IS_0>& lhs, const posit<NBITS_IS_4, ES_IS_0>& rhs) {
			return lhs._bits < rhs._bits;
		}
		inline bool operator> (const posit<NBITS_IS_4, ES_IS_0>& lhs, const posit<NBITS_IS_4, ES_IS_0>& rhs) {
			return operator< (rhs, lhs);
		}
		inline bool operator<=(const posit<NBITS_IS_4, ES_IS_0>& lhs, const posit<NBITS_IS_4, ES_IS_0>& rhs) {
			return operator< (lhs, rhs) || operator==(lhs, rhs);
		}
		inline bool operator>=(const posit<NBITS_IS_4, ES_IS_0>& lhs, const posit<NBITS_IS_4, ES_IS_0>& rhs) {
			return !operator< (lhs, rhs);
		}

		inline posit<NBITS_IS_4, ES_IS_0> operator+(const posit<NBITS_IS_4, ES_IS_0>& lhs, const posit<NBITS_IS_4, ES_IS_0>& rhs) {
			posit<NBITS_IS_4, ES_IS_0> sum = lhs;
			sum += rhs;
			return sum;
		}


	}
}
