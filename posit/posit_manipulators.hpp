#pragma once
// posit_manipulators.hpp: definitions of helper functions for posit type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <iomanip>
#include <cmath>  // for frexp/frexpf
#include <typeinfo>  // for typeid()

// This file contains functions that use the posit type.
// If you have helper functions that the posit type could use, but does not depend on 
// the posit type, you can add them to the file posit_helpers.hpp.

namespace sw {
	namespace unum {

		// DEBUG/REPORTING HELPERS

		template<typename Ty>
		std::string dynamic_range(Ty v) {
			std::stringstream ss;
			ss << std::setw(13) << typeid(v).name();
			ss << "                       ";
			ss << "minexp scale " << std::setw(10) << std::numeric_limits<Ty>::min_exponent << "     ";
			ss << "maxexp scale " << std::setw(10) << std::numeric_limits<Ty>::max_exponent << "     ";
			ss << "minimum " << std::setw(12) << std::numeric_limits<Ty>::min() << "     ";
			ss << "maximum " << std::setw(12) << std::numeric_limits<Ty>::max() << "     ";
			return ss.str();
		}

		// Report the posit minpos/maxpos scales
		template<size_t nbits, size_t es>
		std::string dynamic_range(const posit<nbits, es>& p) {
			std::stringstream ss;
			ss << " posit<" << std::setw(3) << nbits << "," << es << "> ";
			ss << "useed scale  " << std::setw(4) << useed_scale<nbits, es>() << "     ";
			ss << "minpos scale " << std::setw(10) << minpos_scale<nbits, es>() << "     ";
			ss << "maxpos scale " << std::setw(10) << maxpos_scale<nbits, es>();
			return ss.str();
		}

		// Generate a type tag for this posit, for example, posit<8,1>
		template<size_t nbits, size_t es>
		std::string type_tag(const posit<nbits, es>& p) {
			std::stringstream ss;
			ss << "posit<" << nbits << "," << es << ">";
			return ss.str();
		}

		// Generate a string representing the posit components: sign, regime, exponent, faction, and value
		template<size_t nbits, size_t es>
		std::string components(const posit<nbits, es>& p) {
			constexpr size_t fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
			std::stringstream ss;
			bool		     	 _sign;
			regime<nbits, es>    _regime;
			exponent<nbits, es>  _exponent;
			fraction<fbits>      _fraction;
			decode(p.get(), _sign, _regime, _exponent, _fraction);

			// TODO: hardcoded field width is governed by pretty printing posit tables, which by construction will always be small posits
			ss << std::setw(14) << p.get() << " " << std::setw(14) << decoded(p)
				<< " Sign : " << std::setw(2) << _sign
				<< " Regime : " << std::setw(3) << _regime.regime_k()
				<< " Exponent : " << std::setw(5) << exponent_value(p)
				<< " Fraction : " << std::setw(8) << std::setprecision(21) << _fraction.value()
				<< " Value : " << std::setw(16) << p;

			return ss.str();
		}

		template<size_t nbits, size_t es>
		std::string component_values_to_string(const posit<nbits, es>& p) {
			std::stringstream ss;
			// TODO: hardcoded field sizes
			if (p.iszero()) {
				ss << " zero    " << std::setw(103) << "b" << p.get();
				return ss.str();
			}
			else if (p.isinf()) {
				ss << " infinite" << std::setw(103) << "b" << p.get();
				return ss.str();
			}

			ss << std::setw(14) << to_binary(p.get())
				<< " Sign : " << std::setw(2) << p.sign()
				<< " Regime : " << p.regime_int()
				<< " Exponent : " << p.exponent_int()
				<< std::hex
				<< " Fraction : " << p.fraction_int()
				<< " Value : " << p.to_int64()
				<< std::dec;
			return ss.str();
		}

		template<size_t nbits, size_t es>
		std::string pretty_print(const posit<nbits, es>& p, int printPrecision = std::numeric_limits<double>::max_digits10) {
			constexpr size_t fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
			std::stringstream ss;
			bool		     	 _sign;
			regime<nbits, es>    _regime;
			exponent<nbits, es>  _exponent;
			fraction<fbits>      _fraction;
			decode(p.get(), _sign, _regime, _exponent, _fraction);
			ss << ( _sign ? "s1 r" : "s0 r" );
			bitblock<nbits-1> r = _regime.get();
			int regimeBits = (int)_regime.nrBits();
			int nrOfRegimeBitsProcessed = 0;
			for (int i = nbits - 2; i >= 0; --i) {
				if (regimeBits > nrOfRegimeBitsProcessed++) {
					ss << (r[i] ? "1" : "0");
				}
			}
			ss << " e";
			bitblock<es> e = _exponent.get();
			int exponentBits = (int)_exponent.nrBits();
			int nrOfExponentBitsProcessed = 0;
			for (int i = int(es) - 1; i >= 0; --i) {
				if (exponentBits > nrOfExponentBitsProcessed++) {
					ss << (e[i] ? "1" : "0");
				}
			}
			ss << " f";
			bitblock<fbits> f = _fraction.get();
			int fractionBits = (int)_fraction.nrBits();
			int nrOfFractionBitsProcessed = 0;
			for (int i = int(p.fbits) - 1; i >= 0; --i) {
				if (fractionBits > nrOfFractionBitsProcessed++) {
					ss << (f[i] ? "1" : "0");
				}
			}
			ss << " q";
			ss << quadrant(p) << " v"
				<< std::setprecision(printPrecision) << p
				<< std::setprecision(0);
			return ss.str();
		}

		template<size_t nbits, size_t es>
		std::string info_print(const posit<nbits, es>& p, int printPrecision = 17) {
			constexpr size_t fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
			std::stringstream ss;
			bool		     	 _sign;
			regime<nbits, es>    _regime;
			exponent<nbits, es>  _exponent;
			fraction<fbits>      _fraction;
			decode(p.get(), _sign, _regime, _exponent, _fraction);

			ss << "raw: " << p.get() << " decoded: " << decoded(p) << " "
				<< quadrant(p) << " "
				<< (_sign ? "negative r" : "positive r")
				<< _regime << " e"
				<< _exponent << " f"
				<< _fraction << " : value "
				<< std::setprecision(printPrecision) << p
				<< std::setprecision(0);
			return ss.str();
		}

		enum ColorCode {
			FG_DEFAULT = 39,
			FG_BLACK = 30,
			FG_RED = 31,
			FG_GREEN = 32,
			FG_YELLOW = 33,
			FG_BLUE = 34,
			FG_MAGENTA = 35,
			FG_CYAN = 36,
			FG_LIGHT_GRAY = 37,
			FG_DARK_GRAY = 90,
			FG_LIGHT_RED = 91,
			FG_LIGHT_GREEN = 92,
			FG_LIGHT_YELLOW = 93,
			FG_LIGHT_BLUE = 94,
			FG_LIGHT_MAGENTA = 95,
			FG_LIGHT_CYAN = 96,
			FG_WHITE = 97,

			BG_DEFAULT = 49,
			BG_BLACK = 40,
			BG_RED = 41,
			BG_GREEN = 42,
			BG_YELLOW = 43,
			BG_BLUE = 44,
			BG_MAGENTA = 45,
			BG_CYAN = 46,
			BG_LIGHT_GRAY = 47,
			BG_DARK_GRAY = 100,
			BG_LIGHT_RED = 101,
			BG_LIGHT_GREEN = 102,
			BG_LIGHT_YELLOW = 103,
			BG_LIGHT_BLUE = 104,
			BG_LIGHT_MAGENTA = 105,
			BG_LIGHT_CYAN = 106,
			BG_WHITE = 107

		};
		class Color {
			ColorCode code;
		public:
			Color(ColorCode pCode) : code(pCode) {}
			friend std::ostream&
				operator<<(std::ostream& os, const Color& mod) {
				return os << "\033[" << mod.code << "m";
			}
		};

		template<size_t nbits, size_t es>
		std::string color_print(const posit<nbits, es>& p) {
			constexpr size_t fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
			std::stringstream ss;
			bool		     	 _sign;
			regime<nbits, es>    _regime;
			exponent<nbits, es>  _exponent;
			fraction<fbits>      _fraction;
			decode(p.get(), _sign, _regime, _exponent, _fraction);

			Color red(ColorCode::FG_RED);
			Color yellow(ColorCode::FG_YELLOW);
			Color blue(ColorCode::FG_BLUE);
			Color magenta(ColorCode::FG_MAGENTA);
			Color cyan(ColorCode::FG_CYAN);
			Color white(ColorCode::FG_WHITE);
			Color def(ColorCode::FG_DEFAULT);
			ss << red << (p.isneg() ? "1" : "0");

			bitblock<nbits - 1> r = _regime.get();
			int regimeBits = (int)_regime.nrBits();
			int nrOfRegimeBitsProcessed = 0;
			for (int i = nbits - 2; i >= 0; --i) {
				if (regimeBits > nrOfRegimeBitsProcessed++) {
					ss << yellow << (r[i] ? "1" : "0");
				}
			}

			bitblock<es> e = _exponent.get();
			int exponentBits = (int)_exponent.nrBits();
			int nrOfExponentBitsProcessed = 0;
			for (int i = int(es) - 1; i >= 0; --i) {
				if (exponentBits > nrOfExponentBitsProcessed++) {
					ss << cyan << (e[i] ? "1" : "0");
				}
			}

			bitblock<posit<nbits, es>::fbits> f = _fraction.get();
			int fractionBits = (int)_fraction.nrBits();
			int nrOfFractionBitsProcessed = 0;
			for (int i = int(p.fbits) - 1; i >= 0; --i) {
				if (fractionBits > nrOfFractionBitsProcessed++) {
					ss << magenta << (f[i] ? "1" : "0");
				}
			}

			ss << def;
			return ss.str();
		}
		
		// generate a full binary representation table for a given posit configuration
		template<size_t nbits, size_t es>
		void GeneratePositTable(std::ostream& ostr, bool csvFormat = false)	{
			static constexpr size_t fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
			const size_t size = (1 << nbits);
			posit<nbits, es>	p;
			if (csvFormat) {
				ostr << "\"Generate Posit Lookup table for a POSIT<" << nbits << "," << es << "> in CSV format\"" << std::endl;
				ostr << "#, Binary, Decoded, k, sign, scale, regime, exponent, fraction, value, posit\n";
				for (size_t i = 0; i < size; i++) {
					p.set_raw_bits(i);
					bool		     	 s;
					regime<nbits, es>    r;
					exponent<nbits, es>  e;
					fraction<fbits>      f;
					decode(p.get(), s, r, e, f);
					ostr << i << ","
						<< p.get() << ","
						<< decoded(p) << ","
						<< r.regime_k() << ","
						<< s << ","
						<< scale(p) << ","
						<< std::right << r << ","
						<< std::right << e << ","
						<< std::right << f << ","
						<< to_string(p, 22) << ","
						<< p
						<< '\n';
				}
				ostr << std::endl;
			}
			else {
				ostr << "Generate Posit Lookup table for a POSIT<" << nbits << "," << es << "> in TXT format" << std::endl;

				const size_t index_column = 5;
				const size_t bin_column = 16;
				const size_t k_column = 8;
				const size_t sign_column = 8;
				const size_t scale_column = 8;
				const size_t regime_column = 16;
				const size_t exponent_column = 16;
				const size_t fraction_column = 16;
				const size_t value_column = 30;
				const size_t posit_format_column = 16;

				ostr << std::setw(index_column) << " # "
					<< std::setw(bin_column) << "Binary"
					<< std::setw(bin_column) << "Decoded"
					<< std::setw(k_column) << "k"
					<< std::setw(sign_column) << "sign"
					<< std::setw(scale_column) << "scale"
					<< std::setw(regime_column) << "regime"
					<< std::setw(exponent_column) << "exponent"
					<< std::setw(fraction_column) << "fraction"
					<< std::setw(value_column) << "value"
					<< std::setw(posit_format_column) << "posit_format"
					<< std::endl;
				for (size_t i = 0; i < size; i++) {
					p.set_raw_bits(i);
					bool		     	 s;
					regime<nbits, es>    r;
					exponent<nbits, es>  e;
					fraction<fbits>      f;
					decode(p.get(), s, r, e, f);
					ostr << std::setw(4) << i << ": "
						<< std::setw(bin_column) << p.get()
						<< std::setw(bin_column) << decoded(p)
						<< std::setw(k_column) << r.regime_k()
						<< std::setw(sign_column) << s
						<< std::setw(scale_column) << scale(p)
						<< std::setw(regime_column) << std::right << to_string(r)
						<< std::setw(exponent_column) << std::right << to_string(e)
						<< std::setw(fraction_column) << std::right << to_string(f)
						<< std::setw(value_column) << to_string(p, 22) << " "
						<< std::setw(posit_format_column) << std::right << p
						<< std::endl;
				}
			}
		}

	}  // namespace unum

}  // namespace sw

