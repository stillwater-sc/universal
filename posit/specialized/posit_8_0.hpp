#pragma once
// posit_8_0.hpp: specialized 8-bit posit using fast compute specialized for posit<8,0>
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
	namespace unum {

		// set the fast specialization variable to indicate that we are running a special template specialization
#ifdef POSIT_FAST_SPECIALIZATION
#define POSIT_FAST_POSIT_8_0 1

			template<>
			class posit<NBITS_IS_8, ES_IS_0> {
			public:
				static constexpr size_t nbits = NBITS_IS_8;
				static constexpr size_t es = ES_IS_0;
				static constexpr size_t sbits = 1;
				static constexpr size_t rbits = nbits - sbits;
				static constexpr size_t ebits = es;
				static constexpr size_t fbits = nbits - 3;
				static constexpr size_t fhbits = fbits + 1;
				static constexpr uint8_t index_shift = 4;

				posit() { _bits = 0; }
				posit(const posit&) = default;
				posit(posit&&) = default;
				posit& operator=(const posit&) = default;
				posit& operator=(posit&&) = default;

				posit(char initial_value) { *this = (long long)initial_value; }
				posit(short initial_value) { *this = (long long)initial_value; }
				posit(int initial_value) { *this = (long long)initial_value; }
				posit(long int initial_value) { *this = (long long)initial_value; }
				posit(long long initial_value) { *this = (long long)initial_value; }
				// assignment operators for native types
				posit& operator=(int rhs) {
					return operator=((long long)(rhs));
				}
				posit& operator=(long long rhs) {
					return float_assign((double)rhs);
				}
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

				posit& set(sw::unum::bitblock<NBITS_IS_8>& raw) {
					_bits = uint8_t(raw.to_ulong());
					return *this;
				}
				posit& set_raw_bits(uint64_t value) {
					_bits = uint8_t(value & 0xff);
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
					return p.set_raw_bits((~_bits) + 1);
				}
				posit& operator+=(const posit& b) {

					return *this;
				}
				posit& operator-=(const posit& b) {

					return *this;
				}
				posit& operator*=(const posit& b) {

					return *this;
				}
				posit& operator/=(const posit& b) {

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
				posit reciprocate() const {
					posit p;

					return p;
				}
				// SELECTORS
				inline bool isnar() const {
					return (_bits == 0x80);
				}
				inline bool iszero() const {
					return (_bits == 0x00);
				}
				inline bool isone() const { // pattern 010000....
					return (_bits == 0x40);
				}
				inline bool isminusone() const { // pattern 110000...
					return (_bits == 0xC0);
				}
				inline bool isneg() const {
					return (_bits & 0x80);
				}
				inline bool ispos() const {
					return !isneg();
				}
				inline bool ispowerof2() const {
					return !(_bits & 0x1);
				}

				inline int sign_value() const { return (_bits & 0x8 ? -1 : 1); }

				bitblock<NBITS_IS_8> get() const { bitblock<NBITS_IS_8> bb; bb = int(_bits); return bb; }
				unsigned long long encoding() const { return (unsigned long long)(_bits); }

				inline void clear() { _bits = 0; }
				inline void setzero() { clear(); }
				inline void setnar() { _bits = 0x80; }

			private:
				uint8_t _bits;

				// Conversion functions
#if POSIT_THROW_ARITHMETIC_EXCEPTION
				int         to_int() const {
					if (iszero()) return 0;
					if (isnar()) throw not_a_real{};
					return int(to_float());
				}
				long        to_long() const {
					if (iszero()) return 0;
					if (isnar()) throw not_a_real{};
					return long(to_double());
				}
				long long   to_long_long() const {
					if (iszero()) return 0;
					if (isnar()) throw not_a_real{};
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
					if (iszero())  return 0.0;
					if (isnar())   return NAN;
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
					if (v.iszero()) {
						setzero();
						return *this;
					}
					if (v.isinf() || v.isnan()) {  // posit encode for FP_INFINITE and NaN as NaR (Not a Real)
						setnar();
						return *this;
					}

					//convert(v);
					_bits = uint8_t(rhs); // TODO: not correct
					return *this;
				}

				// I/O operators
				friend std::ostream& operator<< (std::ostream& ostr, const posit<NBITS_IS_8, 0>& p);
				friend std::istream& operator>> (std::istream& istr, posit<NBITS_IS_8, 0>& p);

				// posit - posit logic functions
				friend bool operator==(const posit<NBITS_IS_8, 0>& lhs, const posit<NBITS_IS_8, 0>& rhs);
				friend bool operator!=(const posit<NBITS_IS_8, 0>& lhs, const posit<NBITS_IS_8, 0>& rhs);
				friend bool operator< (const posit<NBITS_IS_8, 0>& lhs, const posit<NBITS_IS_8, 0>& rhs);
				friend bool operator> (const posit<NBITS_IS_8, 0>& lhs, const posit<NBITS_IS_8, 0>& rhs);
				friend bool operator<=(const posit<NBITS_IS_8, 0>& lhs, const posit<NBITS_IS_8, 0>& rhs);
				friend bool operator>=(const posit<NBITS_IS_8, 0>& lhs, const posit<NBITS_IS_8, 0>& rhs);

			};

			// posit I/O operators
			// generate a posit format ASCII format nbits.esxNN...NNp
			inline std::ostream& operator<<(std::ostream& ostr, const posit<NBITS_IS_8, ES_IS_0>& p) {
				// to make certain that setw and left/right operators work properly
				// we need to transform the posit into a string
				std::stringstream ss;
#if POSIT_ROUNDING_ERROR_FREE_IO_FORMAT
				ss << NBITS_IS_8 << '.' << ES_IS_0 << 'x' << to_hex(p.get()) << 'p';
#else
				std::streamsize prec = ostr.precision();
				std::streamsize width = ostr.width();
				std::ios_base::fmtflags ff;
				ff = ostr.flags();
				ss.flags(ff);
				ss << std::showpos << std::setw(width) << std::setprecision(prec) << (long double)p;
#endif
				return ostr << ss.str();
			}

			// read an ASCII float or posit format: nbits.esxNN...NNp, for example: 32.2x80000000p
			inline std::istream& operator>> (std::istream& istr, posit<NBITS_IS_8, ES_IS_0>& p) {
				std::string txt;
				istr >> txt;
				if (!parse(txt, p)) {
					std::cerr << "unable to parse -" << txt << "- into a posit value\n";
				}
				return istr;
			}

			// convert a posit value to a string using "nar" as designation of NaR
			std::string to_string(const posit<NBITS_IS_8, ES_IS_0>& p, std::streamsize precision) {
				if (p.isnar()) {
					return std::string("nar");
				}
				std::stringstream ss;
				ss << std::setprecision(precision) << float(p);
				return ss.str();
			}

			// posit - posit binary logic operators
			inline bool operator==(const posit<NBITS_IS_8, ES_IS_0>& lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
				return lhs._bits == rhs._bits;
			}
			inline bool operator!=(const posit<NBITS_IS_8, ES_IS_0>& lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
				return !operator==(lhs, rhs);
			}
			inline bool operator< (const posit<NBITS_IS_8, ES_IS_0>& lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
				return *(signed char*)(&lhs._bits) < *(signed char*)(&rhs._bits);
			}
			inline bool operator> (const posit<NBITS_IS_8, ES_IS_0>& lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
				return operator< (rhs, lhs);
			}
			inline bool operator<=(const posit<NBITS_IS_8, ES_IS_0>& lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
				return operator< (lhs, rhs) || operator==(lhs, rhs);
			}
			inline bool operator>=(const posit<NBITS_IS_8, ES_IS_0>& lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
				return !operator< (lhs, rhs);
			}

			inline posit<NBITS_IS_8, ES_IS_0> operator+(const posit<NBITS_IS_8, ES_IS_0>& lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
				posit<NBITS_IS_8, ES_IS_0> sum = lhs;
				sum += rhs;
				return sum;
			}

#if POSIT_ENABLE_LITERALS
			// posit - literal logic functions

			// posit - int logic operators
			inline bool operator==(const posit<NBITS_IS_8, ES_IS_0>& lhs, int rhs) {
				return operator==(lhs, posit<NBITS_IS_8, ES_IS_0>(rhs));
			}
			inline bool operator!=(const posit<NBITS_IS_8, ES_IS_0>& lhs, int rhs) {
				return !operator==(lhs, posit<NBITS_IS_8, ES_IS_0>(rhs));
			}
			inline bool operator< (const posit<NBITS_IS_8, ES_IS_0>& lhs, int rhs) {
				return operator<(lhs, posit<NBITS_IS_8, ES_IS_0>(rhs));
			}
			inline bool operator> (const posit<NBITS_IS_8, ES_IS_0>& lhs, int rhs) {
				return operator< (posit<NBITS_IS_8, ES_IS_0>(rhs), lhs);
			}
			inline bool operator<=(const posit<NBITS_IS_8, ES_IS_0>& lhs, int rhs) {
				return operator< (lhs, posit<NBITS_IS_8, ES_IS_0>(rhs)) || operator==(lhs, posit<NBITS_IS_8, ES_IS_0>(rhs));
			}
			inline bool operator>=(const posit<NBITS_IS_8, ES_IS_0>& lhs, int rhs) {
				return !operator<(lhs, posit<NBITS_IS_8, ES_IS_0>(rhs));
			}

			// int - posit logic operators
			inline bool operator==(int lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
				return posit<NBITS_IS_8, ES_IS_0>(lhs) == rhs;
			}
			inline bool operator!=(int lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
				return !operator==(posit<NBITS_IS_8, ES_IS_0>(lhs), rhs);
			}
			inline bool operator< (int lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
				return operator<(posit<NBITS_IS_8, ES_IS_0>(lhs), rhs);
			}
			inline bool operator> (int lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
				return operator< (posit<NBITS_IS_8, ES_IS_0>(rhs), lhs);
			}
			inline bool operator<=(int lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
				return operator< (posit<NBITS_IS_8, ES_IS_0>(lhs), rhs) || operator==(posit<NBITS_IS_8, ES_IS_0>(lhs), rhs);
			}
			inline bool operator>=(int lhs, const posit<NBITS_IS_8, ES_IS_0>& rhs) {
				return !operator<(posit<NBITS_IS_8, ES_IS_0>(lhs), rhs);
			}

#endif
	}

#else 
#define POSIT_FAST_POSIT_8_0 0
#endif

}
