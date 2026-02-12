#pragma once
// posit_3_1.hpp: specialized 3-bit posit using lookup table arithmetic
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// DO NOT USE DIRECTLY!
// the compile guards in this file are only valid in the context of the specialization logic
// configured in the main <universal/posit/posit>

#ifndef POSIT_FAST_POSIT_3_1
#define POSIT_FAST_POSIT_3_1 0
#endif

#include <universal/utility/directives.hpp>

namespace sw { namespace universal {

		// set the fast specialization variable to indicate that we are running a special template specialization
#if POSIT_FAST_POSIT_3_1
UNIVERSAL_COMPILER_MESSAGE("Fast specialization of posit<3,1>")

			constexpr uint8_t posit_3_1_addition_lookup[64] = {
				0,1,0,3,1,1,0,3,2,0,2,3,3,3,3,3,
				0,1,0,3,1,1,0,3,2,0,2,3,3,3,3,3,
				0,1,0,3,1,1,0,3,2,0,2,3,3,3,3,3,
				0,1,0,3,1,1,0,3,2,0,2,3,3,3,3,3,
			};

			constexpr uint8_t posit_3_1_subtraction_lookup[64] = {
				0,2,1,3,1,0,1,3,2,2,0,3,3,3,3,3,
				0,2,1,3,1,0,1,3,2,2,0,3,3,3,3,3,
				0,2,1,3,1,0,1,3,2,2,0,3,3,3,3,3,
				0,2,1,3,1,0,1,3,2,2,0,3,3,3,3,3,
			};

			constexpr uint8_t posit_3_1_multiplication_lookup[64] = {
				0,0,0,3,1,1,2,3,0,2,1,3,3,3,3,3,
				0,0,0,3,1,1,2,3,0,2,1,3,3,3,3,3,
				0,0,0,3,1,1,2,3,0,2,1,3,3,3,3,3,
				0,0,0,3,1,1,2,3,0,2,1,3,3,3,3,3,
			};

			constexpr uint8_t posit_3_1_division_lookup[64] = {
				3,0,0,3,3,1,2,3,3,2,1,3,3,3,3,3,
				3,0,0,3,3,1,2,3,3,2,1,3,3,3,3,3,
				3,0,0,3,3,1,2,3,3,2,1,3,3,3,3,3,
				3,0,0,3,3,1,2,3,3,2,1,3,3,3,3,3,
			};

			constexpr uint8_t posit_3_1_reciprocal_lookup[8] = {
				3,1,2,3,3,1,2,3,
			};

			template<>
			class posit<NBITS_IS_3, ES_IS_1> {
			public:
				static constexpr unsigned nbits = NBITS_IS_2;
				static constexpr unsigned es = ES_IS_1;
				static constexpr unsigned sbits = 1;
				static constexpr unsigned rbits = nbits - sbits;
				static constexpr unsigned ebits = 0;			// <--- special case that needed this specialization
				static constexpr unsigned fbits = 0;
				static constexpr unsigned fhbits = fbits + 1;
				static constexpr uint8_t index_shift = 3;

				posit() { _bits = 0; }
				posit(const posit&) = default;
				posit(posit&&) = default;
				posit& operator=(const posit&) = default;
				posit& operator=(posit&&) = default;

				// specific value constructor
				constexpr posit(const SpecificValue code) : _bits(0) {
					switch (code) {
					case SpecificValue::infpos:
					case SpecificValue::maxpos:
						maxpos();
						break;
					case SpecificValue::minpos:
						minpos();
						break;
					case SpecificValue::zero:
					default:
						zero();
						break;
					case SpecificValue::minneg:
						minneg();
						break;
					case SpecificValue::infneg:
					case SpecificValue::maxneg:
						maxneg();
						break;
					case SpecificValue::qnan:
					case SpecificValue::snan:
					case SpecificValue::nar:
						setnar();
						break;
					}
				}

				posit(int initial_value)         { *this = initial_value; }
				posit(float initial_value)       { *this = float_assign(initial_value); }
				posit(double initial_value)      { *this = float_assign(initial_value); }
				posit(long double initial_value) { *this = float_assign(initial_value); }

				// assignment operators for native types
				posit& operator=(int rhs) noexcept {
					// only valid integers are -1, 0, 1
					_bits = 0x0;
					if (rhs <= -1) {
						_bits = 0x2;   // value is -1, or -maxpos
					}
					else if (rhs == 0) {
						_bits = 0x0;   // value is 0
					}
					else if (1 <= rhs) {
						_bits = 0x1;   // value is 1, or maxpos
					}
					return *this;
				}
				posit& operator=(float rhs) noexcept         { return float_assign(rhs); }
				posit& operator=(double rhs) noexcept        { return float_assign(rhs); }
				posit& operator=(long double rhs) noexcept   { return float_assign(rhs); }

				explicit operator long double() const        { return to_long_double(); }
				explicit operator double() const             { return to_double(); }
				explicit operator float() const              { return to_float(); }
				explicit operator long long() const          { return to_long_long(); }
				explicit operator long() const               { return to_long(); }
				explicit operator int() const                { return to_int(); }
				explicit operator unsigned long long() const { return to_long_long(); }
				explicit operator unsigned long() const      { return to_long(); }
				explicit operator unsigned int() const       { return to_int(); }

				posit& setBitblock(sw::universal::bitblock<NBITS_IS_3>& raw) {
					_bits = uint8_t(raw.to_ulong());
					return *this;
				}
				posit& setbits(uint64_t value) {
					_bits = uint8_t(value & 0x07);
					return *this;
				}
				posit operator-() const {
					if (iszero()) {
						return *this;
					}
					if (isnar()) {
						return *this;
					}
					posit p;
					return p.setbits((~_bits) + 1);
				}
				posit& operator+=(const posit& b) {
					uint16_t index = (_bits << index_shift) | b._bits;
					_bits = posit_3_1_addition_lookup[index];
					return *this;
				}
				posit& operator-=(const posit& b) {
					uint16_t index = (_bits << index_shift) | b._bits;
					_bits = posit_3_1_subtraction_lookup[index];
					return *this;
				}
				posit& operator*=(const posit& b) {
					uint16_t index = (_bits << index_shift) | b._bits;
					_bits = posit_3_1_multiplication_lookup[index];
					return *this;
				}
				posit& operator/=(const posit& b) {
					uint16_t index = (_bits << index_shift) | b._bits;
					_bits = posit_3_1_division_lookup[index];
					return *this;
				}
				posit& operator++() {
					_bits = (_bits + 1) & 0x07;
					return *this;
				}
				posit operator++(int) {
					posit tmp(*this);
					operator++();
					return tmp;
				}
				posit& operator--() {
					_bits = (_bits - 1) & 0x07;
					return *this;
				}
				posit operator--(int) {
					posit tmp(*this);
					operator--();
					return tmp;
				}
				posit reciprocal() const {
					posit p;
					p.setbits(posit_3_1_reciprocal_lookup[_bits & 0x07]);
					return p;
				}
				
				// SELECTORS
				bool sign() const { return (_bits & 0x4u); }
				bool isnar() const { return (_bits == 0x4u); }
				bool iszero() const { return (_bits == 0); }
				bool isone() const { // pattern 010....
					return (_bits == 0x2u);
				}
				bool isminusone() const { // pattern 110...
					return (_bits == 0x6u);
				}
				bool isneg() const { return (_bits & 0x4u); }
				bool ispos() const { return !isneg(); }
				bool ispowerof2() const { return !(_bits & 0x1u); }

				int sign_value() const { return (_bits & 0x4u ? -1 : 1); }

				bitblock<NBITS_IS_3> get() const { bitblock<NBITS_IS_3> bb; bb = int(_bits); return bb; }
				unsigned int bits() const { return (unsigned int)(_bits & 0x7u); }

				void clear()    { _bits = 0; }
				void setzero()  { clear(); }
				void setnar()   { _bits = 0x4u; }
				posit& minpos() {
					clear();
					return ++(*this);
				}
				posit& maxpos() {
					setnar();
					return --(*this);
				}
				posit& zero() {
					clear();
					return *this;
				}
				posit& minneg() {
					clear();
					return --(*this);
				}
				posit& maxneg() {
					setnar();
					return ++(*this);
				}

			private:
				uint8_t _bits;

				// Conversion functions
#if POSIT_THROW_ARITHMETIC_EXCEPTION
				int         to_int() const {
					if (iszero()) return 0;
					if (isnar()) throw posit_nar{};
					return int(to_float());
				}
				long        to_long() const {
					if (iszero()) return 0;
					if (isnar()) throw posit_nar{};
					return long(to_double());
				}
				long long   to_long_long() const {
					if (iszero()) return 0;
					if (isnar()) throw posit_nar{};
					return long(to_long_double());
				}
#else
				int         to_int() const {
					if (iszero()) return 0;
					if (isnar())  return int(INFINITY);
					return int(to_float());
				}
				long        to_long() const {
					if (iszero()) return 0;
					if (isnar())  return long(INFINITY);
					return long(to_double());
				}
				long long   to_long_long() const {
					if (iszero()) return 0;
					if (isnar())  return (long long)(INFINITY);
					return long(to_long_double());
				}
#endif
				float       to_float() const {
					return (float)to_double();
				}
				double      to_double() const {
					if (iszero())	return 0.0;
					if (isnar())	return NAN;
					bool		     	 _sign;
					positRegime<nbits, es>    _regime;
					positExponent<nbits, es>  _exponent;
					positFraction<fbits>      _fraction;
					bitblock<nbits>		 _raw_bits;
					_raw_bits.reset();
					uint64_t mask = 1;
					for (unsigned i = 0; i < nbits; i++) {
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
					if (iszero())  return 0.0;
					if (isnar())   return NAN;
					bool		     	 _sign;
					positRegime<nbits, es>    _regime;
					positExponent<nbits, es>  _exponent;
					positFraction<fbits>      _fraction;
					bitblock<nbits>		 _raw_bits;
					_raw_bits.reset();
					uint64_t mask = 1;
					for (unsigned i = 0; i < nbits; i++) {
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
					internal::value<dfbits> v((T)rhs);

					// special case processing
					if (v.isinf() || v.isnan()) {  // posit encode for FP_INFINITE and NaN as NaR (Not a Real)
						setnar();
						return *this;
					}
					if (v.iszero()) {
						setzero();
						return *this;
					}
					//bool _sign = v.sign();
					//int  _scale = v.scale();
					// value range of a posit<3,1> is
					// -4 -1 -0.25 0 0.25 1 4
					if (rhs <= -2) {
						_bits = 0b101; // -4
					}
					else if (rhs < -0.5) {
						_bits = 0b110; // -1
					}
					else if (rhs < 0) {
						_bits = 0b111; // -0.25
					}
					else if (rhs < 0.5) {
						_bits = 0b001; //  0.25
					}
					else if (rhs <= 2) {
						_bits = 0b010; //  1
					}
					else {
						_bits = 4;
					}
					return *this;
				}

				// I/O operators
				friend std::ostream& operator<< (std::ostream& ostr, const posit<NBITS_IS_3, 0>& p);
				friend std::istream& operator>> (std::istream& istr, posit<NBITS_IS_3, 0>& p);

				// posit - posit logic functions
				friend bool operator==(const posit<NBITS_IS_3, ES_IS_1>& lhs, const posit<NBITS_IS_3, ES_IS_1>& rhs);
				friend bool operator!=(const posit<NBITS_IS_3, ES_IS_1>& lhs, const posit<NBITS_IS_3, ES_IS_1>& rhs);
				friend bool operator< (const posit<NBITS_IS_3, ES_IS_1>& lhs, const posit<NBITS_IS_3, ES_IS_1>& rhs);
				friend bool operator> (const posit<NBITS_IS_3, ES_IS_1>& lhs, const posit<NBITS_IS_3, ES_IS_1>& rhs);
				friend bool operator<=(const posit<NBITS_IS_3, ES_IS_1>& lhs, const posit<NBITS_IS_3, ES_IS_1>& rhs);
				friend bool operator>=(const posit<NBITS_IS_3, ES_IS_1>& lhs, const posit<NBITS_IS_3, ES_IS_1>& rhs);

			};

			// posit I/O operators
			inline std::ostream& operator<<(std::ostream& ostr, const posit<NBITS_IS_3, ES_IS_1>& p) {
				return ostr << NBITS_IS_3 << '.' << ES_IS_1 << 'x' << to_hex(p.get()) << 'p';
			}

			// convert a posit value to a string using "nar" as designation of NaR
			inline std::string to_string(const posit<NBITS_IS_3, ES_IS_1>& p, std::streamsize precision) {
				if (p.isnar()) {
					return std::string("nar");
				}
				std::stringstream ss;
				ss << std::setprecision(precision) << float(p);
				return ss.str();
			}

			// posit - posit binary logic operators
			inline bool operator==(const posit<NBITS_IS_3, ES_IS_1>& lhs, const posit<NBITS_IS_3, ES_IS_1>& rhs) {
				return lhs._bits == rhs._bits;
			}
			inline bool operator!=(const posit<NBITS_IS_3, ES_IS_1>& lhs, const posit<NBITS_IS_3, ES_IS_1>& rhs) {
				return !operator==(lhs, rhs);
			}
			inline bool operator< (const posit<NBITS_IS_3, ES_IS_1>& lhs, const posit<NBITS_IS_3, ES_IS_1>& rhs) {
				return lhs._bits < rhs._bits;
			}
			inline bool operator> (const posit<NBITS_IS_3, ES_IS_1>& lhs, const posit<NBITS_IS_3, ES_IS_1>& rhs) {
				return operator< (rhs, lhs);
			}
			inline bool operator<=(const posit<NBITS_IS_3, ES_IS_1>& lhs, const posit<NBITS_IS_3, ES_IS_1>& rhs) {
				return operator< (lhs, rhs) || operator==(lhs, rhs);
			}
			inline bool operator>=(const posit<NBITS_IS_3, ES_IS_1>& lhs, const posit<NBITS_IS_3, ES_IS_1>& rhs) {
				return !operator< (lhs, rhs);
			}

			inline posit<NBITS_IS_3, ES_IS_1> operator+(const posit<NBITS_IS_3, ES_IS_1>& lhs, const posit<NBITS_IS_3, ES_IS_1>& rhs) {
				posit<NBITS_IS_3, ES_IS_1> sum = lhs;
				sum += rhs;
				return sum;
			}
#else  // POSIT_FAST_POSIT_3_1
// too verbose #pragma message("Standard posit<3,1>")
#	define POSIT_FAST_POSIT_3_1 0
#endif // POSIT_FAST_POSIT_3_1
	
}} // namespace sw::universal
